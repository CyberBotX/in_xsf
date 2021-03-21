/*
 * SSEQ Player - SDAT SWAV (Waveform/Sample) structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Nintendo DS Nitro Composer (SDAT) Specification document found at
 * http://www.feshrine.net/hacking/doc/nds-sdat.html
 */

#pragma once

#include <vector>
#include <cstdint>

struct PseudoFile;

struct SWAV
{
	std::uint8_t waveType;
	std::uint8_t loop;
	std::uint16_t sampleRate;
	std::uint16_t time;
	std::uint32_t loopOffset;
	std::uint32_t nonLoopLength;
	std::vector<std::int16_t> data;
	const std::int16_t *dataptr;

	SWAV();

	void Read(PseudoFile &file);
	void DecodeADPCM(const std::uint8_t *origData, std::uint32_t len);
};
