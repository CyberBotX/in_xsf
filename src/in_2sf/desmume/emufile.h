/*
The MIT License

Copyright (C) 2009-2012 DeSmuME team

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

//don't use emufile for files bigger than 2GB! you have been warned! some day this will be fixed.

#ifndef EMUFILE_H
#define EMUFILE_H

#include <vector>
#include <algorithm>
#include <string>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cmath>

#include "types.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
/*#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <io.h>*/
#include "windowsh_wrapper.h"
#include <io.h>
#else
#include <unistd.h>
#endif

class EMUFILE
{
protected:
	bool failbit;

public:
	EMUFILE() : failbit(false)
	{
	}

	//returns a new EMUFILE which is guranteed to be in memory. the EMUFILE you call this on may be deleted. use the returned EMUFILE in its place
	//virtual EMUFILE* memwrap() = 0;

	virtual ~EMUFILE() {}

	//static bool readAllBytes(std::vector<uint8_t> *buf, const std::string &fname);

	bool fail(bool unset = false)
	{
		bool ret = failbit;
		if (unset)
			unfail();
		return ret;
	}
	void unfail() { failbit = false; }

	//bool eof() { return size() == ftell(); }

	size_t fread(void *ptr, size_t bytes)
	{
		return _fread(ptr, bytes);
	}

	//void unget() { fseek(-1, SEEK_CUR); }

	//virtuals
public:
	//virtual FILE *get_fp() = 0;

	//virtual int fprintf(const char *format, ...) = 0;

	//virtual int fgetc() = 0;
	//virtual int fputc(int c) = 0;

	virtual size_t _fread(void *ptr, size_t bytes) = 0;

	//removing these return values for now so we can find any code that might be using them and make sure
	//they handle the return values correctly

	//virtual void fwrite(const void *ptr, size_t bytes) = 0;

	/*void write64le(uint64_t *val);
	void write64le(uint64_t val);
	size_t read64le(uint64_t *val);
	uint64_t read64le();
	void write32le(uint32_t *val);
	void write32le(int32_t *val) { write32le((uint32_t *)val); }
	void write32le(uint32_t val);
	size_t read32le(uint32_t *val);
	size_t read32le(int32_t *val);
	uint32_t read32le();
	void write16le(uint16_t *val);
	void write16le(int16_t *val) { write16le((uint16_t *)val); }
	void write16le(uint16_t val);
	size_t read16le(int16_t *Bufo);
	size_t read16le(uint16_t *val);
	uint16_t read16le();
	void write8le(uint8_t *val);
	void write8le(uint8_t val);
	size_t read8le(uint8_t *val);
	uint8_t read8le();
	void writedouble(double* val);
	void writedouble(double val);
	double readdouble();
	size_t readdouble(double* val);*/

	virtual int fseek(int offset, int origin) = 0;

	virtual size_t ftell() = 0;
	virtual size_t size() = 0;

	//virtual void truncate(int32_t length) = 0;
};

//todo - handle read-only specially?
class EMUFILE_MEMORY : public EMUFILE
{
protected:
	std::vector<uint8_t> *vec;
	bool ownvec;
	int32_t pos, len;

	void reserve(uint32_t amt)
	{
		if (vec->size() < amt)
			vec->resize(amt);
	}

public:
	EMUFILE_MEMORY(std::vector<uint8_t> *underlying) : vec(underlying), ownvec(false), pos(0), len(static_cast<int32_t>(underlying->size())) { }
	EMUFILE_MEMORY(uint32_t preallocate) : vec(new std::vector<uint8_t>()), ownvec(true), pos(0), len(0)
	{
		vec->resize(preallocate);
		len = preallocate;
	}
	EMUFILE_MEMORY() : vec(new std::vector<uint8_t>()), ownvec(true), pos(0), len(0) { vec->reserve(1024); }
	EMUFILE_MEMORY(void *Buf, int32_t Size) : vec(new std::vector<uint8_t>()), ownvec(true), pos(0), len(Size)
	{
		vec->resize(Size);
		if (Size)
			memcpy(&(*vec)[0], Buf, Size);
	}

	~EMUFILE_MEMORY()
	{
		if (ownvec)
			delete vec;
	}

	//virtual EMUFILE *memwrap();

	/*virtual void truncate(int32_t length)
	{
		vec->resize(length);
		len = length;
		if (pos>length)
			pos = length;
	}*/

	uint8_t *buf() {
		if(size()==0) reserve(1);
		return &(*vec)[0];
	}

