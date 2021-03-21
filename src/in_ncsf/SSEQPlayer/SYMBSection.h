/*
 * SSEQ Player - SDAT SYMB (Symbol/Filename) Section structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#pragma once

#include <map>
#include "common.h"

struct SYMBRecord
{
	std::map<uint32_t, std::string> entries;

	SYMBRecord();

	void Read(PseudoFile &file, uint32_t startOffset);
};

struct SYMBSection
{
	SYMBRecord SEQrecord;
	SYMBRecord BANKrecord;
	SYMBRecord WAVEARCrecord;
	SYMBRecord PLAYERrecord;

	SYMBSection();

	void Read(PseudoFile &file);
};
