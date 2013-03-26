/*
	Copyright (C) 2008-2010 DeSmuME team

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

//TODO - dismantle this file

#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <cstdio>
#include <cstring>

#include "types.h"

extern const uint8_t logo_data[156];

#ifdef WIN32
# include "windowsh_wrapper.h"
//# define WIN32_LEAN_AND_MEAN
//# include <winsock2.h>

/*# define CLASSNAME "DeSmuME"

extern HINSTANCE hAppInst;

extern bool romloaded;

extern char IniName[MAX_PATH];
extern void GetINIPath();
extern void WritePrivateProfileInt(char *appname, char *keyname, int val, char *file);

bool GetPrivateProfileBool(const char *appname, const char *keyname, bool defval, const char *filename);
void WritePrivateProfileBool(char *appname, char *keyname, bool val, char *file);
#else // non Windows
#define sscanf_s sscanf*/
#endif

/*template<typename T> T reverseBits(T x)
{
	T h = 0;

	for (unsigned i = 0; i < sizeof(T) * 8; ++i)
	{
		h = (h << 1) + (x & 1);
		x >>= 1;
	}

	return h;
}*/

/*template<typename T> char *intToBin(T val)
{
	char buf[256] = "";
	for (int i = sizeof(T) * 8, t = 0;  i > 0; --i, ++t)
		buf[i - 1] = val & (1<<t) ? '1' : '0';
	return _strdup(buf);
}*/

//extern char *trim(char *s, int len = -1);
//extern char *removeSpecialChars(char *s);

#endif