	//std::vector<uint8_t> *get_vec() { return vec; };

	//virtual FILE *get_fp() { return NULL; }

	/*virtual int fprintf(const char *format, ...)
	{
		va_list argptr;
		va_start(argptr, format);

		//we dont generate straight into the buffer because it will null terminate (one more byte than we want)
		int amt = vsnprintf(0, 0, format,argptr);
		char *tempbuf = new char[amt + 1];

		va_end(argptr);
		va_start(argptr, format);
		vsprintf(tempbuf, format, argptr);

		fwrite(tempbuf, amt);
		delete [] tempbuf;

		va_end(argptr);
		return amt;
	};*/

	/*virtual int fgetc()
	{
		uint8_t temp;

		//need an optimized codepath
		//if(_fread(&temp,1) != 1)
		//	return EOF;
		//else return temp;
		uint32_t remain = len-pos;
		if (remain < 1)
		{
			failbit = true;
			return -1;
		}
		temp = buf()[pos];
		++pos;
		return temp;
	}*/

	/*virtual int fputc(int c)
	{
		uint8_t temp = static_cast<uint8_t>(c);
		//TODO
		//if(fwrite(&temp,1)!=1) return EOF;
		fwrite(&temp, 1);

		return 0;
	}*/

	virtual size_t _fread(void *ptr, size_t bytes);

	//removing these return values for now so we can find any code that might be using them and make sure
	//they handle the return values correctly

	/*virtual void fwrite(const void *ptr, size_t bytes)
	{
		reserve(pos + bytes);
		memcpy(buf() + pos, ptr, bytes);
		pos += bytes;
		len = std::max(pos, len);
	}*/

	virtual int fseek(int offset, int origin)
	{
		//work differently for read-only...?
		switch (origin)
		{
			case SEEK_SET:
				pos = offset;
				break;
			case SEEK_CUR:
				pos += offset;
				break;
			case SEEK_END:
				pos = size() + offset;
				break;
			default:
				assert(false);
		}
		reserve(pos);
		return 0;
	}

	virtual size_t ftell()
	{
		return pos;
	}

	/*void trim()
	{
		vec->resize(len);
	}*/

	virtual size_t size() { return len; }
};

class EMUFILE_FILE : public EMUFILE
{
protected:
	FILE *fp;
	std::string fname;
	char mode[16];

private:
	void open(const char *fn, const char *Mode)
	{
		fp = fopen(fn, Mode);
		if (!fp)
			failbit = true;
		this->fname = fn;
		strcpy(this->mode, Mode);
	}

public:
	EMUFILE_FILE(const std::string &fn, const char *Mode) { open(fn.c_str(), Mode); }
	EMUFILE_FILE(const char *fn, const char *Mode) { open(fn, Mode); }

	virtual ~EMUFILE_FILE()
	{
		if (fp)
			fclose(fp);
	}

	/*virtual FILE *get_fp()
	{
		return fp;
	}*/

	//virtual EMUFILE *memwrap();

	//bool is_open() { return fp != NULL; }

	//virtual void truncate(int32_t length);

	/*virtual int fprintf(const char *format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		int ret = ::vfprintf(fp, format, argptr);
		va_end(argptr);
		return ret;
	};*/

	/*virtual int fgetc()
	{
		return ::fgetc(fp);
	}*/

	/*virtual int fputc(int c)
	{
		return ::fputc(c, fp);
	}*/

	virtual size_t _fread(void *ptr, size_t bytes)
	{
		size_t ret = ::fread(ptr, 1, bytes, fp);
		if (ret < bytes)
			failbit = true;
		return ret;
	}

	//removing these return values for now so we can find any code that might be using them and make sure
	//they handle the return values correctly

	/*virtual void fwrite(const void *ptr, size_t bytes)
	{
		size_t ret = ::fwrite(ptr, 1, bytes, fp);
		if (ret < bytes)
			failbit = true;
	}*/

	virtual int fseek(int offset, int origin)
	{
		return ::fseek(fp, offset, origin);
	}

	virtual size_t ftell()
	{
		return static_cast<uint32_t>(::ftell(fp));
	}

	virtual size_t size()
	{
		int oldpos = ftell();
		fseek(0, SEEK_END);
		int len = ftell();
		fseek(oldpos, SEEK_SET);
		return len;
	}

	/*virtual void fflush()
	{
		::fflush(fp);
	}*/
};

#endif
