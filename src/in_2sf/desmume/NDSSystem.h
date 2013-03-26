/*
	Copyright (C) 2006 yopyop
	Copyright (C) 2008-2012 DeSmuME team

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

#ifndef NDSSYSTEM_H
#define NDSSYSTEM_H

#include <cstring>

#include "armcpu.h"
#include "MMU.h"
//#include "driver.h"
//#include "GPU.h"
#include "SPU.h"
#include "mem.h"
//#include "wifi.h"
#include "emufile.h"
#include "firmware.h"

#include <string>

#if defined(_WINDOWS) && !defined(WXPORT)
#include "pathsettings.h"
#endif

template<typename Type>
struct buttonstruct {
	union {
		struct {
			// changing the order of these fields would break stuff
			//fRLDUTSBAYXWEg
			Type G; // debug
			Type E; // right shoulder
			Type W; // left shoulder
			Type X;
			Type Y;
			Type A;
			Type B;
			Type S; // start
			Type T; // select
			Type U; // up
			Type D; // down
			Type L; // left
			Type R; // right
			Type F; // lid
		};
		Type array[14];
	};
};

//extern buttonstruct<bool> Turbo;
extern buttonstruct<int> TurboTime;
//extern buttonstruct<bool> AutoHold;

//int NDS_WritePNG(const char *fname);

extern volatile bool execute;
//extern bool click;

/*
 * The firmware language values
 */
/*#define NDS_FW_LANG_JAP 0
#define NDS_FW_LANG_ENG 1
#define NDS_FW_LANG_FRE 2
#define NDS_FW_LANG_GER 3
#define NDS_FW_LANG_ITA 4
#define NDS_FW_LANG_SPA 5
#define NDS_FW_LANG_CHI 6
#define NDS_FW_LANG_RES 7*/


//#define LOG_ARM9
//#define LOG_ARM7

struct NDS_header
{
       char     gameTile[12];
       char     gameCode[4];
       uint16_t      makerCode;
       uint8_t       unitCode;
       uint8_t       deviceCode;
       uint8_t       cardSize;
       uint8_t       cardInfo[8];
       uint8_t       flags;
	   uint8_t		romversion;

       uint32_t      ARM9src;
       uint32_t      ARM9exe;
       uint32_t      ARM9cpy;
       uint32_t      ARM9binSize;

       uint32_t      ARM7src;
       uint32_t      ARM7exe;
       uint32_t      ARM7cpy;
       uint32_t      ARM7binSize;

       uint32_t      FNameTblOff;
       uint32_t      FNameTblSize;

       uint32_t      FATOff;
       uint32_t      FATSize;

       uint32_t     ARM9OverlayOff;
       uint32_t     ARM9OverlaySize;
       uint32_t     ARM7OverlayOff;
       uint32_t     ARM7OverlaySize;

       uint32_t     unknown2a;
       uint32_t     unknown2b;

       uint32_t     IconOff;
       uint16_t     CRC16;
       uint16_t     ROMtimeout;
       uint32_t     ARM9unk;
       uint32_t     ARM7unk;

       uint8_t      unknown3c[8];
       uint32_t     ROMSize;
       uint32_t     HeaderSize;
       uint8_t      unknown5[56];
       uint8_t      logo[156];
       uint16_t     logoCRC16;
       uint16_t     headerCRC16;
       uint8_t      reserved[160];
};

//extern void debug();
//void emu_halt();

extern uint64_t nds_timer;
void NDS_Reschedule();
//void NDS_RescheduleGXFIFO(uint32_t cost);
void NDS_RescheduleDMA();
void NDS_RescheduleTimers();

/*enum ENSATA_HANDSHAKE
{
	ENSATA_HANDSHAKE_none = 0,
	ENSATA_HANDSHAKE_query = 1,
	ENSATA_HANDSHAKE_ack = 2,
	ENSATA_HANDSHAKE_confirm = 3,
	ENSATA_HANDSHAKE_complete = 4
};*/

enum NDS_CONSOLE_TYPE
{
	NDS_CONSOLE_TYPE_FAT,
	NDS_CONSOLE_TYPE_LITE,
	NDS_CONSOLE_TYPE_IQUE,
	NDS_CONSOLE_TYPE_DSI
};

struct NDSSystem
{
	//int32_t wifiCycle;
	int32_t cycles;
	uint64_t timerCycle[2][4];
	uint32_t VCount;
	uint32_t old;

	//uint16_t touchX;
	//uint16_t touchY;
	//bool isTouch;
	//uint16_t pad;

