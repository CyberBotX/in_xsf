/*
 * SSEQ Player - Track structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2014-10-13
 *
 * Adapted from source code of FeOS Sound System
 * By fincs
 * https://github.com/fincs/FSS
 */

#include <functional>
#include <cstdlib>
#include "Track.h"
#include "Player.h"
#include "common.h"

Track::Track()
{
	this->Zero();
}

void Track::Init(uint8_t handle, Player *player, const uint8_t *dataPos, int n)
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
	memset(this->loopCount, 0, sizeof(this->loopCount));
	this->overriding() = false;
	this->lastComparisonResult = this->processCommand = true;

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

void Track::ClearState()
{
	this->state.reset();
	this->state.set(TS_ALLOCBIT);
	this->state.set(TS_NOTEWAIT);
	this->prio = this->ply->prio + 64;

	this->pos = this->startPos;
	this->stackPos = 0;

	this->wait = 0;
	this->patch = 0;
	this->portaKey = 60;
	this->portaTime = 0;
	this->sweepPitch = 0;
	this->vol = 64;
	this->expr = 127;
	this->pan = 0;
	this->pitchBendRange = 2;
	this->pitchBend = this->transpose = 0;

	this->a = this->d = this->s = this->r = 0xFF;

	this->modType = 0;
	this->modRange = 1;
	this->modSpeed = 16;
	this->modDelay = 10;
	this->modDepth = 0;
}

void Track::Free()
{
	this->state.reset();
	this->updateFlags.reset();
}

