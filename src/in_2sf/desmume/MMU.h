/*
	Copyright (C) 2006 yopyop
	Copyright (C) 2007 shash
	Copyright (C) 2007-2012 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MMU_H
#define MMU_H

#include "FIFO.h"
#include "mem.h"
#include "registers.h"
#include "mc.h"
#include "bits.h"
#include "readwrite.h"
//#include "debug.h"

#ifdef HAVE_LUA
#include "lua-engine.h"
#endif

#define ARMCPU_ARM7 1
#define ARMCPU_ARM9 0
#define ARMPROC (PROCNUM ? NDS_ARM7:NDS_ARM9)

typedef const uint8_t TWaitState;

enum EDMAMode
{
	EDMAMode_Immediate = 0,
	EDMAMode_VBlank = 1,
	EDMAMode_HBlank = 2,
	EDMAMode_HStart = 3,
	EDMAMode_MemDisplay = 4,
	EDMAMode_Card = 5,
	EDMAMode_GBASlot = 6,
	EDMAMode_GXFifo = 7,
	EDMAMode7_Wifi = 8,
	EDMAMode7_GBASlot = 9
};

enum EDMABitWidth
{
	EDMABitWidth_16 = 0,
	EDMABitWidth_32 = 1
};

enum EDMASourceUpdate
{
	EDMASourceUpdate_Increment = 0,
	EDMASourceUpdate_Decrement = 1,
	EDMASourceUpdate_Fixed = 2,
	EDMASourceUpdate_Invalid = 3
};

enum EDMADestinationUpdate
{
	EDMADestinationUpdate_Increment = 0,
	EDMADestinationUpdate_Decrement = 1,
	EDMADestinationUpdate_Fixed = 2,
	EDMADestinationUpdate_IncrementReload = 3
};

//TODO
//n.b. this may be a bad idea, for complex registers like the dma control register.
//we need to know exactly what part was written to, instead of assuming all 32bits were written.
class TRegister_32
{
public:
	virtual uint32_t read32() = 0;
	virtual void write32(const uint32_t val) = 0;
	void write(const int size, const uint32_t adr, const uint32_t val) {
		if(size==32) write32(val);
		else {
			const uint32_t offset = adr&3;
			if(size==8) {
				printf("WARNING! 8BIT DMA ACCESS\n");
				uint32_t mask = 0xFF<<(offset<<3);
				write32((read32()&~mask)|(val<<(offset<<3)));
			}
			else if(size==16) {
				uint32_t mask = 0xFFFF<<(offset<<3);
				write32((read32()&~mask)|(val<<(offset<<3)));
			}
		}
	}

	uint32_t read(const int size, const uint32_t adr)
	{
		if(size==32) return read32();
		else {
			const uint32_t offset = adr&3;
			if(size==8) { printf("WARNING! 8BIT DMA ACCESS\n"); return (read32()>>(offset<<3))&0xFF; }
			else return (read32()>>(offset<<3))&0xFFFF;
		}
	}
};

struct TGXSTAT : public TRegister_32
{
	TGXSTAT() {
		gxfifo_irq = se = tr = tb = sb = 0;
		fifo_empty = true;
		fifo_low = false;
	}
	uint8_t tb; //test busy
	uint8_t tr; //test result
	uint8_t se; //stack error
	uint8_t sb; //stack busy
	uint8_t gxfifo_irq; //irq configuration

	bool fifo_empty, fifo_low;

	virtual uint32_t read32();
	virtual void write32(const uint32_t val);

	//void savestate(EMUFILE *f);
	bool loadstate(EMUFILE *f);
};

void triggerDma(EDMAMode mode);

class DivController
{
public:
	DivController()
		: mode(0), busy(0)
	{}
	void exec();
	uint8_t mode, busy, div0;
	uint16_t read16() { return mode|(busy<<15)|(div0<<14); }
	void write16(uint16_t val) {
		mode = val&3;
		//todo - do we clear the div0 flag here or is that strictly done by the divider unit?
	}
	/*void savestate(EMUFILE* os)
	{
		write8le(&mode,os);
		write8le(&busy,os);
		write8le(&div0,os);
	}*/
	bool loadstate(EMUFILE* is, int)
	{
		int ret = 1;
		ret &= read8le(&mode,is);
		ret &= read8le(&busy,is);
		ret &= read8le(&div0,is);
		return ret==1;
	}
};

class SqrtController
{
public:
	SqrtController()
		: mode(0), busy(0)
	{}
	void exec();
	uint8_t mode, busy;
	uint16_t read16() { return mode|(busy<<15); }
	void write16(uint16_t val) { mode = val&1; }
	/*void savestate(EMUFILE* os)
	{
		write8le(&mode,os);
		write8le(&busy,os);
	}*/
	bool loadstate(EMUFILE* is, int)
	{
		int ret=1;
		ret &= read8le(&mode,is);
		ret &= read8le(&busy,is);
		return ret==1;
	}
};

