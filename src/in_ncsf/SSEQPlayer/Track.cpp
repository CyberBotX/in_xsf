/*
 * SSEQ Player - Track structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Adapted from source code of FeOS Sound System
 * By fincs
 * https://github.com/fincs/FSS
 */

#include <algorithm>
#include <functional>
#include <cstddef>
#include <cstdint>
#include "Player.h"
#include "SBNK.h"
#include "SSEQ.h"
#include "SWAR.h"
#include "SWAV.h"
#include "Track.h"
#include "common.h"
#include "consts.h"
#include "convert.h"

Track::Track()
{
	this->Zero();
}

// Original FSS Function: Player_InitTrack
void Track::Init(std::uint8_t handle, Player *player, const std::uint8_t *dataPos, std::uint8_t n)
{
	this->trackId = handle;
	this->num = n;
	this->ply = player;
	this->startPos = dataPos;
	this->ClearState();
}

void Track::Zero()
{
	this->trackId = -1;

	this->state.reset();
	this->num = this->prio = 0;
	this->ply = nullptr;

	this->startPos = this->pos = nullptr;
	std::fill_n(&this->stack[0], FSS_TRACKSTACKSIZE, StackValue());
	this->stackPos = 0;
	std::fill_n(&this->loopCount[0], FSS_TRACKSTACKSIZE, static_cast<std::uint8_t>(0));
	this->overriding() = false;
	this->lastComparisonResult = true;

	this->wait = 0;
	this->patch = 0;
	this->portaKey = this->portaTime = 0;
	this->sweepPitch = 0;
	this->vol = this->expr = 0;
	this->pan = 0;
	this->pitchBendRange = 0;
	this->pitchBend = this->transpose = 0;

	this->a = this->d = this->s = this->r = 0;

	this->modType = this->modSpeed = this->modDepth = this->modRange = 0;
	this->modDelay = 0;

	this->updateFlags.reset();
}

// Original FSS Function: Track_ClearState
void Track::ClearState()
{
	this->state.reset();
	this->state.set(ConvertFuncs::ToIntegral(TrackState::AllocateBit));
	this->state.set(ConvertFuncs::ToIntegral(TrackState::NoteWait));
	this->prio = this->ply->prio + 64;

	this->pos = this->startPos;
	this->stackPos = 0;

	this->wait = 0;
	this->patch = 0;
	this->portaKey = 60;
	this->portaTime = 0;
	this->sweepPitch = 0;
	this->vol = this->expr = 127;
	this->pan = 0;
	this->pitchBendRange = 2;
	this->pitchBend = this->transpose = 0;

	this->a = this->d = this->s = this->r = 0xFF;

	this->modType = 0;
	this->modRange = 1;
	this->modSpeed = 16;
	this->modDelay = 0;
	this->modDepth = 0;
}

// Original FSS Function: Track_Free
void Track::Free()
{
	this->state.reset();
	this->updateFlags.reset();
}

