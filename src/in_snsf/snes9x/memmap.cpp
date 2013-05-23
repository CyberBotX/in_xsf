/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/

#include <vector>
#include <algorithm>
#include <string>
#include <numeric>
#include <cassert>
#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "sdd1.h"

static const uint32_t crc32Table[] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

// deinterleave

static void S9xDeinterleaveType1(int size, uint8_t *base)
{
	uint8_t blocks[256];
	int nblocks = size >> 16;

	for (int i = 0; i < nblocks; ++i)
	{
		blocks[i * 2] = i + nblocks;
		blocks[i * 2 + 1] = i;
	}

	for (int i = 0; i < nblocks * 2; ++i)
		for (int j = i; j < nblocks * 2; ++j)
			if (blocks[j] == i)
			{
				std::swap_ranges(&base[blocks[i] * 0x8000], &base[(blocks[i] + 1) * 0x8000], &base[blocks[j] * 0x8000]);
				std::swap(blocks[i], blocks[j]);
				break;
			}
}

// allocation and deallocation

bool CMemory::Init()
{
	this->RAM.reset(new uint8_t[0x20000]);
	this->SRAM.reset(new uint8_t[0x20000]);
	this->VRAM.reset(new uint8_t[0x10000]);
	this->RealROM.reset(new uint8_t[MAX_ROM_SIZE + 0x200 + 0x8000]);

	if (!this->RAM || !this->SRAM || !this->VRAM || !this->RealROM)
	{
		this->Deinit();
		return false;
	}

	std::fill(&this->RAM[0], &this->RAM[0x20000], 0);
	std::fill(&this->SRAM[0], &this->SRAM[0x20000], 0);
	std::fill(&this->VRAM[0], &this->VRAM[0x10000], 0);
	std::fill(&this->RealROM[0], &this->RealROM[MAX_ROM_SIZE + 0x200 + 0x8000], 0);

	// FillRAM uses first 32K of ROM image area, otherwise space just
	// wasted. Might be read by the SuperFX code.

	this->FillRAM = &this->RealROM[0];

	// Add 0x8000 to ROM image pointer to stop SuperFX code accessing
	// unallocated memory (can cause crash on some ports).

	this->ROM = &this->RealROM[0x8000];

	return true;
}

void CMemory::Deinit()
{
	this->RAM.reset();
	this->SRAM.reset();
	this->VRAM.reset();
	this->ROM = nullptr;
	this->RealROM.reset();

	this->Safe(nullptr);
}

// file management and ROM detection

static bool allASCII(uint8_t *b, int size)
{
	for (int i = 0; i < size; ++i)
		if (b[i] < 32 || b[i] > 126)
			return false;

	return true;
}

int CMemory::ScoreHiROM(bool skip_header, int32_t romoff)
{
	uint8_t *buf = &this->ROM[0xff00 + romoff + (skip_header ? 0x200 : 0)];
	int score = 0;

	if (buf[0xd5] & 0x1)
		score += 2;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score -= 2;

	if (buf[0xd4] == 0x20)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (buf[0xde] + (buf[0xdf] << 8))
			++score;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced after looking at a scan by Cowering

	if (this->CalculatedSize > 1024 * 1024 * 3)
		score += 4;

	if ((1 << (buf[0xd7] - 7)) > 48)
		--score;

	if (!allASCII(&buf[0xb0], 6))
		--score;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		--score;

	return score;
}

int CMemory::ScoreLoROM(bool skip_header, int32_t romoff)
{
	uint8_t *buf = &this->ROM[0x7f00 + romoff + (skip_header ? 0x200 : 0)];
	int score = 0;

	if (!(buf[0xd5] & 0x1))
		score += 3;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (buf[0xde] + (buf[0xdf] << 8))
			++score;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced per Cowering suggestion

	if (this->CalculatedSize <= 1024 * 1024 * 16)
		score += 2;

	if ((1 << (buf[0xd7] - 7)) > 48)
		--score;

	if (!allASCII(&buf[0xb0], 6))
		--score;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		--score;

	return score;
}