class DmaController
{
public:
	uint8_t enable, irq, repeatMode, _startmode;
	uint8_t userEnable;
	uint32_t wordcount;
	EDMAMode startmode;
	EDMABitWidth bitWidth;
	EDMASourceUpdate sar;
	EDMADestinationUpdate dar;
	uint32_t saddr, daddr;
	uint32_t saddr_user, daddr_user;

	//indicates whether the dma needs to be checked for triggering
	bool dmaCheck;

	//indicates whether the dma right now is logically running
	//(though for now we copy all the data when it triggers)
	bool running;

	bool paused;

	//this flag will sometimes be set when a start condition is triggered
	//other conditions may be automatically triggered based on scanning conditions
	bool triggered;

	uint64_t nextEvent;

	int procnum, chan;

	//void savestate(EMUFILE *f);
	bool loadstate(EMUFILE *f);

	void exec();
	template<int PROCNUM> void doCopy();
	void doPause();
	void doStop();
	void doSchedule();
	void tryTrigger(EDMAMode mode);

	DmaController() :
		enable(0), irq(0), repeatMode(0), _startmode(0),
		wordcount(0), startmode(EDMAMode_Immediate),
		bitWidth(EDMABitWidth_16),
		sar(EDMASourceUpdate_Increment), dar(EDMADestinationUpdate_Increment),
		//if saddr isnt cleared then rings of fate will trigger copy protection
		//by inspecting dma3 saddr when it boots
		saddr(0), daddr(0),
		saddr_user(0), daddr_user(0),
		dmaCheck(false),
		running(false),
		paused(false),
		triggered(false),
		nextEvent(0),
		sad(&saddr_user),
		dad(&daddr_user)
	{
		sad.controller = this;
		dad.controller = this;
		ctrl.controller = this;
		regs[0] = &sad;
		regs[1] = &dad;
		regs[2] = &ctrl;
	}

	class AddressRegister : public TRegister_32 {
	public:
		//we pass in a pointer to the controller here so we can alert it if anything changes
		DmaController* controller;
		uint32_t * const ptr;
		AddressRegister(uint32_t* _ptr)
			: ptr(_ptr)
		{}
		virtual uint32_t read32() {
			return *ptr;
		}
		virtual void write32(const uint32_t val) {
			*ptr = val;
		}
	};

	class ControlRegister : public TRegister_32 {
	public:
		//we pass in a pointer to the controller here so we can alert it if anything changes
		DmaController* controller;
		ControlRegister() {}
		virtual uint32_t read32() {
			return controller->read32();
		}
		virtual void write32(const uint32_t val) {
			return controller->write32(val);
		}
	};

	AddressRegister sad, dad;
	ControlRegister ctrl;
	TRegister_32* regs[3];

	void write32(const uint32_t val);
	uint32_t read32();

};

enum ECardMode
{
	CardMode_Normal = 0,
	CardMode_KEY1,
	CardMode_KEY2
};

typedef struct
{

	uint8_t command[8];

	uint32_t address;
	uint32_t transfer_count;

	ECardMode mode;

	// NJSD stuff
	int blocklen;

} nds_dscard;

struct MMU_struct
{
	//ARM9 mem
	uint8_t ARM9_ITCM[0x8000];
    uint8_t ARM9_DTCM[0x4000];

	//u8 MAIN_MEM[4*1024*1024]; //expanded from 4MB to 8MB to support debug consoles
	//u8 MAIN_MEM[8*1024*1024]; //expanded from 8MB to 16MB to support dsi
	uint8_t MAIN_MEM[16*1024*1024]; //expanded from 8MB to 16MB to support dsi
    uint8_t ARM9_REG[0x1000000];
    uint8_t ARM9_BIOS[0x8000];
    uint8_t ARM9_VMEM[0x800];

	#include "PACKED.h"
	struct {
		uint8_t ARM9_LCD[0xA4000];
		//an extra 128KB for blank memory, directly after arm9_lcd, so that
		//we can easily map things to the end of arm9_lcd to represent
		//an unmapped state
		uint8_t blank_memory[0x20000];
	};
	#include "PACKED_END.h"

    uint8_t ARM9_OAM[0x800];

	uint8_t* ExtPal[2][4];
	uint8_t* ObjExtPal[2][2];

	struct TextureInfo {
		uint8_t* texPalSlot[6];
		uint8_t* textureSlotAddr[4];
	} texInfo;

	//ARM7 mem
	uint8_t ARM7_BIOS[0x4000];
	uint8_t ARM7_ERAM[0x10000];
	uint8_t ARM7_REG[0x10000];
	uint8_t ARM7_WIRAM[0x10000];

