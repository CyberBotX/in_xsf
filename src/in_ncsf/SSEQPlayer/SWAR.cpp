/*
 * SSEQ Player - SDAT SWAR (Wave Archive) structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#include <stdexcept>
#include <string>
#include <vector>
#include <cstdint>
#include "NDSStdHeader.h"
#include "SWAR.h"
#include "SWAV.h"
#include "common.h"

SWAR::SWAR(const std::string &fn) : filename(fn), swavs(), info()
{
}

void SWAR::Read(PseudoFile &file)
{
	std::uint32_t startOfSWAR = file.pos;
	NDSStdHeader header;
	header.Read(file);
	header.Verify("SWAR", 0x0100FEFF);
	std::int8_t type[4];
	file.ReadLE(type);
	if (!VerifyHeader(type, "DATA"))
		throw std::runtime_error("SWAR DATA structure invalid");
	file.ReadLE<std::uint32_t>(); // size
	std::uint32_t reserved[8];
	file.ReadLE(reserved);
	std::uint32_t count = file.ReadLE<std::uint32_t>();
	auto offsets = std::vector<std::uint32_t>(count);
	file.ReadLE(offsets);
	for (std::uint32_t i = 0; i < count; ++i)
		if (offsets[i])
		{
			file.pos = startOfSWAR + offsets[i];
			this->swavs[i] = SWAV();
			this->swavs[i].Read(file);
		}
}
