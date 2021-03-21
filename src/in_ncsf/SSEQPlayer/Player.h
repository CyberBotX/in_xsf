/*
 * SSEQ Player - Player structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Adapted from source code of FeOS Sound System
 * By fincs
 * https://github.com/fincs/FSS
 */

#pragma once

#include <bitset>
#include <cstdint>
#include "Channel.h"
#include "Track.h"
#include "consts.h"

enum class ChannelAllocateType;
struct SSEQ;

struct Player
{
	std::uint8_t prio, nTracks;
	std::uint16_t tempo, tempoCount, tempoRate /* 8.8 fixed point */;
	std::int16_t masterVol, sseqVol;

	const SSEQ *sseq;

	std::uint8_t trackIds[FSS_TRACKCOUNT];
	Track tracks[FSS_MAXTRACKS];
	Channel channels[16];
	std::bitset<16> allowedChannels;
	std::int16_t variables[32];

	std::uint32_t sampleRate;
	Interpolation interpolation;

	Player();

	bool Setup(const SSEQ *sseq);
	void ClearState();
	void FreeTracks();
	void Stop(bool bKillSound);
	int ChannelAlloc(ChannelAllocateType type, int prio);
	int TrackAlloc();
	void Run();
	void UpdateTracks();
	void Timer();
};