	// VRAM mapping
	uint8_t VRAM_MAP[4][32];
	uint32_t LCD_VRAM_ADDR[10];
	uint8_t LCDCenable[10];

	//Shared ram
	uint8_t SWIRAM[0x8000];

	//Card rom & ram
	uint8_t * CART_ROM;

	//Unused ram
	uint8_t UNUSED_RAM[4];

	//this is here so that we can trap glitchy emulator code
	//which is accessing offsets 5,6,7 of unused ram due to unaligned accesses
	//(also since the emulator doesn't prevent unaligned accesses)
	uint8_t MORE_UNUSED_RAM[4];

	static uint8_t * MMU_MEM[2][256];
	static uint32_t MMU_MASK[2][256];

	uint8_t ARM9_RW_MODE;

	uint32_t DTCMRegion;
	uint32_t ITCMRegion;

	uint16_t timer[2][4];
	int32_t timerMODE[2][4];
	uint32_t timerON[2][4];
	uint32_t timerRUN[2][4];
	uint16_t timerReload[2][4];

	uint32_t reg_IME[2];
	uint32_t reg_IE[2];

	//these are the user-controlled IF bits. some IF bits are generated as necessary from hardware conditions
	uint32_t reg_IF_bits[2];
	//these flags are set occasionally to indicate that an irq should have entered the pipeline, and processing will be deferred a tiny bit to help emulate things
	uint32_t reg_IF_pending[2];

	//uint32_t reg_DISP3DCNT_bits;

	template<int PROCNUM> uint32_t gen_IF();

	bool divRunning;
	int64_t divResult;
	int64_t divMod;
	uint64_t divCycles;

	bool sqrtRunning;
	uint32_t sqrtResult;
	uint64_t sqrtCycles;

	uint16_t SPI_CNT;
	uint16_t SPI_CMD;
	uint16_t AUX_SPI_CNT;
	uint16_t AUX_SPI_CMD;

	//uint64_t gfx3dCycles;

	uint8_t powerMan_CntReg;
	bool powerMan_CntRegWritten;
	uint8_t powerMan_Reg[5];

	memory_chip_t fw;

	nds_dscard dscard[2];
};

//everything in here is derived from libnds behaviours. no hardware tests yet
class DSI_TSC
{
public:
	DSI_TSC();
	void reset_command();
	uint16_t write16(uint16_t val);
	//bool save_state(EMUFILE* os);
	//bool load_state(EMUFILE* is);

private:
	uint16_t read16();
	uint8_t reg_selection;
	uint8_t read_flag;
	int32_t state;
	int32_t readcount;

	//registers[0] contains the current page.
	//we are going to go ahead and save these out in case we want to change the way this is emulated in the future..
	//we may want to poke registers in here at more convenient times and have the TSC dumbly pluck them out,
	//rather than generate the values on the fly
	uint8_t registers[0x80];
};

//this contains things which can't be memzeroed because they are smarter classes
struct MMU_struct_new
{
	MMU_struct_new() ;
	BackupDevice backupDevice;
	DmaController dma[2][4];
	TGXSTAT gxstat;
	SqrtController sqrt;
	DivController div;
	DSI_TSC dsi_tsc;

	void write_dma(const int proc, const int size, const uint32_t adr, const uint32_t val);
	uint32_t read_dma(const int proc, const int size, const uint32_t adr);
	bool is_dma(const uint32_t adr) { return adr >= _REG_DMA_CONTROL_MIN && adr <= _REG_DMA_CONTROL_MAX; }
};

extern MMU_struct MMU;
extern MMU_struct_new MMU_new;


/*struct armcpu_memory_iface {*/
  /** the 32 bit instruction prefetch */
  //uint32_t FASTCALL (*prefetch32)( void *data, uint32_t adr);

  /** the 16 bit instruction prefetch */
  //uint16_t FASTCALL (*prefetch16)( void *data, uint32_t adr);

  /** read 8 bit data value */
  //uint8_t FASTCALL (*read8)( void *data, uint32_t adr);
  /** read 16 bit data value */
  //uint16_t FASTCALL (*read16)( void *data, uint32_t adr);
  /** read 32 bit data value */
  //uint32_t FASTCALL (*read32)( void *data, uint32_t adr);

  /** write 8 bit data value */
  //void FASTCALL (*write8)( void *data, uint32_t adr, uint8_t val);
  /** write 16 bit data value */
  //void FASTCALL (*write16)( void *data, uint32_t adr, uint16_t val);
  /** write 32 bit data value */
  /*void FASTCALL (*write32)( void *data, uint32_t adr, uint32_t val);

  void *data;
};*/


