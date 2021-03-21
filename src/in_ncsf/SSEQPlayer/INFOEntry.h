/*
 * SSEQ Player - SDAT INFO Entry structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#pragma once

#include <cstdint>

struct PseudoFile;

struct INFOEntry
{
	virtual ~INFOEntry()
	{
	}

	virtual void Read(PseudoFile &file) = 0;
};

struct INFOEntrySEQ : INFOEntry
{
	std::uint32_t fileID;
	std::uint16_t bank;
	std::uint8_t vol;
	std::uint8_t ply;

	INFOEntrySEQ();

	void Read(PseudoFile &file);
};

struct INFOEntryBANK : INFOEntry
{
	std::uint32_t fileID;
	std::uint16_t waveArc[4];

	INFOEntryBANK();

	void Read(PseudoFile &file);
};

struct INFOEntryWAVEARC : INFOEntry
{
	std::uint32_t fileID;

	INFOEntryWAVEARC();

	void Read(PseudoFile &file);
};

struct INFOEntryPLAYER : INFOEntry
{
	std::uint16_t channelMask;

	INFOEntryPLAYER();

	void Read(PseudoFile &file);
};
