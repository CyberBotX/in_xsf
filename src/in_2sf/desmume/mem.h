/*
	Copyright (C) 2005 Theo Berkau
	Copyright (C) 2005-2006 Guillaume Duhamel
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

#ifndef MEM_H
#define MEM_H

#include <cstdlib>
#include <cassert>

#include "types.h"

//this was originally declared in MMU.h but we suffered some organizational problems and had to remove it
enum MMU_ACCESS_TYPE
{
	MMU_AT_CODE, //used for cpu prefetches
	MMU_AT_DATA, //used for cpu read/write
	MMU_AT_GPU, //used for gpu read/write
	MMU_AT_DMA, //used for dma read/write (blocks access to TCM)
	MMU_AT_DEBUG //used for emulator debugging functions (bypasses some debug handling)
};

static inline uint8_t T1ReadByte(uint8_t* const mem, const uint32_t addr)
{
   return mem[addr];
}

static inline uint16_t T1ReadWord_guaranteedAligned(void* const mem, const uint32_t addr)
{
	assert((addr&1)==0);
#ifdef WORDS_BIGENDIAN
   return (((uint8_t*)mem)[addr + 1] << 8) | ((uint8_t*)mem)[addr];
#else
   return *(uint16_t*)((uint8_t*)mem + addr);
#endif
}

static inline uint16_t T1ReadWord(void* const mem, const uint32_t addr)
{
#ifdef WORDS_BIGENDIAN
   return (((uint8_t*)mem)[addr + 1] << 8) | ((uint8_t*)mem)[addr];
#else
   return *((uint16_t *) ((uint8_t*)mem + addr));
#endif
}

static inline uint32_t T1ReadLong_guaranteedAligned(uint8_t* const  mem, const uint32_t addr)
{
	assert((addr&3)==0);
#ifdef WORDS_BIGENDIAN
   return  mem[addr + 3] << 24 | mem[addr + 2] << 16 |
           mem[addr + 1] << 8 | mem[addr];
#else
	return *(uint32_t*)(mem + addr);
#endif
}


static inline uint32_t T1ReadLong(uint8_t* const  mem, uint32_t addr)
{
   addr &= ~3;
#ifdef WORDS_BIGENDIAN
   return  mem[addr + 3] << 24 | mem[addr + 2] << 16 |
           mem[addr + 1] << 8 | mem[addr];
#else
   return *(uint32_t*)(mem + addr);
#endif
}

static inline uint64_t T1ReadQuad(uint8_t* const mem, const uint32_t addr)
{
#ifdef WORDS_BIGENDIAN
   return  uint64_t(mem[addr + 7]) << 56 | uint64_t(mem[addr + 6]) << 48 |
           uint64_t(mem[addr + 5]) << 40 | uint64_t(mem[addr + 4]) << 32 |
           uint64_t(mem[addr + 3]) << 24 | uint64_t(mem[addr + 2]) << 16 |
           uint64_t(mem[addr + 1]) << 8  | uint64_t(mem[addr    ]);
#else
   return *((uint64_t *) (mem + addr));
#endif
}

static inline void T1WriteByte(uint8_t* const mem, const uint32_t addr, const uint8_t val)
{
   mem[addr] = val;
}

static inline void T1WriteWord(uint8_t* const mem, const uint32_t addr, const uint16_t val)
{
#ifdef WORDS_BIGENDIAN
   mem[addr + 1] = val >> 8;
   mem[addr] = val & 0xFF;
#else
   *((uint16_t *) (mem + addr)) = val;
#endif
}

static inline void T1WriteLong(uint8_t* const mem, const uint32_t addr, const uint32_t val)
{
#ifdef WORDS_BIGENDIAN
   mem[addr + 3] = val >> 24;
   mem[addr + 2] = (val >> 16) & 0xFF;
   mem[addr + 1] = (val >> 8) & 0xFF;
   mem[addr] = val & 0xFF;
#else
   *((uint32_t *) (mem + addr)) = val;
#endif
}

static inline void T1WriteQuad(uint8_t* const mem, const uint32_t addr, const uint64_t val)
{
#ifdef WORDS_BIGENDIAN
	mem[addr + 7] = (val >> 56);
	mem[addr + 6] = (val >> 48) & 0xFF;
	mem[addr + 5] = (val >> 40) & 0xFF;
	mem[addr + 4] = (val >> 32) & 0xFF;
	mem[addr + 3] = (val >> 24) & 0xFF;
    mem[addr + 2] = (val >> 16) & 0xFF;
    mem[addr + 1] = (val >> 8) & 0xFF;
    mem[addr] = val & 0xFF;
#else
	*((uint64_t *) (mem + addr)) = val;
#endif
}

//static inline uint8_t T2ReadByte(uint8_t* const  mem, const uint32_t addr)
//{
//#ifdef WORDS_BIGENDIAN
//   return mem[addr ^ 1];
//#else
//   return mem[addr];
//#endif
//}
//

/*static inline uint16_t HostReadWord(uint8_t* const mem, const uint32_t addr)
{
   return *((uint16_t *) (mem + addr));
}*/

//
//static inline uint32_t T2ReadLong(uint8_t* const mem, const uint32_t addr)
//{
//#ifdef WORDS_BIGENDIAN
//   return *((uint16_t *) (mem + addr + 2)) << 16 | *((uint16_t *) (mem + addr));
//#else
//   return *((uint32_t *) (mem + addr));
//#endif
//}
//
//static inline void T2WriteByte(uint8_t* const mem, const uint32_t addr, const uint8_t val)
//{
//#ifdef WORDS_BIGENDIAN
//   mem[addr ^ 1] = val;
//#else
//   mem[addr] = val;
//#endif
//}

/*static inline void HostWriteWord(uint8_t* const mem, const uint32_t addr, const uint16_t val)
{
   *((uint16_t *) (mem + addr)) = val;
}*/

/*static inline void HostWriteTwoWords(uint8_t* const mem, const uint32_t addr, const uint32_t val)
{
#ifdef WORDS_BIGENDIAN
   *((uint16_t *) (mem + addr + 2)) = val >> 16;
   *((uint16_t *) (mem + addr)) = val & 0xFFFF;
#else
   *((uint32_t *) (mem + addr)) = val;
#endif
}*/

#endif