void MMU_Init();
void MMU_DeInit();

void MMU_Reset();

void MMU_setRom(uint8_t * rom, uint32_t mask);
void MMU_unsetRom();

//void print_memory_profiling();

// Memory reading/writing (old)
//uint8_t FASTCALL MMU_read8(uint32_t proc, uint32_t adr);
//uint16_t FASTCALL MMU_read16(uint32_t proc, uint32_t adr);
//uint32_t FASTCALL MMU_read32(uint32_t proc, uint32_t adr);
//void FASTCALL MMU_write8(uint32_t proc, uint32_t adr, uint8_t val);
//void FASTCALL MMU_write16(uint32_t proc, uint32_t adr, uint16_t val);
//void FASTCALL MMU_write32(uint32_t proc, uint32_t adr, uint32_t val);

//template<int PROCNUM> void FASTCALL MMU_doDMA(uint32_t num);

//The base ARM memory interfaces
//extern struct armcpu_memory_iface arm9_base_memory_iface;
//extern struct armcpu_memory_iface arm7_base_memory_iface;
//extern struct armcpu_memory_iface arm9_direct_memory_iface;

#define VRAM_BANKS 9
#define VRAM_BANK_A 0
#define VRAM_BANK_B 1
#define VRAM_BANK_C 2
#define VRAM_BANK_D 3
#define VRAM_BANK_E 4
#define VRAM_BANK_F 5
#define VRAM_BANK_G 6
#define VRAM_BANK_H 7
#define VRAM_BANK_I 8

#define VRAM_PAGE_ABG 0
#define VRAM_PAGE_BBG 128
#define VRAM_PAGE_AOBJ 256
#define VRAM_PAGE_BOBJ 384


struct VramConfiguration {

	enum Purpose {
		OFF, INVALID, ABG, BBG, AOBJ, BOBJ, LCDC, ARM7, TEX, TEXPAL, ABGEXTPAL, BBGEXTPAL, AOBJEXTPAL, BOBJEXTPAL
	};

	struct BankInfo {
		Purpose purpose;
		int ofs;
	} banks[VRAM_BANKS];

	inline void clear() {
		for(int i=0;i<VRAM_BANKS;i++) {
			banks[i].ofs = 0;
			banks[i].purpose = OFF;
		}
	}

	//std::string describePurpose(Purpose p);
	//std::string describe();
};

extern VramConfiguration vramConfiguration;

#define VRAM_ARM9_PAGES 512
extern uint8_t vram_arm9_map[VRAM_ARM9_PAGES];
/*inline void* MMU_gpu_map(uint32_t vram_addr)
{
	//this is supposed to map a single gpu vram address to emulator host memory
	//but it returns a pointer to some zero memory in case of accesses to unmapped memory.
	//this correctly handles the case with tile accesses to unmapped memory.
	//it could also potentially go through a different LUT than vram_arm9_map in case we discover
	//that it needs to be set up with different or no mirroring
	//(I think it is a reasonable possibility that only the cpu has the nutty mirroring rules)
	//
	//if this system isn't used, Fantasy Aquarium displays garbage in the first ingame screen
	//due to it storing 0x0F0F or somesuch in screen memory which points to a ridiculously big tile
	//which should contain all 0 pixels

	uint32_t vram_page = (vram_addr>>14)&(VRAM_ARM9_PAGES-1);
	uint32_t ofs = vram_addr & 0x3FFF;
	vram_page = vram_arm9_map[vram_page];
	//blank pages are handled by the extra 16KB of blank memory at the end of ARM9_LCD
	//and the fact that blank pages are mapped to appear at that location
	return MMU.ARM9_LCD + (vram_page<<14) + ofs;
}*/


template<int PROCNUM, MMU_ACCESS_TYPE AT> uint8_t _MMU_read08(uint32_t addr);
template<int PROCNUM, MMU_ACCESS_TYPE AT> uint16_t _MMU_read16(uint32_t addr);
template<int PROCNUM, MMU_ACCESS_TYPE AT> uint32_t _MMU_read32(uint32_t addr);
template<int PROCNUM, MMU_ACCESS_TYPE AT> void _MMU_write08(uint32_t addr, uint8_t val);
template<int PROCNUM, MMU_ACCESS_TYPE AT> void _MMU_write16(uint32_t addr, uint16_t val);
template<int PROCNUM, MMU_ACCESS_TYPE AT> void _MMU_write32(uint32_t addr, uint32_t val);