bool CMemory::LoadROMSNSF(const uint8_t *lrombuf, int32_t lromsize, const uint8_t *srambuf, int32_t sramsize)
{
	int retry_count = 0;

	std::fill(&this->ROM[0], &this->ROM[MAX_ROM_SIZE], 0);

again:
	this->CalculatedSize = 0;
	this->ExtendedFormat = NOPE;

	int32_t totalFileSize = std::min<int32_t>(MAX_ROM_SIZE, lromsize);
	if (!totalFileSize)
		return false;
	std::copy(&lrombuf[0], &lrombuf[totalFileSize], &this->ROM[0]);
	SNESGameFixes.SRAMInitialValue = 0xff;
	std::fill(&this->SRAM[0], &this->SRAM[0x20000], SNESGameFixes.SRAMInitialValue);
	if (srambuf && sramsize)
		std::copy(&srambuf[0], &srambuf[sramsize], &this->SRAM[0]);

	int hi_score = this->ScoreHiROM(false);
	int lo_score = this->ScoreLoROM(false);

	if (((hi_score > lo_score && this->ScoreHiROM(true) > hi_score) || (hi_score <= lo_score && this->ScoreLoROM(true) > lo_score)))
	{
		memmove(&this->ROM[0], &this->ROM[512], totalFileSize - 512);
		totalFileSize -= 512;
		// modifying ROM, so we need to rescore
		hi_score = this->ScoreHiROM(false);
		lo_score = this->ScoreLoROM(false);
	}

	this->CalculatedSize = (totalFileSize / 0x2000) * 0x2000;

	if (this->CalculatedSize > 0x400000 &&
		this->ROM[0x7fd5] + (this->ROM[0x7fd6] << 8) != 0x4332 && // exclude S-DD1
		this->ROM[0x7fd5] + (this->ROM[0x7fd6] << 8) != 0x4532 &&
		this->ROM[0xffd5] + (this->ROM[0xffd6] << 8) != 0xF93a && // exclude SPC7110
		this->ROM[0xffd5] + (this->ROM[0xffd6] << 8) != 0xF53a)
		this->ExtendedFormat = YEAH;

	// if both vectors are invalid, it's type 1 interleaved LoROM
	if (this->ExtendedFormat == NOPE && this->ROM[0x7ffc] + (this->ROM[0x7ffd] << 8) < 0x8000 && this->ROM[0xfffc] + (this->ROM[0xfffd] << 8) < 0x8000)
	{
		if (!Settings.ForceNotInterleaved)
			S9xDeinterleaveType1(totalFileSize, &this->ROM[0]);
	}

	// CalculatedSize is now set, so rescore
	hi_score = this->ScoreHiROM(false);
	lo_score = this->ScoreLoROM(false);

	uint8_t *RomHeader = &this->ROM[0];

	if (this->ExtendedFormat != NOPE)
	{
		int swappedhirom = this->ScoreHiROM(false, 0x400000);
		int swappedlorom = this->ScoreLoROM(false, 0x400000);

		// set swapped here
		if (std::max(swappedlorom, swappedhirom) >= std::max(lo_score, hi_score))
		{
			this->ExtendedFormat = BIGFIRST;
			hi_score = swappedhirom;
			lo_score = swappedlorom;
			RomHeader += 0x400000;
		}
		else
			this->ExtendedFormat = SMALLFIRST;
	}

	bool tales = false;

	bool interleaved = false;

	if (lo_score >= hi_score)
	{
		this->LoROM = true;
		this->HiROM = false;

		// ignore map type byte if not 0x2x or 0x3x
		if ((RomHeader[0x7fd5] & 0xf0) == 0x20 || (RomHeader[0x7fd5] & 0xf0) == 0x30)
		{
			switch (RomHeader[0x7fd5] & 0xf)
			{
				case 1:
					interleaved = true;
					break;

				case 5:
					interleaved = true;
					tales = true;
					break;
			}
		}
	}
	else
	{
		this->LoROM = false;
		this->HiROM = true;

		if ((RomHeader[0xffd5] & 0xf0) == 0x20 || (RomHeader[0xffd5] & 0xf0) == 0x30)
		{
			switch (RomHeader[0xffd5] & 0xf)
			{
				case 0:
				case 3:
					interleaved = true;
					break;
			}
		}
	}

	// this two games fail to be detected
	if (!strncmp(reinterpret_cast<char *>(&this->ROM[0x7fc0]), "YUYU NO QUIZ DE GO!GO!", 22) || !strncmp(reinterpret_cast<char *>(&this->ROM[0xffc0]), "BATMAN--REVENGE JOKER",  21))
	{
		this->LoROM = true;
		this->HiROM = false;
		interleaved = false;
		tales = false;
	}

	if (!Settings.ForceNotInterleaved && interleaved)
	{
		if (tales)
		{
			if (this->ExtendedFormat == BIGFIRST)
			{
				S9xDeinterleaveType1(0x400000, &this->ROM[0]);
				S9xDeinterleaveType1(this->CalculatedSize - 0x400000, &this->ROM[0x400000]);
			}
			else
			{
				S9xDeinterleaveType1(this->CalculatedSize - 0x400000, &this->ROM[0]);
				S9xDeinterleaveType1(0x400000, &this->ROM[this->CalculatedSize - 0x400000]);
			}

			this->LoROM = false;
			this->HiROM = true;
		}
		else
		{
			std::swap(this->LoROM, this->HiROM);
			S9xDeinterleaveType1(this->CalculatedSize, &this->ROM[0]);
		}

		hi_score = this->ScoreHiROM(false);
		lo_score = this->ScoreLoROM(false);

		if (((this->HiROM && (lo_score >= hi_score || hi_score < 0)) || (this->LoROM && (hi_score > lo_score || lo_score < 0))) && !retry_count)
		{
			Settings.ForceNotInterleaved = true;
			++retry_count;
			goto again;
		}
	}

	if (this->ExtendedFormat == SMALLFIRST)
		tales = true;

	if (tales)
	{
		auto tmp = std::vector<uint8_t>(this->CalculatedSize - 0x400000);
		memmove(&tmp[0], &this->ROM[0], this->CalculatedSize - 0x400000);
		memmove(&this->ROM[0], &this->ROM[this->CalculatedSize - 0x400000], 0x400000);
		memmove(&this->ROM[0x400000], &tmp[0], this->CalculatedSize - 0x400000);
	}

	memset(&SNESGameFixes, 0, sizeof(SNESGameFixes));
	SNESGameFixes.SRAMInitialValue = 0x60;

	this->InitROM();

	S9xReset();

	return true;
}

