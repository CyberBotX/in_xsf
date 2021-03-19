/*
 * SSEQ Player - SDAT SSEQ (Sequence) structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#include <stdexcept>
#include "SSEQ.h"
#include "NDSStdHeader.h"

SSEQ::SSEQ(const std::string &fn) : filename(fn), data(), bank(nullptr), info()
{
}

void SSEQ::Read(PseudoFile &file)
{
	uint32_t startOfSSEQ = file.pos;
	NDSStdHeader header;
	header.Read(file);
	header.Verify("SSEQ", 0x0100FEFF);
	int8_t type[4];
	file.ReadLE(type);
	if (!VerifyHeader(type, "DATA"))
		throw std::runtime_error("SSEQ DATA structure invalid");
	uint32_t size = file.ReadLE<uint32_t>();
	uint32_t dataOffset = file.ReadLE<uint32_t>();
	this->data.resize(size - 12, 0);
	file.pos = startOfSSEQ + dataOffset;
	file.ReadLE(this->data);
}
