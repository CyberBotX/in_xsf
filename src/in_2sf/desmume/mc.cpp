/*
	Copyright (C) 2006 thoduv
	Copyright (C) 2006-2007 Theo Berkau
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

#include <cstdlib>
#include <cstring>

//#include "debug.h"
#include "types.h"
#include "mc.h"
//#include "movie.h"
#include "readwrite.h"
#include "NDSSystem.h"

#define FW_CMD_READ             0x03
#define FW_CMD_WRITEDISABLE     0x04
#define FW_CMD_READSTATUS       0x05
#define FW_CMD_WRITEENABLE      0x06
#define FW_CMD_PAGEWRITE        0x0A
#define FW_CMD_READ_ID			0x9F

#define BM_CMD_AUTODETECT       0xFF
#define BM_CMD_WRITESTATUS      0x01
#define BM_CMD_WRITELOW         0x02
#define BM_CMD_READLOW          0x03
#define BM_CMD_WRITEDISABLE     0x04
#define BM_CMD_READSTATUS       0x05
#define BM_CMD_WRITEENABLE      0x06
#define BM_CMD_WRITEHIGH        0x0A
#define BM_CMD_READHIGH         0x0B

/* FLASH*/
#define COMM_PAGE_WRITE		0x0A
#define COMM_PAGE_ERASE		0xDB
#define COMM_SECTOR_ERASE	0xD8
#define COMM_CHIP_ERASE		0xC7
#define CARDFLASH_READ_BYTES_FAST	0x0B    /* Not used*/
#define CARDFLASH_DEEP_POWDOWN		0xB9    /* Not used*/
#define CARDFLASH_WAKEUP			0xAB    /* Not used*/

//since r2203 this was 0x00.
//but baby pals proves finally that it should be 0xFF:
//the game reads its initial sound volumes from uninitialized data, and if it is 0, the game will be silent
//if it is 0xFF then the game starts with its sound and music at max, as presumably it is supposed to.
//so in r3303 I finally changed it (no$ appears definitely to initialized to 0xFF)
static const uint8_t kUninitializedSaveDataValue = 0xFF;

static const char* kDesmumeSaveCookie = "|-DESMUME SAVE-|";

static const uint32_t saveSizes[] = {512,			// 4k
								8*1024,			// 64k
								32*1024,		// 512k
								64*1024,		// 1Mbit
								256*1024,		// 2Mbit
								512*1024,		// 4Mbit
								1024*1024,		// 8Mbit
								2048*1024,		// 16Mbit
								4096*1024,		// 32Mbit
								8192*1024,		// 64Mbit
								16384*1024,		// 128Mbit
								32768*1024,		// 256Mbit
								65536*1024,		// 512Mbit
								0xFFFFFFFF};
static const uint32_t saveSizes_count = ARRAY_SIZE(saveSizes);

//the lookup table from user save types to save parameters
const int save_types[][2] = {
        {MC_TYPE_AUTODETECT,1},
        {MC_TYPE_EEPROM1,MC_SIZE_4KBITS},
        {MC_TYPE_EEPROM2,MC_SIZE_64KBITS},
        {MC_TYPE_EEPROM2,MC_SIZE_512KBITS},
        {MC_TYPE_FRAM,MC_SIZE_256KBITS},
        {MC_TYPE_FLASH,MC_SIZE_2MBITS},
		{MC_TYPE_FLASH,MC_SIZE_4MBITS},
		{MC_TYPE_FLASH,MC_SIZE_8MBITS},
		{MC_TYPE_FLASH,MC_SIZE_16MBITS},
		{MC_TYPE_FLASH,MC_SIZE_32MBITS},
		{MC_TYPE_FLASH,MC_SIZE_64MBITS},
		{MC_TYPE_FLASH,MC_SIZE_128MBITS},
		{MC_TYPE_FLASH,MC_SIZE_256MBITS},
		{MC_TYPE_FLASH,MC_SIZE_512MBITS}
};

/*const char *save_names[] = {
		"EEPROM 4kbit",
		"EEPROM 64kbit",
		"EEPROM 512kbit",
		"FRAM 256kbit",
		"FLASH 2Mbit",
		"FLASH 4Mbit",
		"FLASH 8Mbit",
		"FLASH 16Mbit",
		"FLASH 32Mbit",
		"FLASH 64Mbit",
		"FLASH 128Mbit",
		"FLASH 256Mbit",
		"FLASH 512Mbit"
};*/

//forces the currently selected backup type to be current
//(can possibly be used to repair poorly chosen save types discovered late in gameplay i.e. pokemon gamers)
/*void backup_forceManualBackupType()
{
	MMU_new.backupDevice.forceManualBackupType();
}*/

/*void backup_setManualBackupType(int type)
{
	CommonSettings.manualBackupType = type;
}*/

