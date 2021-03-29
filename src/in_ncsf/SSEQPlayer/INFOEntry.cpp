/*
 * SSEQ Player - SDAT INFO Entry structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#include <algorithm>
#include <cstdint>
#include "INFOEntry.h"
#include "common.h"

INFOEntrySEQ::INFOEntrySEQ() : fileID(0), bank(0), vol(0), ply(0)
{
}

void INFOEntrySEQ::Read(PseudoFile &file)
{
	this->fileID = file.ReadLE<std::uint32_t>();
	this->bank = file.ReadLE<std::uint16_t>();
	this->vol = file.ReadLE<std::uint8_t>();
	if (!this->vol)
		this->vol = 0x7F; // Prevents nothing for volume
	file.ReadLE<std::uint8_t>(); // cpr
	file.ReadLE<std::uint8_t>(); // ppr
	this->ply = file.ReadLE<std::uint8_t>();
}

INFOEntryBANK::INFOEntryBANK() : fileID(0)
{
	std::fill_n(&this->waveArc[0], 4, static_cast<std::uint16_t>(0));
}

void INFOEntryBANK::Read(PseudoFile &file)
{
	this->fileID = file.ReadLE<std::uint32_t>();
	file.ReadLE(this->waveArc);
}

INFOEntryWAVEARC::INFOEntryWAVEARC() : fileID(0)
{
}

void INFOEntryWAVEARC::Read(PseudoFile &file)
{
	this->fileID = file.ReadLE<std::uint32_t>();
}

INFOEntryPLAYER::INFOEntryPLAYER() : channelMask(0)
{
}

void INFOEntryPLAYER::Read(PseudoFile &file)
{
	file.ReadLE<std::uint16_t>(); // maxSeqs
	this->channelMask = file.ReadLE<std::uint16_t>();
	file.ReadLE<std::uint32_t>(); // heapSize
}