// Original FSS Function: Note_On
int Track::NoteOn(std::uint8_t key, int vel, int len)
{
	auto sbnk = this->ply->sseq->bank;

	if (this->patch >= sbnk->entries.size())
		return -1;

	bool bIsPCM = true;
	Channel *chn = nullptr;
	int nCh = -1;

	auto &entry = sbnk->entries[this->patch];
	const SBNKInstrument *noteDef = nullptr;
	int fRecord = entry.record;

	if (fRecord == 16)
	{
		if (!(entry.instruments[0].lowNote <= key && key <= entry.instruments[entry.instruments.size() - 1].highNote))
			return -1;
		int rn = key - entry.instruments[0].lowNote;
		noteDef = &entry.instruments[rn];
		fRecord = noteDef->record;
	}
	else if (fRecord == 17)
	{
		std::size_t reg, entries;
		for (reg = 0, entries = entry.instruments.size(); reg < entries; ++reg)
			if (key <= entry.instruments[reg].highNote)
				break;
		if (reg == entries)
			return -1;

		noteDef = &entry.instruments[reg];
		fRecord = noteDef->record;
	}

	if (!fRecord)
		return -1;
	else if (fRecord == 1)
	{
		if (!noteDef)
			noteDef = &entry.instruments[0];
	}
	else if (fRecord < 4)
	{
		// PSG
		// fRecord = 2 -> PSG tone, pNoteDef->wavid -> PSG duty
		// fRecord = 3 -> PSG noise
		bIsPCM = false;
		if (!noteDef)
			noteDef = &entry.instruments[0];
		if (fRecord == 3)
		{
			nCh = this->ply->ChannelAlloc(ChannelAllocateType::Noise, this->prio);
			if (nCh < 0)
				return -1;
			chn = &this->ply->channels[nCh];
			chn->tempReg.CR = SOUND_FORMAT_PSG | SCHANNEL_ENABLE;
		}
		else
		{
			nCh = this->ply->ChannelAlloc(ChannelAllocateType::PSG, this->prio);
			if (nCh < 0)
				return -1;
			chn = &this->ply->channels[nCh];
			chn->tempReg.CR = SOUND_FORMAT_PSG | SCHANNEL_ENABLE | SOUND_DUTY(noteDef->swav & 0x7);
		}
		chn->tempReg.TIMER = static_cast<std::uint16_t>(-SOUND_FREQ(262 * 8)); // key #60 (C4)
		chn->reg.samplePosition = -1;
		chn->reg.psgX = 0x7FFF;
	}

	if (bIsPCM)
	{
		nCh = this->ply->ChannelAlloc(ChannelAllocateType::PCM, this->prio);
		if (nCh < 0)
			return -1;
		chn = &this->ply->channels[nCh];

		auto swav = &sbnk->waveArc[noteDef->swar]->swavs.find(noteDef->swav)->second;
		chn->tempReg.CR = SOUND_FORMAT(swav->waveType & 3) | SOUND_LOOP(!!swav->loop) | SCHANNEL_ENABLE;
		chn->tempReg.SOURCE = swav;
		chn->tempReg.TIMER = swav->time;
		chn->tempReg.REPEAT_POINT = swav->loopOffset;
		chn->tempReg.LENGTH = swav->nonLoopLength;
		chn->reg.samplePosition = -3;
	}

	chn->state = ChannelState::Start;
	chn->trackId = this->trackId;
	chn->flags.reset();
	chn->prio = this->prio;
	chn->key = key;
	chn->orgKey = noteDef->noteNumber;
	chn->velocity = Cnv_Sust(vel);
	chn->pan = static_cast<int>(noteDef->pan) - 64;
	chn->modDelayCnt = 0;
	chn->modCounter = 0;
	chn->noteLength = len;
	chn->reg.sampleIncrease = 0;

	chn->attackLvl = Cnv_Attack(this->a == 0xFF ? noteDef->attackRate : this->a);
	chn->decayRate = Cnv_Fall(this->d == 0xFF ? noteDef->decayRate : this->d);
	chn->sustainLvl = this->s == 0xFF ? noteDef->sustainLevel : this->s;
	chn->releaseRate = Cnv_Fall(this->r == 0xFF ? noteDef->releaseRate : this->r);

	chn->UpdateVol(*this);
	chn->UpdatePan(*this);
	chn->UpdateTune(*this);
	chn->UpdateMod(*this);
	chn->UpdatePorta(*this);

	this->portaKey = key;

	return nCh;
}

// Original FSS Function: Note_On_Tie
int Track::NoteOnTie(std::uint8_t key, int vel)
{
	// Find an existing note
	int i;
	Channel *chn = nullptr;
	for (i = 0; i < 16; ++i)
	{
		chn = &this->ply->channels[i];
		if (chn->state > ChannelState::None && chn->trackId == this->trackId && chn->state != ChannelState::Release)
			break;
	}

	if (i == 16)
		// Can't find note -> create an endless one
		return this->NoteOn(key, vel, -1);

	chn->flags.reset();
	chn->prio = this->prio;
	chn->key = key;
	chn->velocity = Cnv_Sust(vel);
	chn->modDelayCnt = 0;
	chn->modCounter = 0;

	chn->UpdateVol(*this);
	//chn->UpdatePan(*this);
	chn->UpdateTune(*this);
	chn->UpdateMod(*this);
	chn->UpdatePorta(*this);

	this->portaKey = key;
	chn->flags.set(ConvertFuncs::ToIntegral(ChannelFlag::UpdateTimer));

	return i;
}