int Track::NoteOn(int key, int vel, int len)
{
	auto sbnk = this->ply->sseq->bank;

	if (this->patch >= sbnk->instruments.size())
		return -1;

	bool bIsPCM = true;
	Channel *chn = nullptr;
	int nCh = -1;

	auto &instrument = sbnk->instruments[this->patch];
	const SBNKInstrumentRange *noteDef = nullptr;
	int fRecord = instrument.record;

	if (fRecord == 16)
	{
		if (!(instrument.ranges[0].lowNote <= key && key <= instrument.ranges[instrument.ranges.size() - 1].highNote))
			return -1;
		int rn = key - instrument.ranges[0].lowNote;
		noteDef = &instrument.ranges[rn];
		fRecord = noteDef->record;
	}
	else if (fRecord == 17)
	{
		size_t reg, ranges;
		for (reg = 0, ranges = instrument.ranges.size(); reg < ranges; ++reg)
			if (key <= instrument.ranges[reg].highNote)
				break;
		if (reg == ranges)
			return -1;

		noteDef = &instrument.ranges[reg];
		fRecord = noteDef->record;
	}

	if (!fRecord)
		return -1;
	else if (fRecord == 1)
	{
		if (!noteDef)
			noteDef = &instrument.ranges[0];
	}
	else if (fRecord < 4)
	{
		// PSG
		// fRecord = 2 -> PSG tone, pNoteDef->wavid -> PSG duty
		// fRecord = 3 -> PSG noise
		bIsPCM = false;
		if (!noteDef)
			noteDef = &instrument.ranges[0];
		if (fRecord == 3)
		{
			nCh = this->ply->ChannelAlloc(TYPE_NOISE, this->prio);
			if (nCh < 0)
				return -1;
			chn = &this->ply->channels[nCh];
			chn->tempReg.CR = SOUND_FORMAT_PSG | SCHANNEL_ENABLE;
		}
		else
		{
			nCh = this->ply->ChannelAlloc(TYPE_PSG, this->prio);
			if (nCh < 0)
				return -1;
			chn = &this->ply->channels[nCh];
			chn->tempReg.CR = SOUND_FORMAT_PSG | SCHANNEL_ENABLE | SOUND_DUTY(noteDef->swav & 0x7);
		}
		// TODO: figure out what pNoteDef->tnote means for PSG channels
		chn->tempReg.TIMER = -SOUND_FREQ(440 * 8); // key #69 (A4)
		chn->reg.samplePosition = -1;
		chn->reg.psgX = 0x7FFF;
	}

	if (bIsPCM)
	{
		nCh = this->ply->ChannelAlloc(TYPE_PCM, this->prio);
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

	chn->state = CS_START;
	chn->trackId = this->trackId;
	chn->flags.reset();
	chn->prio = this->prio;
	chn->key = key;
	chn->orgKey = bIsPCM ? noteDef->noteNumber : 69;
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

int Track::NoteOnTie(int key, int vel)
{
	// Find an existing note
	int i;
	Channel *chn = nullptr;
	for (i = 0; i < 16; ++i)
	{
		chn = &this->ply->channels[i];
		if (chn->state > CS_NONE && chn->trackId == this->trackId && chn->state != CS_RELEASE)
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
	chn->flags.set(CF_UPDTMR);

	return i;
}

void Track::ReleaseAllNotes()
{
	for (int i = 0; i < 16; ++i)
	{
		Channel &chn = this->ply->channels[i];
		if (chn.state > CS_NONE && chn.trackId == this->trackId && chn.state != CS_RELEASE)
			chn.Release();
	}
}

enum SseqCommand
{
	SSEQ_CMD_REST = 0x80,
	SSEQ_CMD_PATCH = 0x81,
	SSEQ_CMD_PAN = 0xC0,
	SSEQ_CMD_VOL = 0xC1,
	SSEQ_CMD_MASTERVOL = 0xC2,
	SSEQ_CMD_PRIO = 0xC6,
	SSEQ_CMD_NOTEWAIT = 0xC7,
	SSEQ_CMD_TIE = 0xC8,
	SSEQ_CMD_EXPR = 0xD5,
	SSEQ_CMD_TEMPO = 0xE1,
	SSEQ_CMD_END = 0xFF,

	SSEQ_CMD_GOTO = 0x94,
	SSEQ_CMD_CALL = 0x95,
	SSEQ_CMD_RET = 0xFD,
	SSEQ_CMD_LOOPSTART = 0xD4,
	SSEQ_CMD_LOOPEND = 0xFC,

	SSEQ_CMD_TRANSPOSE = 0xC3,
	SSEQ_CMD_PITCHBEND = 0xC4,
	SSEQ_CMD_PITCHBENDRANGE = 0xC5,

	SSEQ_CMD_ATTACK = 0xD0,
	SSEQ_CMD_DECAY = 0xD1,
	SSEQ_CMD_SUSTAIN = 0xD2,
	SSEQ_CMD_RELEASE = 0xD3,

	SSEQ_CMD_PORTAKEY = 0xC9,
	SSEQ_CMD_PORTAFLAG = 0xCE,
	SSEQ_CMD_PORTATIME = 0xCF,
	SSEQ_CMD_SWEEPPITCH = 0xE3,

	SSEQ_CMD_MODDEPTH = 0xCA,
	SSEQ_CMD_MODSPEED = 0xCB,
	SSEQ_CMD_MODTYPE = 0xCC,
	SSEQ_CMD_MODRANGE = 0xCD,
	SSEQ_CMD_MODDELAY = 0xE0,

	SSEQ_CMD_RANDOM = 0xA0,
	SSEQ_CMD_PRINTVAR = 0xD6,
	SSEQ_CMD_IF = 0xA2,
	SSEQ_CMD_FROMVAR = 0xA1,
	SSEQ_CMD_SETVAR = 0xB0,
	SSEQ_CMD_ADDVAR = 0xB1,
	SSEQ_CMD_SUBVAR = 0xB2,
	SSEQ_CMD_MULVAR = 0xB3,
	SSEQ_CMD_DIVVAR = 0xB4,
	SSEQ_CMD_SHIFTVAR = 0xB5,
	SSEQ_CMD_RANDVAR = 0xB6,
	SSEQ_CMD_CMP_EQ = 0xB8,
	SSEQ_CMD_CMP_GE = 0xB9,
	SSEQ_CMD_CMP_GT = 0xBA,
	SSEQ_CMD_CMP_LE = 0xBB,
	SSEQ_CMD_CMP_LT = 0xBC,
	SSEQ_CMD_CMP_NE = 0xBD,

	SSEQ_CMD_MUTE = 0xD7 // Unsupported
};

static auto varFuncSet = [](int16_t, int16_t value) { return value; };
static auto varFuncAdd = [](int16_t var, int16_t value) -> int16_t { return var + value; };
static auto varFuncSub = [](int16_t var, int16_t value) -> int16_t { return var - value; };
static auto varFuncMul = [](int16_t var, int16_t value) -> int16_t { return var * value; };
static auto varFuncDiv = [](int16_t var, int16_t value) -> int16_t { return var / value; };
static auto varFuncShift = [](int16_t var, int16_t value) -> int16_t
{
	if (value < 0)
		return var >> -value;
	else
		return var << value;
};
static auto varFuncRand = [](int16_t, int16_t value) -> int16_t
{
	if (value < 0)
		return -(std::rand() % (-value + 1));
	else
		return std::rand() % (value + 1);
};

static inline std::function<int16_t (int16_t, int16_t)> VarFunc(int cmd)
{
	switch (cmd)
	{
		case SSEQ_CMD_SETVAR:
			return varFuncSet;
		case SSEQ_CMD_ADDVAR:
			return varFuncAdd;
		case SSEQ_CMD_SUBVAR:
			return varFuncSub;
		case SSEQ_CMD_MULVAR:
			return varFuncMul;
		case SSEQ_CMD_DIVVAR:
			return varFuncDiv;
		case SSEQ_CMD_SHIFTVAR:
			return varFuncShift;
		case SSEQ_CMD_RANDVAR:
			return varFuncRand;
		default:
			return nullptr;
	}
}

static auto compareFuncEq = [](int16_t a, int16_t b) { return a == b; };
static auto compareFuncGe = [](int16_t a, int16_t b) { return a >= b; };
static auto compareFuncGt = [](int16_t a, int16_t b) { return a > b; };
static auto compareFuncLe = [](int16_t a, int16_t b) { return a <= b; };
static auto compareFuncLt = [](int16_t a, int16_t b) { return a < b; };
static auto compareFuncNe = [](int16_t a, int16_t b) { return a != b; };

static inline std::function<bool (int16_t, int16_t)> CompareFunc(int cmd)
{
	switch (cmd)
	{
		case SSEQ_CMD_CMP_EQ:
			return compareFuncEq;
		case SSEQ_CMD_CMP_GE:
			return compareFuncGe;
		case SSEQ_CMD_CMP_GT:
			return compareFuncGt;
		case SSEQ_CMD_CMP_LE:
			return compareFuncLe;
		case SSEQ_CMD_CMP_LT:
			return compareFuncLt;
		case SSEQ_CMD_CMP_NE:
			return compareFuncNe;
		default:
			return nullptr;
	}
}

void Track::Run()
{
	// Indicate "heartbeat" for this track
	this->updateFlags.set(TUF_LEN);

	// Exit if the track has already ended
	if (this->state[TS_END])
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
			int key = cmd + this->transpose;
			int vel;
			int len;
			if (this->overriding())
			{
				vel = this->overriding.extraValue;
				len = this->overriding.value;
			}
			else
			{
				vel = read8(pData);
				len = readvl(pData);
			}
			if (this->processCommand)
			{
				if (this->state[TS_NOTEWAIT])
					this->wait = len;
				if (this->state[TS_TIEBIT])
					this->NoteOnTie(key, vel);
				else
					this->NoteOn(key, vel, len);
			}
		}
		else
		{
			int value;
			switch (cmd)
			{
				//-----------------------------------------------------------------
				// Main commands
				//-----------------------------------------------------------------

				case SSEQ_CMD_REST:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = readvl(pData);
					if (this->processCommand)
						this->wait = value;
					break;

				case SSEQ_CMD_PATCH:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = readvl(pData);
					if (this->processCommand)
						this->patch = value;
					break;

				case SSEQ_CMD_GOTO:
					value = read24(pData);
					if (this->processCommand)
						*pData = &this->ply->sseq->data[value];
					break;

				case SSEQ_CMD_CALL:
					value = read24(pData);
					if (this->processCommand && this->stackPos < FSS_TRACKSTACKSIZE)
					{
						const uint8_t *dest = &this->ply->sseq->data[value];
						this->stack[this->stackPos++] = StackValue(STACKTYPE_CALL, *pData);
						*pData = dest;
					}
					break;

				case SSEQ_CMD_RET:
					if (this->processCommand && this->stackPos && this->stack[this->stackPos - 1].type == STACKTYPE_CALL)
						*pData = this->stack[--this->stackPos].dest;
					break;

				case SSEQ_CMD_PAN:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
					{
						this->pan = value - 64;
						this->updateFlags.set(TUF_PAN);
					}
					break;

				case SSEQ_CMD_VOL:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
					{
						this->vol = value;
						this->updateFlags.set(TUF_VOL);
					}
					break;

				case SSEQ_CMD_MASTERVOL:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
					{
						this->ply->masterVol = Cnv_Sust(value);
						for (uint8_t i = 0; i < this->ply->nTracks; ++i)
							this->ply->tracks[this->ply->trackIds[i]].updateFlags.set(TUF_VOL);
					}
					break;

				case SSEQ_CMD_PRIO:
					value = read8(pData);
					if (this->processCommand)
						this->prio = this->ply->prio + value;
					// Update here?
					break;

				case SSEQ_CMD_NOTEWAIT:
					value = read8(pData);
					if (this->processCommand)
						this->state.set(TS_NOTEWAIT, !!value);
					break;

				case SSEQ_CMD_TIE:
					value = read8(pData);
					if (this->processCommand)
					{
						this->state.set(TS_TIEBIT, !!value);
						this->ReleaseAllNotes();
					}
					break;

				case SSEQ_CMD_EXPR:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
					{
						this->expr = value;
						this->updateFlags.set(TUF_VOL);
					}
					break;

				case SSEQ_CMD_TEMPO:
					value = read16(pData);
					if (this->processCommand)
						this->ply->tempo = value;
					break;

				case SSEQ_CMD_END:
					if (this->processCommand)
					{
						this->state.set(TS_END);
						return;
					}
					break;

				case SSEQ_CMD_LOOPSTART:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand && this->stackPos < FSS_TRACKSTACKSIZE)
					{
						this->loopCount[this->stackPos] = value;
						this->stack[this->stackPos++] = StackValue(STACKTYPE_LOOP, *pData);
					}
					break;

				case SSEQ_CMD_LOOPEND:
					if (this->processCommand && this->stackPos && this->stack[this->stackPos - 1].type == STACKTYPE_LOOP)
					{
						const uint8_t *rPos = this->stack[this->stackPos - 1].dest;
						uint8_t &nR = this->loopCount[this->stackPos - 1];
						uint8_t prevR = nR;
						if (!prevR)
							*pData = rPos;
						else
						{
							if (--nR)
								*pData = rPos;
							else
								--this->stackPos;
						}
					}
					break;

				//-----------------------------------------------------------------
				// Tuning commands
				//-----------------------------------------------------------------

				case SSEQ_CMD_TRANSPOSE:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
						this->transpose = value;
					break;

				case SSEQ_CMD_PITCHBEND:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
					{
						this->pitchBend = value;
						this->updateFlags.set(TUF_TIMER);
					}
					break;

				case SSEQ_CMD_PITCHBENDRANGE:
					value = read8(pData);
					if (this->processCommand)
					{
						this->pitchBendRange = value;
						this->updateFlags.set(TUF_TIMER);
					}
					break;

				//-----------------------------------------------------------------
				// Envelope-related commands
				//-----------------------------------------------------------------

				case SSEQ_CMD_ATTACK:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
						this->a = value;
					break;

				case SSEQ_CMD_DECAY:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
						this->d = value;
					break;

				case SSEQ_CMD_SUSTAIN:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
						this->s = value;
					break;

				case SSEQ_CMD_RELEASE:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
						this->r = value;
					break;

				//-----------------------------------------------------------------
				// Portamento-related commands
				//-----------------------------------------------------------------

				case SSEQ_CMD_PORTAKEY:
					value = read8(pData);
					if (this->processCommand)
					{
						this->portaKey = value + this->transpose;
						this->state.set(TS_PORTABIT);
						// Update here?
					}
					break;

				case SSEQ_CMD_PORTAFLAG:
					value = read8(pData);
					if (this->processCommand)
					{
						this->state.set(TS_PORTABIT, !!value);
						// Update here?
					}
					break;

				case SSEQ_CMD_PORTATIME:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
						this->portaTime = value;
					// Update here?
					break;

				case SSEQ_CMD_SWEEPPITCH:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read16(pData);
					if (this->processCommand)
						this->sweepPitch = value;
					// Update here?
					break;

				//-----------------------------------------------------------------
				// Modulation-related commands
				//-----------------------------------------------------------------

				case SSEQ_CMD_MODDEPTH:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
					{
						this->modDepth = value;
						this->updateFlags.set(TUF_MOD);
					}
					break;

				case SSEQ_CMD_MODSPEED:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read8(pData);
					if (this->processCommand)
					{
						this->modSpeed = value;
						this->updateFlags.set(TUF_MOD);
					}
					break;

				case SSEQ_CMD_MODTYPE:
					value = read8(pData);
					if (this->processCommand)
					{
						this->modType = value;
						this->updateFlags.set(TUF_MOD);
					}
					break;

				case SSEQ_CMD_MODRANGE:
					value = read8(pData);
					if (this->processCommand)
					{
						this->modRange = value;
						this->updateFlags.set(TUF_MOD);
					}
					break;

				case SSEQ_CMD_MODDELAY:
					if (this->overriding())
						value = this->overriding.value;
					else
						value = read16(pData);
					if (this->processCommand)
					{
						this->modDelay = value;
						this->updateFlags.set(TUF_MOD);
					}
					break;

				//-----------------------------------------------------------------
				// Randomness-related commands
				//-----------------------------------------------------------------

				case SSEQ_CMD_RANDOM:
				{
					if (this->processCommand)
						this->overriding() = true;
					this->overriding.cmd = read8(pData);
					if ((this->overriding.cmd >= SSEQ_CMD_SETVAR && this->overriding.cmd <= SSEQ_CMD_CMP_NE) || this->overriding.cmd < 0x80)
						this->overriding.extraValue = read8(pData);
					int16_t minVal = read16(pData);
					int16_t maxVal = read16(pData);
					this->overriding.value = (std::rand() % (maxVal - minVal + 1)) + minVal;
					break;
				}

				//-----------------------------------------------------------------
				// Variable-related commands
				//-----------------------------------------------------------------

				case SSEQ_CMD_FROMVAR:
					if (this->processCommand)
						this->overriding() = true;
					this->overriding.cmd = read8(pData);
					if ((this->overriding.cmd >= SSEQ_CMD_SETVAR && this->overriding.cmd <= SSEQ_CMD_CMP_NE) || this->overriding.cmd < 0x80)
						this->overriding.extraValue = read8(pData);
					this->overriding.value = this->ply->variables[read8(pData)];
					break;

				case SSEQ_CMD_SETVAR:
				case SSEQ_CMD_ADDVAR:
				case SSEQ_CMD_SUBVAR:
				case SSEQ_CMD_MULVAR:
				case SSEQ_CMD_DIVVAR:
				case SSEQ_CMD_SHIFTVAR:
				case SSEQ_CMD_RANDVAR:
				{
					int8_t varNo;
					if (this->overriding())
					{
						varNo = this->overriding.extraValue;
						value = this->overriding.value;
					}
					else
					{
						varNo = read8(pData);
						value = read16(pData);
					}
					if (cmd == SSEQ_CMD_DIVVAR && !value) // Division by 0, skip it to prevent crashing
						break;
					if (this->processCommand)
						this->ply->variables[varNo] = VarFunc(cmd)(this->ply->variables[varNo], value);
					break;
				}

				//-----------------------------------------------------------------
				// Conditional-related commands
				//-----------------------------------------------------------------

				case SSEQ_CMD_CMP_EQ:
				case SSEQ_CMD_CMP_GE:
				case SSEQ_CMD_CMP_GT:
				case SSEQ_CMD_CMP_LE:
				case SSEQ_CMD_CMP_LT:
				case SSEQ_CMD_CMP_NE:
				{
					int8_t varNo;
					if (this->overriding())
					{
						varNo = this->overriding.extraValue;
						value = this->overriding.value;
					}
					else
					{
						varNo = read8(pData);
						value = read16(pData);
					}
					if (this->processCommand)
						this->lastComparisonResult = CompareFunc(cmd)(this->ply->variables[varNo], value);
					break;
				}

				case SSEQ_CMD_IF:
					this->processCommand = this->lastComparisonResult;
					break;

				case SSEQ_CMD_PRINTVAR:
					++*pData;
					break;

				case SSEQ_CMD_MUTE: // UNSUPPORTED
					++*pData;
			}
		}

		if (cmd != SSEQ_CMD_RANDOM && cmd != SSEQ_CMD_FROMVAR)
			this->overriding() = false;
		if (cmd != SSEQ_CMD_IF)
			this->processCommand = true;
	}
}
