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


#ifndef _MEMMAP_H_
#define _MEMMAP_H_

#define MEMMAP_BLOCK_SIZE	(0x1000)
#define MEMMAP_NUM_BLOCKS	(0x1000000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_SHIFT		(12)
#define MEMMAP_MASK			(MEMMAP_BLOCK_SIZE - 1)

struct CMemory
{
	enum
	{ MAX_ROM_SIZE = 0x800000 };

	enum file_formats
	{ FILE_ZIP, FILE_JMA, FILE_DEFAULT };

	enum
	{ NOPE, YEAH, BIGFIRST, SMALLFIRST };

	enum
	{ MAP_TYPE_I_O, MAP_TYPE_ROM, MAP_TYPE_RAM };

	enum
	{
		MAP_CPU,
		MAP_PPU,
		MAP_LOROM_SRAM,
		MAP_LOROM_SRAM_B,
		MAP_HIROM_SRAM,
		MAP_DSP,
		MAP_SA1RAM,
		MAP_BWRAM,
		MAP_BWRAM_BITMAP,
		MAP_BWRAM_BITMAP2,
		MAP_SPC7110_ROM,
		MAP_SPC7110_DRAM,
		MAP_RONLY_SRAM,
		MAP_C4,
		MAP_OBC_RAM,
		MAP_SETA_DSP,
		MAP_SETA_RISC,
		MAP_BSX,
		MAP_NONE,
		MAP_LAST
	};

	uint8_t	NSRTHeader[32];
	int32_t	HeaderCount;

	uint8_t	*RAM;
	uint8_t	*ROM;
	uint8_t	*SRAM;
	uint8_t	*VRAM;
	uint8_t	*FillRAM;
	uint8_t	*BWRAM;
	uint8_t	*C4RAM;
	uint8_t	*OBC1RAM;
	uint8_t	*BSRAM;
	uint8_t	*BIOSROM;

	uint8_t	*Map[MEMMAP_NUM_BLOCKS];
	uint8_t	*WriteMap[MEMMAP_NUM_BLOCKS];
	uint8_t	BlockIsRAM[MEMMAP_NUM_BLOCKS];
	uint8_t	BlockIsROM[MEMMAP_NUM_BLOCKS];
	uint8_t	ExtendedFormat;

	char	ROMFilename[PATH_MAX + 1];
	char	ROMName[ROM_NAME_LEN];
	char	RawROMName[ROM_NAME_LEN];
	char	ROMId[5];
	int32_t	CompanyId;
	uint8_t	ROMRegion;
	uint8_t	ROMSpeed;
	uint8_t	ROMType;
	uint8_t	ROMSize;
	uint32_t	ROMChecksum;
	uint32_t	ROMComplementChecksum;
	uint32_t	ROMCRC32;
	int32_t	ROMFramesPerSecond;

	bool	HiROM;
	bool	LoROM;
	uint8_t	SRAMSize;
	uint32_t	SRAMMask;
	uint32_t	CalculatedSize;
	uint32_t	CalculatedChecksum;

	// ports can assign this to perform some custom action upon loading a ROM (such as adjusting controls)
	void	(*PostRomInitFunc)();

	bool	Init();
	void	Deinit();

	int		ScoreHiROM (bool, int32_t romoff = 0);
	int		ScoreLoROM (bool, int32_t romoff = 0);
	//uint32_t	HeaderRemove (uint32_t, int32_t &, uint8_t *);
	//uint32_t	FileLoader (uint8_t *, const char *, int32_t);
	/*bool	LoadROM (const char *);
	bool	LoadMultiCart (const char *, const char *);
	bool	LoadSufamiTurbo (const char *, const char *);
	bool	LoadSameGame (const char *, const char *);
	bool	LoadSRAM (const char *);
	bool	SaveSRAM (const char *);
	void	ClearSRAM (bool onlyNonSavedSRAM = 0);
	bool	LoadSRTC();
	bool	SaveSRTC();*/
	bool LoadROMSNSF(const unsigned char *, int32_t, const unsigned char *, int32_t);

	char *	Safe (const char *);
	char *	SafeANK (const char *);
	void	ParseSNESHeader (uint8_t *);
	void	InitROM();

	uint32_t	map_mirror (uint32_t, uint32_t);
	void	map_lorom (uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void	map_hirom (uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void	map_lorom_offset (uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void	map_hirom_offset (uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void	map_space (uint32_t, uint32_t, uint32_t, uint32_t, uint8_t *);
	void	map_index (uint32_t, uint32_t, uint32_t, uint32_t, int, int);
	void	map_System();
	void	map_WRAM();
	void	map_LoROMSRAM();
	void	map_HiROMSRAM();
	//void	map_DSP();
	//void	map_C4();
	//void	map_OBC1();
	//void	map_SetaRISC();
	//void	map_SetaDSP();
	void	map_WriteProtectROM();
	void	Map_Initialize();
	void	Map_LoROMMap();
	void	Map_NoMAD1LoROMMap();
	void	Map_JumboLoROMMap();
	void	Map_ROM24MBSLoROMMap();
	void	Map_SRAM512KLoROMMap();
	//void	Map_SufamiTurboLoROMMap();
	//void	Map_SufamiTurboPseudoLoROMMap();
	//void	Map_SuperFXLoROMMap();
	//void	Map_SetaDSPLoROMMap();
	void	Map_SDD1LoROMMap();
	//void	Map_SA1LoROMMap();
	void	Map_HiROMMap();
	void	Map_ExtendedHiROMMap();
	//void	Map_SameGameHiROMMap();
	//void	Map_SPC7110HiROMMap();

	uint16_t	checksum_calc_sum (uint8_t *, uint32_t);
	uint16_t	checksum_mirror_sum (uint8_t *, uint32_t &, uint32_t mask = 0x800000);
	void	Checksum_Calculate();

	bool	match_na (const char *);
	bool	match_nn (const char *);
	//bool	match_nc (const char *);
	bool	match_id (const char *);
	void	ApplyROMFixes();
	//void	CheckForAnyPatch (const char *, bool, int32_t &);

	//void	MakeRomInfoText (char *);

	//const char *	MapType();
	//const char *	StaticRAMSize();
	//const char *	Size();
	//const char *	Revision();
	//const char *	KartContents();
	//const char *	Country();
	//const char *	PublishingCompany();
};

struct SMulti
{
	int		cartType;
	int32_t	cartSizeA, cartSizeB;
	int32_t	sramSizeA, sramSizeB;
	uint32_t	sramMaskA, sramMaskB;
	uint32_t	cartOffsetA, cartOffsetB;
	uint8_t	*sramA, *sramB;
	char	fileNameA[PATH_MAX + 1], fileNameB[PATH_MAX + 1];
};

extern CMemory	Memory;
extern SMulti	Multi;

void S9xAutoSaveSRAM();
bool LoadZip(const char *, int32_t *, int32_t *, uint8_t *);

enum s9xwrap_t
{
	WRAP_NONE,
	WRAP_BANK,
	WRAP_PAGE
};

enum s9xwriteorder_t
{
	WRITE_01,
	WRITE_10
};

#include "getset.h"

#endif
