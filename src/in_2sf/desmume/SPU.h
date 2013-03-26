/*
	Copyright 2006 Theo Berkau
	Copyright (C) 2006-2010 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SPU_H
#define SPU_H

#include <iosfwd>
#include <string>
#include <cassert>

#include "types.h"
#include "matrix.h"
#include "emufile.h"

#include "metaspu/metaspu.h"

#define SNDCORE_DEFAULT         -1
#define SNDCORE_DUMMY           0

#define CHANSTAT_STOPPED          0
#define CHANSTAT_PLAY             1

//who made these static? theyre used in multiple places.
//inline uint32_t sputrunc(float f) { return u32floor(f); }
//inline uint32_t sputrunc(double d) { return u32floor(d); }
inline int32_t spumuldiv7(int32_t val, uint8_t multiplier) {
	assert(multiplier <= 127);
	return multiplier == 127 ? val : ((val * multiplier) >> 7);
}

enum SPUInterpolationMode
{
	SPUInterpolation_None = 0,
	SPUInterpolation_Linear = 1,
	SPUInterpolation_Cosine = 2
};

struct SoundInterface_struct
{
   int id;
   const char *Name;
   int (*Init)(int buffersize);
   void (*DeInit)();
   void (*UpdateAudio)(int16_t *buffer, uint32_t num_samples);
   uint32_t (*GetAudioSpace)();
   void (*MuteAudio)();
   void (*UnMuteAudio)();
   void (*SetVolume)(int volume);
   void (*ClearBuffer)();
};

extern SoundInterface_struct SNDDummy;
extern SoundInterface_struct SNDFile;
extern int SPU_currentCoreNum;

struct channel_struct
{
	channel_struct()
	{}
	uint32_t num;
   uint8_t vol;
   uint8_t datashift;
   uint8_t hold;
   uint8_t pan;
   uint8_t waveduty;
   uint8_t repeat;
   uint8_t format;
   uint8_t keyon;
   uint8_t status;
   uint32_t addr;
   uint16_t timer;
   uint16_t loopstart;
   uint32_t length;
   uint32_t totlength;
   double double_totlength_shifted;
   union {
		int8_t *buf8;
		int16_t *buf16;
   };
   double sampcnt;
   double sampinc;
   // ADPCM specific
   uint32_t lastsampcnt;
   int16_t pcm16b, pcm16b_last;
   int16_t loop_pcm16b;
   int32_t index;
   int loop_index;
   uint16_t x;
   int16_t psgnoise_last;
};

class SPUFifo
{
public:
	SPUFifo();
	void enqueue(int16_t val);
	int16_t dequeue();
	int16_t buffer[16];
	int32_t head,tail,size;
	//void save(EMUFILE* fp);
	bool load(EMUFILE* fp);
	void reset();
};

class SPU_struct
{
public:
	SPU_struct(int buffersize);
   uint32_t bufpos;
   uint32_t buflength;
   int32_t *sndbuf;
   int32_t lastdata; //the last sample that a channel generated
   int16_t *outbuf;
   uint32_t bufsize;
   channel_struct channels[16];

   //registers
   struct REGS {
	   REGS()
			: mastervol(0)
			, ctl_left(0)
			, ctl_right(0)
			, ctl_ch1bypass(0)
			, ctl_ch3bypass(0)
			, masteren(0)
			, soundbias(0)
	   {}

	   uint8_t mastervol;
	   uint8_t ctl_left, ctl_right;
	   uint8_t ctl_ch1bypass, ctl_ch3bypass;
	   uint8_t masteren;
	   uint16_t soundbias;

	   enum LeftOutputMode
	   {
		   LOM_LEFT_MIXER=0, LOM_CH1=1, LOM_CH3=2, LOM_CH1_PLUS_CH3=3
	   };

	   enum RightOutputMode
	   {
		   ROM_RIGHT_MIXER=0, ROM_CH1=1, ROM_CH3=2, ROM_CH1_PLUS_CH3=3
	   };

	   struct CAP {
		   CAP()
			   : add(0), source(0), oneshot(0), bits8(0), active(0), dad(0), len(0)
		   {}
		   uint8_t add, source, oneshot, bits8, active;
		   uint32_t dad;
		   uint16_t len;
		   struct Runtime {
			   Runtime()
				   : running(0), curdad(0), maxdad(0)
			   {}
			   uint8_t running;
			   uint32_t curdad;
			   uint32_t maxdad;
			   double sampcnt;
			   SPUFifo fifo;
		   } runtime;
	   } cap[2];
   } regs;

   void reset();
   ~SPU_struct();
   void KeyOff(int channel);
   void KeyOn(int channel);
   void KeyProbe(int channel);
   void ProbeCapture(int which);
   void WriteByte(uint32_t addr, uint8_t val);
   uint8_t ReadByte(uint32_t addr);
   uint16_t ReadWord(uint32_t addr);
   uint32_t ReadLong(uint32_t addr);
   void WriteWord(uint32_t addr, uint16_t val);
   void WriteLong(uint32_t addr, uint32_t val);

   //kills all channels but leaves SPU otherwise running normally
   void ShutUp();
};

int SPU_ChangeSoundCore(int coreid, int buffersize);
//SoundInterface_struct *SPU_SoundCore();

void SPU_ReInit();
int SPU_Init(int coreid, int buffersize);
//void SPU_Pause(int pause);
//void SPU_SetVolume(int volume);
void SPU_SetSynchMode(ESynchMode mode, ESynchMethod method);
//void SPU_ClearOutputBuffer();
void SPU_Reset();
void SPU_DeInit();
void SPU_KeyOn(int channel);
void SPU_WriteByte(uint32_t addr, uint8_t val);
void SPU_WriteWord(uint32_t addr, uint16_t val);
void SPU_WriteLong(uint32_t addr, uint32_t val);
uint8_t SPU_ReadByte(uint32_t addr);
uint16_t SPU_ReadWord(uint32_t addr);
uint32_t SPU_ReadLong(uint32_t addr);
void SPU_Emulate_core();
void SPU_Emulate_user(bool mix = true);

extern SPU_struct *SPU_core, *SPU_user;
extern int spu_core_samples;

//void spu_savestate(EMUFILE* os);
bool spu_loadstate(EMUFILE* is, int size);

/*enum WAVMode
{
	WAVMODE_ANY = -1,
	WAVMODE_CORE = 0,
	WAVMODE_USER = 1
};*/

/*class WavWriter
{
public:
	WavWriter();
	bool open(const std::string & fname);
	void close();
	void update(void* soundData, int numSamples);
	bool isRecording() const;
	WAVMode mode;
private:
	FILE *spufp;
};*/

//void WAV_End();
//bool WAV_Begin(const char* fname, WAVMode mode=WAVMODE_CORE);
//bool WAV_IsRecording(WAVMode mode=WAVMODE_ANY);
//void WAV_WavSoundUpdate(void* soundData, int numSamples, WAVMode mode=WAVMODE_CORE);

// we should make this configurable eventually
// but at least defining it somewhere is probably a step in the right direction
#define DESMUME_SAMPLE_RATE 44100
//#define DESMUME_SAMPLE_RATE 48000

#endif
