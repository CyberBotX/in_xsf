/*
 * SSEQ Player - SDAT SYMB (Symbol/Filename) Section structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#include <stdexcept>
#include <vector>
#include <cstdint>
#include "SYMBSection.h"
#include "common.h"

SYMBRecord::SYMBRecord() : entries()
{
}

void SYMBRecord::Read(PseudoFile &file, std::uint32_t startOffset)
{
	std::uint32_t count = file.ReadLE<std::uint32_t>();
	auto entryOffsets = std::vector<std::uint32_t>(count);
	file.ReadLE(entryOffsets);
	for (std::uint32_t i = 0; i < count; ++i)
		if (entryOffsets[i])
		{
			file.pos = startOffset + entryOffsets[i];
			this->entries[i] = file.ReadNullTerminatedString();
		}
}

SYMBSection::SYMBSection() : SEQrecord(), BANKrecord(), WAVEARCrecord(), PLAYERrecord()
{
}

void SYMBSection::Read(PseudoFile &file)
{
	std::uint32_t startOfSYMB = file.pos;
	std::int8_t type[4];
	file.ReadLE(type);
	if (!VerifyHeader(type, "SYMB"))
		throw std::runtime_error("SDAT SYMB Section invalid");
	file.ReadLE<std::uint32_t>(); // size
	std::uint32_t recordOffsets[8];
	file.ReadLE(recordOffsets);
	if (recordOffsets[REC_SEQ])
	{
		file.pos = startOfSYMB + recordOffsets[REC_SEQ];
		this->SEQrecord.Read(file, startOfSYMB);
	}
	if (recordOffsets[REC_BANK])
	{
		file.pos = startOfSYMB + recordOffsets[REC_BANK];
		this->BANKrecord.Read(file, startOfSYMB);
	}
	if (recordOffsets[REC_WAVEARC])
	{
		file.pos = startOfSYMB + recordOffsets[REC_WAVEARC];
		this->WAVEARCrecord.Read(file, startOfSYMB);
	}
	if (recordOffsets[REC_PLAYER])
	{
		file.pos = startOfSYMB + recordOffsets[REC_PLAYER];
		this->PLAYERrecord.Read(file, startOfSYMB);
	}
}
