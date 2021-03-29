/*
 * SSEQ Player - Channel structures
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Adapted from source code of FeOS Sound System
 * By fincs
 * https://github.com/fincs/FSS
 *
 * Some code/concepts from DeSmuME
 * http://desmume.org/
 */

#pragma once

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include "common.h"
#include "consts.h"

struct SWAV;
struct Track;

/*
 * This structure is meant to be similar to what is stored in the actual
 * Nintendo DS's sound registers. Items that were not being used by this
 * player have been removed, and items which help the simulated registers
 * have been added.
 */
struct NDSSoundRegister
{
	// Control Register
	std::uint8_t volumeMul;
	std::uint8_t volumeDiv;
	std::uint8_t panning;
	std::uint8_t waveDuty;
	std::uint8_t repeatMode;
	std::uint8_t format;
	bool enable;

	// Data Source Register
	const SWAV *source;

	// Timer Register
	std::uint16_t timer;

	// PSG Handling, not a DS register
	std::uint16_t psgX;
	std::int16_t psgLast;
	std::uint32_t psgLastCount;

	// The following are taken from DeSmuME
	double samplePosition;
	double sampleIncrease;

	// Loopstart Register
	std::uint32_t loopStart;

	// Length Register
	std::uint32_t length;

	std::uint32_t totalLength;

	NDSSoundRegister();

	void ClearControlRegister();
	void SetControlRegister(std::uint32_t reg);
};

/*
 * From FeOS Sound System, this is temporary storage of what will go into
 * the Nintendo DS sound registers. It is kept separate as the original code
 * from FeOS Sound System utilized this to hold data prior to passing it into
 * the DS's registers.
 */
struct TempSndReg
{
	std::uint32_t CR;
	const SWAV *SOURCE;
	std::uint16_t TIMER;
	std::uint32_t REPEAT_POINT, LENGTH;

	TempSndReg();
};

struct Player;

/*
 * This creates a ring buffer, which will store N samples of SWAV
 * data, duplicated. The way it is duplicated is done as follows:
 * the samples are stored in the center of the buffer, and on both
 * sides is half of the data, the first half being after the data
 * and the second half begin before the data. This in essence
 * mirrors the data while allowing a pointer to always be retrieved
 * and no extra copies of the buffer are created. Part of the idea
 * for this came from kode54's original buffer implementation, but
 * this has been designed to make sure that there are no delays in
 * accessing the SWAVs samples and also doesn't use 0s before the
 * start of the SWAV or use 0s after the end of a non-looping SWAV.
 */
template<std::size_t N> struct RingBuffer
{
	std::int16_t buffer[N * 2];
	std::size_t bufferPos, getPos;

	RingBuffer() : bufferPos(N / 2), getPos(N / 2)
	{
		std::fill_n(&this->buffer[0], N * 2, static_cast<std::int16_t>(0));
	}
	void Clear()
	{
		std::fill_n(&this->buffer[0], N * 2, static_cast<std::int16_t>(0));
		this->bufferPos = this->getPos = N / 2;
	}
	void PushSample(std::int16_t sample)
	{
		this->buffer[this->bufferPos] = sample;
		if (this->bufferPos >= N)
			this->buffer[this->bufferPos - N] = sample;
		else
			this->buffer[this->bufferPos + N] = sample;
		++this->bufferPos;
		if (this->bufferPos >= N * 3 / 2)
			this->bufferPos -= N;
	}
	void PushSamples(const std::int16_t *samples, std::size_t size)
	{
		if (this->bufferPos + size > N * 3 / 2)
		{
			std::size_t free = N * 3 / 2 - this->bufferPos;
			std::copy_n(&samples[0], free, &this->buffer[this->bufferPos]);
			std::copy(&samples[free], &samples[size], &this->buffer[N / 2]);
		}
		else
			std::copy_n(&samples[0], size, &this->buffer[this->bufferPos]);
		std::size_t rightFree = this->bufferPos < N ? N - this->bufferPos : 0;
		if (rightFree < size)
		{
			if (!rightFree)
			{
				std::size_t leftStart = this->bufferPos - N;
				std::size_t leftSize = std::min(N / 2 - leftStart, size);
				std::copy_n(&samples[0], leftSize, &this->buffer[leftStart]);
				if (leftSize < size)
					std::copy(&samples[leftSize], &samples[size], &this->buffer[N * 3 / 2]);
			}
			else
			{
				std::copy_n(&samples[0], rightFree, &this->buffer[this->bufferPos + N]);
				std::copy(&samples[rightFree], &samples[size], &this->buffer[0]);
			}
		}
		else
			std::copy_n(&samples[0], size, &this->buffer[this->bufferPos + N]);
		this->bufferPos += size;
		if (this->bufferPos >= N * 3 / 2)
			this->bufferPos -= N;
	}
	const std::int16_t *GetBuffer() const
	{
		return &this->buffer[this->getPos];
	}
	void NextSample()
	{
		++this->getPos;
		if (this->getPos >= N * 3 / 2)
			this->getPos -= N;
	}
};

struct Channel
{
	std::int8_t chnId;

	TempSndReg tempReg;
	ChannelState state;
	std::int8_t trackId; // -1 = none
	std::uint8_t prio;
	bool manualSweep;

	std::bitset<ToIntegral(ChannelFlag::Bits)> flags;
	std::int8_t pan; // -64 .. 63
	std::int16_t extAmpl;

	std::int16_t velocity;
	std::int8_t extPan;
	std::uint8_t key;

	int ampl; // 7 fractionary bits
	int extTune; // in 64ths of a semitone

	std::uint8_t orgKey;

	std::uint8_t modType, modSpeed, modDepth, modRange;
	std::uint16_t modDelay, modDelayCnt, modCounter;

	std::uint32_t sweepLen, sweepCnt;
	std::int16_t sweepPitch;

	std::uint8_t attackLvl, sustainLvl;
	std::uint16_t decayRate, releaseRate;

	/*
	 * These were originally global variables in FeOS Sound System, but
	 * since they were linked to a certain channel anyways, I moved them
	 * into this class.
	 */
	int noteLength;
	std::uint16_t vol;

	const Player *ply;
	NDSSoundRegister reg;

	/*
	 * Lookup tables for the Sinc interpolation, to
	 * avoid the need to call the sin/cos functions all the time.
	 * These are static as they will not change between channels or runs
	 * of the program.
	 */
	static bool initializedLUTs;
	static const unsigned SINC_RESOLUTION = 8192;
	static const unsigned SINC_WIDTH = 8;
	static const unsigned SINC_SAMPLES = SINC_RESOLUTION * SINC_WIDTH;
	static double sinc_lut[SINC_SAMPLES + 1];
	static double window_lut[SINC_SAMPLES + 1];

	RingBuffer<SINC_WIDTH * 2> ringBuffer;

	Channel();

	void UpdateVol(const Track &trk);
	void UpdatePan(const Track &trk);
	void UpdateTune(const Track &trk);
	void UpdateMod(const Track &trk);
	void UpdatePorta(const Track &trk);
	void Release();
	void Kill();
	void UpdateTrack();
	void Update();
	std::int32_t Interpolate();
	std::int32_t GenerateSample();
	void IncrementSample();
};