template<int PROCNUM> inline uint8_t _MMU_read08(uint32_t addr) { return _MMU_read08<PROCNUM, MMU_AT_DATA>(addr); }
template<int PROCNUM> inline uint16_t _MMU_read16(uint32_t addr) { return _MMU_read16<PROCNUM, MMU_AT_DATA>(addr); }
template<int PROCNUM> inline uint32_t _MMU_read32(uint32_t addr) { return _MMU_read32<PROCNUM, MMU_AT_DATA>(addr); }
template<int PROCNUM> inline void _MMU_write08(uint32_t addr, uint8_t val) { _MMU_write08<PROCNUM, MMU_AT_DATA>(addr,val); }
template<int PROCNUM> inline void _MMU_write16(uint32_t addr, uint16_t val) { _MMU_write16<PROCNUM, MMU_AT_DATA>(addr,val); }
template<int PROCNUM> inline void _MMU_write32(uint32_t addr, uint32_t val) { _MMU_write32<PROCNUM, MMU_AT_DATA>(addr,val); }

void FASTCALL _MMU_ARM9_write08(uint32_t adr, uint8_t val);
void FASTCALL _MMU_ARM9_write16(uint32_t adr, uint16_t val);
void FASTCALL _MMU_ARM9_write32(uint32_t adr, uint32_t val);
uint8_t  FASTCALL _MMU_ARM9_read08(uint32_t adr);
uint16_t FASTCALL _MMU_ARM9_read16(uint32_t adr);
uint32_t FASTCALL _MMU_ARM9_read32(uint32_t adr);

void FASTCALL _MMU_ARM7_write08(uint32_t adr, uint8_t val);
void FASTCALL _MMU_ARM7_write16(uint32_t adr, uint16_t val);
void FASTCALL _MMU_ARM7_write32(uint32_t adr, uint32_t val);
uint8_t  FASTCALL _MMU_ARM7_read08(uint32_t adr);
uint16_t FASTCALL _MMU_ARM7_read16(uint32_t adr);
uint32_t FASTCALL _MMU_ARM7_read32(uint32_t adr);

extern uint32_t partie;

extern uint32_t _MMU_MAIN_MEM_MASK;
extern uint32_t _MMU_MAIN_MEM_MASK16;
extern uint32_t _MMU_MAIN_MEM_MASK32;
/*inline void SetupMMU(bool debugConsole) {
	if(debugConsole) _MMU_MAIN_MEM_MASK = 0x7FFFFF;
	else _MMU_MAIN_MEM_MASK = 0x3FFFFF;
	_MMU_MAIN_MEM_MASK16 = _MMU_MAIN_MEM_MASK & ~1;
	_MMU_MAIN_MEM_MASK32 = _MMU_MAIN_MEM_MASK & ~3;
}*/
void SetupMMU(bool debugConsole, bool dsi);

/*inline void CheckMemoryDebugEvent(EDEBUG_EVENT event, const MMU_ACCESS_TYPE type, const uint32_t procnum, const uint32_t addr, const uint32_t size, const uint32_t val)
{
	//TODO - ugh work out a better prefetch event system
	if(type == MMU_AT_CODE && event == DEBUG_EVENT_READ)
		event = DEBUG_EVENT_EXECUTE;
	if(CheckDebugEvent(event))
	{
		DebugEventData.memAccessType = type;
		DebugEventData.procnum = procnum;
		DebugEventData.addr = addr;
		DebugEventData.size = size;
		DebugEventData.val = val;
		HandleDebugEvent(event);
	}
}*/


//ALERT!!!!!!!!!!!!!!
//the following inline functions dont do the 0x0FFFFFFF mask.
//this may result in some unexpected behavior

inline uint8_t _MMU_read08(const int PROCNUM, const MMU_ACCESS_TYPE AT, const uint32_t addr)
{
	//CheckMemoryDebugEvent(DEBUG_EVENT_READ,AT,PROCNUM,addr,8,0);

	//special handling for DMA: read 0 from TCM
	if(PROCNUM==ARMCPU_ARM9 && AT == MMU_AT_DMA)
	{
		if(addr<0x02000000) return 0; //itcm
		if((addr&(~0x3FFF)) == MMU.DTCMRegion) return 0; //dtcm
	}

#ifdef HAVE_LUA
	CallRegisteredLuaMemHook(addr, 1, /*FIXME*/ 0, LUAMEMHOOK_READ);
#endif

	if(PROCNUM==ARMCPU_ARM9)
		if((addr&(~0x3FFF)) == MMU.DTCMRegion)
		{
			//Returns data from DTCM (ARM9 only)
			return T1ReadByte(MMU.ARM9_DTCM, addr & 0x3FFF);
		}

	if ( (addr & 0x0F000000) == 0x02000000)
		return T1ReadByte( MMU.MAIN_MEM, addr & _MMU_MAIN_MEM_MASK);

	if(PROCNUM==ARMCPU_ARM9) return _MMU_ARM9_read08(addr);
	else return _MMU_ARM7_read08(addr);
}

