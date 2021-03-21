/*
 * SSEQ Player - Nintendo DS Standard Header structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#pragma once

#include <string>
#include <cstdint>

struct PseudoFile;

struct NDSStdHeader
{
	std::int8_t type[4];
	std::uint32_t magic;

	NDSStdHeader();

	void Read(PseudoFile &file);
	void Verify(const std::string &typeToCheck, std::uint32_t magicToCheck);
};