// Original FSS Function: Track_ReleaseAllNotes
void Track::ReleaseAllNotes()
{
	for (auto &chn : this->ply->channels)
		if (chn.state > ChannelState::None && chn.trackId == this->trackId && chn.state != ChannelState::Release)
			chn.Release();
}

enum class SSEQCommand
{
	AllocateTrack = 0xFE, // Silently ignored
	OpenTrack = 0x93,

	Rest = 0x80,
	Patch = 0x81,
	Pan = 0xC0,
	Volume = 0xC1,
	MasterVolume = 0xC2,
	Priority = 0xC6,
	NoteWait = 0xC7,
	Tie = 0xC8,
	Expression = 0xD5,
	Tempo = 0xE1,
	End = 0xFF,

	Goto = 0x94,
	Call = 0x95,
	Return = 0xFD,
	LoopStart = 0xD4,
	LoopEnd = 0xFC,

	Transpose = 0xC3,
	PitchBend = 0xC4,
	PitchBendRange = 0xC5,

	Attack = 0xD0,
	Decay = 0xD1,
	Sustain = 0xD2,
	Release = 0xD3,

	PortamentoKey = 0xC9,
	PortamentoFlag = 0xCE,
	PortamentoTime = 0xCF,
	SweepPitch = 0xE3,

	ModulationDepth = 0xCA,
	ModulationSpeed = 0xCB,
	ModulationType = 0xCC,
	ModulationRange = 0xCD,
	ModulationDelay = 0xE0,

	Random = 0xA0,
	PrintVariable = 0xD6,
	If = 0xA2,
	FromVariable = 0xA1,
	SetVariable = 0xB0,
	AddVariable = 0xB1,
	SubtractVariable = 0xB2,
	MultiplyVariable = 0xB3,
	DivideVariable = 0xB4,
	ShiftVariable = 0xB5,
	RandomVariable = 0xB6,
	CompareEqualTo = 0xB8,
	CompareGreaterThanOrEqualTo = 0xB9,
	CompareGreaterThan = 0xBA,
	CompareLessThanOrEqualTo = 0xBB,
	CompareLessThan = 0xBC,
	CompareNotEqualTo = 0xBD,

	Mute = 0xD7 // Unsupported
};

static const std::uint8_t VariableByteCount = 1 << 7;
static const std::uint8_t ExtraByteOnNoteOrVarOrCmp = 1 << 6;

static inline std::uint8_t SseqCommandByteCount(int cmd)
{
	if (cmd < 0x80)
		return 1 | VariableByteCount;
	else
		switch (static_cast<SSEQCommand>(cmd))
		{
			case SSEQCommand::Rest:
			case SSEQCommand::Patch:
				return VariableByteCount;

			case SSEQCommand::Pan:
			case SSEQCommand::Volume:
			case SSEQCommand::MasterVolume:
			case SSEQCommand::Priority:
			case SSEQCommand::NoteWait:
			case SSEQCommand::Tie:
			case SSEQCommand::Expression:
			case SSEQCommand::LoopStart:
			case SSEQCommand::Transpose:
			case SSEQCommand::PitchBend:
			case SSEQCommand::PitchBendRange:
			case SSEQCommand::Attack:
			case SSEQCommand::Decay:
			case SSEQCommand::Sustain:
			case SSEQCommand::Release:
			case SSEQCommand::PortamentoKey:
			case SSEQCommand::PortamentoFlag:
			case SSEQCommand::PortamentoTime:
			case SSEQCommand::ModulationDepth:
			case SSEQCommand::ModulationSpeed:
			case SSEQCommand::ModulationType:
			case SSEQCommand::ModulationRange:
			case SSEQCommand::PrintVariable:
			case SSEQCommand::Mute:
				return 1;

			case SSEQCommand::AllocateTrack:
			case SSEQCommand::Tempo:
			case SSEQCommand::SweepPitch:
			case SSEQCommand::ModulationDelay:
				return 2;

			case SSEQCommand::Goto:
			case SSEQCommand::Call:
			case SSEQCommand::SetVariable:
			case SSEQCommand::AddVariable:
			case SSEQCommand::SubtractVariable:
			case SSEQCommand::MultiplyVariable:
			case SSEQCommand::DivideVariable:
			case SSEQCommand::ShiftVariable:
			case SSEQCommand::RandomVariable:
			case SSEQCommand::CompareEqualTo:
			case SSEQCommand::CompareGreaterThanOrEqualTo:
			case SSEQCommand::CompareGreaterThan:
			case SSEQCommand::CompareLessThanOrEqualTo:
			case SSEQCommand::CompareLessThan:
			case SSEQCommand::CompareNotEqualTo:
				return 3;

			case SSEQCommand::OpenTrack:
				return 4;

			case SSEQCommand::FromVariable:
				return 1 | ExtraByteOnNoteOrVarOrCmp; // Technically 2 bytes with an additional 1, leaving 1 off because we will be reading it to determine if the additional byte is needed

			case SSEQCommand::Random:
				return 4 | ExtraByteOnNoteOrVarOrCmp; // Technically 5 bytes with an additional 1, leaving 1 off because we will be reading it to determine if the additional byte is needed

			default:
				return 0;
		}
}