void mc_init(memory_chip_t *mc, int type)
{
        mc->com = 0;
        mc->addr = 0;
        mc->addr_shift = 0;
        mc->data.clear();
        mc->size = 0;
        mc->write_enable = false;
        mc->writeable_buffer = false;
        mc->type = type;
        mc->autodetectsize = 0;

        switch(mc->type)
        {
           case MC_TYPE_EEPROM1:
              mc->addr_size = 1;
              break;
           case MC_TYPE_EEPROM2:
           case MC_TYPE_FRAM:
              mc->addr_size = 2;
              break;
           case MC_TYPE_FLASH:
              mc->addr_size = 3;
              break;
           default: break;
        }
}

uint8_t *mc_alloc(memory_chip_t *mc, uint32_t size)
{
	/*uint8_t *buffer;
	buffer = new uint8_t[size];
	memset(buffer,0,size);*/

	//if (mc->data) delete [] mc->data;
	mc->data.clear();
	//mc->data = buffer;
	//if(!buffer) { return NULL; }
	mc->data.resize(size, 0);
	mc->size = size;
	mc->writeable_buffer = true;

	//return buffer;
	return NULL;
}

void mc_free(memory_chip_t *mc)
{
    //if(mc->data) delete[] mc->data;
	mc->data.clear();
    mc_init(mc, 0);
}

void fw_reset_com(memory_chip_t *mc)
{
	if(mc->com == FW_CMD_PAGEWRITE)
	{
		if (mc->fp)
		{
			fseek(mc->fp, 0, SEEK_SET);
			fwrite(&mc->data[0], mc->size, 1, mc->fp);
		}

		if (mc->isFirmware&&CommonSettings.UseExtFirmware)
		{
			// copy User Settings 1 to User Settings 0 area
			memcpy(&mc->data[0x3FE00], &mc->data[0x3FF00], 0x100);

			printf("Firmware: save config");
			FILE *fp = fopen(mc->userfile, "wb");
			if (fp)
			{
				if (fwrite(&mc->data[0x3FF00], 1, 0x100, fp) == 0x100)		// User Settings
				{
					if (fwrite(&mc->data[0x0002A], 1, 0x1D6, fp) == 0x1D6)  // WiFi Settings
					{
						if (fwrite(&mc->data[0x3FA00], 1, 0x300, fp) == 0x300)  // WiFi AP Settings
							printf(" - done\n");
						else
							printf(" - failed\n");
					}
				}
				fclose(fp);
			}
			else
				printf(" - failed\n");
		}

		mc->write_enable = false;
	}

	mc->com = 0;
}

uint8_t fw_transfer(memory_chip_t *mc, uint8_t data)
{
	if(mc->com == FW_CMD_READ || mc->com == FW_CMD_PAGEWRITE) /* check if we are in a command that needs 3 bytes address */
	{
		if(mc->addr_shift > 0)   /* if we got a complete address */
		{
			mc->addr_shift--;
			mc->addr |= data << (mc->addr_shift * 8); /* argument is a byte of address */
		}
		else    /* if we have received 3 bytes of address, proceed command */
		{
			switch(mc->com)
			{
				case FW_CMD_READ:
					if(mc->addr < mc->size)  /* check if we can read */
					{
						data = mc->data[mc->addr];       /* return byte */
						mc->addr++;      /* then increment address */
					}
					break;

				case FW_CMD_PAGEWRITE:
					if(mc->addr < mc->size)
					{
						mc->data[mc->addr] = data;       /* write byte */
						mc->addr++;
					}
					break;
			}

		}
	}
	else if(mc->com == FW_CMD_READ_ID)
	{
		switch(mc->addr)
		{
		//here is an ID string measured from an old ds fat: 62 16 00 (0x62=sanyo)
		//but we chose to use an ST from martin's ds fat string so programs might have a clue as to the firmware size:
		//20 40 12
		case 0:
			data = 0x20;
			mc->addr=1;
			break;
		case 1:
			data = 0x40; //according to gbatek this is the device ID for the flash on someone's ds fat
			mc->addr=2;
			break;
		case 2:
			data = 0x12;
			mc->addr = 0;
			break;
		}
	}
	else if(mc->com == FW_CMD_READSTATUS)
	{
		return mc->write_enable ? 0x02 : 0x00;
	}
	else	//finally, check if it's a new command
	{
		switch(data)
		{
			case 0: break;	//nothing

			case FW_CMD_READ_ID:
				mc->addr = 0;
				mc->com = FW_CMD_READ_ID;
				break;

			case FW_CMD_READ:    //read command
				mc->addr = 0;
				mc->addr_shift = 3;
				mc->com = FW_CMD_READ;
				break;

			case FW_CMD_WRITEENABLE:     //enable writing
				if(mc->writeable_buffer) { mc->write_enable = true; }
				break;

			case FW_CMD_WRITEDISABLE:    //disable writing

				mc->write_enable = false;
				break;

			case FW_CMD_PAGEWRITE:       //write command
				if(mc->write_enable)
				{
					mc->addr = 0;
					mc->addr_shift = 3;
					mc->com = FW_CMD_PAGEWRITE;
				}
				else { data = 0; }
				break;

			case FW_CMD_READSTATUS:  //status register command
				mc->com = FW_CMD_READSTATUS;
				break;

			default:
				printf("Unhandled FW command: %02X\n", data);
				break;
		}
	}

	return data;
}

