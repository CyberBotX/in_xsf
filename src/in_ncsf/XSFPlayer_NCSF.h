/*
 * xSF - NCSF Player
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-21
 *
 * Partially based on the vio*sf framework
 *
 * Utilizes a modified FeOS Sound System for playback
 * https://github.com/fincs/FSS
 */

#ifndef XSFPLAYER_NCSF_H
#define XSFPLAYER_NCSF_H

#include <memory>
#include <bitset>
#include "XSFPlayer.h"
#include "SSEQPlayer/SDAT.h"
#include "SSEQPlayer/Player.h"

class XSFPlayer_NCSF : public XSFPlayer
{
	uint32_t sseq;
	std::vector<uint8_t> sdatData;
	std::unique_ptr<SDAT> sdat;
	Player player;
	double secondsPerSample, secondsIntoPlayback, secondsUntilNextClock;
	std::bitset<16> mutes;

	void MapNCSFSection(const std::vector<uint8_t> &section);
	bool MapNCSF(XSFFile *xSFToLoad);
	bool RecursiveLoadNCSF(XSFFile *xSFToLoad, int level);
	bool LoadNCSF();
public:
	XSFPlayer_NCSF(const std::string &filename);
	XSFPlayer_NCSF(const std::wstring &filename);
	bool Load();
	void GenerateSamples(std::vector<uint8_t> &buf, unsigned offset, unsigned samples);
	void Terminate() { }

	void SetInterpolation(unsigned interpolation);
	void SetMutes(const std::bitset<16> &newMutes);
};

#endif
