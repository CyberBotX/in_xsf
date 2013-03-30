/*
	Copyright (C) 2006 Normmatt
	Copyright (C) 2006 Theo Berkau
	Copyright (C) 2007 Pascal Giard
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

#include <stack>
#include <set>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif
#include "saves.h"
#include "MMU.h"
#include "NDSSystem.h"
//#include "render3D.h"
#include "cp15.h"
//#include "GPU_osd.h"
#include "version.h"

#include "readwrite.h"
//#include "gfx3d.h"
//#include "movie.h"
//#include "mic.h"
#include "MMU_timing.h"

#include "path.h"

#ifdef _WINDOWS
//#include "main.h"
#endif

//int lastSaveState = 0;		//Keeps track of last savestate used for quick save/load functions

//void*v is actually a void** which will be indirected before reading
//since this isnt supported right now, it is declared in here to make things compile
//#define SS_INDIRECT            0x80000000

//savestates_t savestates[NB_STATES];

#define SAVESTATE_VERSION       12
static const char* magic = "DeSmuME SState\0";

//a savestate chunk loader can set this if it wants to permit a silent failure (for compatibility)
static bool SAV_silent_fail_flag;

SFORMAT SF_ARM7[]={
	{ "7INS", 4, 1, &NDS_ARM7.instruction },
	{ "7INA", 4, 1, &NDS_ARM7.instruct_adr },
	{ "7INN", 4, 1, &NDS_ARM7.next_instruction },
	{ "7REG", 4,16, NDS_ARM7.R },
	{ "7CPS", 4, 1, &NDS_ARM7.CPSR },
	{ "7SPS", 4, 1, &NDS_ARM7.SPSR },
	{ "7DUS", 4, 1, &NDS_ARM7.R13_usr },
	{ "7EUS", 4, 1, &NDS_ARM7.R14_usr },
	{ "7DSV", 4, 1, &NDS_ARM7.R13_svc },
	{ "7ESV", 4, 1, &NDS_ARM7.R14_svc },
	{ "7DAB", 4, 1, &NDS_ARM7.R13_abt },
	{ "7EAB", 4, 1, &NDS_ARM7.R14_abt },
	{ "7DUN", 4, 1, &NDS_ARM7.R13_und },
	{ "7EUN", 4, 1, &NDS_ARM7.R14_und },
	{ "7DIR", 4, 1, &NDS_ARM7.R13_irq },
	{ "7EIR", 4, 1, &NDS_ARM7.R14_irq },
	{ "78FI", 4, 1, &NDS_ARM7.R8_fiq },
	{ "79FI", 4, 1, &NDS_ARM7.R9_fiq },
	{ "7AFI", 4, 1, &NDS_ARM7.R10_fiq },
	{ "7BFI", 4, 1, &NDS_ARM7.R11_fiq },
	{ "7CFI", 4, 1, &NDS_ARM7.R12_fiq },
	{ "7DFI", 4, 1, &NDS_ARM7.R13_fiq },
	{ "7EFI", 4, 1, &NDS_ARM7.R14_fiq },
	{ "7SVC", 4, 1, &NDS_ARM7.SPSR_svc },
	{ "7ABT", 4, 1, &NDS_ARM7.SPSR_abt },
	{ "7UND", 4, 1, &NDS_ARM7.SPSR_und },
	{ "7IRQ", 4, 1, &NDS_ARM7.SPSR_irq },
	{ "7FIQ", 4, 1, &NDS_ARM7.SPSR_fiq },
	{ "7int", 4, 1, &NDS_ARM7.intVector },
	{ "7LDT", 1, 1, &NDS_ARM7.LDTBit },
	{ "7Wai", 4, 1, &NDS_ARM7.waitIRQ },
	{ "7hef", 4, 1, &NDS_ARM7.halt_IE_and_IF },
	{ "7iws", 1, 1, &NDS_ARM7.intrWaitARM_state },
	{ NULL, 0, 0, NULL }
};

SFORMAT SF_ARM9[]={
	{ "9INS", 4, 1, &NDS_ARM9.instruction},
	{ "9INA", 4, 1, &NDS_ARM9.instruct_adr},
	{ "9INN", 4, 1, &NDS_ARM9.next_instruction},
	{ "9REG", 4,16, NDS_ARM9.R},
	{ "9CPS", 4, 1, &NDS_ARM9.CPSR},
	{ "9SPS", 4, 1, &NDS_ARM9.SPSR},
	{ "9DUS", 4, 1, &NDS_ARM9.R13_usr},
	{ "9EUS", 4, 1, &NDS_ARM9.R14_usr},
	{ "9DSV", 4, 1, &NDS_ARM9.R13_svc},
	{ "9ESV", 4, 1, &NDS_ARM9.R14_svc},
	{ "9DAB", 4, 1, &NDS_ARM9.R13_abt},
	{ "9EAB", 4, 1, &NDS_ARM9.R14_abt},
	{ "9DUN", 4, 1, &NDS_ARM9.R13_und},
	{ "9EUN", 4, 1, &NDS_ARM9.R14_und},
	{ "9DIR", 4, 1, &NDS_ARM9.R13_irq},
	{ "9EIR", 4, 1, &NDS_ARM9.R14_irq},
	{ "98FI", 4, 1, &NDS_ARM9.R8_fiq},
	{ "99FI", 4, 1, &NDS_ARM9.R9_fiq},
	{ "9AFI", 4, 1, &NDS_ARM9.R10_fiq},
	{ "9BFI", 4, 1, &NDS_ARM9.R11_fiq},
	{ "9CFI", 4, 1, &NDS_ARM9.R12_fiq},
	{ "9DFI", 4, 1, &NDS_ARM9.R13_fiq},
	{ "9EFI", 4, 1, &NDS_ARM9.R14_fiq},
	{ "9SVC", 4, 1, &NDS_ARM9.SPSR_svc},
	{ "9ABT", 4, 1, &NDS_ARM9.SPSR_abt},
	{ "9UND", 4, 1, &NDS_ARM9.SPSR_und},
	{ "9IRQ", 4, 1, &NDS_ARM9.SPSR_irq},
	{ "9FIQ", 4, 1, &NDS_ARM9.SPSR_fiq},
	{ "9int", 4, 1, &NDS_ARM9.intVector},
	{ "9LDT", 1, 1, &NDS_ARM9.LDTBit},
	{ "9Wai", 4, 1, &NDS_ARM9.waitIRQ},
	{ "9hef", 4, 1, &NDS_ARM9.halt_IE_and_IF },
	{ "9iws", 1, 1, &NDS_ARM7.intrWaitARM_state },
	{ NULL, 0, 0, NULL }
};

SFORMAT SF_MEM[]={
	{ "ITCM", 1, sizeof(MMU.ARM9_ITCM),   MMU.ARM9_ITCM},
	{ "DTCM", 1, sizeof(MMU.ARM9_DTCM),   MMU.ARM9_DTCM},

	 //for legacy purposes, WRAX is a separate variable. shouldnt be a problem.
	{ "WRAM", 1, 0x400000, MMU.MAIN_MEM},
	{ "WRAX", 1, 0x400000, MMU.MAIN_MEM+0x400000},

	//NOTE - this is not as large as the allocated memory.
	//the memory is overlarge due to the way our memory map system is setup
	//but there are actually no more registers than this
	{ "9REG", 1, 0x2000,   MMU.ARM9_REG},

	{ "VMEM", 1, sizeof(MMU.ARM9_VMEM),    MMU.ARM9_VMEM},
	{ "OAMS", 1, sizeof(MMU.ARM9_OAM),    MMU.ARM9_OAM},

	//this size is specially chosen to avoid saving the blank space at the end
	{ "LCDM", 1, 0xA4000,		MMU.ARM9_LCD},
	{ NULL, 0, 0, NULL }
};

SFORMAT SF_NDS[]={
	//{ "_WCY", 4, 1, &nds.wifiCycle},
	{ "_TCY", 8, 8, nds.timerCycle},
	{ "_VCT", 4, 1, &nds.VCount},
	{ "_OLD", 4, 1, &nds.old},
	{ "_TPX", 2, 1, NULL},
	{ "_TPY", 2, 1, NULL},
	{ "_TPB", 4, 1, NULL},
	{ "_DBG", 4, 1, NULL},
	{ "_ENS", 4, 1, NULL},
	{ "_ENH", 4, 1, NULL},
	{ "_ENI", 4, 1, NULL},
	{ "_SLP", 4, 1, &nds.sleeping},
	{ "_FBS", 4, 1, &nds.freezeBus},
	{ "_CEJ", 4, 1, &nds.cardEjected},
	{ "_P00", 1, 1, NULL},
	{ "_P01", 1, 1, NULL},
	//{ "_P02", 1, 1, &nds.power1.gfx3d_render},
	//{ "_P03", 1, 1, &nds.power1.gfx3d_geometry},
	{ "_P04", 1, 1, NULL},
	{ "_P05", 1, 1, NULL},
	{ "_P06", 1, 1, NULL},
	//{ "_P07", 1, 1, &nds.power2.wifi},
	{ NULL, 0, 0, NULL }
};

SFORMAT SF_MMU[]={
	{ "M7BI", 1, sizeof(MMU.ARM7_BIOS), MMU.ARM7_BIOS},
	{ "M7ER", 1, sizeof(MMU.ARM7_ERAM), MMU.ARM7_ERAM},
	{ "M7RG", 1, sizeof(MMU.ARM7_REG), MMU.ARM7_REG},
	{ "M7WI", 1, sizeof(MMU.ARM7_WIRAM), MMU.ARM7_WIRAM},
	{ "MSWI", 1, sizeof(MMU.SWIRAM), MMU.SWIRAM},
	{ "M9RW", 1, 1,       &MMU.ARM9_RW_MODE},
	{ "MDTC", 4, 1,       &MMU.DTCMRegion},
	{ "MITC", 4, 1,       &MMU.ITCMRegion},
	{ "MTIM", 2, 8,       MMU.timer},
	{ "MTMO", 4, 8,       MMU.timerMODE},
	{ "MTON", 4, 8,       MMU.timerON},
	{ "MTRN", 4, 8,       MMU.timerRUN},
	{ "MTRL", 2, 8,       MMU.timerReload},
	{ "MIME", 4, 2,       MMU.reg_IME},
	{ "MIE_", 4, 2,       MMU.reg_IE},
	{ "MIF_", 4, 2,       MMU.reg_IF_bits},

	//{ "MGXC", 8, 1,       &MMU.gfx3dCycles},

	{ "M_SX", 1, 2,       &MMU.SPI_CNT},
	{ "M_SC", 1, 2,       &MMU.SPI_CMD},
	{ "MASX", 1, 2,       &MMU.AUX_SPI_CNT},
	{ "MASC", 1, 2,       &MMU.AUX_SPI_CMD},

	{ "MDV1", 4, 1,       &MMU.divRunning},
	{ "MDV2", 8, 1,       &MMU.divResult},
	{ "MDV3", 8, 1,       &MMU.divMod},
	{ "MDV5", 8, 1,       &MMU.divCycles},

	{ "MSQ1", 4, 1,       &MMU.sqrtRunning},
	{ "MSQ2", 4, 1,       &MMU.sqrtResult},
	{ "MSQ4", 8, 1,       &MMU.sqrtCycles},

	//begin memory chips
	{ "BUCO", 1, 1,       &MMU.fw.com},
	{ "BUAD", 4, 1,       &MMU.fw.addr},
	{ "BUAS", 1, 1,       &MMU.fw.addr_shift},
	{ "BUAZ", 1, 1,       &MMU.fw.addr_size},
	{ "BUWE", 4, 1,       &MMU.fw.write_enable},
	{ "BUWR", 4, 1,       &MMU.fw.writeable_buffer},
	//end memory chips

	{ "MC0A", 4, 1,       &MMU.dscard[0].address},
	{ "MC0T", 4, 1,       &MMU.dscard[0].transfer_count},
	{ "MC1A", 4, 1,       &MMU.dscard[1].address},
	{ "MC1T", 4, 1,       &MMU.dscard[1].transfer_count},
	//{ "MCHT", 4, 1,       &MMU.CheckTimers},
	//{ "MCHD", 4, 1,       &MMU.CheckDMAs},

	//fifos
	{ "F0TH", 1, 1,       &ipc_fifo[0].head},
	{ "F0TL", 1, 1,       &ipc_fifo[0].tail},
	{ "F0SZ", 1, 1,       &ipc_fifo[0].size},
	{ "F0BF", 4, 16,      ipc_fifo[0].buf},
	{ "F1TH", 1, 1,       &ipc_fifo[1].head},
	{ "F1TL", 1, 1,       &ipc_fifo[1].tail},
	{ "F1SZ", 1, 1,       &ipc_fifo[1].size},
	{ "F1BF", 4, 16,      ipc_fifo[1].buf},

	{ "FDHD", 4, 1,       NULL},
	{ "FDTL", 4, 1,       NULL},
	{ "FDBF", 4, 0x6000,  NULL},

	{ "PMCN", 1, 1,			&MMU.powerMan_CntReg},
	{ "PMCW", 4, 1,			&MMU.powerMan_CntRegWritten},
	{ "PMCR", 1, 5,			&MMU.powerMan_Reg},

	{ "MR3D", 4, 1,		NULL},

	{ NULL, 0, 0, NULL }
};

/*static uint32_t tmpu32;
SFORMAT SF_MOVIE[]={
	{ "FRAC", 4, 1, &tmpu32},
	{ "LAGC", 4, 1, &tmpu32},
	{ NULL, 0, 0, NULL }
};*/

