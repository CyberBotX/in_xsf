/*
 * SSEQ Player - SDAT structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#include <stdexcept>
#include <string>
#include <cstdint>
#include "FATSection.h"
#include "INFOSection.h"
#include "NDSStdHeader.h"
#include "SBNK.h"
#include "SDAT.h"
#include "SSEQ.h"
#include "SWAR.h"
#include "SYMBSection.h"
#include "common.h"

SDAT::SDAT(PseudoFile &file, std::uint32_t sseqToLoad) : sseq(), sbnk(), player()
{
	// Read sections
	NDSStdHeader header;
	header.Read(file);
	header.Verify("SDAT", 0x0100FEFF);
	std::uint32_t SYMBOffset = file.ReadLE<std::uint32_t>();
	file.ReadLE<std::uint32_t>(); // SYMB size
	std::uint32_t INFOOffset = file.ReadLE<std::uint32_t>();
	file.ReadLE<std::uint32_t>(); // INFO size
	std::uint32_t FATOffset = file.ReadLE<std::uint32_t>();
	file.ReadLE<std::uint32_t>(); // FAT Size
	SYMBSection symbSection;
	if (SYMBOffset)
	{
		file.pos = SYMBOffset;
		symbSection.Read(file);
	}
	file.pos = INFOOffset;
	INFOSection infoSection;
	infoSection.Read(file);
	file.pos = FATOffset;
	FATSection fatSection;
	fatSection.Read(file);

	if (infoSection.SEQrecord.entries.empty())
		throw std::logic_error("No SSEQ records found in SDAT");

	if (!infoSection.SEQrecord.entries.count(sseqToLoad))
		throw std::range_error("SSEQ of " + std::to_string(sseqToLoad) + " is not found");

	// Read SSEQ
	if (infoSection.SEQrecord.entries.count(sseqToLoad))
	{
		std::uint32_t fileID = infoSection.SEQrecord.entries[sseqToLoad].fileID;
		std::string name = "SSEQ" + NumToHexString(fileID).substr(2);
		if (SYMBOffset)
			name = NumToHexString(sseqToLoad).substr(6) + " - " + symbSection.SEQrecord.entries[sseqToLoad];
		file.pos = fatSection.records[fileID].offset;
		SSEQ *newSSEQ = new SSEQ(name);
		newSSEQ->info = infoSection.SEQrecord.entries[sseqToLoad];
		newSSEQ->Read(file);
		this->sseq.reset(newSSEQ);

		// Read SBNK for this SSEQ
		std::uint16_t bank = newSSEQ->info.bank;
		fileID = infoSection.BANKrecord.entries[bank].fileID;
		name = "SBNK" + NumToHexString(fileID).substr(2);
		if (SYMBOffset)
			name = NumToHexString(bank).substr(2) + " - " + symbSection.BANKrecord.entries[bank];
		file.pos = fatSection.records[fileID].offset;
		SBNK *newSBNK = new SBNK(name);
		newSSEQ->bank = newSBNK;
		newSBNK->info = infoSection.BANKrecord.entries[bank];
		newSBNK->Read(file);
		this->sbnk.reset(newSBNK);

		// Read SWARs for this SBNK
		for (int i = 0; i < 4; ++i)
			if (newSBNK->info.waveArc[i] != 0xFFFF)
			{
				std::uint16_t waveArc = newSBNK->info.waveArc[i];
				fileID = infoSection.WAVEARCrecord.entries[waveArc].fileID;
				name = "SWAR" + NumToHexString(fileID).substr(2);
				if (SYMBOffset)
					name = NumToHexString(waveArc).substr(2) + " - " + symbSection.WAVEARCrecord.entries[waveArc];
				file.pos = fatSection.records[fileID].offset;
				SWAR *newSWAR = new SWAR(name);
				newSBNK->waveArc[i] = newSWAR;
				newSWAR->info = infoSection.WAVEARCrecord.entries[waveArc];
				newSWAR->Read(file);
				this->swar[i].reset(newSWAR);
			}
			else
				this->swar[i].release();

		// Get PLAYER for this SSEQ, if it exists
		if (!infoSection.PLAYERrecord.entries.empty())
			this->player = infoSection.PLAYERrecord.entries[this->sseq->info.ply];
		if (!this->player.channelMask)
			this->player.channelMask = 0xFFFF;
	}
}