// initialization

char *CMemory::Safe(const char *s)
{
	static std::unique_ptr<char[]> safe;
	static int safe_len = 0;

	if (!s)
	{
		if (safe)
			safe.reset();

		return nullptr;
	}

	int len = strlen(s);
	if (!safe || len + 1 > safe_len)
	{
		safe_len = len + 1;
		safe.reset(new char[safe_len]);
	}

	for (int i = 0; i < len; ++i)
	{
		if (s[i] >= 32 && s[i] < 127)
			safe[i] = s[i];
		else
			safe[i] = '_';
	}

	safe[len] = 0;

	return safe.get();
}

void CMemory::ParseSNESHeader(uint8_t *RomHeader)
{
	strncpy(this->ROMName, reinterpret_cast<char *>(&RomHeader[0x10]), ROM_NAME_LEN - 1);

	this->SRAMSize = RomHeader[0x28];
	this->ROMSpeed = RomHeader[0x25];
	this->ROMType = RomHeader[0x26];
	this->ROMRegion = RomHeader[0x29];

	std::copy(&RomHeader[0x02], &RomHeader[0x06], &this->ROMId[0]);
}

void CMemory::InitROM()
{
	//// Parse ROM header and read ROM informatoin

	std::fill(&this->ROMId[0], &this->ROMId[5], 0);

	uint8_t *RomHeader = &this->ROM[0x7FB0];
	if (this->ExtendedFormat == BIGFIRST)
		RomHeader += 0x400000;
	if (this->HiROM)
		RomHeader += 0x8000;

	this->ParseSNESHeader(RomHeader);

	//// Detect and initialize chips
	//// detection codes are compatible with NSRT

	uint32_t identifier = ((this->ROMType & 0xff) << 8) + (this->ROMSpeed & 0xff);

	Settings.SDD1 = identifier == 0x4332 || identifier == 0x4532;

	//// Map memory and calculate checksum

	this->Map_Initialize();

	if (this->HiROM)
	{
		if (this->ExtendedFormat != NOPE)
			this->Map_ExtendedHiROMMap();
		else
			this->Map_HiROMMap();
	}
	else
	{
		if (Settings.SDD1)
			this->Map_SDD1LoROMMap();
		else if (this->ExtendedFormat != NOPE)
			this->Map_JumboLoROMMap();
		else if (!strncmp(this->ROMName, "WANDERERS FROM YS", 17))
			this->Map_NoMAD1LoROMMap();
		else if (!strncmp(this->ROMName, "SOUND NOVEL-TCOOL", 17) || !strncmp(this->ROMName, "DERBY STALLION 96", 17))
			this->Map_ROM24MBSLoROMMap();
		else if (!strncmp(this->ROMName, "THOROUGHBRED BREEDER3", 21) || !strncmp(this->ROMName, "RPG-TCOOL 2", 11))
			this->Map_SRAM512KLoROMMap();
		else
			this->Map_LoROMMap();
	}

	//// Build more ROM information

	// NTSC/PAL
	Settings.PAL = this->ROMRegion >= 2 && this->ROMRegion <= 12;

	// truncate cart name
	this->ROMName[ROM_NAME_LEN - 1] = 0;
	if (strlen(this->ROMName))
	{
		char *p = this->ROMName + strlen(this->ROMName);
		if (p > this->ROMName + 21 && this->ROMName[20] == ' ')
			p = this->ROMName + 21;
		while (p > this->ROMName && *(p - 1) == ' ')
			--p;
		*p = 0;
	}

	// SRAM size
	this->SRAMMask = this->SRAMSize ? ((1 << (this->SRAMSize + 3)) * 128) - 1 : 0;

	//// Initialize emulation

	Timings.H_Max_Master = SNES_CYCLES_PER_SCANLINE;
	Timings.H_Max = Timings.H_Max_Master;
	Timings.HBlankStart = SNES_HBLANK_START_HC;
	Timings.HBlankEnd = SNES_HBLANK_END_HC;
	Timings.HDMAInit = SNES_HDMA_INIT_HC;
	Timings.HDMAStart = SNES_HDMA_START_HC;
	Timings.RenderPos = SNES_RENDER_START_HC;
	Timings.V_Max_Master = Settings.PAL ? SNES_MAX_PAL_VCOUNTER : SNES_MAX_NTSC_VCOUNTER;
	Timings.V_Max = Timings.V_Max_Master;
	/* From byuu: The total delay time for both the initial (H)DMA sync (to the DMA clock),
	   and the end (H)DMA sync (back to the last CPU cycle's mcycle rate (6, 8, or 12)) always takes between 12-24 mcycles.
	   Possible delays: { 12, 14, 16, 18, 20, 22, 24 }
	   XXX: Snes9x can't emulate this timing :( so let's use the average value... */
	Timings.DMACPUSync = 18;
	/* If the CPU is halted (i.e. for DMA) while /NMI goes low, the NMI will trigger
	   after the DMA completes (even if /NMI goes high again before the DMA
	   completes). In this case, there is a 24-30 cycle delay between the end of DMA
	   and the NMI handler, time enough for an instruction or two. */
	// Wild Guns, Mighty Morphin Power Rangers - The Fighting Edition
	Timings.NMIDMADelay = 24;
	Timings.IRQPendCount = 0;

	//// Hack games

	this->ApplyROMFixes();

	sprintf(this->ROMName, "%s", Safe(this->ROMName));
	sprintf(this->ROMId, "%s", Safe(this->ROMId));

	Settings.ForceNotInterleaved = false;
}