static std::uint32_t RandomU{ 0x12345678 };

static std::uint16_t CalcRandom()
{
	RandomU = RandomU * 1664525 + 1013904223;
	return static_cast<std::uint16_t>(RandomU >> 16);
}

static auto varFuncSet = [](std::int16_t, std::int16_t value) { return value; };
static auto varFuncAdd = [](std::int16_t var, std::int16_t value) -> std::int16_t { return var + value; };
static auto varFuncSub = [](std::int16_t var, std::int16_t value) -> std::int16_t { return var - value; };
static auto varFuncMul = [](std::int16_t var, std::int16_t value) -> std::int16_t { return var * value; };
static auto varFuncDiv = [](std::int16_t var, std::int16_t value) -> std::int16_t { return var / value; };
static auto varFuncShift = [](std::int16_t var, std::int16_t value) -> std::int16_t
{
	if (value < 0)
		return var >> -value;
	else
		return var << value;
};
static auto varFuncRand = [](std::int16_t, std::int16_t value) -> std::int16_t
{
	if (value < 0)
		return -(CalcRandom() % (-value + 1));
	else
		return CalcRandom() % (value + 1);
};

static inline std::function<std::int16_t (std::int16_t, std::int16_t)> VarFunc(int cmd)
{
	switch (static_cast<SSEQCommand>(cmd))
	{
		case SSEQCommand::SetVariable:
			return varFuncSet;
		case SSEQCommand::AddVariable:
			return varFuncAdd;
		case SSEQCommand::SubtractVariable:
			return varFuncSub;
		case SSEQCommand::MultiplyVariable:
			return varFuncMul;
		case SSEQCommand::DivideVariable:
			return varFuncDiv;
		case SSEQCommand::ShiftVariable:
			return varFuncShift;
		case SSEQCommand::RandomVariable:
			return varFuncRand;
		default:
			return nullptr;
	}
}

static auto compareFuncEq = [](std::int16_t a, std::int16_t b) { return a == b; };
static auto compareFuncGe = [](std::int16_t a, std::int16_t b) { return a >= b; };
static auto compareFuncGt = [](std::int16_t a, std::int16_t b) { return a > b; };
static auto compareFuncLe = [](std::int16_t a, std::int16_t b) { return a <= b; };
static auto compareFuncLt = [](std::int16_t a, std::int16_t b) { return a < b; };
static auto compareFuncNe = [](std::int16_t a, std::int16_t b) { return a != b; };

static inline std::function<bool (std::int16_t, std::int16_t)> CompareFunc(int cmd)
{
	switch (static_cast<SSEQCommand>(cmd))
	{
		case SSEQCommand::CompareEqualTo:
			return compareFuncEq;
		case SSEQCommand::CompareGreaterThanOrEqualTo:
			return compareFuncGe;
		case SSEQCommand::CompareGreaterThan:
			return compareFuncGt;
		case SSEQCommand::CompareLessThanOrEqualTo:
			return compareFuncLe;
		case SSEQCommand::CompareLessThan:
			return compareFuncLt;
		case SSEQCommand::CompareNotEqualTo:
			return compareFuncNe;
		default:
			return nullptr;
	}
}

