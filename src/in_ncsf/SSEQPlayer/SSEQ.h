/*
 * SSEQ Player - SDAT SSEQ (Sequence) structure
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
struct SBNK;

struct SSEQ
{
	std::string filename;
	std::vector<std::uint8_t> data;

	const SBNK *bank;
	INFOEntrySEQ info;

	SSEQ(const std::string &fn = "");

	void Read(PseudoFile &file);
};
