/*
 * SSEQ Player - Nintendo DS Standard Header structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include "NDSStdHeader.h"
#include "common.h"

NDSStdHeader::NDSStdHeader() : magic(0)
{
	std::fill_n(&this->type[0], 4, 0);
}

void NDSStdHeader::Read(PseudoFile &file)
{
	file.ReadLE(this->type);
	this->magic = file.ReadLE<std::uint32_t>();
	file.ReadLE<std::uint32_t>(); // file size
	file.ReadLE<std::uint16_t>(); // structure size
	file.ReadLE<std::uint16_t>(); // # of blocks
}

void NDSStdHeader::Verify(const std::string &typeToCheck, std::uint32_t magicToCheck)
{
	if (!VerifyHeader(this->type, typeToCheck) || this->magic != magicToCheck)
		throw std::runtime_error("NDS Standard Header for " + typeToCheck + " invalid");
}