// memory map

uint32_t CMemory::map_mirror(uint32_t size, uint32_t pos)
{
	// from bsnes
	if (!size)
		return 0;
	if (pos < size)
		return pos;

	uint32_t mask = 1 << 31;
	while (!(pos & mask))
		mask >>= 1;

	if (size <= (pos & mask))
		return this->map_mirror(size, pos - mask);
	else
		return mask + this->map_mirror(size - mask, pos - mask);
}

void CMemory::map_lorom(uint32_t bank_s, uint32_t bank_e, uint32_t addr_s, uint32_t addr_e, uint32_t size)
{
	for (uint32_t c = bank_s; c <= bank_e; ++c)
		for (uint32_t i = addr_s; i <= addr_e; i += 0x1000)
		{
			uint32_t p = (c << 4) | (i >> 12);
			uint32_t addr = (c & 0x7f) * 0x8000;
			this->Map[p] = &this->ROM[this->map_mirror(size, addr) - (i & 0x8000)];
			this->BlockIsROM[p] = true;
			this->BlockIsRAM[p] = false;
		}
}

void CMemory::map_hirom(uint32_t bank_s, uint32_t bank_e, uint32_t addr_s, uint32_t addr_e, uint32_t size)
{
	for (uint32_t c = bank_s; c <= bank_e; ++c)
		for (uint32_t i = addr_s; i <= addr_e; i += 0x1000)
		{
			uint32_t p = (c << 4) | (i >> 12);
			uint32_t addr = c << 16;
			this->Map[p] = &this->ROM[this->map_mirror(size, addr)];
			this->BlockIsROM[p] = true;
			this->BlockIsRAM[p] = false;
		}
}