/*static void mmu_savestate(EMUFILE* os)
{
	uint32_t version = 5;
	write32le(version,os);

	//version 2:
	MMU_new.backupDevice.save_state(os);

	//version 3:
	MMU_new.gxstat.savestate(os);
	for(int i=0;i<2;i++)
		for(int j=0;j<4;j++)
			MMU_new.dma[i][j].savestate(os);

	MMU_timing.arm9codeFetch.savestate(os, version);
	MMU_timing.arm9dataFetch.savestate(os, version);
	MMU_timing.arm7codeFetch.savestate(os, version);
	MMU_timing.arm7dataFetch.savestate(os, version);
	MMU_timing.arm9codeCache.savestate(os, version);
	MMU_timing.arm9dataCache.savestate(os, version);

	//version 4:
	MMU_new.sqrt.savestate(os);
	MMU_new.div.savestate(os);
}*/

/*static uint16_t tmpu16;
static uint16_t tmpu16_array3[3];
static uint8_t tmpu8;
static uint8_t tmpu8_array3[3];
static uint32_t tmpu32_array3[3];
static uint8_t tmpu8_array6[6];
static uint64_t tmpu64;
static uint8_t tmpu8_array105[105];
static uint16_t tmpu16_array4096[0x1000];
static uint16_t tmpu16_array2048[0x800];
static uint8_t tmpu8_array4096[4096];
SFORMAT SF_WIFI[]={
	{ "W000", 4, 1, &tmpu32},
	{ "W010", 4, 1, &tmpu32},

	{ "W020", 2, 1, &tmpu16},
	{ "W030", 2, 1, &tmpu16},

	{ "W040", 2, 1, &tmpu16},
	{ "W050", 2, 1, &tmpu16},

	{ "W060", 2, 1, &tmpu16},
	{ "W070", 2, 1, &tmpu16},
	{ "W080", 4, 1, &tmpu32},

	{ "W090", 2, 3, tmpu16_array3},
	{ "W100", 2, 1, &tmpu16},
	{ "W110", 2, 1, &tmpu16},
	{ "W120", 2, 1, &tmpu16},
	{ "W130", 2, 1, &tmpu16},
	{ "W140", 4, 1, &tmpu32},
	{ "W150", 1, 1, &tmpu8},
	{ "W160", 1, 3, tmpu8_array3},
	{ "W170", 4, 3, tmpu32_array3},
	{ "W180", 4, 3, tmpu32_array3},
	{ "W190", 4, 3, tmpu32_array3},

	{ "W200", 2, 1, &tmpu16},
	{ "W210", 2, 1, &tmpu16},

	{ "W220", 1, 6, tmpu8_array6},
	{ "W230", 1, 6, tmpu8_array6},

	{ "W240", 2, 1, &tmpu16},
	{ "W250", 2, 1, &tmpu16},
	{ "W260", 2, 1, &tmpu16},

	{ "W270", 4, 1, &tmpu32},
	{ "W280", 8, 1, &tmpu64},
	{ "W290", 4, 1, &tmpu32},
	{ "W300", 8, 1, &tmpu64},
	{ "W310", 4, 1, &tmpu32},
	{ "W320", 2, 1, &tmpu16},
	{ "W330", 4, 1, &tmpu32},

	{ "WR00", 4, 1, &tmpu32},
	{ "WR01", 4, 1, &tmpu32},
	{ "WR02", 4, 1, &tmpu32},
	{ "WR03", 4, 1, &tmpu32},
	{ "WR04", 4, 1, &tmpu32},
	{ "WR05", 4, 1, &tmpu32},
	{ "WR06", 4, 1, &tmpu32},
	{ "WR07", 4, 1, &tmpu32},
	{ "WR08", 4, 1, &tmpu32},
	{ "WR09", 4, 1, &tmpu32},
	{ "WR10", 4, 1, &tmpu32},
	{ "WR11", 4, 1, &tmpu32},
	{ "WR12", 4, 1, &tmpu32},

	{ "W340", 1, 105, tmpu8_array105},

	{ "W350", 2, 1, &tmpu16},
	{ "W360", 2, 1, &tmpu16},
	{ "W370", 4, 1, &tmpu32},
	{ "W380", 2, 1, &tmpu16},

	{ "W400", 2, 0x1000, tmpu16_array4096},
	{ "W410", 2, 1, &tmpu16},
	{ "W420", 2, 1, &tmpu16},
	{ "W430", 2, 1, &tmpu16},
	{ "W460", 2, 1, &tmpu16},
	{ "W470", 2, 1, &tmpu16},
	{ "W480", 2, 1, &tmpu16},
	{ "W490", 2, 1, &tmpu16},
	{ "W500", 2, 1, &tmpu16},
	{ "W510", 2, 1, &tmpu16},
	{ "W520", 2, 1, &tmpu16},
	{ "W530", 2, 1, &tmpu16},
	{ "W540", 2, 1, &tmpu16},

	{ "W550", 4, 1, &tmpu32},
	{ "W560", 4, 1, &tmpu32},
	{ "W570", 4, 1, &tmpu32},

	{ "W580", 2, 0x800, tmpu16_array2048},
	{ "W590", 2, 1, &tmpu16},

	{ "WX00", 8, 1, &tmpu64},
	{ "WX10", 1, 4096, tmpu8_array4096},
	{ "WX20", 4, 1, &tmpu32},
	{ "WX30", 4, 1, &tmpu32},
	{ "WX40", 4, 1, &tmpu32},

	{ NULL, 0, 0, NULL }
};*/

