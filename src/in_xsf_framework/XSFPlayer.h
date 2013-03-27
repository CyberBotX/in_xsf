/*
 * xSF - Core Player
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-25
 *
 * Partially based on the vio*sf framework
 */

#ifndef XSFPLAYER_H
#define XSFPLAYER_H

#include "XSFFile.h"

#ifdef WINAMP_PLUGIN
# include <winamp/out.h>
#endif

// This is a base class, a player for a specific type of xSF should inherit from this.
class XSFPlayer
{
protected:
	static const unsigned CHECK_SILENCE_BIAS = 0x8000000;
	static const unsigned CHECK_SILENCE_LEVEL = 7;

	XSFFile *xSF;
	unsigned sampleRate, detectedSilenceSample, detectedSilenceSec, skipSilenceOnStartSec, lengthSample, fadeSample, currentSample;
	unsigned long prevSampleL, prevSampleR;
	int lengthInMS, fadeInMS;
	double volume;
	bool ignoreVolume;

	XSFPlayer();
	XSFPlayer(const XSFPlayer &xSFPLayer);
public:
	// These are not defined in XSFPlayer.cpp, they should be defined in your own player's source.  The Create functions should return a pointer to your player's class.
	static const char *WinampDescription;
	static const char *WinampExts;
	static XSFPlayer *Create(const std::string &fn);
	static XSFPlayer *Create(const std::wstring &fn);

	virtual ~XSFPlayer() { if (this->xSF) delete this->xSF; }
	XSFPlayer &operator=(const XSFPlayer &xSFPlayer);
	XSFFile *GetXSFFile() { return this->xSF; }
	const XSFFile *GetXSFFile() const { return this->xSF; }
	unsigned GetLengthInSamples() const { return this->lengthSample + this->fadeSample; }
	unsigned GetSampleRate() const { return this->sampleRate; }
	void SetSampleRate(unsigned newSampleRate) { this->sampleRate = newSampleRate; }
	void IgnoreVolume() { this->ignoreVolume = true; }
	virtual bool Load();
	bool FillBuffer(std::vector<uint8_t> &buf, unsigned &samplesWritten);
	virtual void GenerateSamples(std::vector<uint8_t> &buf, unsigned offset, unsigned samples) = 0;
	void SeekTop();
#ifdef WINAMP_PLUGIN
	int Seek(unsigned seekPosition, volatile int *killswitch, std::vector<uint8_t> &buf, Out_Module *outMod);
#endif
	virtual void Terminate() = 0;
};

#endif