/*bool BackupDevice::save_state(EMUFILE* os)
{
	uint32_t version = 2;
	//v0
	write32le(version,os);
	write32le(write_enable,os);
	write32le(com,os);
	write32le(addr_size,os);
	write32le(addr_counter,os);
	write32le((uint32_t)state,os);
	writebuffer(data,os);
	writebuffer(data_autodetect,os);
	//v1
	write32le(addr,os);
	//v2
	write8le(motionInitState,os);
	write8le(motionFlag,os);
	return true;
}*/

bool BackupDevice::load_state(EMUFILE* is)
{
	uint32_t version;
	if(read32le(&version,is)!=1) return false;
	//if(version>=0)
	{
		readbool(&write_enable,is);
		read32le(&com,is);
		read32le(&addr_size,is);
		read32le(&addr_counter,is);
		uint32_t temp;
		read32le(&temp,is);
		state = (STATE)temp;
		readbuffer(data,is);
		readbuffer(data_autodetect,is);
	}
	if(version>=1)
		read32le(&addr,is);
	if(version>=2)
	{
		read8le(&motionInitState,is);
		read8le(&motionFlag,is);
	}

	return true;
}

BackupDevice::BackupDevice()
{
	//isMovieMode = false;
	reset();
}

//due to unfortunate shortcomings in the emulator architecture,
//at reset-time, we won't have a filename to the .dsv file.
//so the only difference between load_rom (init) and reset is that
//one of them saves the filename
void BackupDevice::load_rom(const char* fn)
{
	//isMovieMode = false;
	this->filename = fn;
	reset();
}

/*void BackupDevice::movie_mode()
{
	isMovieMode = true;
	reset();
}*/

void BackupDevice::reset_hardware()
{
	write_enable = false;
	com = 0;
	addr = addr_counter = 0;
	motionInitState = MOTION_INIT_STATE_IDLE;
	motionFlag = MOTION_FLAG_NONE;
	state = DETECTING;
	flushPending = false;
	lazyFlushPending = false;
}

void BackupDevice::reset()
{
	memset(&info, 0, sizeof(info));
	reset_hardware();
	resize(0);
	data_autodetect.resize(0);
	addr_size = 0;
	loadfile();

	//if the user has requested a manual choice for backup type, and we havent imported a raw save file, then apply it now
	if(state == DETECTING && CommonSettings.manualBackupType != MC_TYPE_AUTODETECT)
	{
		state = RUNNING;
		int savetype = save_types[CommonSettings.manualBackupType][0];
		int savesize = save_types[CommonSettings.manualBackupType][1];
		ensure((uint32_t)savesize); //expand properly if necessary
		resize(savesize); //truncate if necessary
		addr_size = addr_size_for_old_save_type(savetype);
		//flush();
	}
}

/*void BackupDevice::close_rom()
{
	//flush();
}*/