static bool mmu_loadstate(EMUFILE* is, int)
{
	//read version
	uint32_t version;
	if(read32le(&version,is) != 1) return false;

	if(version == 0 || version == 1)
	{
		uint32_t bupmem_size = 0;
		uint32_t addr_size = 0xFFFFFFFF;

		if(version == 0)
		{
			//version 0 was buggy and didnt save the type.
			//it would silently fail if there was a size mismatch
			SAV_silent_fail_flag = true;
			if(read32le(&bupmem_size,is) != 1) return false;
			//if(bupmem_size != MMU.bupmem.size) return false; //mismatch between current initialized and saved size
			addr_size = BackupDevice::addr_size_for_old_save_size(bupmem_size);
		}
		else if(version == 1)
		{
			//version 1 reinitializes the save system with the type that was saved
			uint32_t bupmem_type;
			if(read32le(&bupmem_type,is) != 1) return false;
			if(read32le(&bupmem_size,is) != 1) return false;
			addr_size = BackupDevice::addr_size_for_old_save_type(bupmem_type);
			if(addr_size == 0xFFFFFFFF)
				addr_size = BackupDevice::addr_size_for_old_save_size(bupmem_size);
		}

		if(addr_size == 0xFFFFFFFF)
			return false;

		uint8_t* temp = new uint8_t[bupmem_size];
		is->fread((char*)temp,bupmem_size);
		MMU_new.backupDevice.load_old_state(addr_size,temp,bupmem_size);
		delete[] temp;
		if(is->fail()) return false;
	}

	if(version < 2) return true;

	bool ok = MMU_new.backupDevice.load_state(is);

	if(version < 3) return true;

	ok &= MMU_new.gxstat.loadstate(is);

	for(int i=0;i<2;i++)
		for(int j=0;j<4;j++)
			ok &= MMU_new.dma[i][j].loadstate(is);

	ok &= MMU_timing.arm9codeFetch.loadstate(is, version);
	ok &= MMU_timing.arm9dataFetch.loadstate(is, version);
	ok &= MMU_timing.arm7codeFetch.loadstate(is, version);
	ok &= MMU_timing.arm7dataFetch.loadstate(is, version);
	ok &= MMU_timing.arm9codeCache.loadstate(is, version);
	ok &= MMU_timing.arm9dataCache.loadstate(is, version);

	if(version < 4) return true;

	ok &= MMU_new.sqrt.loadstate(is,version);
	ok &= MMU_new.div.loadstate(is,version);

	//to prevent old savestates from confusing IF bits, mask out ones which had been stored but should have been generated
	MMU.reg_IF_bits[0] &= ~0x00200000;
	MMU.reg_IF_bits[1] &= ~0x00000000;

	//MMU_new.gxstat.fifo_low = gxFIFO.size <= 127;
	//MMU_new.gxstat.fifo_empty = gxFIFO.size == 0;

	/*if(version < 5)
		MMU.reg_DISP3DCNT_bits = T1ReadWord(MMU.ARM9_REG,0x60);*/

	return ok;
}

/*static void cp15_saveone(armcp15_t *cp15, EMUFILE* os)
{
	write32le(cp15->IDCode,os);
	write32le(cp15->cacheType,os);
    write32le(cp15->TCMSize,os);
    write32le(cp15->ctrl,os);
    write32le(cp15->DCConfig,os);
    write32le(cp15->ICConfig,os);
    write32le(cp15->writeBuffCtrl,os);
    write32le(cp15->und,os);
    write32le(cp15->DaccessPerm,os);
    write32le(cp15->IaccessPerm,os);
    write32le(cp15->protectBaseSize0,os);
    write32le(cp15->protectBaseSize1,os);
    write32le(cp15->protectBaseSize2,os);
    write32le(cp15->protectBaseSize3,os);
    write32le(cp15->protectBaseSize4,os);
    write32le(cp15->protectBaseSize5,os);
    write32le(cp15->protectBaseSize6,os);
    write32le(cp15->protectBaseSize7,os);
    write32le(cp15->cacheOp,os);
    write32le(cp15->DcacheLock,os);
    write32le(cp15->IcacheLock,os);
    write32le(cp15->ITCMRegion,os);
    write32le(cp15->DTCMRegion,os);
    write32le(cp15->processID,os);
    write32le(cp15->RAM_TAG,os);
    write32le(cp15->testState,os);
    write32le(cp15->cacheDbg,os);
    for(int i=0;i<8;i++) write32le(cp15->regionWriteMask_USR[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionWriteMask_SYS[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionReadMask_USR[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionReadMask_SYS[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionExecuteMask_USR[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionExecuteMask_SYS[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionWriteSet_USR[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionWriteSet_SYS[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionReadSet_USR[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionReadSet_SYS[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionExecuteSet_USR[i],os);
    for(int i=0;i<8;i++) write32le(cp15->regionExecuteSet_SYS[i],os);
}*/

/*static void cp15_savestate(EMUFILE* os)
{
	//version
	write32le(1,os);

	cp15_saveone((armcp15_t *)NDS_ARM9.coproc[15],os);
	//ARM7 not have coprocessor
	//cp15_saveone((armcp15_t *)NDS_ARM7.coproc[15],os);
}*/