	uint8_t *FW_ARM9BootCode;
	uint8_t *FW_ARM7BootCode;
	uint32_t FW_ARM9BootCodeAddr;
	uint32_t FW_ARM7BootCodeAddr;
	uint32_t FW_ARM9BootCodeSize;
	uint32_t FW_ARM7BootCodeSize;

	bool sleeping;
	bool cardEjected;
	uint32_t freezeBus;

	//this is not essential NDS runtime state.
	//it was perhaps a mistake to put it here.
	//it is far less important than the above.
	//maybe I should move it.
	//int32_t idleCycles[2];
	//int32_t runCycleCollector[2][16];
	//int32_t idleFrameCounter;
	//int32_t cpuloopIterationCount; //counts the number of times during a frame that a reschedule happened

	//if the game was booted on a debug console, this is set
	//bool debugConsole;

	//console type must be copied in when the system boots. it can't be changed on the fly.
	int ConsoleType;
	bool Is_DSI() { return ConsoleType == NDS_CONSOLE_TYPE_DSI; }

	//set if the user requests ensata emulation
	//bool ensataEmulation;

	//there is a hack in the ipc sync for ensata. this tracks its state
	//uint32_t ensataIpcSyncCounter;

	//maintains the state of the ensata handshaking protocol
	//uint32_t ensataHandshake;

	/*struct {
		uint8_t lcd, gpuMain, gfx3d_render, gfx3d_geometry, gpuSub, dispswap;
	} power1;*/ //POWCNT1

	/*struct {
		uint8_t speakers, wifi*/ /*(initial value=0)*//*;
	} power2;*/ //POWCNT2

	bool isInVblank() const { return VCount >= 192; }
	bool isIn3dVblank() const { return VCount >= 192 && VCount<215; }
};

/** /brief A touchscreen calibration point.
 */
/*struct NDS_fw_touchscreen_cal {
  uint16_t adc_x;
  uint16_t adc_y;

  uint8_t screen_x;
  uint8_t screen_y;
};*/

#define MAX_FW_NICKNAME_LENGTH 10
#define MAX_FW_MESSAGE_LENGTH 26

struct NDS_fw_config_data {
  NDS_CONSOLE_TYPE ds_type;

  uint8_t fav_colour;
  uint8_t birth_month;
  uint8_t birth_day;

  uint16_t nickname[MAX_FW_NICKNAME_LENGTH];
  uint8_t nickname_len;

  uint16_t message[MAX_FW_MESSAGE_LENGTH];
  uint8_t message_len;

  uint8_t language;

  /* touchscreen calibration */
  //struct NDS_fw_touchscreen_cal touch_cal[2];
};

extern NDSSystem nds;

#ifdef GDB_STUB
int NDS_Init( struct armcpu_memory_iface *arm9_mem_if,
              struct armcpu_ctrl_iface **arm9_ctrl_iface,
              struct armcpu_memory_iface *arm7_mem_if,
              struct armcpu_ctrl_iface **arm7_ctrl_iface);
#else
int NDS_Init ();
#endif

//void Desmume_InitOnce();

void NDS_DeInit();

bool NDS_SetROM(uint8_t * rom, uint32_t mask);
NDS_header * NDS_getROMHeader();

struct RomBanner
{
	RomBanner(bool defaultInit);
	uint16_t version; //Version  (0001h)
	uint16_t crc16; //CRC16 across entries 020h..83Fh
	uint8_t reserved[0x1C]; //Reserved (zero-filled)
	uint8_t bitmap[0x200]; //Icon Bitmap  (32x32 pix) (4x4 tiles, each 4x8 bytes, 4bit depth)
	uint16_t palette[0x10]; //Icon Palette (16 colors, 16bit, range 0000h-7FFFh) (Color 0 is transparent, so the 1st palette entry is ignored)
	enum { NUM_TITLES = 6 };
	union {
		struct {
			uint16_t title_jp[0x80]; //Title 0 Japanese (128 characters, 16bit Unicode)
			uint16_t title_en[0x80]; //Title 1 English  ("")
			uint16_t title_fr[0x80]; //Title 2 French   ("")
			uint16_t title_de[0x80]; //Title 3 German   ("")
			uint16_t title_it[0x80]; //Title 4 Italian  ("")
			uint16_t title_es[0x80]; //Title 5 Spanish  ("")
		};
		uint16_t titles[NUM_TITLES][0x80];
	};
	uint8_t end0xFF[0x1C0];
  //840h  ?    (Maybe newer/chinese firmware do also support chinese title?)
  //840h  -    End of Icon/Title structure (next 1C0h bytes usually FFh-filled)
};