inline uint16_t _MMU_read16(const int PROCNUM, const MMU_ACCESS_TYPE AT, const uint32_t addr)
{
	//CheckMemoryDebugEvent(DEBUG_EVENT_READ,AT,PROCNUM,addr,16,0);

	//special handling for DMA: read 0 from TCM
	if(PROCNUM==ARMCPU_ARM9 && AT == MMU_AT_DMA)
	{
		if(addr<0x02000000) return 0; //itcm
		if((addr&(~0x3FFF)) == MMU.DTCMRegion) return 0; //dtcm
	}

#ifdef HAVE_LUA
	CallRegisteredLuaMemHook(addr, 2, /*FIXME*/ 0, LUAMEMHOOK_READ);
#endif

	//special handling for execution from arm9, since we spend so much time in there
	if(PROCNUM==ARMCPU_ARM9 && AT == MMU_AT_CODE)
	{
		if ((addr & 0x0F000000) == 0x02000000)
			return T1ReadWord_guaranteedAligned( MMU.MAIN_MEM, addr & _MMU_MAIN_MEM_MASK16);

		if(addr<0x02000000)
			return T1ReadWord_guaranteedAligned(MMU.ARM9_ITCM, addr&0x7FFE);

		goto dunno;
	}

	if(PROCNUM==ARMCPU_ARM9)
		if((addr&(~0x3FFF)) == MMU.DTCMRegion)
		{
			//Returns data from DTCM (ARM9 only)
			return T1ReadWord_guaranteedAligned(MMU.ARM9_DTCM, addr & 0x3FFE);
		}

	if ( (addr & 0x0F000000) == 0x02000000)
		return T1ReadWord_guaranteedAligned( MMU.MAIN_MEM, addr & _MMU_MAIN_MEM_MASK16);

dunno:
	if(PROCNUM==ARMCPU_ARM9) return _MMU_ARM9_read16(addr);
	else return _MMU_ARM7_read16(addr);
}

inline uint32_t _MMU_read32(const int PROCNUM, const MMU_ACCESS_TYPE AT, const uint32_t addr)
{
	//CheckMemoryDebugEvent(DEBUG_EVENT_READ,AT,PROCNUM,addr,32,0);

	//special handling for DMA: read 0 from TCM
	if(PROCNUM==ARMCPU_ARM9 && AT == MMU_AT_DMA)
	{
		if(addr<0x02000000) return 0; //itcm
		if((addr&(~0x3FFF)) == MMU.DTCMRegion) return 0; //dtcm
	}

#ifdef HAVE_LUA
	CallRegisteredLuaMemHook(addr, 4, /*FIXME*/ 0, LUAMEMHOOK_READ);
#endif

	//special handling for execution from arm9, since we spend so much time in there
	if(PROCNUM==ARMCPU_ARM9 && AT == MMU_AT_CODE)
	{
		if ( (addr & 0x0F000000) == 0x02000000)
			return T1ReadLong_guaranteedAligned( MMU.MAIN_MEM, addr & _MMU_MAIN_MEM_MASK32);

		if(addr<0x02000000)
			return T1ReadLong_guaranteedAligned(MMU.ARM9_ITCM, addr&0x7FFC);

		//what happens when we execute from DTCM? nocash makes it look like we get 0xFFFFFFFF but i can't seem to verify it
		//historically, desmume would fall through to its old memory map struct
		//which would return unused memory (0)
		//it seems the hardware returns 0 or something benign because in actuality 0xFFFFFFFF is an undefined opcode
		//and we know our handling for that is solid

		goto dunno;
	}

	//special handling for execution from arm7. try reading from main memory first
	if(PROCNUM==ARMCPU_ARM7)
	{
		if ( (addr & 0x0F000000) == 0x02000000)
			return T1ReadLong_guaranteedAligned( MMU.MAIN_MEM, addr & _MMU_MAIN_MEM_MASK32);
		else if((addr & 0xFF800000) == 0x03800000)
			return T1ReadLong_guaranteedAligned(MMU.ARM7_ERAM, addr&0xFFFC);
		else if((addr & 0xFF800000) == 0x03000000)
			return T1ReadLong_guaranteedAligned(MMU.SWIRAM, addr&0x7FFC);
	}


	//for other arm9 cases, we have to check from dtcm first because it is patched on top of the main memory range
	if(PROCNUM==ARMCPU_ARM9)
	{
		if((addr&(~0x3FFF)) == MMU.DTCMRegion)
		{
			//Returns data from DTCM (ARM9 only)
			return T1ReadLong_guaranteedAligned(MMU.ARM9_DTCM, addr & 0x3FFC);
		}

		if ( (addr & 0x0F000000) == 0x02000000)
			return T1ReadLong_guaranteedAligned( MMU.MAIN_MEM, addr & _MMU_MAIN_MEM_MASK32);
	}

dunno:
	if(PROCNUM==ARMCPU_ARM9) return _MMU_ARM9_read32(addr);
	else return _MMU_ARM7_read32(addr);
}