/*void BackupDevice::reset_command()
{
	//printf("MC RESET\n");
	//for a performance hack, save files are only flushed after each reset command
	//(hopefully, after each page)
	if(flushPending)
	{
		//flush();
		flushPending = false;
		lazyFlushPending = false;
	}

	if(state == DETECTING && data_autodetect.size()>0)
	{
		//we can now safely detect the save address size
		uint32_t autodetect_size = data_autodetect.size();

		printf("Autodetecting with autodetect_size=%d\n",autodetect_size);

		const uint8_t sm64_sig[] = {0x01,0x80,0x00,0x00};
		if(autodetect_size == 4 && !memcmp(&data_autodetect[0],sm64_sig,4))
		{
			addr_size = 2;
		}
		else //detect based on rules
			switch(autodetect_size)
			{
			case 0:
			case 1:
				printf("Catastrophic error while autodetecting save type.\nIt will need to be specified manually\n");
				#ifdef _WINDOWS
				MessageBoxA(0,"Catastrophic Error Code: Camel;\nyour save type has not been autodetected correctly;\nplease report to developers",0,0);
				#endif
				addr_size = 1; //choose 1 just to keep the busted savefile from growing too big
				break;
			case 2:
				 //the modern typical case for small eeproms
				addr_size = 1;
				break;
			case 3:
				//another modern typical case..
				//but unfortunately we select this case for spider-man 3, when what it meant to do was
				//present the archaic 1+2 case
				//it seems that over the hedge does this also.
				addr_size = 2;
				break;
			case 4:
				//a modern typical case
				addr_size = 3;
				break;
			default:
				//the archaic case: write the address and then some modulo-4 number of bytes
				//why modulo 4? who knows.
				addr_size = autodetect_size & 3;
				break;
			}

		state = RUNNING;
		data_autodetect.resize(0);
		//flush();
	}

	com = 0;
}*/
/*uint8_t BackupDevice::data_command(uint8_t val, int cpu)
{
	//printf("MC CMD: %02X\n",val);

	//motion: some guessing here... hope it doesn't break anything
	if(com == BM_CMD_READLOW && motionInitState == MOTION_INIT_STATE_RECEIVED_4_B && val == 0)
	{
		motionInitState = MOTION_INIT_STATE_IDLE;
		motionFlag |= MOTION_FLAG_ENABLED;
		//return 0x04; //return 0x04 to enable motion!!!!!
		return 0; //but we return 0 to disable it! since we don't emulate it anyway
	}

	//if the game firmly believes we have motion support, then ignore all motion commands, since theyre not emulated.
	if(motionFlag & MOTION_FLAG_SENSORMODE)
	{
		return 0;
	}

	if(com == BM_CMD_READLOW || com == BM_CMD_WRITELOW)
	{
		//handle data or address
		if(state == DETECTING)
		{
			if(com == BM_CMD_WRITELOW)
			{
				printf("Unexpected backup device initialization sequence using writes!\n");
			}

			//just buffer the data until we're no longer detecting
			data_autodetect.push_back(val);
			val = 0;
		}
		else
		{
			if(addr_counter<addr_size)
			{
				//continue building address
				addr <<= 8;
				addr |= val;
				addr_counter++;
				//if(addr_counter==addr_size) printf("ADR: %08X\n",addr);
			}
			else
			{
				//why does tomb raider underworld access 0x180 and go clear through to 0x280?
				//should this wrap around at 0 or at 0x100?
				if(addr_size == 1) addr &= 0x1FF;

				//address is complete
				ensure(addr+1);
				if(com == BM_CMD_READLOW)
				{
					//printf("READ ADR: %08X\n",addr);
					val = data[addr];
					//flushPending = true; //is this a good idea? it may slow stuff down, but it is helpful for debugging
					lazyFlushPending = true; //lets do this instead
					//printf("read: %08X\n",addr);
				}
				else
				{
					if(write_enable)
					{
						//printf("WRITE ADR: %08X\n",addr);
						data[addr] = val;
						flushPending = true;
						//printf("writ: %08X\n",addr);
					}
				}
				addr++;

			}
		}
	}
	else if(com == BM_CMD_READSTATUS)
	{
		//handle request to read status
		//LOG("Backup Memory Read Status: %02X\n", write_enable << 1);
		return (write_enable << 1) | (3<<2);
	}
	else
	{
		//there is no current command. receive one
		switch(val)
		{
			case 0: break; //??

			case 0xFE:
				if(motionInitState == MOTION_INIT_STATE_IDLE) { motionInitState = MOTION_INIT_STATE_FE; return 0; }
				break;
			case 0xFD:
				if(motionInitState == MOTION_INIT_STATE_FE) { motionInitState = MOTION_INIT_STATE_FD; return 0; }
				break;
			case 0xFB:
				if(motionInitState == MOTION_INIT_STATE_FD) { motionInitState = MOTION_INIT_STATE_FB; return 0; }
				break;
			case 0xF8:
				//enable sensor mode
				if(motionInitState == MOTION_INIT_STATE_FD)
				{
					motionInitState = MOTION_INIT_STATE_IDLE;
					motionFlag |= MOTION_FLAG_SENSORMODE;
					return 0;
				}
				break;
			case 0xF9:
				//disable sensor mode
				if(motionInitState == MOTION_INIT_STATE_FD)
				{
					motionInitState = MOTION_INIT_STATE_IDLE;
					motionFlag &= ~MOTION_FLAG_SENSORMODE;
					return 0;
				}
				break;

			case 8:
				printf("COMMAND%c: Unverified Backup Memory command: %02X FROM %08X\n",(cpu==ARMCPU_ARM9)?'9':'7',val, (cpu==ARMCPU_ARM9)?NDS_ARM9.instruct_adr:NDS_ARM7.instruct_adr);
				val = 0xAA;
				break;

			case BM_CMD_WRITEDISABLE:
				switch(motionInitState)
				{
				case MOTION_INIT_STATE_IDLE: motionInitState = MOTION_INIT_STATE_RECEIVED_4; break;
				case MOTION_INIT_STATE_RECEIVED_4: motionInitState = MOTION_INIT_STATE_RECEIVED_4_B; break;
				}
				write_enable = false;
				break;

			case BM_CMD_READSTATUS:
				com = BM_CMD_READSTATUS;
				val = (write_enable << 1) | (3<<2);
				break;

			case BM_CMD_WRITEENABLE:
				write_enable = true;
				break;

			case BM_CMD_WRITELOW:
			case BM_CMD_READLOW:
				//printf("XLO: %08X\n",addr);
				com = val;
				addr_counter = 0;
				addr = 0;
				break;

			case BM_CMD_WRITEHIGH:
			case BM_CMD_READHIGH:
				//printf("XHI: %08X\n",addr);
				if(val == BM_CMD_WRITEHIGH) val = BM_CMD_WRITELOW;
				if(val == BM_CMD_READHIGH) val = BM_CMD_READLOW;
				com = val;
				addr_counter = 0;
				addr = 0;
				if(addr_size==1) {
					//"write command that's only available on ST M95040-W that I know of"
					//this makes sense, since this device would only have a 256 bytes address space with writelow
					//and writehigh would allow access to the upper 256 bytes
					//but it was detected in pokemon diamond also during the main save process
					addr = 0x1;
				}
				break;

			default:
				printf("COMMAND%c: Unhandled Backup Memory command: %02X FROM %08X\n",(cpu==ARMCPU_ARM9)?'9':'7',val, (cpu==ARMCPU_ARM9)?NDS_ARM9.instruct_adr:NDS_ARM7.instruct_adr);
				break;
		} //switch(val)

		//motion control state machine broke, return to ground
		motionInitState = MOTION_INIT_STATE_IDLE;
	}
	return val;
}*/