// Original FSS Function: Track_Run
void Track::Run()
{
	// Indicate "heartbeat" for this track
	this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Length));

	// Exit if the track has already ended
	if (this->state[ConvertFuncs::ToIntegral(TrackState::End)])
		return;

	if (this->wait)
	{
		--this->wait;
		if (this->wait)
			return;
	}

	auto pData = &this->pos;

	while (!this->wait)
	{
		int cmd;
		if (this->overriding())
			cmd = this->overriding.cmd;
		else
			cmd = read8(pData);
		if (cmd < 0x80)
		{
			// Note on
			std::uint8_t key = static_cast<std::uint8_t>(cmd + this->transpose);
			int vel = this->overriding.val<std::uint8_t>(pData, read8, true);
			int len = this->overriding.val<int>(pData, readvl);
			if (this->state[ConvertFuncs::ToIntegral(TrackState::NoteWait)])
				this->wait = len;
			if (this->state[ConvertFuncs::ToIntegral(TrackState::TieBit)])
				this->NoteOnTie(key, vel);
			else
				this->NoteOn(key, vel, len);
		}
		else
		{
			int value;
			switch (static_cast<SSEQCommand>(cmd))
			{
				//-----------------------------------------------------------------
				// Main commands
				//-----------------------------------------------------------------

				case SSEQCommand::OpenTrack:
				{
					std::uint8_t tNum = read8(pData);
					auto trackPos = &this->ply->sseq->data[read24(pData)];
					int newTrack = this->ply->TrackAlloc();
					if (newTrack != -1)
					{
						this->ply->tracks[newTrack].Init(static_cast<std::uint8_t>(newTrack), this->ply, trackPos, tNum);
						this->ply->trackIds[this->ply->nTracks++] = static_cast<std::uint8_t>(newTrack);
					}
					break;
				}

				case SSEQCommand::Rest:
					this->wait = this->overriding.val<int>(pData, readvl);
					break;

				case SSEQCommand::Patch:
					this->patch = this->overriding.val<std::uint16_t>(pData, readvl);
					break;

				case SSEQCommand::Goto:
					*pData = &this->ply->sseq->data[read24(pData)];
					break;

				case SSEQCommand::Call:
					value = read24(pData);
					if (this->stackPos < FSS_TRACKSTACKSIZE)
					{
						const std::uint8_t *dest = &this->ply->sseq->data[value];
						this->stack[this->stackPos++] = StackValue(StackType::Call, *pData);
						*pData = dest;
					}
					break;

				case SSEQCommand::Return:
					if (this->stackPos && this->stack[this->stackPos - 1].type == StackType::Call)
						*pData = this->stack[--this->stackPos].dest;
					break;

				case SSEQCommand::Pan:
					this->pan = this->overriding.val<std::uint8_t>(pData, read8) - 64;
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Pan));
					break;

				case SSEQCommand::Volume:
					this->vol = this->overriding.val<std::uint8_t>(pData, read8);
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Volume));
					break;

				case SSEQCommand::MasterVolume:
					this->ply->masterVol = Cnv_Sust(this->overriding.val<std::uint8_t>(pData, read8));
					for (std::uint8_t i = 0; i < this->ply->nTracks; ++i)
						this->ply->tracks[this->ply->trackIds[i]].updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Volume));
					break;

				case SSEQCommand::Priority:
					this->prio = static_cast<std::uint8_t>(this->ply->prio + read8(pData));
					// Update here?
					break;

				case SSEQCommand::NoteWait:
					this->state.set(ConvertFuncs::ToIntegral(TrackState::NoteWait), !!read8(pData));
					break;

				case SSEQCommand::Tie:
					this->state.set(ConvertFuncs::ToIntegral(TrackState::TieBit), !!read8(pData));
					this->ReleaseAllNotes();
					break;

				case SSEQCommand::Expression:
					this->expr = this->overriding.val<std::uint8_t>(pData, read8);
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Volume));
					break;

				case SSEQCommand::Tempo:
					this->ply->tempo = read16(pData);
					break;

				case SSEQCommand::End:
					this->state.set(ConvertFuncs::ToIntegral(TrackState::End));
					return;

				case SSEQCommand::LoopStart:
					value = this->overriding.val<std::uint8_t>(pData, read8);
					if (this->stackPos < FSS_TRACKSTACKSIZE)
					{
						this->loopCount[this->stackPos] = static_cast<std::uint8_t>(value);
						this->stack[this->stackPos++] = StackValue(StackType::Loop, *pData);
					}
					break;

				case SSEQCommand::LoopEnd:
					if (this->stackPos && this->stack[this->stackPos - 1].type == StackType::Loop)
					{
						const std::uint8_t *rPos = this->stack[this->stackPos - 1].dest;
						std::uint8_t &nR = this->loopCount[this->stackPos - 1];
						std::uint8_t prevR = nR;
						if (!prevR || --nR)
							*pData = rPos;
						else
							--this->stackPos;
					}
					break;

				//-----------------------------------------------------------------
				// Tuning commands
				//-----------------------------------------------------------------

				case SSEQCommand::Transpose:
					this->transpose = this->overriding.val<std::int8_t>(pData, read8);
					break;

				case SSEQCommand::PitchBend:
					this->pitchBend = this->overriding.val<std::int8_t>(pData, read8);
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Timer));
					break;

				case SSEQCommand::PitchBendRange:
					this->pitchBendRange = read8(pData);
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Timer));
					break;

				//-----------------------------------------------------------------
				// Envelope-related commands
				//-----------------------------------------------------------------

				case SSEQCommand::Attack:
					this->a = this->overriding.val<std::uint8_t>(pData, read8);
					break;

				case SSEQCommand::Decay:
					this->d = this->overriding.val<std::uint8_t>(pData, read8);
					break;

				case SSEQCommand::Sustain:
					this->s = this->overriding.val<std::uint8_t>(pData, read8);
					break;

				case SSEQCommand::Release:
					this->r = this->overriding.val<std::uint8_t>(pData, read8);
					break;

				//-----------------------------------------------------------------
				// Portamento-related commands
				//-----------------------------------------------------------------

				case SSEQCommand::PortamentoKey:
					this->portaKey = static_cast<std::uint8_t>(read8(pData) + this->transpose);
					this->state.set(ConvertFuncs::ToIntegral(TrackState::PortamentoBit));
					// Update here?
					break;

				case SSEQCommand::PortamentoFlag:
					this->state.set(ConvertFuncs::ToIntegral(TrackState::PortamentoBit), !!read8(pData));
					// Update here?
					break;

				case SSEQCommand::PortamentoTime:
					this->portaTime = this->overriding.val<std::uint8_t>(pData, read8);
					// Update here?
					break;

				case SSEQCommand::SweepPitch:
					this->sweepPitch = this->overriding.val<std::int16_t>(pData, read16);
					this->state.set(ConvertFuncs::ToIntegral(TrackState::PortamentoBit));
					// Update here?
					break;

				//-----------------------------------------------------------------
				// Modulation-related commands
				//-----------------------------------------------------------------

				case SSEQCommand::ModulationDepth:
					this->modDepth = this->overriding.val<std::uint8_t>(pData, read8);
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Modulation));
					break;

				case SSEQCommand::ModulationSpeed:
					this->modSpeed = this->overriding.val<std::uint8_t>(pData, read8);
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Modulation));
					break;

				case SSEQCommand::ModulationType:
					this->modType = read8(pData);
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Modulation));
					break;

				case SSEQCommand::ModulationRange:
					this->modRange = read8(pData);
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Modulation));
					break;

				case SSEQCommand::ModulationDelay:
					this->modDelay = this->overriding.val<std::uint16_t>(pData, read16);
					this->updateFlags.set(ConvertFuncs::ToIntegral(TrackUpdateFlag::Modulation));
					break;

				//-----------------------------------------------------------------
				// Randomness-related commands
				//-----------------------------------------------------------------

				case SSEQCommand::Random:
				{
					this->overriding() = true;
					this->overriding.cmd = read8(pData);
					if ((this->overriding.cmd >= ConvertFuncs::ToIntegral(SSEQCommand::SetVariable) && this->overriding.cmd <= ConvertFuncs::ToIntegral(SSEQCommand::CompareNotEqualTo)) || this->overriding.cmd < 0x80)
						this->overriding.extraValue = read8(pData);
					std::int16_t minVal = static_cast<std::int16_t>(read16(pData));
					std::int16_t maxVal = static_cast<std::int16_t>(read16(pData));
					this->overriding.value = (CalcRandom() % (maxVal - minVal + 1)) + minVal;
					break;
				}

				//-----------------------------------------------------------------
				// Variable-related commands
				//-----------------------------------------------------------------

				case SSEQCommand::FromVariable:
					this->overriding() = true;
					this->overriding.cmd = read8(pData);
					if ((this->overriding.cmd >= ConvertFuncs::ToIntegral(SSEQCommand::SetVariable) && this->overriding.cmd <= ConvertFuncs::ToIntegral(SSEQCommand::CompareNotEqualTo)) || this->overriding.cmd < 0x80)
						this->overriding.extraValue = read8(pData);
					this->overriding.value = this->ply->variables[read8(pData)];
					break;

				case SSEQCommand::SetVariable:
				case SSEQCommand::AddVariable:
				case SSEQCommand::SubtractVariable:
				case SSEQCommand::MultiplyVariable:
				case SSEQCommand::DivideVariable:
				case SSEQCommand::ShiftVariable:
				case SSEQCommand::RandomVariable:
				{
					std::int8_t varNo = this->overriding.val<std::int8_t>(pData, read8, true);
					value = this->overriding.val<std::int16_t>(pData, read16);
					if (cmd == ConvertFuncs::ToIntegral(SSEQCommand::DivideVariable) && !value) // Division by 0, skip it to prevent crashing
						break;
					this->ply->variables[varNo] = VarFunc(cmd)(this->ply->variables[varNo], static_cast<std::int16_t>(value));
					break;
				}

				//-----------------------------------------------------------------
				// Conditional-related commands
				//-----------------------------------------------------------------

				case SSEQCommand::CompareEqualTo:
				case SSEQCommand::CompareGreaterThanOrEqualTo:
				case SSEQCommand::CompareGreaterThan:
				case SSEQCommand::CompareLessThanOrEqualTo:
				case SSEQCommand::CompareLessThan:
				case SSEQCommand::CompareNotEqualTo:
				{
					std::int8_t varNo = this->overriding.val<std::int8_t>(pData, read8, true);
					value = this->overriding.val<std::int16_t>(pData, read16);
					this->lastComparisonResult = CompareFunc(cmd)(this->ply->variables[varNo], static_cast<std::int16_t>(value));
					break;
				}

				case SSEQCommand::If:
					if (!this->lastComparisonResult)
					{
						int nextCmd = read8(pData);
						std::uint8_t cmdBytes = SseqCommandByteCount(nextCmd);
						bool variableBytes = !!(cmdBytes & VariableByteCount);
						bool extraByte = !!(cmdBytes & ExtraByteOnNoteOrVarOrCmp);
						cmdBytes &= ~(VariableByteCount | ExtraByteOnNoteOrVarOrCmp);
						if (extraByte)
						{
							int extraCmd = read8(pData);
							if ((extraCmd >= ConvertFuncs::ToIntegral(SSEQCommand::SetVariable) && extraCmd <= ConvertFuncs::ToIntegral(SSEQCommand::CompareNotEqualTo)) || extraCmd < 0x80)
								++cmdBytes;
						}
						*pData += cmdBytes;
						if (variableBytes)
							readvl(pData);
					}
					break;

				default:
					*pData += SseqCommandByteCount(cmd);
			}
		}

		if (cmd != ConvertFuncs::ToIntegral(SSEQCommand::Random) && cmd != ConvertFuncs::ToIntegral(SSEQCommand::FromVariable))
			this->overriding() = false;
	}
}