static bool cp15_loadone(armcp15_t *cp15, EMUFILE* is)
{
	if(!read32le(&cp15->IDCode,is)) return false;
	if(!read32le(&cp15->cacheType,is)) return false;
    if(!read32le(&cp15->TCMSize,is)) return false;
    if(!read32le(&cp15->ctrl,is)) return false;
    if(!read32le(&cp15->DCConfig,is)) return false;
    if(!read32le(&cp15->ICConfig,is)) return false;
    if(!read32le(&cp15->writeBuffCtrl,is)) return false;
    if(!read32le(&cp15->und,is)) return false;
    if(!read32le(&cp15->DaccessPerm,is)) return false;
    if(!read32le(&cp15->IaccessPerm,is)) return false;
    if(!read32le(&cp15->protectBaseSize0,is)) return false;
    if(!read32le(&cp15->protectBaseSize1,is)) return false;
    if(!read32le(&cp15->protectBaseSize2,is)) return false;
    if(!read32le(&cp15->protectBaseSize3,is)) return false;
    if(!read32le(&cp15->protectBaseSize4,is)) return false;
    if(!read32le(&cp15->protectBaseSize5,is)) return false;
    if(!read32le(&cp15->protectBaseSize6,is)) return false;
    if(!read32le(&cp15->protectBaseSize7,is)) return false;
    if(!read32le(&cp15->cacheOp,is)) return false;
    if(!read32le(&cp15->DcacheLock,is)) return false;
    if(!read32le(&cp15->IcacheLock,is)) return false;
    if(!read32le(&cp15->ITCMRegion,is)) return false;
    if(!read32le(&cp15->DTCMRegion,is)) return false;
    if(!read32le(&cp15->processID,is)) return false;
    if(!read32le(&cp15->RAM_TAG,is)) return false;
    if(!read32le(&cp15->testState,is)) return false;
    if(!read32le(&cp15->cacheDbg,is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionWriteMask_USR[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionWriteMask_SYS[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionReadMask_USR[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionReadMask_SYS[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionExecuteMask_USR[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionExecuteMask_SYS[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionWriteSet_USR[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionWriteSet_SYS[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionReadSet_USR[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionReadSet_SYS[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionExecuteSet_USR[i],is)) return false;
    for(int i=0;i<8;i++) if(!read32le(&cp15->regionExecuteSet_SYS[i],is)) return false;

    return true;
}

static bool cp15_loadstate(EMUFILE* is, int)
{
	//read version
	uint32_t version;
	if(read32le(&version,is) != 1) return false;
	if(version > 1) return false;

	if(!cp15_loadone((armcp15_t *)NDS_ARM9.coproc[15],is)) return false;

	if(version == 0)
	{
		//ARM7 not have coprocessor
		uint8_t *tmp_buf = new uint8_t [sizeof(armcp15_t)];
		if (!tmp_buf) return false;
		if(!cp15_loadone((armcp15_t *)tmp_buf,is)) return false;
		delete [] tmp_buf;
		tmp_buf = NULL;
	}

	return true;
}



/* Format time and convert to string */
/*static char * format_time(time_t cal_time)
{
  struct tm *time_struct;
  static char str[64];

  time_struct=localtime(&cal_time);
  strftime(str, sizeof str, "%d-%b-%Y %H:%M:%S", time_struct);

  return str;
}*/

/*void clear_savestates()
{
  uint8_t i;
  for( i = 0; i < NB_STATES; i++ )
    savestates[i].exists = false;
}*/

// Scan for existing savestates and update struct
/*void scan_savestates()
{
  struct stat sbuf;
  char filename[MAX_PATH+1];

  clear_savestates();

  for(int i = 0; i < NB_STATES; i++ )
    {
     path.getpathnoext(path.STATES, filename);

	  if (strlen(filename) + strlen(".dst") + strlen("-2147483648")*/ /* = biggest string for i */ /*>MAX_PATH) return ;
      sprintf(filename+strlen(filename), ".ds%d", i);
      if( stat(filename,&sbuf) == -1 ) continue;
      savestates[i].exists = true;
      strncpy(savestates[i].date, format_time(sbuf.st_mtime),40);
	  savestates[i].date[40-1] = '\0';
    }

  return ;
}*/

/*void savestate_slot(int num)
{
   struct stat sbuf;
   char filename[MAX_PATH+1];

	lastSaveState = num;		//Set last savestate used

    path.getpathnoext(path.STATES, filename);

   if (strlen(filename) + strlen(".dsx") + strlen("-2147483648")*/ /* = biggest string for num *//* >MAX_PATH) return ;
   sprintf(filename+strlen(filename), ".ds%d", num);*/

   /*if (savestate_save(filename))
   {
	   osd->setLineColor(255, 255, 255);
	   osd->addLine("Saved to %i slot", num);
   }
   else
   {
	   osd->setLineColor(255, 0, 0);
	   osd->addLine("Error saving %i slot", num);
	   return;
   }*/

   /*if (num >= 0 && num < NB_STATES)
   {
	   if (stat(filename,&sbuf) != -1)
	   {
		   savestates[num].exists = true;
		   strncpy(savestates[num].date, format_time(sbuf.st_mtime),40);
		   savestates[num].date[40-1] = '\0';
	   }
   }
}*/

/*void loadstate_slot(int num)
{
   char filename[MAX_PATH];

   lastSaveState = num;		//Set last savestate used

    path.getpathnoext(path.STATES, filename);

   if (strlen(filename) + strlen(".dsx") + strlen("-2147483648")*/ /* = biggest string for num *//* >MAX_PATH) return ;
   sprintf(filename+strlen(filename), ".ds%d", num);*/
   /*if (savestate_load(filename))
   {
	   osd->setLineColor(255, 255, 255);
	   osd->addLine("Loaded from %i slot", num);
   }
   else
   {
	   osd->setLineColor(255, 0, 0);
	   osd->addLine("Error loading %i slot", num);
   }*/
//}


// note: guessSF is so we don't have to do a linear search through the SFORMAT array every time
// in the (most common) case that we already know where the next entry is.
static const SFORMAT *CheckS(const SFORMAT *guessSF, const SFORMAT *firstSF, uint32_t size, uint32_t count, char *desc)
{
	const SFORMAT *sf = guessSF ? guessSF : firstSF;
	while(sf->v)
	{
		//NOT SUPPORTED RIGHT NOW
		//if(sf->size==~0)		// Link to another SFORMAT structure.
		//{
		//	SFORMAT *tmp;
		//	if((tmp= CheckS((SFORMAT *)sf->v, tsize, desc) ))
		//		return tmp;
		//	sf++;
		//	continue;
		//}
		if(!memcmp(desc,sf->desc,4))
		{
			if(sf->size != size || sf->count != count)
				return 0;
			return sf;
		}

		// failed to find it, have to keep iterating
		if(guessSF)
		{
			sf = firstSF;
			guessSF = NULL;
		}
		else
		{
			sf++;
		}
	}
	return 0;
}


static bool ReadStateChunk(EMUFILE* is, const SFORMAT *sf, int size)
{
	const SFORMAT *tmp = NULL;
	const SFORMAT *guessSF = NULL;
	size_t temp = is->ftell();

	while(is->ftell()<temp+size)
	{
		uint32_t sz, count;

		char toa[4];
		is->fread(toa,4);
		if(is->fail())
			return false;

		if(!read32le(&sz,is)) return false;
		if(!read32le(&count,is)) return false;

		if((tmp=CheckS(guessSF,sf,sz,count,toa)))
		{
		#ifdef LOCAL_LE
			// no need to ever loop one at a time if not flipping byte order
			is->fread((char *)tmp->v,sz*count);
		#else
			if(sz == 1) {
				//special case: read a huge byte array
				is->fread((char *)tmp->v,count);
			} else {
				for(unsigned int i=0;i<count;i++)
				{
					is->fread((char *)tmp->v + i*sz,sz);
                    FlipByteOrder((uint8_t*)tmp->v + i*sz,sz);
				}
			}
		#endif
			guessSF = tmp + 1;
		}
		else
		{
			is->fseek(sz*count,SEEK_CUR);
			guessSF = NULL;
		}
	} // while(...)
	return true;
}



/*static int SubWrite(EMUFILE* os, const SFORMAT *sf)
{
	uint32_t acc=0;

#ifdef DEBUG
	std::set<std::string> keyset;
#endif

	const SFORMAT* temp = sf;
	while(temp->v) {
		const SFORMAT* seek = sf;
		while(seek->v && seek != temp) {
			if(!strcmp(seek->desc,temp->desc)) {
				printf("ERROR! duplicated chunk name: %s\n", temp->desc);
			}
			seek++;
		}
		temp++;
	}

	while(sf->v)
	{
		//not supported right now
		//if(sf->size==~0)		//Link to another struct
		//{
		//	uint32_t tmp;

		//	if(!(tmp=SubWrite(os,(SFORMAT *)sf->v)))
		//		return 0;
		//	acc+=tmp;
		//	sf++;
		//	continue;
		//}

		int count = sf->count;
		int size = sf->size;

        //add size of current node to the accumulator
		acc += 4 + sizeof(sf->size) + sizeof(sf->count);
		acc += count * size;

		if(os)			//Are we writing or calculating the size of this block?
		{
			os->fwrite(sf->desc,4);
			write32le(sf->size,os);
			write32le(sf->count,os);

			#ifdef DEBUG
			//make sure we dont dup any keys
			if(keyset.find(sf->desc) != keyset.end())
			{
				printf("duplicate save key!\n");
				assert(false);
			}
			keyset.insert(sf->desc);
			#endif


		#ifdef LOCAL_LE
			// no need to ever loop one at a time if not flipping byte order
			os->fwrite((char *)sf->v,size*count);
		#else
			if(size == 1) {
				//special case: write a huge byte array
				os->fwrite((char *)sf->v,count);
			} else {
				for(int i=0;i<count;i++) {
					FlipByteOrder((uint8_t*)sf->v + i*size, size);
					os->fwrite((char*)sf->v + i*size,size);
					//Now restore the original byte order.
					FlipByteOrder((uint8_t*)sf->v + i*size, size);
				}
			}
		#endif
		}
		sf++;
	}

	return acc;
}*/

/*static int savestate_WriteChunk(EMUFILE* os, int type, const SFORMAT *sf)
{
	write32le(type,os);
	if(!sf) return 4;
	int bsize = SubWrite((EMUFILE*)0,sf);
	write32le(bsize,os);

	if(!SubWrite(os,sf))
	{
		return 8;
	}
	return bsize+8;
}*/

/*static void savestate_WriteChunk(EMUFILE* os, int type, void (*saveproc)(EMUFILE* os))
{
	uint32_t pos1 = os->ftell();

	//write the type, size(placeholder), and data
	write32le(type,os);
	os->fseek(4, SEEK_CUR); // skip the size, we write that later
	saveproc(os);

	//get the size
	uint32_t pos2 = os->ftell();
	assert(pos2 != (uint32_t)-1); // if this assert fails, saveproc did something bad
	uint32_t size = (pos2 - pos1) - (2 * sizeof(uint32_t));

	//fill in the actual size
	os->fseek(pos1 + sizeof(uint32_t),SEEK_SET);
	write32le(size,os);
	os->fseek(pos2,SEEK_SET);*/

/*
// old version of this function,
// for reference in case the new one above starts misbehaving somehow:

	// - this is retarded. why not write placeholders for size and then write directly to the stream
	//and then go back and fill them in

	//get the size
	memorystream mstemp;
	saveproc(&mstemp);
	mstemp.flush();
	uint32_t size = mstemp.size();

	//write the type, size, and data
	write32le(type,os);
	write32le(size,os);
	os->write(mstemp.buf(),size);
*/
//}

//static void writechunks(EMUFILE* os);

/*bool savestate_save(EMUFILE* outstream, int compressionLevel)
{
	#ifndef HAVE_LIBZ
	compressionLevel = Z_NO_COMPRESSION;
	#endif

	EMUFILE_MEMORY ms;
	EMUFILE* os;

	if(compressionLevel != Z_NO_COMPRESSION)
	{
		//generate the savestate in memory first
		os = (EMUFILE*)&ms;
		writechunks(os);
	}
	else
	{
		os = outstream;
		os->fseek(32,SEEK_SET); //skip the header
		writechunks(os);
	}

	//save the length of the file
	uint32_t len = os->ftell();

	uint32_t comprlen = 0xFFFFFFFF;
	uint8_t* cbuf;

	//compress the data
	int error = Z_OK;
	if(compressionLevel != Z_NO_COMPRESSION)
	{
		cbuf = ms.buf();
		uLongf comprlen2;
		//worst case compression.
		//zlib says "0.1% larger than sourceLen plus 12 bytes"
		comprlen = (len>>9)+12 + len;
		cbuf = new uint8_t[comprlen];
		// Workaround to make it compile under linux 64bit
		comprlen2 = comprlen;
		//error = compress2(cbuf,&comprlen2,ms.buf(),len,compressionLevel);
		comprlen = (uint32_t)comprlen2;
	}

	//dump the header
	outstream->fseek(0,SEEK_SET);
	outstream->fwrite(magic,16);
	write32le(SAVESTATE_VERSION,outstream);
	write32le(EMU_DESMUME_VERSION_NUMERIC(),outstream); //desmume version
	write32le(len,outstream); //uncompressed length
	write32le(comprlen,outstream); //compressed length (-1 if it is not compressed)

	if(compressionLevel != Z_NO_COMPRESSION)
	{
		outstream->fwrite((char*)cbuf,comprlen==(uint32_t)-1?len:comprlen);
		delete[] cbuf;
	}

	return error == Z_OK;
}*/

/*bool savestate_save (const char *file_name)
{
	EMUFILE_MEMORY ms;
	size_t elems_written;
#ifdef HAVE_LIBZ
	if(!savestate_save(&ms, Z_DEFAULT_COMPRESSION))
#else
	if(!savestate_save(&ms, 0))
#endif
		return false;
	FILE* file = fopen(file_name,"wb");
	if(file)
	{
		elems_written = fwrite(ms.buf(),1,ms.size(),file);
		fclose(file);
		return elems_written == ms.size();
	} else return false;
}*/

//extern SFORMAT SF_RTC[];

/*static void writechunks(EMUFILE* os) {
	savestate_WriteChunk(os,1,SF_ARM9);
	savestate_WriteChunk(os,2,SF_ARM7);
	savestate_WriteChunk(os,3,cp15_savestate);
	savestate_WriteChunk(os,4,SF_MEM);
	savestate_WriteChunk(os,5,SF_NDS);
	savestate_WriteChunk(os,51,nds_savestate);
	savestate_WriteChunk(os,60,SF_MMU);
	savestate_WriteChunk(os,61,mmu_savestate);
	//savestate_WriteChunk(os,7,gpu_savestate);
	savestate_WriteChunk(os,8,spu_savestate);
	//savestate_WriteChunk(os,81,mic_savestate);
	//savestate_WriteChunk(os,90,SF_GFX3D);
	//savestate_WriteChunk(os,91,gfx3d_savestate);
	//savestate_WriteChunk(os,100,SF_MOVIE);
	//savestate_WriteChunk(os,101,mov_savestate);
	//savestate_WriteChunk(os,110,SF_WIFI);
	//savestate_WriteChunk(os,120,SF_RTC);
	savestate_WriteChunk(os,0xFFFFFFFF,(SFORMAT*)0);
}*/

/*static bool fake_gpu_loadstate(EMUFILE* is, int size)
{
	//read version
	uint32_t version;

	//sigh.. shouldve used a new version number
	if(size == 256*192*2*2)
		version = 0;
	else if(size== 0x30024)
	{
		read32le(&version,is);
		version = 1;
	}
	else
		if(read32le(&version,is) != 1) return false;


	if(version<0||version>1) return false;

	uint8_t tmpu8_array[4*256*192];
	is->fread((char*)tmpu8_array,sizeof(tmpu8_array));

	if(version==1)
	{
		uint32_t tmpu32;
		for (int i = 0; i < 8; ++i)
			read32le(&tmpu32, is);*/
		/*read32le(&MainScreen.gpu->affineInfo[0].x,is);
		read32le(&MainScreen.gpu->affineInfo[0].y,is);
		read32le(&MainScreen.gpu->affineInfo[1].x,is);
		read32le(&MainScreen.gpu->affineInfo[1].y,is);
		read32le(&SubScreen.gpu->affineInfo[0].x,is);
		read32le(&SubScreen.gpu->affineInfo[0].y,is);
		read32le(&SubScreen.gpu->affineInfo[1].x,is);
		read32le(&SubScreen.gpu->affineInfo[1].y,is);*/
		//removed per nitsuja feedback. anyway, this same thing will happen almost immediately in gpu line=0
		//MainScreen.gpu->refreshAffineStartRegs(-1,-1);
		//SubScreen.gpu->refreshAffineStartRegs(-1,-1);
	/*}

	//MainScreen.gpu->updateBLDALPHA();
	//SubScreen.gpu->updateBLDALPHA();
	return !is->fail();
}*/

/*static bool fake_mic_loadstate(EMUFILE* is, int size)
{
	uint32_t version;
	if(read32le(&version,is) != 1) return false;
	if(version > 1 || version == 0) { is->fseek(size-4, SEEK_CUR); return true; }

	uint8_t tmpu8_Array[2][4096];
	is->fread((char*)tmpu8_Array[0], 4096);
	is->fread((char*)tmpu8_Array[1], 4096);
	uint16_t tmpu16;
	read16le(&tmpu16,is);
	uint8_t tmpu8;
	read8le(&tmpu8,is);
	read8le(&tmpu8,is);
	uint32_t tmpu32;
	read32le(&tmpu32,is);
	return true;
}*/

/*static uint32_t tmpu32_array16[16];
static uint32_t tmpu32_array64[64];
static uint32_t tmpu32_array4[4];
static uint16_t tmpu16_array6[6];
static uint8_t tmpu8_array4[4];
static uint16_t tmpu16_array32[32];
static uint8_t tmpu8_array128[128];
static uint8_t tmpu8_arrayAlot[4*256*192];
SFORMAT SF_GFX3D[]={
	{ "GCTL", 4, 1, &tmpu32}, // no longer regenerated indirectly, see comment in loadstate()
	{ "GPAT", 4, 1, &tmpu32},
	{ "GPAP", 4, 1, &tmpu32},
	{ "GINB", 4, 1, &tmpu32},
	{ "GTFM", 4, 1, &tmpu32},
	{ "GTPA", 4, 1, &tmpu32},
	{ "GMOD", 4, 1, &tmpu32},
	{ "GMTM", 4,16, tmpu32_array16},
	{ "GMCU", 4,64, tmpu32_array64},
	{ "ML4I", 1, 1, &tmpu8},
	{ "ML3I", 1, 1, &tmpu8},
	{ "MM4I", 1, 1, &tmpu8},
	{ "MM3I", 1, 1, &tmpu8},
	{ "MMxI", 1, 1, &tmpu8},
	{ "GSCO", 4, 1, tmpu32_array4},
	{ "GCOI", 1, 1, &tmpu8},
	{ "GVFM", 4, 1, &tmpu32},
	{ "GTRN", 4, 4, tmpu32_array4},
	{ "GTRI", 1, 1, &tmpu8},
	{ "GSCA", 4, 4, tmpu32_array4},
	{ "GSCI", 1, 1, &tmpu8},
	{ "G_T_", 4, 1, &tmpu32},
	{ "G_S_", 4, 1, &tmpu32},
	{ "GL_T", 4, 1, &tmpu32},
	{ "GL_S", 4, 1, &tmpu32},
	{ "GLCM", 4, 1, &tmpu32},
	{ "GLIN", 4, 1, &tmpu32},
	{ "GLI2", 4, 1, &tmpu32},
	{ "GLSB", 4, 1, &tmpu32},
	{ "GLBT", 4, 1, &tmpu32},
	{ "GLPT", 4, 1, &tmpu32},
	{ "GLPC", 4, 4, tmpu32_array4},
	{ "GBTC", 2, 6, tmpu16_array6},
	{ "GFHE", 4, 1, &gxFIFO.head},
	{ "GFTA", 4, 1, &gxFIFO.tail},
	{ "GFSZ", 4, 1, &gxFIFO.size},
	{ "GFCM", 1, HACK_GXIFO_SIZE, &gxFIFO.cmd[0]},
	{ "GFPM", 4, HACK_GXIFO_SIZE, &gxFIFO.param[0]},
	{ "GPHE", 1, 1, &gxPIPE.head},
	{ "GPTA", 1, 1, &gxPIPE.tail},
	{ "GPSZ", 1, 1, &gxPIPE.size},
	{ "GPCM", 1, 4, &gxPIPE.cmd[0]},
	{ "GPPM", 4, 4, &gxPIPE.param[0]},
	{ "GCOL", 1, 4, &tmpu8_array4},
	{ "GLCO", 4, 4, tmpu32_array4},
	{ "GLDI", 4, 4, tmpu32_array4},
	{ "GMDI", 2, 1, &tmpu16},
	{ "GMAM", 2, 1, &tmpu16},
	{ "GMSP", 2, 1, &tmpu16},
	{ "GMEM", 2, 1, &tmpu16},
	{ "GFLP", 4, 1, &tmpu32},
	{ "GDRP", 4, 1, &tmpu32},
	{ "GSET", 4, 1, &tmpu32},
	{ "GSEA", 4, 1, &tmpu32},
	{ "GSEB", 4, 1, &tmpu32},
	{ "GSEX", 4, 1, &tmpu32},
	{ "GSEE", 4, 1, &tmpu32},
	{ "GSEC", 4, 1, &tmpu32},
	{ "GSEF", 4, 1, &tmpu32},
	{ "GSEO", 4, 1, &tmpu32},
	{ "GFSH", 4, 1, &tmpu32},
	{ "GSSH", 4, 1, &tmpu32},
	{ "GSWB", 4, 1, &tmpu32},
	{ "GSSM", 4, 1, &tmpu32},
	{ "GSAR", 1, 1, &tmpu8},
	{ "GSVP", 4, 1, &tmpu32},
	{ "GSCC", 4, 1, &tmpu32},
	{ "GSCD", 4, 1, &tmpu32},
	{ "GSFC", 4, 4, tmpu32_array4},
	{ "GSFO", 4, 1, &tmpu32},
	{ "GST4", 2, 32, tmpu16_array32},
	{ "GSSU", 1, 128, tmpu8_array128},
	{ "GSSI", 4, 1, &tmpu32},
	{ "GSAF", 4, 1, &tmpu32},
	{ "GSPF", 4, 1, &tmpu32},
	//------------------------
	{ "GTST", 4, 1, &tmpu32},
	{ "GTVC", 4, 1, &tmpu32},
	{ "GTVM", 4, 4, tmpu32_array4},
	{ "GTVF", 4, 1, &tmpu32},
	{ "G3CX", 1, 4*256*192, tmpu8_arrayAlot},
	{ 0 }
};*/

/*static uint8_t tmpu8_array8[8];
static SFORMAT SF_RTC[]={
	{ "R000", 1, 1, &tmpu8},
	{ "R010", 1, 1, &tmpu8},
	{ "R020", 1, 1, &tmpu8},
	{ "R030", 1, 1, &tmpu8},

	{ "R040", 1, 1, &tmpu8},
	{ "R050", 1, 1, &tmpu8},
	{ "R060", 1, 1, &tmpu8},
	{ "R070", 1, 1, &tmpu8},
	{ "R080", 1, 1, &tmpu8},
	{ "R090", 1, 1, &tmpu8},
	{ "R100", 1, 1, &tmpu8},
	{ "R110", 2, 1, &tmpu16},

	{ "R120", 1, 1, &tmpu8},
	{ "R130", 1, 1, &tmpu8},
	{ "R140", 1, 1, &tmpu8},
	{ "R150", 1, 8, tmpu8_array8},

	{ "R160", 1, 8, tmpu8_array8},

	{ NULL, 0, 0, NULL }
};*/

/*static bool fake_gfx_hardware_loadstate(EMUFILE *f)
{
	uint32_t version;
	if(read32le(&version,f) != 1) return false;
	if(version != 0) return false;

	uint32_t tempsize;
	read32le(&tempsize,f);
	uint32_t commandCursor = 4-tempsize;
	//for(uint32_t i=0;i<commandCursor;i++) commandsPending[i].command = 0;
	uint8_t tmpu8;
	for(uint32_t i=commandCursor;i<4;i++) read8le(&commandsPending[i].command&tmpu8,f);
	read32le(&tempsize,f);
	//for(uint32_t i=0;i<commandCursor;i++) commandsPending[i].countdown = 0;
	for(uint32_t i=commandCursor;i<4;i++) read8le(&commandsPending[i].countdown&tmpu8,f);

	read8le(&countdown&tmpu8,f);

	return true;
}*/

/*static bool fake_gfx3d_loadstate(EMUFILE* is, int size)
{
	int version;
	if(read32le(&version,is) != 1) return false;
	if(size==8) version = 0;


	//gfx3d_glPolygonAttrib_cache();
	//gfx3d_glTexImage_cache();
	//gfx3d_glLightDirection_cache(0);
	//gfx3d_glLightDirection_cache(1);
	//gfx3d_glLightDirection_cache(2);
	//gfx3d_glLightDirection_cache(3);

	//jiggle the lists. and also wipe them. this is clearly not the best thing to be doing.
	//listTwiddle = 0;
	//polylist = &polylists[listTwiddle];
	//vertlist = &vertlists[listTwiddle];

#define OSREAD(x) is->fread((char*)&(x),sizeof((x)));
	if(version>=1)
	{
		int tmpint;
		//OSREAD(vertlist->count);
		//OSREAD(tmpint);
		is->fread(&tmpint, sizeof(int));
		//for(int i=0;i<vertlist->count;i++)
		for (int i = 0; i < tmpint; ++i)
		{
			//vertlist->list[i].load(is);
			float tmpfloat;
			uint8_t tmpu8;
			for (int j = 0; j < 6; ++j)
				//OSREAD(tmpfloat);
				is->fread(&tmpfloat, sizeof(float));
			for (int j = 0; j < 3; ++j)
				//OSREAD(tmpu8);
				is->fread(&tmpu8, sizeof(uint8_t));
			for (int j = 0; j < 3; ++j)
				//OSREAD(tmpfloat);
				is->fread(&tmpfloat, sizeof(float));
		}
		//OSREAD(polylist->count);
		//OSREAD(tmpint);
		is->fread(&tmpint, sizeof(int));
		//for(int i=0;i<polylist->count;i++)
		for (int i = 0; i < tmpint; ++i)
		{
			//polylist->list[i].load(is);
			int tmpint2;
			uint16_t tmpu16;
			uint32_t tmpu32;
			float tmpfloat;
			//OSREAD(tmpint2);
			is->fread(&tmpint2, sizeof(int));
			for (int j = 0; j < 4; ++j)
				//OSREAD(tmpu16);
				is->fread(&tmpu16, sizeof(uint16_t));
			for (int j = 0; j < 4; ++j)
				//OSREAD(tmpu32);
				is->fread(&tmpu32, sizeof(uint32_t));
			for (int j = 0; j < 2; ++j)
				//OSREAD(tmpfloat);
				is->fread(&tmpfloat, sizeof(float));
		}
	}

	if(version>=2)
	{
		for(int i=0;i<4;i++)
		{
			uint32_t tmpu32;
			//OSREAD(mtxStack[i].position);
			//OSREAD(tmpu32);
			is->fread(&tmpu32, sizeof(uint32_t));*/
			/*for(int j=0;j<mtxStack[i].size*16;j++)
				OSREAD(mtxStack[i].matrix[j]);*/
			//if (!i || i == 3)
				/*for (int j = 0; j < (!i || i == 3 ? 1 : 31) * 16; ++j)
					//OSREAD(tmpu32)
					is->fread(&tmpu32, sizeof(uint32_t));
			//else
				//for (int j = 0; j < 31 * 16; ++j)
					//OSREAD(tmpu32);
		}
	}

	if(version>=3) {
		fake_gfx_hardware_loadstate(is);
	}*/

	/*gfx3d.polylist = &polylists[listTwiddle^1];
	gfx3d.vertlist = &vertlists[listTwiddle^1];
	gfx3d.polylist->count=0;
	gfx3d.vertlist->count=0;*/

	/*if(version >= 4)
	{
		uint32_t tmpu32_array16[16];
		//OSREAD(cacheLightDirection);
		//OSREAD(tmpu32_array16);
		is->fread(tmpu32_array16, sizeof(tmpu32_array16));
		//OSREAD(cacheHalfVector);
		//OSREAD(tmpu32_array16);
		is->fread(tmpu32_array16, sizeof(tmpu32_array16));
	}

	return true;
}*/

//static void fake_MovieRecord_parsePad(EMUFILE* fp/*, uint16_t& pad*/)
/*{

	char buf[13];
	fp->fread(buf,13);*/
	/*pad = 0;
	for(int i=0;i<13;i++)
	{
		pad <<= 1;
		pad |= ((buf[i]=='.'||buf[i]==' ')?0:1);
	}*/
//}

/*static void fake_MovieRecord_parse(MovieData* md, EMUFILE* fp)
{
	//by the time we get in here, the initial pipe has already been extracted

	//extract the commands
	uint8_t tmpu8 = u32DecFromIstream(fp);

	fp->fgetc(); //eat the pipe

	fake_MovieRecord_parsePad(fp*//*, pad*///);
	/*touch.x*///tmpu8 = u32DecFromIstream(fp);
	/*touch.y*///tmpu8 = u32DecFromIstream(fp);
	/*touch.touch*///tmpu8 = u32DecFromIstream(fp);

	/*fp->fgetc(); //eat the pipe

	//should be left at a newline
}*/

/*static bool fake_LoadFM2(MovieData& movieData, EMUFILE* fp, int size, bool stopAfterHeader)
{
	//TODO - start with something different. like 'desmume movie version 1"
	int curr = fp->ftell();

	//movie must start with "version 1"
	char buf[9];
	curr = fp->ftell();
	fp->fread(buf,9);
	fp->fseek(curr, SEEK_SET);
//	if(fp->fail()) return false;
	if(memcmp(buf,"version 1",9))
		return false;

	std::string key,value;
	enum {
		NEWLINE, KEY, SEPARATOR, VALUE, RECORD, COMMENT
	} state = NEWLINE;
	bool bail = false;
	for(;;)
	{
		bool iswhitespace, isrecchar, isnewline;
		int c;
		if(size--<=0) goto bail;
		c = fp->fgetc();
		if(c == -1)
			goto bail;
		iswhitespace = (c==' '||c=='\t');
		isrecchar = (c=='|');
		isnewline = (c==10||c==13);
		if(isrecchar && movieData.binaryFlag && !stopAfterHeader)
		{
			LoadFM2_binarychunk(movieData, fp, size);
			return true;
		}
		switch(state)
		{
		case NEWLINE:
			if(isnewline) goto done;
			if(iswhitespace) goto done;
			if(isrecchar)
				goto dorecord;
			//must be a key
			key = "";
			value = "";
			goto dokey;
			break;
		case RECORD:
			{
				dorecord:
				if (stopAfterHeader) return true;
				//int currcount = movieData.records.size();
				//movieData.records.resize(currcount+1);
				int preparse = fp->ftell();
				//movieData.records[currcount].parse(&movieData, fp);
				fake_MovieRecord_parse(fp);
				int postparse = fp->ftell();
				size -= (postparse-preparse);
				state = NEWLINE;
				break;
			}

		case KEY:
			dokey: //dookie
			state = KEY;
			if(iswhitespace) goto doseparator;
			if(isnewline) goto commit;
			key += c;
			break;
		case SEPARATOR:
			doseparator:
			state = SEPARATOR;
			if(isnewline) goto commit;
			if(!iswhitespace) goto dovalue;
			break;
		case VALUE:
			dovalue:
			state = VALUE;
			if(isnewline) goto commit;
			value += c;
			break;
		case COMMENT:
		default:
			break;
		}
		goto done;

		bail:
		bail = true;
		if(state == VALUE) goto commit;
		goto done;
		commit:
		//movieData.installValue(key,value);
		state = NEWLINE;
		done: ;
		if(bail) break;
	}

	return true;
}*/

/*static bool fake_mov_loadstate(EMUFILE* fp, int size)
{
	//load_successful = false;
	static const int kMOVI = 0x49564F4D;
	static const int kNOMO = 0x4F4D4F4E;

	uint32_t cookie;
	if(read32le(&cookie,fp) != 1) return false;
	if(cookie == kNOMO)
	{*/
		/*if(movieMode == MOVIEMODE_RECORD || movieMode == MOVIEMODE_PLAY)
			FinishPlayback();*/
		/*return true;
	}
	else if(cookie != kMOVI)
		return false;

	size -= 4;*/

	/*if (!movie_readonly && autoMovieBackup && freshMovie) //If auto-backup is on, movie has not been altered this session and the movie is in read+write mode
	{
		FCEUI_MakeBackupMovie(false);	//Backup the movie before the contents get altered, but do not display messages
	}*/

	//a little rule: cant load states in read+write mode with a movie from an archive.
	//so we are going to switch it to readonly mode in that case
//	if(!movie_readonly
//		//*&& FCEU_isFileInArchive(curMovieFilename)*/
//		) {
//		FCEU_PrintError("Cannot loadstate in Read+Write with movie from archive. Movie is now Read-Only.");
//		movie_readonly = true;
//	}

	//MovieData tempMovieData = MovieData();
	//int curr = fp->ftell();
	//if(!LoadFM2(tempMovieData, fp, size, false)) {
	/*if (!fake_LoadFM2(fp, size, false)) {*/

	//	is->seekg((uint32_t)curr+size);
	/*	extern bool FCEU_state_loading_old_format;
		if(FCEU_state_loading_old_format) {
			if(movieMode == MOVIEMODE_PLAY || movieMode == MOVIEMODE_RECORD) {
				FCEUI_StopMovie();
				FCEU_PrintError("You have tried to use an old savestate while playing a movie. This is unsupported (since the old savestate has old-format movie data in it which can't be converted on the fly)");
			}
		}*/
		/*return false;
	}*/

	//complex TAS logic for when a savestate is loaded:
	//----------------
	//if we are playing or recording and toggled read-only:
	//  then, the movie we are playing must match the guid of the one stored in the savestate or else error.
	//  the savestate is assumed to be in the same timeline as the current movie.
	//  if the current movie is not long enough to get to the savestate's frame#, then it is an error.
	//  the movie contained in the savestate will be discarded.
	//  the emulator will be put into play mode.
	//if we are playing or recording and toggled read+write
	//  then, the movie we are playing must match the guid of the one stored in the savestate or else error.
	//  the movie contained in the savestate will be loaded into memory
	//  the frames in the movie after the savestate frame will be discarded
	//  the in-memory movie will have its rerecord count incremented
	//  the in-memory movie will be dumped to disk as fcm.
	//  the emulator will be put into record mode.
	//if we are doing neither:
	//  then, we must discard this movie and just load the savestate


	/*if(movieMode != MOVIEMODE_INACTIVE)
	{
		//handle moviefile mismatch
		if(tempMovieData.guid != currMovieData.guid)
		{
			//mbg 8/18/08 - this code  can be used to turn the error message into an OK/CANCEL
			#ifdef WIN32
				//std::string msg = "There is a mismatch between savestate's movie and current movie.\ncurrent: " + currMovieData.guid.toString() + "\nsavestate: " + tempMovieData.guid.toString() + "\n\nThis means that you have loaded a savestate belonging to a different movie than the one you are playing now.\n\nContinue loading this savestate anyway?";
				//extern HWND pwindow;
				//int result = MessageBox(pwindow,msg.c_str(),"Error loading savestate",MB_OKCANCEL);
				//if(result == IDCANCEL)
				//	return false;
			#else
				FCEU_PrintError("Mismatch between savestate's movie and current movie.\ncurrent: %s\nsavestate: %s\n",currMovieData.guid.toString().c_str(),tempMovieData.guid.toString().c_str());
				return false;
			#endif
		}

		closeRecordingMovie();

		if(!movie_readonly)
		{
			currMovieData = tempMovieData;
			currMovieData.rerecordCount = currRerecordCount;
		}

		if(currFrameCounter > (int)currMovieData.records.size())
		{
			// if the frame counter is longer than our current movie,
			// switch to "finished" mode.
			// this is a mode that behaves like "inactive"
			// except it permits switching to play/record by loading an earlier savestate.
			// (and we continue to store the finished movie in savestates made while finished)
			osd->setLineColor(255,0,0); // let's make the text red too to hopefully catch the user's attention a bit.
			FinishPlayback();
			osd->setLineColor(255,255,255);

			//FCEU_PrintError("Savestate is from a frame (%d) after the final frame in the movie (%d). This is not permitted.", currFrameCounter, currMovieData.records.size()-1);
			//return false;
		}
		else if(movie_readonly)
		{
			//-------------------------------------------------------------
			//this code would reload the movie from disk. allegedly it is helpful to hexers, but
			//it is way too slow with dsm format. so it is only here as a reminder, and in case someone
			//wants to play with it
			//-------------------------------------------------------------
			//{
			//	fstream fs (curMovieFilename);
			//	if(!LoadFM2(tempMovieData, &fs, INT_MAX, false))
			//	{
			//		FCEU_PrintError("Failed to reload DSM after loading savestate");
			//	}
			//	fs.close();
			//	currMovieData = tempMovieData;
			//}
			//-------------------------------------------------------------

			movieMode = MOVIEMODE_PLAY;
		}
		else
		{
		//	#ifdef _S9XLUA_H
		//	if(!FCEU_LuaRerecordCountSkip())
				currRerecordCount++;
		//	#endif

			currMovieData.rerecordCount = currRerecordCount;
			currMovieData.truncateAt(currFrameCounter);

			openRecordingMovie(curMovieFilename);
			if(!osRecordingMovie)
			{
			   osd->setLineColor(255, 0, 0);
			   osd->addLine("Can't save movie file!");
			}

			//printf("DUMPING MOVIE: %d FRAMES\n",currMovieData.records.size());
			currMovieData.dump(osRecordingMovie, false);
			movieMode = MOVIEMODE_RECORD;
		}
	}

	load_successful = true;
	freshMovie = false;*/

	//// Maximus: Show the last input combination entered from the
	//// movie within the state
	//if(current!=0) // <- mz: only if playing or recording a movie
	//	memcpy(&cur_input_display, joop, 4);

	/*return true;
}*/

static bool ReadStateChunks(EMUFILE* is, int32_t totalsize)
{
	bool ret = true;
	while(totalsize > 0)
	{
		uint32_t size;
		uint32_t t;
		if(!read32le(&t,is))  { ret=false; break; }
		if(t == 0xFFFFFFFF) goto done;
		if(!read32le(&size,is))  { ret=false; break; }
		switch(t)
		{
			case 1: if(!ReadStateChunk(is,SF_ARM9,size)) ret=false; break;
			case 2: if(!ReadStateChunk(is,SF_ARM7,size)) ret=false; break;
			case 3: if(!cp15_loadstate(is,size)) ret=false; break;
			case 4: if(!ReadStateChunk(is,SF_MEM,size)) ret=false; break;
			case 5: if(!ReadStateChunk(is,SF_NDS,size)) ret=false; break;
			case 51: if(!nds_loadstate(is,size)) ret=false; break;
			case 60: if(!ReadStateChunk(is,SF_MMU,size)) ret=false; break;
			case 61: if(!mmu_loadstate(is,size)) ret=false; break;
			case 7: /*if(!fake_gpu_loadstate(is,size)) ret=false;*/ break;
			case 8: if(!spu_loadstate(is,size)) ret=false; break;
			case 81:/* if(!fake_mic_loadstate(is,size)) ret=false; break;*/
			case 90:/* if(!ReadStateChunk(is,SF_GFX3D,size)) ret=false; break;*/
			case 91:/* if(!fake_gfx3d_loadstate(is,size)) ret=false; break;*/
			case 100:/* if(!ReadStateChunk(is,SF_MOVIE, size)) ret=false; break;*/
			case 101:/* if(!fake_mov_loadstate(is, size)) ret=false; break;*/
			case 110:/* if(!ReadStateChunk(is,SF_WIFI,size)) ret=false; break;*/
			case 120:/* if(!ReadStateChunk(is,SF_RTC,size)) ret=false;*/ break;
			default:
				ret=false;
				break;
		}
		if(!ret)
			return false;
	}
done:

	return ret;
}

void loadstate()
{
    // This should regenerate the vram banks
    for (int i = 0; i < 0xA; i++)
       _MMU_write08<ARMCPU_ARM9>(0x04000240+i, _MMU_read08<ARMCPU_ARM9>(0x04000240+i));

    // This should regenerate the graphics power control register
    _MMU_write16<ARMCPU_ARM9>(0x04000304, _MMU_read16<ARMCPU_ARM9>(0x04000304));

	// This should regenerate the graphics configuration
	//zero 27-jul-09 : was formerly up to 7F but that wrote to dispfifo which is dumb (one of nitsuja's desynch bugs [that he found, not caused])
	//so then i brought it down to 66 but this resulted in a conceptual bug with affine start registers, which shouldnt get regenerated
	//so then i just made this exhaustive list
 //   for (int i = REG_BASE_DISPA; i<=REG_BASE_DISPA + 0x66; i+=2)
	//_MMU_write16<ARMCPU_ARM9>(i, _MMU_read16<ARMCPU_ARM9>(i));
 //   for (int i = REG_BASE_DISPB; i<=REG_BASE_DISPB + 0x7F; i+=2)
	//_MMU_write16<ARMCPU_ARM9>(i, _MMU_read16<ARMCPU_ARM9>(i));
	static const uint8_t mainRegenAddr[] = {0x00,0x02,0x08,0x0a,0x0c,0x0e,0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x50,0x52,0x54,0x64,0x66,0x6c};
	static const uint8_t subRegenAddr[] =  {0x00,0x02,0x08,0x0a,0x0c,0x0e,0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x50,0x52,0x54,0x6c};
	for(uint32_t i=0;i<ARRAY_SIZE(mainRegenAddr);i++)
		_MMU_write16<ARMCPU_ARM9>(REG_BASE_DISPA+mainRegenAddr[i], _MMU_read16<ARMCPU_ARM9>(REG_BASE_DISPA+mainRegenAddr[i]));
	for(uint32_t i=0;i<ARRAY_SIZE(subRegenAddr);i++)
		_MMU_write16<ARMCPU_ARM9>(REG_BASE_DISPB+subRegenAddr[i], _MMU_read16<ARMCPU_ARM9>(REG_BASE_DISPB+subRegenAddr[i]));
	// no need to restore 0x60 since control and MMU.ARM9_REG are both in the savestates, and restoring it could mess up the ack bits anyway

	SetupMMU(/*nds.Is_DebugConsole()*/false,nds.Is_DSI());

	//execute = !driver->EMU_IsEmulationPaused();
	execute = true;
}

bool savestate_load(EMUFILE* is)
{
	SAV_silent_fail_flag = false;
	char header[16];
	is->fread(header,16);
	if(is->fail() || memcmp(header,magic,16))
		return false;

	uint32_t ssversion,dversion,len,comprlen;
	if(!read32le(&ssversion,is)) return false;
	if(!read32le(&dversion,is)) return false;
	if(!read32le(&len,is)) return false;
	if(!read32le(&comprlen,is)) return false;

	if(ssversion != SAVESTATE_VERSION) return false;

	std::vector<uint8_t> buf(len);

	if(comprlen != 0xFFFFFFFF) {
#ifndef HAVE_LIBZ
		//without libz, we can't decompress this savestate
		return false;
#endif
		std::vector<char> cbuf(comprlen);
		is->fread(&cbuf[0],comprlen);
		if(is->fail()) return false;

#ifdef HAVE_LIBZ
		uLongf uncomprlen = len;
		int error = uncompress((uint8_t*)&buf[0],&uncomprlen,(uint8_t*)&cbuf[0],comprlen);
		if(error != Z_OK || uncomprlen != len)
			return false;
#endif
	} else {
		is->fread((char*)&buf[0],len-32);
	}

	//GO!! READ THE SAVESTATE
	//THERE IS NO GOING BACK NOW
	//reset the emulator first to clean out the host's state

	//while the series of resets below should work,
	//we are testing the robustness of the savestate system with this full reset.
	//the full reset wipes more things, so we can make sure that they are being restored correctly
	//extern bool _HACK_DONT_STOPMOVIE;
	//_HACK_DONT_STOPMOVIE = true;
	NDS_Reset();
	//_HACK_DONT_STOPMOVIE = false;

	//reset some options to their old defaults which werent saved
	//nds.debugConsole = false;

	//GPU_Reset(MainScreen.gpu, 0);
	//GPU_Reset(SubScreen.gpu, 1);
	//gfx3d_reset();
	//gpu3D->NDS_3D_Reset();
	//SPU_Reset();

	EMUFILE_MEMORY mstemp(&buf);
	bool x = ReadStateChunks(&mstemp,(int32_t)len);

	if(!x && !SAV_silent_fail_flag)
	{
		printf("Error loading savestate. It failed halfway through;\nSince there is no savestate backup system, your current game session is wrecked");
#ifdef _WINDOWS
		//HACK! we really need a better way to handle this kind of feedback
		MessageBoxA(0,"Error loading savestate. It failed halfway through;\nSince there is no savestate backup system, your current game session is wrecked",0,0);
#endif
		return false;
	}

	loadstate();

	/*if((nds.debugConsole!=0) != CommonSettings.DebugConsole) {
		printf("WARNING: forcing console debug mode to: debugmode=%s\n",nds.debugConsole?"true":"false");
	}*/

	return true;
}

/*bool savestate_load(const char *file_name)
{
	EMUFILE_FILE f(file_name,"rb");
	if(f.fail()) return false;

	return savestate_load(&f);
}*/

//static std::stack<EMUFILE_MEMORY*> rewindFreeList;
//static std::vector<EMUFILE_MEMORY*> rewindbuffer;

//int rewindstates = 16;
//int rewindinterval = 4;

/*void rewindsave () {*/

	/*if(currFrameCounter % rewindinterval)
		return;*/

	//printf("rewindsave"); printf("%d%s", currFrameCounter, "\n");


	/*EMUFILE_MEMORY *ms;
	if(!rewindFreeList.empty()) {
		ms = rewindFreeList.top();
		rewindFreeList.pop();
	} else {
		ms = new EMUFILE_MEMORY(1024*1024*12);
	}

	if(!savestate_save(ms, Z_NO_COMPRESSION))
		return;

	rewindbuffer.push_back(ms);

	if((int)rewindbuffer.size() > rewindstates) {
		delete *rewindbuffer.begin();
		rewindbuffer.erase(rewindbuffer.begin());
	}
}*/

/*void dorewind()
{*/

	/*if(currFrameCounter % rewindinterval)
		return;*/

	//printf("rewind\n");

	/*nds.debugConsole = false;

	int size = rewindbuffer.size();

	if(size < 1) {
		printf("rewind buffer empty\n");
		return;
	}

	printf("%d", size);

	EMUFILE_MEMORY* loadms = rewindbuffer[size-1];
	loadms->fseek(32, SEEK_SET);

	ReadStateChunks(loadms,loadms->size()-32);
	loadstate();

	if(rewindbuffer.size()>1)
	{
		rewindFreeList.push(loadms);
		rewindbuffer.pop_back();
	}

}*/