//guarantees that the data buffer has room enough for the specified number of bytes
void BackupDevice::ensure(uint32_t Addr)
{
	uint32_t size = data.size();
	if(size<Addr)
	{
		resize(Addr);
	}
}

void BackupDevice::resize(uint32_t size)
{
	size_t old_size = data.size();
	data.resize(size);
	for(uint32_t i=old_size;i<size;i++)
		data[i] = kUninitializedSaveDataValue;
}

uint32_t BackupDevice::addr_size_for_old_save_size(int bupmem_size)
{
	switch(bupmem_size) {
		case MC_SIZE_4KBITS:
			return 1;
		case MC_SIZE_64KBITS:
		case MC_SIZE_256KBITS:
		case MC_SIZE_512KBITS:
			return 2;
		case MC_SIZE_1MBITS:
		case MC_SIZE_2MBITS:
		case MC_SIZE_4MBITS:
		case MC_SIZE_8MBITS:
		case MC_SIZE_16MBITS:
		case MC_SIZE_64MBITS:
			return 3;
		default:
			return 0xFFFFFFFF;
	}
}

uint32_t BackupDevice::addr_size_for_old_save_type(int bupmem_type)
{
	switch(bupmem_type)
	{
		case MC_TYPE_EEPROM1:
			return 1;
		case MC_TYPE_EEPROM2:
		case MC_TYPE_FRAM:
              return 2;
		case MC_TYPE_FLASH:
			return 3;
		default:
			return 0xFFFFFFFF;
	}
}


void BackupDevice::load_old_state(uint32_t addrSize, uint8_t* Data, uint32_t datasize)
{
	state = RUNNING;
	this->addr_size = addrSize;
	resize(datasize);
	memcpy(&this->data[0],Data,datasize);

	//dump back out as a dsv, just to keep things sane
	//flush();
}

//======================================================================= no$GBA
//=======================================================================
//=======================================================================

static int no_gba_unpackSAV(void *in_buf, uint32_t fsize, void *out_buf, uint32_t &size)
{
	const char no_GBA_HEADER_ID[] = "NocashGbaBackupMediaSavDataFile";
	const char no_GBA_HEADER_SRAM_ID[] = "SRAM";
	uint8_t	*src = (uint8_t *)in_buf;
	uint8_t	*dst = (uint8_t *)out_buf;
	uint32_t src_pos = 0;
	uint32_t dst_pos = 0;
	uint8_t	cc = 0;
	uint32_t	size_unpacked = 0;
	//uint32_t	size_packed = 0;
	uint32_t	compressMethod = 0;

	if (fsize < 0x50) return 1;

	for (int i = 0; i < 0x1F; i++)
	{
		if (src[i] != no_GBA_HEADER_ID[i]) return 2;
	}
	if (src[0x1F] != 0x1A) return 2;
	for (int i = 0; i < 0x4; i++)
	{
		if (src[i+0x40] != no_GBA_HEADER_SRAM_ID[i]) return 2;
	}

	compressMethod = *((uint32_t*)(src+0x44));

	if (compressMethod == 0)				// unpacked
	{
		size_unpacked = *((uint32_t*)(src+0x48));
		src_pos = 0x4C;
		for (uint32_t i = 0; i < size_unpacked; i++)
		{
			dst[dst_pos++] = src[src_pos++];
		}
		size = dst_pos;
		return 0;
	}

	if (compressMethod == 1)				// packed (method 1)
	{
		//size_packed = *((uint32_t*)(src+0x48));
		size_unpacked = *((uint32_t*)(src+0x4C));

		src_pos = 0x50;
		while (true)
		{
			cc = src[src_pos++];

			if (cc == 0)
			{
				size = dst_pos;
				return 0;
			}

			if (cc == 0x80)
			{
				uint16_t tsize = *((uint16_t*)(src+src_pos+1));
				for (int t = 0; t < tsize; t++)
					dst[dst_pos++] = src[src_pos];
				src_pos += 3;
				continue;
			}

			if (cc > 0x80)		// repeat
			{
				cc -= 0x80;
				for (int t = 0; t < cc; t++)
					dst[dst_pos++] = src[src_pos];
				src_pos++;
				continue;
			}
			// copy
			for (int t = 0; t < cc; t++)
				dst[dst_pos++] = src[src_pos++];
		}
		size = dst_pos;
		return 0;
	}
	return 200;
}

