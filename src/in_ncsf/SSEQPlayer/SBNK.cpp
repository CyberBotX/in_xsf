/*
 * SSEQ Player - SDAT SBNK (Sound Bank) structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include "NDSStdHeader.h"
#include "SBNK.h"
#include "common.h"

SBNKInstrumentRange::SBNKInstrumentRange(std::uint8_t lowerNote, std::uint8_t upperNote, int recordType) : lowNote(lowerNote), highNote(upperNote),
	record(recordType), swav(0), swar(0), noteNumber(0), attackRate(0), decayRate(0), sustainLevel(0), releaseRate(0), pan(0)
{
}

void SBNKInstrumentRange::Read(PseudoFile &file)
{
	this->swav = file.ReadLE<std::uint16_t>();
	this->swar = file.ReadLE<std::uint16_t>();
	this->noteNumber = file.ReadLE<std::uint8_t>();
	this->attackRate = file.ReadLE<std::uint8_t>();
	this->decayRate = file.ReadLE<std::uint8_t>();
	this->sustainLevel = file.ReadLE<std::uint8_t>();
	this->releaseRate = file.ReadLE<std::uint8_t>();
	this->pan = file.ReadLE<std::uint8_t>();
}

SBNKInstrument::SBNKInstrument() : record(0), ranges()
{
}

void SBNKInstrument::Read(PseudoFile &file, std::uint32_t startOffset)
{
	this->record = file.ReadLE<std::uint8_t>();
	std::uint16_t offset = file.ReadLE<std::uint16_t>();
	file.ReadLE<std::uint8_t>();
	std::uint32_t endOfInst = file.pos;
	file.pos = startOffset + offset;
	if (this->record)
	{
		if (this->record == 16)
		{
			std::uint8_t lowNote = file.ReadLE<std::uint8_t>();
			std::uint8_t highNote = file.ReadLE<std::uint8_t>();
			std::uint8_t num = highNote - lowNote + 1;
			for (std::uint8_t i = 0; i < num; ++i)
			{
				std::uint16_t thisRecord = file.ReadLE<std::uint16_t>();
				auto range = SBNKInstrumentRange(lowNote + i, lowNote + i, thisRecord);
				range.Read(file);
				this->ranges.push_back(range);
			}
		}
		else if (this->record == 17)
		{
			std::uint8_t thisRanges[8];
			file.ReadLE(thisRanges);
			std::uint8_t i = 0;
			while (i < 8 && thisRanges[i])
			{
				std::uint16_t thisRecord = file.ReadLE<std::uint16_t>();
				std::uint8_t lowNote = i ? thisRanges[i - 1] + 1 : 0;
				std::uint8_t highNote = thisRanges[i];
				auto range = SBNKInstrumentRange(lowNote, highNote, thisRecord);
				range.Read(file);
				this->ranges.push_back(range);
				++i;
			}
		}
		else
		{
			auto range = SBNKInstrumentRange(0, 127, this->record);
			range.Read(file);
			this->ranges.push_back(range);
		}
	}
	file.pos = endOfInst;
}

SBNK::SBNK(const std::string &fn) : filename(fn), instruments(), info()
{
	std::fill_n(&this->waveArc[0], 4, nullptr);
}

void SBNK::Read(PseudoFile &file)
{
	std::uint32_t startOfSBNK = file.pos;
	NDSStdHeader header;
	header.Read(file);
	header.Verify("SBNK", 0x0100FEFF);
	std::int8_t type[4];
	file.ReadLE(type);
	if (!VerifyHeader(type, "DATA"))
		throw std::runtime_error("SBNK DATA structure invalid");
	file.ReadLE<std::uint32_t>(); // size
	std::uint32_t reserved[8];
	file.ReadLE(reserved);
	std::uint32_t count = file.ReadLE<std::uint32_t>();
	this->instruments.resize(count);
	for (std::uint32_t i = 0; i < count; ++i)
		this->instruments[i].Read(file, startOfSBNK);
}