inline void _MMU_write08(const int PROCNUM, const MMU_ACCESS_TYPE AT, const uint32_t addr, uint8_t val)
{
	//CheckMemoryDebugEvent(DEBUG_EVENT_WRITE,AT,PROCNUM,addr,8,val);

	//special handling for DMA: discard writes to TCM
	if(PROCNUM==ARMCPU_ARM9 && AT == MMU_AT_DMA)
	{
		if(addr<0x02000000) return; //itcm
		if((addr&(~0x3FFF)) == MMU.DTCMRegion) return; //dtcm
	}

	if(PROCNUM==ARMCPU_ARM9)
		if((addr&(~0x3FFF)) == MMU.DTCMRegion)
		{
			T1WriteByte(MMU.ARM9_DTCM, addr & 0x3FFF, val);
#ifdef HAVE_LUA
			CallRegisteredLuaMemHook(addr, 1, val, LUAMEMHOOK_WRITE);
#endif
			return;
		}

	if ( (addr & 0x0F000000) == 0x02000000) {
		T1WriteByte( MMU.MAIN_MEM, addr & _MMU_MAIN_MEM_MASK, val);
#ifdef HAVE_LUA
		CallRegisteredLuaMemHook(addr, 1, val, LUAMEMHOOK_WRITE);
#endif
		return;
	}

	if(PROCNUM==ARMCPU_ARM9) _MMU_ARM9_write08(addr,val);
	else _MMU_ARM7_write08(addr,val);
#ifdef HAVE_LUA
	CallRegisteredLuaMemHook(addr, 1, val, LUAMEMHOOK_WRITE);
#endif
}

inline void _MMU_write16(const int PROCNUM, const MMU_ACCESS_TYPE AT, const uint32_t addr, uint16_t val)
{
	//CheckMemoryDebugEvent(DEBUG_EVENT_WRITE,AT,PROCNUM,addr,16,val);

	//special handling for DMA: discard writes to TCM
	if(PROCNUM==ARMCPU_ARM9 && AT == MMU_AT_DMA)
	{
		if(addr<0x02000000) return; //itcm
		if((addr&(~0x3FFF)) == MMU.DTCMRegion) return; //dtcm
	}

	if(PROCNUM==ARMCPU_ARM9)
		if((addr&(~0x3FFF)) == MMU.DTCMRegion)
		{
			T1WriteWord(MMU.ARM9_DTCM, addr & 0x3FFE, val);
#ifdef HAVE_LUA
			CallRegisteredLuaMemHook(addr, 2, val, LUAMEMHOOK_WRITE);
#endif
			return;
		}

	if ( (addr & 0x0F000000) == 0x02000000) {
		T1WriteWord( MMU.MAIN_MEM, addr & _MMU_MAIN_MEM_MASK16, val);
#ifdef HAVE_LUA
		CallRegisteredLuaMemHook(addr, 2, val, LUAMEMHOOK_WRITE);
#endif
		return;
	}

	if(PROCNUM==ARMCPU_ARM9) _MMU_ARM9_write16(addr,val);
	else _MMU_ARM7_write16(addr,val);
#ifdef HAVE_LUA
	CallRegisteredLuaMemHook(addr, 2, val, LUAMEMHOOK_WRITE);
#endif
}

inline void _MMU_write32(const int PROCNUM, const MMU_ACCESS_TYPE AT, const uint32_t addr, uint32_t val)
{
	//CheckMemoryDebugEvent(DEBUG_EVENT_WRITE,AT,PROCNUM,addr,32,val);

	//special handling for DMA: discard writes to TCM
	if(PROCNUM==ARMCPU_ARM9 && AT == MMU_AT_DMA)
	{
		if(addr<0x02000000) return; //itcm
		if((addr&(~0x3FFF)) == MMU.DTCMRegion) return; //dtcm
	}

	if(PROCNUM==ARMCPU_ARM9)
		if((addr&(~0x3FFF)) == MMU.DTCMRegion)
		{
			T1WriteLong(MMU.ARM9_DTCM, addr & 0x3FFC, val);
#ifdef HAVE_LUA
			CallRegisteredLuaMemHook(addr, 4, val, LUAMEMHOOK_WRITE);
#endif
			return;
		}

	if ( (addr & 0x0F000000) == 0x02000000) {
		T1WriteLong( MMU.MAIN_MEM, addr & _MMU_MAIN_MEM_MASK32, val);
#ifdef HAVE_LUA
		CallRegisteredLuaMemHook(addr, 4, val, LUAMEMHOOK_WRITE);
#endif
		return;
	}

	if(PROCNUM==ARMCPU_ARM9) _MMU_ARM9_write32(addr,val);
	else _MMU_ARM7_write32(addr,val);
#ifdef HAVE_LUA
	CallRegisteredLuaMemHook(addr, 4, val, LUAMEMHOOK_WRITE);
#endif
}