static uint32_t no_gba_savTrim(void *buf, uint32_t size)
{
	uint32_t rows = size / 16;
	uint32_t pos = (size - 16);
	uint8_t	*src = (uint8_t*)buf;

	for (unsigned int i = 0; i < rows; i++, pos -= 16)
	{
		if (src[pos] == 0xFF)
		{
			for (int t = 0; t < 16; t++)
			{
				if (src[pos+t] != 0xFF) return pos+16;
			}
		}
		else
		{
			return pos+16;
		}
	}
	return size;
}

static uint32_t no_gba_fillLeft(uint32_t size)
{
	for (uint32_t i = 1; i < ARRAY_SIZE(save_types); i++)
	{
		if (size <= (uint32_t)save_types[i][1])
			return size + (save_types[i][1] - size);
	}
	return size;
}

bool BackupDevice::load_no_gba(const char *fname)
{
	FILE	*fsrc = fopen(fname, "rb");
	uint8_t		*in_buf = NULL;
	uint8_t		*out_buf = NULL;

	if (fsrc)
	{
		uint32_t fsize = 0;
		fseek(fsrc, 0, SEEK_END);
		fsize = ftell(fsrc);
		fseek(fsrc, 0, SEEK_SET);
		//printf("Open %s file (size %i bytes)\n", fname, fsize);

		in_buf = new uint8_t [fsize];

		if (fread(in_buf, 1, fsize, fsrc) == fsize)
		{
			out_buf = new uint8_t [8 * 1024 * 1024 / 8];
			uint32_t size = 0;

			memset(out_buf, 0xFF, 8 * 1024 * 1024 / 8);
			if (no_gba_unpackSAV(in_buf, fsize, out_buf, size) == 0)
			{
				//printf("New size %i byte(s)\n", size);
				size = no_gba_savTrim(out_buf, size);
				//printf("--- new size after trim %i byte(s)\n", size);
				size = no_gba_fillLeft(size);
				//printf("--- new size after fill %i byte(s)\n", size);
				raw_applyUserSettings(size);
				data.resize(size);
				for (uint32_t tt = 0; tt < size; tt++)
					data[tt] = out_buf[tt];

				//dump back out as a dsv, just to keep things sane
				//flush();
				printf("---- Loaded no$GBA save\n");

				if (in_buf) delete [] in_buf;
				if (out_buf) delete [] out_buf;
				fclose(fsrc);
				return true;
			}
			if (out_buf) delete [] out_buf;
		}
		if (in_buf) delete [] in_buf;
		fclose(fsrc);
	}

	return false;
}

/*bool BackupDevice::save_no_gba(const char* fname)
{
	FILE* outf = fopen(fname,"wb");
	if(!outf) return false;
	uint32_t size = data.size();
	uint32_t padSize = pad_up_size(size);
	if(data.size()>0)
		fwrite(&data[0],1,size,outf);
	for(uint32_t i=size;i<padSize;i++)
		fputc(0xFF,outf);

	if (padSize < 512 * 1024)
	{
		for(uint32_t i=padSize; i<512 * 1024; i++)
			fputc(0xFF,outf);
	}
	fclose(outf);
	return true;
}*/
//======================================================================= end
//=======================================================================
//======================================================================= no$GBA


