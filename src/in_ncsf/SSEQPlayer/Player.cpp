/*
 * SSEQ Player - Player structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Adapted from source code of FeOS Sound System
 * By fincs
 * https://github.com/fincs/FSS
 */

#include <algorithm>
#include <cstdint>
#include "Player.h"
#include "SSEQ.h"
#include "common.h"
#include "consts.h"
#include "convert.h"

Player::Player() : prio(0), nTracks(0), tempo(0), tempoCount(0), tempoRate(0), masterVol(0), sseqVol(0), sseq(nullptr), allowedChannels(0), sampleRate(0),
	interpolation(Interpolation::None)
{
	std::fill_n(&this->trackIds[0], FSS_TRACKCOUNT, static_cast<std::uint8_t>(0));
	for (std::int8_t i = 0; i < 16; ++i)
	{
		this->channels[i].chnId = i;
		this->channels[i].ply = this;
	}
	std::fill_n(&this->variables[0], 32, static_cast<std::int16_t>(-1));
}

// Original FSS Function: Player_Setup
bool Player::Setup(const SSEQ *sseqToPlay)
{
	this->sseq = sseqToPlay;

	int firstTrack = this->TrackAlloc();
	if (firstTrack == -1)
		return false;
	this->tracks[firstTrack].Init(static_cast<std::uint8_t>(firstTrack), this, nullptr, 0);

	this->nTracks = 1;
	this->trackIds[0] = static_cast<std::uint8_t>(firstTrack);

	this->tracks[firstTrack].startPos = this->tracks[firstTrack].pos = &this->sseq->data[0];

	this->ClearState();

	return true;
}

// Original FSS Function: Player_ClearState
void Player::ClearState()
{
	this->tempo = 120;
	this->tempoCount = 0;
	this->tempoRate = 0x100;
	this->masterVol = 0; // this is actually the highest level
	std::fill_n(&this->variables[0], 32, static_cast<std::int16_t>(-1));
}

// Original FSS Function: Player_FreeTracks
void Player::FreeTracks()
{
	for (std::uint8_t i = 0; i < this->nTracks; ++i)
		this->tracks[this->trackIds[i]].Free();
	this->nTracks = 0;
}

// Original FSS Function: Player_Stop
void Player::Stop(bool bKillSound)
{
	this->ClearState();
	for (std::uint8_t i = 0; i < this->nTracks; ++i)
	{
		std::uint8_t trackId = this->trackIds[i];
		this->tracks[trackId].ClearState();
		for (auto &chn : this->channels)
			if (chn.state != ChannelState::None && chn.trackId == trackId)
			{
				if (bKillSound)
					chn.Kill();
				else
					chn.Release();
			}
	}
	this->FreeTracks();
}

// Original FSS Function: Chn_Alloc
int Player::ChannelAlloc(ChannelAllocateType type, int priority)
{
	static const std::uint8_t pcmChnArray[] = { 4, 5, 6, 7, 2, 0, 3, 1, 8, 9, 10, 11, 14, 12, 15, 13 };
	static const std::uint8_t psgChnArray[] = { 8, 9, 10, 11, 12, 13 };
	static const std::uint8_t noiseChnArray[] = { 14, 15 };
	static const std::uint8_t arraySizes[] = { sizeof(pcmChnArray), sizeof(psgChnArray), sizeof(noiseChnArray) };
	static const std::uint8_t *const arrayArray[] = { pcmChnArray, psgChnArray, noiseChnArray };

	auto chnArray = arrayArray[ConvertFuncs::ToIntegral(type)];
	int arraySize = arraySizes[ConvertFuncs::ToIntegral(type)];

	int curChnNo = -1;
	for (int i = 0; i < arraySize; ++i)
	{
		int thisChnNo = chnArray[i];
		if (!this->allowedChannels[thisChnNo])
			continue;
		if (curChnNo != -1)
		{
			Channel &thisChn = this->channels[thisChnNo];
			Channel &curChn = this->channels[curChnNo];
			if (curChnNo != -1 && thisChn.prio >= curChn.prio)
			{
				if (thisChn.prio != curChn.prio)
					continue;
				if (curChn.vol <= thisChn.vol)
					continue;
			}
		}
		curChnNo = thisChnNo;
	}

	if (curChnNo == -1 || priority < this->channels[curChnNo].prio)
		return -1;
	this->channels[curChnNo].noteLength = -1;
	this->channels[curChnNo].vol = 0x7FF;
	return curChnNo;
}

// Original FSS Function: Track_Alloc
int Player::TrackAlloc()
{
	for (int i = 0; i < FSS_MAXTRACKS; ++i)
	{
		Track &thisTrk = this->tracks[i];
		if (!thisTrk.state[ConvertFuncs::ToIntegral(TrackState::AllocateBit)])
		{
			thisTrk.Zero();
			thisTrk.state.set(ConvertFuncs::ToIntegral(TrackState::AllocateBit));
			thisTrk.updateFlags.reset();
			return i;
		}
	}
	return -1;
}

// Original FSS Function: Player_Run
void Player::Run()
{
	while (this->tempoCount >= 240)
	{
		this->tempoCount -= 240;
		for (std::uint8_t i = 0; i < this->nTracks; ++i)
			this->tracks[this->trackIds[i]].Run();
	}
	this->tempoCount += (static_cast<int>(this->tempo) * static_cast<int>(this->tempoRate)) >> 8;
}

void Player::UpdateTracks()
{
	for (auto &chn : this->channels)
		chn.UpdateTrack();
	for (auto &trk : this->tracks)
		trk.updateFlags.reset();
}

// Original FSS Function: Snd_Timer
void Player::Timer()
{
	this->UpdateTracks();

	for (auto &chn : this->channels)
		chn.Update();

	this->Run();
}