//#ifdef MMU_ENABLE_ACL
//	void FASTCALL MMU_write8_acl(uint32_t proc, uint32_t adr, uint8_t val);
//	void FASTCALL MMU_write16_acl(uint32_t proc, uint32_t adr, uint16_t val);
//	void FASTCALL MMU_write32_acl(uint32_t proc, uint32_t adr, uint32_t val);
//	uint8_t FASTCALL MMU_read8_acl(uint32_t proc, uint32_t adr, uint32_t access);
//	uint16_t FASTCALL MMU_read16_acl(uint32_t proc, uint32_t adr, uint32_t access);
//	uint32_t FASTCALL MMU_read32_acl(uint32_t proc, uint32_t adr, uint32_t access);
//#else
//	#define MMU_write8_acl(proc, adr, val)  _MMU_write08<proc>(adr, val)
//	#define MMU_write16_acl(proc, adr, val) _MMU_write16<proc>(adr, val)
//	#define MMU_write32_acl(proc, adr, val) _MMU_write32<proc>(adr, val)
//	#define MMU_read8_acl(proc,adr,access)  _MMU_read08<proc>(adr)
//	#define MMU_read16_acl(proc,adr,access) ((access==CP15_ACCESS_EXECUTE)?_MMU_read16<proc,MMU_AT_CODE>(adr):_MMU_read16<proc,MMU_AT_DATA>(adr))
//	#define MMU_read32_acl(proc,adr,access) ((access==CP15_ACCESS_EXECUTE)?_MMU_read32<proc,MMU_AT_CODE>(adr):_MMU_read32<proc,MMU_AT_DATA>(adr))
//#endif

// Use this macros for reading/writing, so the GDB stub isn't broken
#ifdef GDB_STUB
	#define READ32(a,b)		cpu->mem_if->read32(a,(b) & 0xFFFFFFFC)
	#define WRITE32(a,b,c)	cpu->mem_if->write32(a,(b) & 0xFFFFFFFC,c)
	#define READ16(a,b)		cpu->mem_if->read16(a,(b) & 0xFFFFFFFE)
	#define WRITE16(a,b,c)	cpu->mem_if->write16(a,(b) & 0xFFFFFFFE,c)
	#define READ8(a,b)		cpu->mem_if->read8(a,b)
	#define WRITE8(a,b,c)	cpu->mem_if->write8(a,b,c)
#else
	#define READ32(a,b)		_MMU_read32<PROCNUM>((b) & 0xFFFFFFFC)
	#define WRITE32(a,b,c)	_MMU_write32<PROCNUM>((b) & 0xFFFFFFFC,c)
	#define READ16(a,b)		_MMU_read16<PROCNUM>((b) & 0xFFFFFFFE)
	#define WRITE16(a,b,c)	_MMU_write16<PROCNUM>((b) & 0xFFFFFFFE,c)
	#define READ8(a,b)		_MMU_read08<PROCNUM>(b)
	#define WRITE8(a,b,c)	_MMU_write08<PROCNUM>(b, c)
#endif

template<int PROCNUM, MMU_ACCESS_TYPE AT>
inline uint8_t _MMU_read08(uint32_t addr) { return _MMU_read08(PROCNUM, AT, addr); }

template<int PROCNUM, MMU_ACCESS_TYPE AT>
inline uint16_t _MMU_read16(uint32_t addr) { return _MMU_read16(PROCNUM, AT, addr); }

template<int PROCNUM, MMU_ACCESS_TYPE AT>
inline uint32_t _MMU_read32(uint32_t addr) { return _MMU_read32(PROCNUM, AT, addr); }

template<int PROCNUM, MMU_ACCESS_TYPE AT>
inline void _MMU_write08(uint32_t addr, uint8_t val) { _MMU_write08(PROCNUM, AT, addr, val); }

template<int PROCNUM, MMU_ACCESS_TYPE AT>
inline void _MMU_write16(uint32_t addr, uint16_t val) { _MMU_write16(PROCNUM, AT, addr, val); }

template<int PROCNUM, MMU_ACCESS_TYPE AT>
inline void _MMU_write32(uint32_t addr, uint32_t val) { _MMU_write32(PROCNUM, AT, addr, val); }

//void FASTCALL MMU_DumpMemBlock(uint8_t proc, uint32_t address, uint32_t size, uint8_t *buffer);

#endif