void BackupDevice::loadfile()
{
	//never use save files if we are in movie mode
	//if(isMovieMode) return;
	if(filename.length() ==0) return; //No sense crashing if no filename supplied

	EMUFILE_FILE* inf = new EMUFILE_FILE(filename.c_str(),"rb");
	if(inf->fail())
	{
		delete inf;
		//no dsv found; we need to try auto-importing a file with .sav extension
		printf("DeSmuME .dsv save file not found. Trying to load an old raw .sav file.\n");

		//change extension to sav
		char tmp[MAX_PATH];
		strcpy(tmp,filename.c_str());
		tmp[strlen(tmp)-3] = 0;
		strcat(tmp,"sav");

		inf = new EMUFILE_FILE(tmp,"rb");
		if(inf->fail())
		{
			delete inf;
			printf("Missing save file %s\n",filename.c_str());
			return;
		}
		delete inf;

		if (!load_no_gba(tmp))
			load_raw(tmp);
	}
	else
	{
		//scan for desmume save footer
		const int32_t cookieLen = (int32_t)strlen(kDesmumeSaveCookie);
		char *sigbuf = new char[cookieLen];
		inf->fseek(-cookieLen, SEEK_END);
		inf->fread(sigbuf,cookieLen);
		int cmp = memcmp(sigbuf,kDesmumeSaveCookie,cookieLen);
		delete[] sigbuf;
		if(cmp)
		{
			//maybe it is a misnamed raw save file. try loading it that way
			printf("Not a DeSmuME .dsv save file. Trying to load as raw.\n");
			delete inf;
			if (!load_no_gba(filename.c_str()))
				load_raw(filename.c_str());
			return;
		}
		//desmume format
		inf->fseek(-cookieLen, SEEK_END);
		inf->fseek(-4, SEEK_CUR);
		uint32_t version = 0xFFFFFFFF;
		read32le(&version,inf);
		if(version!=0) {
			printf("Unknown save file format\n");
			return;
		}
		inf->fseek(-24, SEEK_CUR);
		read32le(&info.size,inf);
		read32le(&info.padSize,inf);
		read32le(&info.type,inf);
		read32le(&info.addr_size,inf);
		read32le(&info.mem_size,inf);

		//uint32_t left = 0;
		/*if (CommonSettings.autodetectBackupMethod == 1)
		{
			if (advsc.isLoaded())
			{
				info.type = advsc.getSaveType();
				if (info.type != 0xFF || info.type != 0xFE)
				{
					u32 adv_size = save_types[info.type+1][1];
					if (info.size > adv_size)
						info.size = adv_size;
					else
						if (info.size < adv_size)
						{
							left = adv_size - info.size;
							info.size = adv_size;
						}
				}
			}
		}*/
		//establish the save data
		resize(info.size);
		inf->fseek(0, SEEK_SET);
		if(info.size>0)
			inf->fread(&data[0],info.size); //read all the raw data we have
		state = RUNNING;
		addr_size = info.addr_size;
		//none of the other fields are used right now

		/*if (CommonSettings.autodetectBackupMethod != 1 && info.type == 0)
		{
			info.type = searchFileSaveType(info.size);
			if (info.type == 0xFF) info.type = 0;
		}
		uint32_t ss = info.size * 8 / 1024;
		if (ss >= 1024)
		{
			ss /= 1024;
			printf("Backup size: %i Mbit\n", ss);
		}
		else
			printf("Backup size: %i Kbit\n", ss);*/

		delete inf;
	}
}

/*bool BackupDevice::save_raw(const char* fn)
{
	FILE* outf = fopen(fn,"wb");
	if(!outf) return false;
	uint32_t size = data.size();
	uint32_t padSize = pad_up_size(size);
	if(data.size()>0)
		fwrite(&data[0],1,size,outf);
	for(uint32_t i=size;i<padSize;i++)
		fputc(kUninitializedSaveDataValue,outf);
	fclose(outf);
	return true;
}*/

/*uint32_t BackupDevice::pad_up_size(uint32_t startSize)
{
	uint32_t size = startSize;
	uint32_t ctr=0;
	while(ctr<saveSizes_count && size > saveSizes[ctr]) ctr++;
	uint32_t padSize = saveSizes[ctr];
	if(padSize == 0xFFFFFFFF)
	{
		printf("PANIC! Couldn't pad up save size. Refusing to pad.\n");
		padSize = startSize;
	}
		return padSize;
}*/

/*void BackupDevice::lazy_flush()
{
	if(flushPending || lazyFlushPending)
	{
		lazyFlushPending = flushPending = false;
		//flush();
	}
}*/

