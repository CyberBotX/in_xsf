/*
 * SSEQ Player - SDAT INFO Section structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#pragma once

#include <map>
#include <type_traits>
#include <cstdint>
#include "INFOEntry.h"

struct PseudoFile;

template<typename T> struct INFORecord
{
	std::enable_if_t<std::is_base_of_v<INFOEntry, T>, std::map<std::uint32_t, T>> entries;

	INFORecord();

	void Read(PseudoFile &file, std::uint32_t startOffset);
};

struct INFOSection
{
	INFORecord<INFOEntrySEQ> SEQrecord;
	INFORecord<INFOEntryBANK> BANKrecord;
	INFORecord<INFOEntryWAVEARC> WAVEARCrecord;
	INFORecord<INFOEntryPLAYER> PLAYERrecord;

	INFOSection();

	void Read(PseudoFile &file);
};
