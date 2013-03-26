/*
	Copyright (C) 2006 Normmatt
	Copyright (C) 2007 Pascal Giard
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

#ifndef _SRAM_H
#define _SRAM_H

#include "types.h"

#define SRAM_ADDRESS	0x0A000000
#define SRAM_SIZE	0x10000
#define NB_STATES       10

//extern int lastSaveState;

/*typedef struct
{
  bool exists;
  char date[40];
} savestates_t;*/


struct SFORMAT
{
	//a string description of the element
	const char *desc;

	//the size of each element
	uint32_t size;

	//the number of each element
	uint32_t count;

	//a void* to the data or a void** to the data
	void *v;
};

//extern savestates_t savestates[NB_STATES];

//void clear_savestates();
//void scan_savestates();
uint8_t sram_read (uint32_t address);
void sram_write (uint32_t address, uint8_t value);
int sram_load (const char *file_name);
int sram_save (const char *file_name);

//bool savestate_load (const char *file_name);
//bool savestate_save (const char *file_name);

//void savestate_slot(int num);
//void loadstate_slot(int num);

void loadstate();
bool savestate_load(class EMUFILE* is);
//bool savestate_save(class EMUFILE* outstream, int compressionLevel);

//void dorewind();
//void rewindsave();

#endif
