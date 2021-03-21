/*
 * SSEQ Player - SDAT SBNK (Sound Bank) structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "INFOEntry.h"

struct PseudoFile;
struct SWAR;

struct SBNKInstrumentRange
{
	std::uint8_t lowNote;
	std::uint8_t highNote;
	std::uint16_t record;
	std::uint16_t swav;
	std::uint16_t swar;
	std::uint8_t noteNumber;
	std::uint8_t attackRate;
	std::uint8_t decayRate;
	std::uint8_t sustainLevel;
	std::uint8_t releaseRate;
	std::uint8_t pan;

	SBNKInstrumentRange(std::uint8_t lowerNote, std::uint8_t upperNote, int recordType);

	void Read(PseudoFile &file);
};

struct SBNKInstrument
{
	std::uint8_t record;
	std::vector<SBNKInstrumentRange> ranges;

	SBNKInstrument();

	void Read(PseudoFile &file, std::uint32_t startOffset);
};

struct SBNK
{
	std::string filename;
	std::vector<SBNKInstrument> instruments;

	const SWAR *waveArc[4];
	INFOEntryBANK info;

	SBNK(const std::string &fn = "");

	void Read(PseudoFile &file);
};