/*void BackupDevice::flush()
{
	//never use save files if we are in movie mode
	if(isMovieMode) return;

	EMUFILE* outf = new EMUFILE_FILE(filename.c_str(),"wb");
	if(!outf->fail())
	{
		if(data.size()>0)
			outf->fwrite(&data[0],data.size());

		//write the footer. we use a footer so that we can maximize the chance of the
		//save file being recognized as a raw save file by other emulators etc.

		//first, pad up to the next largest known save size.
		uint32_t size = data.size();
		uint32_t padSize = pad_up_size(size);

		for(uint32_t i=size;i<padSize;i++)
			outf->fputc(kUninitializedSaveDataValue);

		//this is just for humans to read
		outf->fprintf("|<--Snip above here to create a raw sav by excluding this DeSmuME savedata footer:");

		//and now the actual footer
		write32le(size,outf); //the size of data that has actually been written
		write32le(padSize,outf); //the size we padded it to
		write32le(0,outf); //save memory type
		write32le(addr_size,outf);
		write32le(0,outf); //save memory size
		write32le(0,outf); //version number
		outf->fprintf("%s", kDesmumeSaveCookie); //this is what we'll use to recognize the desmume format save

		delete outf;
	}
	else
	{
		delete outf;
		printf("Unable to open savefile %s\n",filename.c_str());
	}
}*/

void BackupDevice::raw_applyUserSettings(uint32_t& size, bool manual)
{
	//respect the user's choice of backup memory type
	if(CommonSettings.manualBackupType == MC_TYPE_AUTODETECT && !manual)
	{
		addr_size = addr_size_for_old_save_size(size);
		resize(size);
	}
	else
	{
		uint32_t type = CommonSettings.manualBackupType;
		/*if (manual)
		{
			uint32_t res = searchFileSaveType(size);
			if (res != 0xFF) type = (res + 1); // +1 - skip autodetect
		}*/
		int savetype = save_types[type][0];
		int savesize = save_types[type][1];
		addr_size = addr_size_for_old_save_type(savetype);
		if((uint32_t)savesize<size) size = savesize;
		resize(savesize);
	}

	state = RUNNING;
}

/*uint32_t BackupDevice::get_save_raw_size(const char* fname)
{
	FILE* inf = fopen(fname,"rb");
	if (!inf) return 0xFFFFFFFF;

	fseek(inf, 0, SEEK_END);
	uint32_t size = (uint32_t)ftell(inf);
	fclose(inf);
	return size;
}*/

bool BackupDevice::load_raw(const char* fn, uint32_t force_size)
{
	FILE* inf = fopen(fn,"rb");

	if (!inf) return false;

	fseek(inf, 0, SEEK_END);
	uint32_t size = (uint32_t)ftell(inf);
	uint32_t left = 0;

	if (force_size > 0)
	{
		if (size > force_size)
			size = force_size;
		else if (size < force_size)
		{
			left = force_size - size;
			size = force_size;
		}
	}

	fseek(inf, 0, SEEK_SET);

	raw_applyUserSettings(size, force_size > 0);

	fread(&data[0],1,size - left,inf);
	fclose(inf);

	//dump back out as a dsv, just to keep things sane
	//flush();

	return true;
}


/*bool BackupDevice::load_duc(const char* fn)
{
  uint32_t size;
   char id[16];
   FILE* file = fopen(fn, "rb");
   if(file == NULL)
      return false;

   fseek(file, 0, SEEK_END);
   size = (uint32_t)ftell(file) - 500;
   fseek(file, 0, SEEK_SET);

   // Make sure we really have the right file
   fread((void *)id, sizeof(char), 16, file);

   if (memcmp(id, "ARDS000000000001", 16) != 0)
   {
	   printf("Not recognized as a valid DUC file\n");
      fclose(file);
      return false;
   }
   // Skip the rest of the header since we don't need it
   fseek(file, 500, SEEK_SET);

   raw_applyUserSettings(size);

   ensure((uint32_t)size);

   fread(&data[0],1,size,file);
   fclose(file);

   //choose

   //flush();

   return true;

}*/

/*bool BackupDevice::load_movie(EMUFILE* is) {

	const int32_t cookieLen = (int32_t)strlen(kDesmumeSaveCookie);

	is->fseek(-cookieLen, SEEK_END);
	is->fseek(-4, SEEK_CUR);

	uint32_t version = 0xFFFFFFFF;
	is->fread((char*)&version,4);
	if(version!=0) {
		printf("Unknown save file format\n");
		return false;
	}
	is->fseek(-24, SEEK_CUR);

	struct{
		uint32_t size,padSize,type,addr_size,mem_size;
	}info;

	is->fread((char*)&info.size,4);
	is->fread((char*)&info.padSize,4);
	is->fread((char*)&info.type,4);
	is->fread((char*)&info.addr_size,4);
	is->fread((char*)&info.mem_size,4);

	//establish the save data
	data.resize(info.size);
	is->fseek(0, SEEK_SET);
	if(info.size>0)
		is->fread((char*)&data[0],info.size);

	state = RUNNING;
	addr_size = info.addr_size;
	//none of the other fields are used right now

	return true;
}*/

/*void BackupDevice::forceManualBackupType()
{
	addr_size = addr_size_for_old_save_size(save_types[CommonSettings.manualBackupType][1]);
	state = RUNNING;
}*/