struct GameInfo
{
	GameInfo()
		: romdata(NULL)
	{}

	void loadData(char* buf, int size)
	{
		resize(size);
		memcpy(romdata,buf,size);
		romsize = (uint32_t)size;
		fillGap();
	}

	void fillGap()
	{
		memset(romdata+romsize,0xFF,allocatedSize-romsize);
	}

	void resize(int size) {
		if(romdata != NULL) delete[] romdata;

		//calculate the necessary mask for the requested size
		mask = size-1;
		mask |= (mask >>1);
		mask |= (mask >>2);
		mask |= (mask >>4);
		mask |= (mask >>8);
		mask |= (mask >>16);

		//now, we actually need to over-allocate, because bytes from anywhere protected by that mask
		//could be read from the rom
		allocatedSize = mask+4;

		romdata = new char[allocatedSize];
		romsize = size;
	}
	uint32_t crc;
	NDS_header header;
	char ROMserial[20];
	char ROMname[20];
	//char ROMfullName[7][0x100];
	//void populate();
	char* romdata;
	uint32_t romsize;
	uint32_t allocatedSize;
	uint32_t mask;
	//const RomBanner& getRomBanner();
	//bool hasRomBanner();
	bool isHomebrew;
};

/*typedef struct TSCalInfo
{
	struct adc
	{
		uint16_t x1, x2;
		uint16_t y1, y2;
		uint16_t width;
		uint16_t height;
	} adc;

	struct scr
	{
		uint8_t x1, x2;
		uint8_t y1, y2;
		uint16_t width;
		uint16_t height;
	} scr;

} TSCalInfo;*/

extern GameInfo gameInfo;


struct UserButtons : buttonstruct<bool>
{
};
struct UserTouch
{
	uint16_t touchX;
	uint16_t touchY;
	bool isTouch;
};
struct UserMicrophone
{
	uint32_t micButtonPressed;
};
struct UserInput
{
	UserButtons buttons;
	UserTouch touch;
	UserMicrophone mic;
};

// set physical user input
// these functions merely request the input to be changed.
// the actual change happens later at a specific time during the frame.
// this is to minimize the risk of desyncs.
//void NDS_setTouchPos(uint16_t x, uint16_t y);
//void NDS_releaseTouch();
//void NDS_setPad(bool right,bool left,bool down,bool up,bool select,bool start,bool B,bool A,bool Y,bool X,bool leftShoulder,bool rightShoulder,bool debug, bool lid);
//void NDS_setMic(bool pressed);

// get physical user input
// not including the results of autofire/etc.
// the effects of calls to "set physical user input" functions will be immediately reflected here though.
//const UserInput& NDS_getRawUserInput();
//const UserInput& NDS_getPrevRawUserInput();

// get final (fully processed) user input
// this should match whatever was or would be sent to the game
//const UserInput& NDS_getFinalUserInput();

// set/get to-be-processed or in-the-middle-of-being-processed user input
// to process input, simply call this function and edit the return value.
// (applying autofire is one example of processing the input.)
// (movie playback is another example.)
// this must be done after the raw user input is set
// and before that input is sent to the game's memory.
//UserInput& NDS_getProcessingUserInput();
//bool NDS_isProcessingUserInput();
// call once per frame to prepare input for processing
//void NDS_beginProcessingInput();
// call once per frame to copy the processed input to the final input
//void NDS_endProcessingInput();

// this is in case something needs reentrancy while processing input
//void NDS_suspendProcessingInput(bool suspend);



//int NDS_LoadROM(const char *filename, const char* logicalFilename=0);
void NDS_FreeROM();
void NDS_Reset();
//int NDS_ImportSave(const char *filename);
//bool NDS_ExportSave(const char *filename);

//void nds_savestate(EMUFILE* os);
bool nds_loadstate(EMUFILE* is, int size);

//int NDS_WriteBMP(const char *filename);

void NDS_Sleep();
//void NDS_ToggleCardEject();

//void NDS_SkipNextFrame();
//#define NDS_SkipFrame(s) if(s) NDS_SkipNext2DFrame();
//void NDS_OmitFrameSkip(int force=0);

//void NDS_debug_break();
//void NDS_debug_continue();
//void NDS_debug_step();

void execHardware_doAllDma(EDMAMode modeNum);

template<bool FORCE> void NDS_exec(int32_t nb = 560190<<1);

//extern int lagframecounter;