void CMemory::map_lorom_offset(uint32_t bank_s, uint32_t bank_e, uint32_t addr_s, uint32_t addr_e, uint32_t size, uint32_t offset)
{
	for (uint32_t c = bank_s; c <= bank_e; ++c)
		for (uint32_t i = addr_s; i <= addr_e; i += 0x1000)
		{
			uint32_t p = (c << 4) | (i >> 12);
			uint32_t addr = ((c - bank_s) & 0x7f) * 0x8000;
			this->Map[p] = &this->ROM[offset + this->map_mirror(size, addr) - (i & 0x8000)];
			this->BlockIsROM[p] = true;
			this->BlockIsRAM[p] = false;
		}
}

void CMemory::map_hirom_offset(uint32_t bank_s, uint32_t bank_e, uint32_t addr_s, uint32_t addr_e, uint32_t size, uint32_t offset)
{
	for (uint32_t c = bank_s; c <= bank_e; ++c)
		for (uint32_t i = addr_s; i <= addr_e; i += 0x1000)
		{
			uint32_t p = (c << 4) | (i >> 12);
			uint32_t addr = (c - bank_s) << 16;
			this->Map[p] = &this->ROM[offset + this->map_mirror(size, addr)];
			this->BlockIsROM[p] = true;
			this->BlockIsRAM[p] = false;
		}
}

void CMemory::map_space(uint32_t bank_s, uint32_t bank_e, uint32_t addr_s, uint32_t addr_e, uint8_t *data)
{
	for (uint32_t c = bank_s; c <= bank_e; ++c)
		for (uint32_t i = addr_s; i <= addr_e; i += 0x1000)
		{
			uint32_t p = (c << 4) | (i >> 12);
			this->Map[p] = data;
			this->BlockIsROM[p] = false;
			this->BlockIsRAM[p] = true;
		}
}

void CMemory::map_index(uint32_t bank_s, uint32_t bank_e, uint32_t addr_s, uint32_t addr_e, int index, int type)
{
	bool isROM = !(type == MAP_TYPE_I_O || type == MAP_TYPE_RAM);
	bool isRAM = type != MAP_TYPE_I_O;

	for (uint32_t c = bank_s; c <= bank_e; ++c)
		for (uint32_t i = addr_s; i <= addr_e; i += 0x1000)
		{
			uint32_t p = (c << 4) | (i >> 12);
			this->Map[p] = reinterpret_cast<uint8_t *>(index);
			this->BlockIsROM[p] = isROM;
			this->BlockIsRAM[p] = isRAM;
		}
}

