/*
 * SSEQ Player - SDAT SWAR (Wave Archive) structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#pragma once

#include <map>
#include "SWAV.h"
#include "INFOEntry.h"
#include "common.h"

struct SWAR
{
	std::string filename;
	std::map<uint32_t, SWAV> swavs;

	INFOEntryWAVEARC info;

	SWAR(const std::string &fn = "");

	void Read(PseudoFile &file);
};
