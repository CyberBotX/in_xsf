/*
	Copyright (C) 2006 thoduv
	Copyright (C) 2006 Theo Berkau
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

#ifndef __FW_H__
#define __FW_H__

#include <vector>
#include <string>
#include <cstdio>

#include "types.h"
#include "emufile.h"
#include "windowsh_wrapper.h"

#define MAX_SAVE_TYPES 13
#define MC_TYPE_AUTODETECT      0x0
#define MC_TYPE_EEPROM1         0x1
#define MC_TYPE_EEPROM2         0x2
#define MC_TYPE_FLASH           0x3
#define MC_TYPE_FRAM            0x4

#define MC_SIZE_4KBITS                  0x000200
#define MC_SIZE_64KBITS                 0x002000
#define MC_SIZE_256KBITS                0x008000
#define MC_SIZE_512KBITS                0x010000
#define MC_SIZE_1MBITS                  0x020000
#define MC_SIZE_2MBITS                  0x040000
#define MC_SIZE_4MBITS                  0x080000
#define MC_SIZE_8MBITS                  0x100000
#define MC_SIZE_16MBITS                 0x200000
#define MC_SIZE_32MBITS                 0x400000
#define MC_SIZE_64MBITS                 0x800000
#define MC_SIZE_128MBITS                0x1000000
#define MC_SIZE_256MBITS                0x2000000
#define MC_SIZE_512MBITS                0x4000000

struct memory_chip_t
{
	uint8_t com;	//persistent command actually handled
	uint32_t addr;        //current address for reading/writing
	uint8_t addr_shift;   //shift for address (since addresses are transfered by 3 bytes units)
	uint8_t addr_size;    //size of addr when writing/reading

	bool write_enable;	//is write enabled ?

	//uint8_t *data;       //memory data
	std::vector<uint8_t> data;
	uint32_t size;       //memory size
	bool writeable_buffer;	//is "data" writeable ?
	int type; //type of Memory
	char *filename;
	FILE *fp;
	uint8_t autodetectbuf[32768];
	int autodetectsize;

	// needs only for firmware
	bool isFirmware;
	char userfile[MAX_PATH];
};

//the new backup system by zeromus
class BackupDevice
{
public:
	BackupDevice();

	//signals the save system that we are in our regular mode, loading up a rom. initializes for that case.
	void load_rom(const char* filename);
	//signals the save system that we are in MOVIE mode. doesnt load up a rom, and never saves it. initializes for that case.
	//void movie_mode();

	void reset();
	//void close_rom();
	//void forceManualBackupType();
	void reset_hardware();
	std::string getFilename() { return filename; }

	//bool save_state(EMUFILE* os);
	bool load_state(EMUFILE* is);

	//commands from mmu
	//void reset_command();
	//uint8_t data_command(uint8_t,int);
	std::vector<uint8_t> data;

	//this info was saved before the last reset (used for savestate compatibility)
	struct SavedInfo
	{
		uint32_t addr_size;
	} savedInfo;

	//and these are used by old savestates
	void load_old_state(uint32_t addr_size, uint8_t* data, uint32_t datasize);
	static uint32_t addr_size_for_old_save_size(int bupmem_size);
	static uint32_t addr_size_for_old_save_type(int bupmem_type);

	//static uint32_t pad_up_size(uint32_t startSize);
	void raw_applyUserSettings(uint32_t& size, bool manual = false);

	//bool load_duc(const char* filename);
	bool load_no_gba(const char *fname);
	//bool save_no_gba(const char* fname);
	bool load_raw(const char* filename, uint32_t force_size = 0);
	//bool save_raw(const char* filename);
	//bool load_movie(EMUFILE* is);

	//call me once a second or so to lazy flush the save data
	//here's the reason for this system: we want to dump save files when theyre READ
	//so that we have a better idea earlier on how large they are. but it slows things down
	//way too much if we flush whenever we read.
	//void lazy_flush();
	//void flush();

	struct {
			uint32_t size,padSize,type,addr_size,mem_size;
		} info;
	//bool isMovieMode;

private:
	std::string filename;

	bool write_enable;	//is write enabled?
	uint32_t com;	//persistent command actually handled
	uint32_t addr_size, addr_counter;
	uint32_t addr;

	std::vector<uint8_t> data_autodetect;
	enum STATE {
		DETECTING = 0, RUNNING = 1
	} state;

	enum MOTION_INIT_STATE
	{
		MOTION_INIT_STATE_IDLE, MOTION_INIT_STATE_RECEIVED_4, MOTION_INIT_STATE_RECEIVED_4_B,
		MOTION_INIT_STATE_FE, MOTION_INIT_STATE_FD, MOTION_INIT_STATE_FB
	};
	enum MOTION_FLAG
	{
		MOTION_FLAG_NONE=0,
		MOTION_FLAG_ENABLED=1,
		MOTION_FLAG_SENSORMODE=2
	};
	uint8_t motionInitState, motionFlag;

	void loadfile();
	bool _loadfile(const char *fname);
	void ensure(uint32_t addr);

	bool flushPending, lazyFlushPending;

private:
	void resize(uint32_t size);
};

#define NDS_FW_SIZE_V1 (256 * 1024)		/* size of fw memory on nds v1 */
#define NDS_FW_SIZE_V2 (512 * 1024)		/* size of fw memory on nds v2 */

void mc_init(memory_chip_t *mc, int type);    /* reset and init values for memory struct */
uint8_t *mc_alloc(memory_chip_t *mc, uint32_t size);  /* alloc mc memory */
//void mc_realloc(memory_chip_t *mc, int type, uint32_t size);      /* realloc mc memory */
//void mc_load_file(memory_chip_t *mc, const char* filename); /* load save file and setup fp */
void mc_free(memory_chip_t *mc);    /* delete mc memory */
void fw_reset_com(memory_chip_t *mc);       /* reset communication with mc */
uint8_t fw_transfer(memory_chip_t *mc, uint8_t data);

//void backup_setManualBackupType(int type);
//void backup_forceManualBackupType();

//extern const char *save_names[];

#endif /*__FW_H__*/