void CMemory::map_System()
{
	// will be overwritten
	this->map_space(0x00, 0x3f, 0x0000, 0x1fff, &this->RAM[0]);
	this->map_index(0x00, 0x3f, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	this->map_index(0x00, 0x3f, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
	this->map_space(0x80, 0xbf, 0x0000, 0x1fff, &this->RAM[0]);
	this->map_index(0x80, 0xbf, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	this->map_index(0x80, 0xbf, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
}

void CMemory::map_WRAM()
{
	// will overwrite others
	this->map_space(0x7e, 0x7e, 0x0000, 0xffff, &this->RAM[0]);
	this->map_space(0x7f, 0x7f, 0x0000, 0xffff, &this->RAM[0x10000]);
}

void CMemory::map_LoROMSRAM()
{
	this->map_index(0x70, 0x7f, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	this->map_index(0xf0, 0xff, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::map_HiROMSRAM()
{
	this->map_index(0x20, 0x3f, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	this->map_index(0xa0, 0xbf, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::map_WriteProtectROM()
{
	std::copy(&this->Map[0], &this->Map[0x1000], &this->WriteMap[0]);

	for (int c = 0; c < 0x1000; ++c)
		if (this->BlockIsROM[c])
			this->WriteMap[c] = reinterpret_cast<uint8_t *>(MAP_NONE);
}

void CMemory::Map_Initialize()
{
	for (int c = 0; c < 0x1000; ++c)
	{
		this->Map[c] = this->WriteMap[c] = reinterpret_cast<uint8_t *>(MAP_NONE);
		this->BlockIsROM[c] = this->BlockIsRAM[c] = false;
	}
}

void CMemory::Map_LoROMMap()
{
	this->map_System();

	this->map_lorom(0x00, 0x3f, 0x8000, 0xffff, this->CalculatedSize);
	this->map_lorom(0x40, 0x7f, 0x0000, 0xffff, this->CalculatedSize);
	this->map_lorom(0x80, 0xbf, 0x8000, 0xffff, this->CalculatedSize);
	this->map_lorom(0xc0, 0xff, 0x0000, 0xffff, this->CalculatedSize);

	this->map_LoROMSRAM();
	this->map_WRAM();

	this->map_WriteProtectROM();
}

void CMemory::Map_NoMAD1LoROMMap()
{
	this->map_System();

	this->map_lorom(0x00, 0x3f, 0x8000, 0xffff, this->CalculatedSize);
	this->map_lorom(0x40, 0x7f, 0x0000, 0xffff, this->CalculatedSize);
	this->map_lorom(0x80, 0xbf, 0x8000, 0xffff, this->CalculatedSize);
	this->map_lorom(0xc0, 0xff, 0x0000, 0xffff, this->CalculatedSize);

	this->map_index(0x70, 0x7f, 0x0000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	this->map_index(0xf0, 0xff, 0x0000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);

	this->map_WRAM();

	this->map_WriteProtectROM();
}

void CMemory::Map_JumboLoROMMap()
{
	// XXX: Which game uses this?
	this->map_System();

	this->map_lorom_offset(0x00, 0x3f, 0x8000, 0xffff, this->CalculatedSize - 0x400000, 0x400000);
	this->map_lorom_offset(0x40, 0x7f, 0x0000, 0xffff, this->CalculatedSize - 0x400000, 0x400000);
	this->map_lorom_offset(0x80, 0xbf, 0x8000, 0xffff, 0x400000, 0);
	this->map_lorom_offset(0xc0, 0xff, 0x0000, 0xffff, 0x400000, 0x200000);

	this->map_LoROMSRAM();
	this->map_WRAM();

	this->map_WriteProtectROM();
}

void CMemory::Map_ROM24MBSLoROMMap()
{
	// PCB: BSC-1A5M-01, BSC-1A7M-10
	this->map_System();

	this->map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x100000, 0);
	this->map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, 0x100000, 0x100000);
	this->map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x100000, 0x200000);
	this->map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, 0x100000, 0x100000);

	this->map_LoROMSRAM();
	this->map_WRAM();

	this->map_WriteProtectROM();
}

void CMemory::Map_SRAM512KLoROMMap()
{
	this->map_System();

	this->map_lorom(0x00, 0x3f, 0x8000, 0xffff, this->CalculatedSize);
	this->map_lorom(0x40, 0x7f, 0x0000, 0xffff, this->CalculatedSize);
	this->map_lorom(0x80, 0xbf, 0x8000, 0xffff, this->CalculatedSize);
	this->map_lorom(0xc0, 0xff, 0x0000, 0xffff, this->CalculatedSize);

	this->map_space(0x70, 0x70, 0x0000, 0xffff, &this->SRAM[0]);
	this->map_space(0x71, 0x71, 0x0000, 0xffff, &this->SRAM[0x8000]);
	this->map_space(0x72, 0x72, 0x0000, 0xffff, &this->SRAM[0x10000]);
	this->map_space(0x73, 0x73, 0x0000, 0xffff, &this->SRAM[0x18000]);

	this->map_WRAM();

	this->map_WriteProtectROM();
}

void CMemory::Map_SDD1LoROMMap()
{
	this->map_System();

	this->map_lorom(0x00, 0x3f, 0x8000, 0xffff, this->CalculatedSize);
	this->map_lorom(0x80, 0xbf, 0x8000, 0xffff, this->CalculatedSize);

	this->map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, this->CalculatedSize, 0);
	this->map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, this->CalculatedSize, 0); // will be overwritten dynamically

	this->map_index(0x70, 0x7f, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);

	this->map_WRAM();

	this->map_WriteProtectROM();
}

void CMemory::Map_HiROMMap()
{
	this->map_System();

	this->map_hirom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	this->map_hirom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	this->map_hirom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	this->map_hirom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	this->map_HiROMSRAM();
	this->map_WRAM();

	this->map_WriteProtectROM();
}

void CMemory::Map_ExtendedHiROMMap()
{
	this->map_System();

	this->map_hirom_offset(0x00, 0x3f, 0x8000, 0xffff, this->CalculatedSize - 0x400000, 0x400000);
	this->map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, this->CalculatedSize - 0x400000, 0x400000);
	this->map_hirom_offset(0x80, 0xbf, 0x8000, 0xffff, 0x400000, 0);
	this->map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, 0x400000, 0);

	this->map_HiROMSRAM();
	this->map_WRAM();

	this->map_WriteProtectROM();
}

// hack

bool CMemory::match_na(const char *str)
{
	return !strcmp(this->ROMName, str);
}

bool CMemory::match_nn(const char *str)
{
	return !strncmp(this->ROMName, str, strlen(str));
}

bool CMemory::match_id(const char *str)
{
	return !strncmp(this->ROMId, str, strlen(str));
}

void CMemory::ApplyROMFixes()
{
	Settings.BlockInvalidVRAMAccess = Settings.BlockInvalidVRAMAccessMaster;

	//// APU timing hacks :(

	Timings.APUSpeedup = 0;
	Timings.APUAllowTimeOverflow = false;

	if (!Settings.DisableGameSpecificHacks)
	{
		if (this->match_id("AVCJ")) // Rendering Ranger R2
			Timings.APUSpeedup = 4;

		if (this->match_na("GAIA GENSOUKI 1 JPN") || // Gaia Gensouki
			this->match_id("JG  ") || // Illusion of Gaia
			this->match_id("CQ  ") || // Stunt Race FX
			this->match_na("SOULBLADER - 1") || // Soul Blader
			this->match_na("SOULBLAZER - 1 USA") || // Soul Blazer
			this->match_na("SLAP STICK 1 JPN") || // Slap Stick
			this->match_id("E9 ") || // Robotrek
			this->match_nn("ACTRAISER") || // Actraiser
			this->match_nn("ActRaiser-2") || // Actraiser 2
			this->match_id("AQT") || // Tenchi Souzou, Terranigma
			this->match_id("ATV") || // Tales of Phantasia
			this->match_id("ARF") || // Star Ocean
			this->match_id("APR") || // Zen-Nippon Pro Wrestling 2 - 3-4 Budoukan
			this->match_id("A4B") || // Super Bomberman 4
			this->match_id("Y7 ") || // U.F.O. Kamen Yakisoban - Present Ban
			this->match_id("Y9 ") || // U.F.O. Kamen Yakisoban - Shihan Ban
			this->match_id("APB") || // Super Bomberman - Panic Bomber W
			this->match_na("DARK KINGDOM") || // Dark Kingdom
			this->match_na("ZAN3 SFC") || // Zan III Spirits
			this->match_na("HIOUDEN") || // Hiouden - Mamono-tachi Tono Chikai
			this->match_na("\xC3\xDD\xBC\xC9\xB3\xC0") || // Tenshi no Uta
			this->match_na("FORTUNE QUEST") || // Fortune Quest - Dice wo Korogase
			this->match_na("FISHING TO BASSING") || // Shimono Masaki no Fishing To Bassing
			this->match_na("OHMONO BLACKBASS") || // Oomono Black Bass Fishing - Jinzouko Hen
			this->match_na("MASTERS") || // Harukanaru Augusta 2 - Masters
			this->match_na("SFC \xB6\xD2\xDD\xD7\xB2\xC0\xDE\xB0") || // Kamen Rider
			this->match_na("ZENKI TENCHIMEIDOU") || // Kishin Douji Zenki - Tenchi Meidou
			this->match_nn("TokyoDome '95Battle 7") || // Shin Nippon Pro Wrestling Kounin '95 - Tokyo Dome Battle 7
			this->match_nn("SWORD WORLD SFC") || // Sword World SFC/2
			this->match_nn("LETs PACHINKO(") || // BS Lets Pachinko Nante Gindama 1/2/3/4
			this->match_nn("THE FISHING MASTER") || // Mark Davis The Fishing Master
			this->match_nn("Parlor") || // Parlor mini/2/3/4/5/6/7, Parlor Parlor!/2/3/4/5
			this->match_na("HEIWA Parlor!Mini8") || // Parlor mini 8
			this->match_nn("SANKYO Fever! \xCC\xA8\xB0\xCA\xDE\xB0!")) // SANKYO Fever! Fever!
			Timings.APUSpeedup = 1;

		if (this->match_na("EARTHWORM JIM 2") || // Earthworm Jim 2
			this->match_na("NBA Hangtime") || // NBA Hang Time
			this->match_na("MSPACMAN") || // Ms Pacman
			this->match_na("THE MASK") || // The Mask
			this->match_na("PRIMAL RAGE") || // Primal Rage
			this->match_na("DOOM TROOPERS")) // Doom Troopers
			Timings.APUAllowTimeOverflow = true;
	}

	S9xAPUTimingSetSpeedup(Timings.APUSpeedup);
	S9xAPUAllowTimeOverflow(Timings.APUAllowTimeOverflow);

	//// Other timing hacks :(

	Timings.HDMAStart = SNES_HDMA_START_HC + Settings.HDMATimingHack - 100;
	Timings.HBlankStart = SNES_HBLANK_START_HC + Timings.HDMAStart - SNES_HDMA_START_HC;
	Timings.IRQTriggerCycles = 10;

	if (!Settings.DisableGameSpecificHacks)
	{
		// The delay to sync CPU and DMA which Snes9x cannot emulate.
		// Some games need really severe delay timing...
		if (this->match_na("BATTLE GRANDPRIX")) // Battle Grandprix
			Timings.DMACPUSync = 20;

		// An infinite loop reads $4212 and waits V-blank end, whereas VIRQ is set V=0.
		// If Snes9x succeeds to escape from the loop before jumping into the IRQ handler, the game goes further.
		// If Snes9x jumps into the IRQ handler before escaping from the loop,
		// Snes9x cannot escape from the loop permanently because the RTI is in the next V-blank.
		if (this->match_na("Aero the AcroBat 2"))
			Timings.IRQPendCount = 2;

		// XXX: What's happening?
		if (this->match_na("X-MEN")) // Spider-Man and the X-Men
			Settings.BlockInvalidVRAMAccess = false;

		//// SRAM initial value

		if (this->match_na("HITOMI3"))
		{
			SRAMSize = 1;
			SRAMMask = ((1 << (SRAMSize + 3)) * 128) - 1;
		}

		// SRAM value fixes
		if (this->match_na("SUPER DRIFT OUT") || // Super Drift Out
			this->match_na("SATAN IS OUR FATHER!") ||
			this->match_na("goemon 4")) // Ganbare Goemon Kirakira Douchuu
			SNESGameFixes.SRAMInitialValue = 0x00;

		// Additional game fixes by sanmaiwashi ...
		// XXX: unnecessary?
		if (this->match_na("SFX \xC5\xB2\xC4\xB6\xDE\xDD\xC0\xDE\xD1\xD3\xC9\xB6\xDE\xC0\xD8 1")) // SD Gundam Gaiden - Knight Gundam Monogatari
			SNESGameFixes.SRAMInitialValue = 0x6b;

		// others: BS and ST-01x games are 0x00.

		//// OAM hacks :(

		// OAM hacks because we don't fully understand the behavior of the SNES.
		// Totally wacky display in 2P mode...
		// seems to need a disproven behavior, so we're definitely overlooking some other bug?
		if (match_nn("UNIRACERS")) // Uniracers
			SNESGameFixes.Uniracers = true;
	}
}
