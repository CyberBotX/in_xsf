/*
 * SSEQ Player - Track structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Adapted from source code of FeOS Sound System
 * By fincs
 * https://github.com/fincs/FSS
 */

#pragma once

#include <bitset>
#include <functional>
#include <cstdint>
#include "common.h"
#include "consts.h"

struct Player;

enum class StackType
{
	Call,
	Loop
};

struct StackValue
{
	StackType type;
	const std::uint8_t *dest;

	StackValue() : type(StackType::Call), dest(nullptr) { }
	StackValue(StackType newType, const std::uint8_t *newDest) : type(newType), dest(newDest) { }
};

struct Override
{
	bool overriding;
	int cmd;
	int value;
	int extraValue;

	Override() : overriding(false), cmd(0), value(0), extraValue(0) { }
	bool operator()() const { return this->overriding; }
	bool &operator()() { return this->overriding; }
	template<typename T> T val(const std::uint8_t **pData, std::function<int (const std::uint8_t **)> reader, bool returnExtra = false)
	{
		if (this->overriding)
			return static_cast<T>(returnExtra ? this->extraValue : this->value);
		else
			return static_cast<T>(reader(pData));
	}
};

struct Track
{
	std::int8_t trackId;

	std::bitset<ToIntegral(TrackState::Bits)> state;
	std::uint8_t num, prio;
	Player *ply;

	const std::uint8_t *startPos;
	const std::uint8_t *pos;
	StackValue stack[FSS_TRACKSTACKSIZE];
	std::uint8_t stackPos;
	std::uint8_t loopCount[FSS_TRACKSTACKSIZE];
	Override overriding;
	bool lastComparisonResult;

	int wait;
	std::uint16_t patch;
	std::uint8_t portaKey, portaTime;
	std::int16_t sweepPitch;
	std::uint8_t vol, expr;
	std::int8_t pan; // -64..63
	std::uint8_t pitchBendRange;
	std::int8_t pitchBend;
	std::int8_t transpose;

	std::uint8_t a, d, s, r;

	std::uint8_t modType, modSpeed, modDepth, modRange;
	std::uint16_t modDelay;

	std::bitset<ToIntegral(TrackUpdateFlag::Bits)> updateFlags;

	Track();

	void Init(std::uint8_t handle, Player *ply, const std::uint8_t *pos, std::uint8_t n);
	void Zero();
	void ClearState();
	void Free();
	int NoteOn(std::uint8_t key, int vel, int len);
	int NoteOnTie(std::uint8_t key, int vel);
	void ReleaseAllNotes();
	void Run();
};
