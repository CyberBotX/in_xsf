/*
	Copyright (C) 2006-2009 DeSmuME team

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

#include "readwrite.h"
#include "types.h"

//well. just for the sake of consistency
/*int write8le(uint8_t b, EMUFILE*os)
{
	os->fwrite((char*)&b,1);
	return 1;
}*/

//well. just for the sake of consistency
int read8le(uint8_t *Bufo, EMUFILE*is)
{
	if(is->_fread((char*)Bufo,1) != 1)
		return 0;
	return 1;
}

///writes a little endian 16bit value to the specified file
/*int write16le(uint16_t b, EMUFILE *fp)
{
	uint8_t s[2];
	s[0]=(uint8_t)b;
	s[1]=(uint8_t)(b>>8);
	fp->fwrite(s,2);
	return 2;
}*/


///writes a little endian 32bit value to the specified file
/*int write32le(uint32_t b, EMUFILE *fp)
{
	uint8_t s[4];
	s[0]=(uint8_t)b;
	s[1]=(uint8_t)(b>>8);
	s[2]=(uint8_t)(b>>16);
	s[3]=(uint8_t)(b>>24);
	fp->fwrite(s,4);
	return 4;
}*/

//void writebool(bool b, EMUFILE* os) { write32le(b?1:0,os); }

/*int write64le(uint64_t b, EMUFILE* os)
{
	uint8_t s[8];
	s[0]=(uint8_t)b;
	s[1]=(uint8_t)(b>>8);
	s[2]=(uint8_t)(b>>16);
	s[3]=(uint8_t)(b>>24);
	s[4]=(uint8_t)(b>>32);
	s[5]=(uint8_t)(b>>40);
	s[6]=(uint8_t)(b>>48);
	s[7]=(uint8_t)(b>>56);
	os->fwrite((char*)&s,8);
	return 8;
}*/


int read32le(uint32_t *Bufo, EMUFILE *fp)
{
	uint32_t buf;
	if(fp->_fread(&buf,4)<4)
		return 0;
#ifdef LOCAL_LE
	*(uint32_t*)Bufo=buf;
#else
	*(uint32_t*)Bufo=((buf&0xFF)<<24)|((buf&0xFF00)<<8)|((buf&0xFF0000)>>8)|((buf&0xFF000000)>>24);
#endif
	return 1;
}

int read16le(uint16_t *Bufo, EMUFILE *is)
{
	uint16_t buf;
	if(is->_fread((char*)&buf,2) != 2)
		return 0;
#ifdef LOCAL_LE
	*Bufo=buf;
#else
	*Bufo = LE_TO_LOCAL_16(buf);
#endif
	return 1;
}

int read64le(uint64_t *Bufo, EMUFILE *is)
{
	uint64_t buf;
	if(is->_fread((char*)&buf,8) != 8)
		return 0;
#ifdef LOCAL_LE
	*Bufo=buf;
#else
	*Bufo = LE_TO_LOCAL_64(buf);
#endif
	return 1;
}

/*static int read32le(uint32_t *Bufo, std::istream *is)
{
	uint32_t buf;
	if(is->read((char*)&buf,4).gcount() != 4)
		return 0;
#ifdef LOCAL_LE
	*(uint32_t*)Bufo=buf;
#else
	*(uint32_t*)Bufo=((buf&0xFF)<<24)|((buf&0xFF00)<<8)|((buf&0xFF0000)>>8)|((buf&0xFF000000)>>24);
#endif
	return 1;
}*/

int readbool(bool *b, EMUFILE* is)
{
	uint32_t temp = 0;
	int ret = read32le(&temp,is);
	*b = temp!=0;
	return ret;
}

int readbuffer(std::vector<uint8_t> &vec, EMUFILE* is)
{
	uint32_t size;
	if(read32le(&size,is) != 1) return 0;
	vec.resize(size);
	if(size>0) is->fread((char*)&vec[0],size);
	return 1;
}

/*int writebuffer(std::vector<uint8_t>& vec, EMUFILE* os)
{
	uint32_t size = vec.size();
	write32le(size,os);
	if(size>0) os->fwrite((char*)&vec[0],size);
	return 1;
}*/
