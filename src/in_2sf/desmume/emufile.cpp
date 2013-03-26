/*
The MIT License

Copyright (C) 2009-2010 DeSmuME team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <vector>

#include "emufile.h"

/*bool EMUFILE::readAllBytes(std::vector<uint8_t>* dstbuf, const std::string& fname)
{
	EMUFILE_FILE file(fname.c_str(),"rb");
	if(file.fail()) return false;
	int size = file.size();
	dstbuf->resize(size);
	file.fread(&dstbuf->at(0),size);
	return true;
}*/

size_t EMUFILE_MEMORY::_fread(void *ptr, size_t bytes){
	uint32_t remain = len-pos;
	uint32_t todo = std::min<uint32_t>(remain,(uint32_t)bytes);
	if(len==0)
	{
		failbit = true;
		return 0;
	}
	if(todo<=4)
	{
		uint8_t* src = buf()+pos;
		uint8_t* dst = (uint8_t*)ptr;
		for(uint32_t i=0;i<todo;i++)
			*dst++ = *src++;
	}
	else
	{
		memcpy(ptr,buf()+pos,todo);
	}
	pos += todo;
	if(todo<bytes)
		failbit = true;
	return todo;
}

/*void EMUFILE_FILE::truncate(int32_t length)
{
	::fflush(fp);
	#if defined(_MSC_VER) || defined(__MINGW32__)
		_chsize(_fileno(fp),length);
	#else
		ftruncate(fileno(fp),length);
	#endif
	fclose(fp);
	fp = NULL;
	open(fname.c_str(),mode);
}*/


/*EMUFILE* EMUFILE_FILE::memwrap()
{
	EMUFILE_MEMORY* mem = new EMUFILE_MEMORY(size());
	if(size()==0) return mem;
	fread(mem->buf(),size());
	return mem;
}*/

/*EMUFILE* EMUFILE_MEMORY::memwrap()
{
	return this;
}*/

/*void EMUFILE::write64le(uint64_t *val)
{
	write64le(*val);
}

void EMUFILE::write64le(uint64_t val)
{
#ifdef LOCAL_BE
	uint8_t s[8];
	s[0]=(uint8_t)val;
	s[1]=(uint8_t)(val>>8);
	s[2]=(uint8_t)(val>>16);
	s[3]=(uint8_t)(val>>24);
	s[4]=(uint8_t)(val>>32);
	s[5]=(uint8_t)(val>>40);
	s[6]=(uint8_t)(val>>48);
	s[7]=(uint8_t)(val>>56);
	fwrite((char*)&s,8);
#else
	fwrite(&val,8);
#endif
}

size_t EMUFILE::read64le(uint64_t *Bufo)
{
	uint64_t buf;
	if(fread((char*)&buf,8) != 8)
		return 0;
#ifndef LOCAL_BE
	*Bufo=buf;
#else
	*Bufo = LE_TO_LOCAL_64(buf);
#endif
	return 1;
}

uint64_t EMUFILE::read64le()
{
	uint64_t temp;
	read64le(&temp);
	return temp;
}

void EMUFILE::write32le(uint32_t *val)
{
	write32le(*val);
}

void EMUFILE::write32le(uint32_t val)
{
#ifdef LOCAL_BE
	uint8_t s[4];
	s[0]=(uint8_t)val;
	s[1]=(uint8_t)(val>>8);
	s[2]=(uint8_t)(val>>16);
	s[3]=(uint8_t)(val>>24);
	fwrite(s,4);
#else
	fwrite(&val,4);
#endif
}

size_t EMUFILE::read32le(int32_t *Bufo) { return read32le((uint32_t *)Bufo); }

size_t EMUFILE::read32le(uint32_t *Bufo)
{
	uint32_t buf;
	if(fread(&buf,4)<4)
		return 0;
#ifndef LOCAL_BE
	*(uint32_t *)Bufo=buf;
#else
	*(uint32_t *)Bufo=((buf&0xFF)<<24)|((buf&0xFF00)<<8)|((buf&0xFF0000)>>8)|((buf&0xFF000000)>>24);
#endif
	return 1;
}

uint32_t EMUFILE::read32le()
{
	uint32_t ret;
	read32le(&ret);
	return ret;
}

void EMUFILE::write16le(uint16_t *val)
{
	write16le(*val);
}

void EMUFILE::write16le(uint16_t val)
{
#ifdef LOCAL_BE
	uint8_t s[2];
	s[0]=(uint8_t)val;
	s[1]=(uint8_t)(val>>8);
	fwrite(s,2);
#else
	fwrite(&val,2);
#endif
}

size_t EMUFILE::read16le(int16_t *Bufo) { return read16le((uint16_t *)Bufo); }

size_t EMUFILE::read16le(uint16_t *Bufo)
{
	uint32_t buf;
	if(fread(&buf,2)<2)
		return 0;
#ifndef LOCAL_BE
	*(uint16_t *)Bufo=buf;
#else
	*Bufo = LE_TO_LOCAL_16(buf);
#endif
	return 1;
}

uint16_t EMUFILE::read16le()
{
	uint16_t ret;
	read16le(&ret);
	return ret;
}

void EMUFILE::write8le(uint8_t *val)
{
	write8le(*val);
}

void EMUFILE::write8le(uint8_t val)
{
	fwrite(&val,1);
}

size_t EMUFILE::read8le(uint8_t *val)
{
	return fread(val,1);
}

uint8_t EMUFILE::read8le()
{
	uint8_t temp;
	fread(&temp,1);
	return temp;
}

void EMUFILE::writedouble(double* val)
{
	write64le(double_to_u64(*val));
}
void EMUFILE::writedouble(double val)
{
	write64le(double_to_u64(val));
}

double EMUFILE::readdouble()
{
	double temp;
	readdouble(&temp);
	return temp;
}

size_t EMUFILE::readdouble(double* val)
{
	uint64_t temp;
	size_t ret = read64le(&temp);
	*val = u64_to_double(temp);
	return ret;
}*/