/*static INLINE void NDS_swapScreen()
{
   uint16_t tmp = MainScreen.offset;
   MainScreen.offset = SubScreen.offset;
   SubScreen.offset = tmp;
}*/

//int NDS_WriteBMP_32bppBuffer(int width, int height, const void* buf, const char *filename);

extern struct TCommonSettings {
	TCommonSettings()
		: //GFX3D_HighResolutionInterpolateColor(true)
		//, GFX3D_EdgeMark(true)
		//, GFX3D_Fog(true)
		//, GFX3D_Texture(true)
		//, GFX3D_Zelda_Shadow_Depth_Hack(0)
		/*,*/ UseExtBIOS(false)
		, SWIFromBIOS(false)
		, PatchSWI3(false)
		, UseExtFirmware(false)
		, BootFromFirmware(false)
		, ConsoleType(NDS_CONSOLE_TYPE_FAT)
		//, DebugConsole(false)
		//, EnsataEmulation(false)
		//, cheatsDisable(false)
		//, num_cores(1)
		, rigorous_timing(false)
		, advanced_timing(true)
		//, micMode(InternalNoise)
		, spuInterpolationMode(SPUInterpolation_Linear)
		, manualBackupType(0)
		, spu_captureMuted(false)
		, spu_advanced(false)
	{
		strcpy(ARM9BIOS, "biosnds9.bin");
		strcpy(ARM7BIOS, "biosnds7.bin");
		strcpy(Firmware, "firmware.bin");
		NDS_FillDefaultFirmwareConfigData(&InternalFirmConf);

		/* WIFI mode: adhoc = 0, infrastructure = 1 */
		//wifi.mode = 1;
		//wifi.infraBridgeAdapter = 0;

		for(int i=0;i<16;i++)
			spu_muteChannels[i] = false;

		/*for(int g=0;g<2;g++)
			for(int x=0;x<5;x++)
				dispLayers[g][x]=true;*/
	}
	//bool GFX3D_HighResolutionInterpolateColor;
	//bool GFX3D_EdgeMark;
	//bool GFX3D_Fog;
	//bool GFX3D_Texture;
	//int  GFX3D_Zelda_Shadow_Depth_Hack;

	bool UseExtBIOS;
	char ARM9BIOS[256];
	char ARM7BIOS[256];
	bool SWIFromBIOS;
	bool PatchSWI3;

	bool UseExtFirmware;
	char Firmware[256];
	bool BootFromFirmware;
	struct NDS_fw_config_data InternalFirmConf;

	NDS_CONSOLE_TYPE ConsoleType;
	//bool DebugConsole;
	//bool EnsataEmulation;

	//bool cheatsDisable;

	//int num_cores;
	//bool single_core() { return num_cores==1; }
	bool rigorous_timing;

	//bool dispLayers[2][5];

	/*FAST_ALIGN*/ bool advanced_timing;

	/*struct _Wifi {
		int mode;
		int infraBridgeAdapter;
	} wifi;*/

	/*enum MicMode
	{
		InternalNoise = 0,
		Sample = 1,
		Random = 2,
		Physical = 3
	} micMode;*/


	SPUInterpolationMode spuInterpolationMode;

	//this is a temporary hack until we straighten out the flushing logic and/or gxfifo
	//int gfx3d_flushMode;

	//this is the user's choice of manual backup type, for cases when the autodetection can't be trusted
	int manualBackupType;

	bool spu_muteChannels[16];
	bool spu_captureMuted;
	bool spu_advanced;

	/*struct _ShowGpu {
		_ShowGpu() : main(true), sub(true) {}
		union {
			struct { bool main,sub; };
			bool screens[2];
		};
	} showGpu;*/

	/*struct _Hud {
		_Hud()
			: ShowInputDisplay(false)
			, ShowGraphicalInputDisplay(false)
			, FpsDisplay(false)
			, FrameCounterDisplay(false)
			, ShowLagFrameCounter(false)
			, ShowMicrophone(false)
			, ShowRTC(false)
		{}
		bool ShowInputDisplay, ShowGraphicalInputDisplay, FpsDisplay, FrameCounterDisplay, ShowLagFrameCounter, ShowMicrophone, ShowRTC;
	} hud;*/

} CommonSettings;


//extern std::string InputDisplayString;
//extern int LagFrameFlag;
//extern int lastLag, TotalLagFrames;

//void MovieSRAM();

//void ClearAutoHold();

//bool ValidateSlot2Access(uint32_t procnum, uint32_t demandSRAMSpeed, uint32_t demand1stROMSpeed, uint32_t demand2ndROMSpeed, int clockbits);

#endif
