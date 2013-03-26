/*
	Copyright (C) 2008-2009 DeSmuME team

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

#ifndef _READWRITE_H_
#define _READWRITE_H_

#include <iostream>
#include <vector>
#include <cstdio>

#include "types.h"
#include "emufile.h"

//well. just for the sake of consistency
//int write8le(uint8_t b, EMUFILE *fp);
//inline int write8le(uint8_t* b, EMUFILE *fp) { return write8le(*b,fp); }
//int write16le(uint16_t b, EMUFILE* os);
//int write32le(uint32_t b, EMUFILE* os);
//int write64le(uint64_t b, EMUFILE* os);
//inline int write_double_le(double b, EMUFILE*is) { uint64_t temp = double_to_u64(b); int ret = write64le(temp,is); return ret; }

int read8le(uint8_t *Bufo, EMUFILE*is);
int read16le(uint16_t *Bufo, EMUFILE*is);
inline int read16le(int16_t *Bufo, EMUFILE*is) { return read16le((uint16_t*)Bufo,is); }
int read32le(uint32_t *Bufo, EMUFILE*is);
inline int read32le(int32_t *Bufo, EMUFILE*is) { return read32le((uint32_t*)Bufo,is); }
int read64le(uint64_t *Bufo, EMUFILE*is);
inline int read_double_le(double *Bufo, EMUFILE*is) { uint64_t temp; int ret = read64le(&temp,is); *Bufo = u64_to_double(temp); return ret; }
int read16le(uint16_t *Bufo, std::istream *is);


/*template<typename T>
int readle(T *Bufo, EMUFILE*is)
{
	CTASSERT(sizeof(T)==1||sizeof(T)==2||sizeof(T)==4||sizeof(T)==8);
	switch(sizeof(T)) {
		case 1: return read8le((uint8_t*)Bufo,is);
		case 2: return read16le((uint16_t*)Bufo,is);
		case 4: return read32le((uint32_t*)Bufo,is);
		case 8: return read64le((uint64_t*)Bufo,is);
		default:
			return 0;
	}
}*/

int readbool(bool *b, EMUFILE* is);
//void writebool(bool b, EMUFILE* os);

int readbuffer(std::vector<uint8_t> &vec, EMUFILE* is);
//int writebuffer(std::vector<uint8_t>& vec, EMUFILE* os);

#endif
