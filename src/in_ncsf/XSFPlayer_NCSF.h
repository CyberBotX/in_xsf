/*
 * xSF - NCSF Player
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 *
 * Utilizes a modified FeOS Sound System for playback
 * https://github.com/fincs/FSS
 */

#pragma once

#include <bitset>
#include <memory>
#include <string>
#include <vector>
#ifdef _DEBUG
# include <cstddef>
#endif
#include <cstdint>
#include "XSFPlayer.h"
#include "SSEQPlayer/SDAT.h"
#include "SSEQPlayer/Player.h"

class XSFPlayer_NCSF : public XSFPlayer
{
	std::uint32_t sseq;
	std::vector<std::uint8_t> sdatData;
	std::unique_ptr<SDAT> sdat;
	Player player;
	double secondsPerSample, secondsIntoPlayback, secondsUntilNextClock;
	std::bitset<16> mutes;

	void MapNCSFSection(const std::vector<std::uint8_t> &section);
	bool MapNCSF(XSFFile *xSFToLoad);
	bool RecursiveLoadNCSF(XSFFile *xSFToLoad, int level);
	bool LoadNCSF();
public:
	XSFPlayer_NCSF(const std::string &filename);
#ifdef _WIN32
	XSFPlayer_NCSF(const std::wstring &filename);
#endif
	~XSFPlayer_NCSF();
	bool Load();
	void GenerateSamples(std::vector<std::uint8_t> &buf, unsigned offset, unsigned samples);
	void Terminate();

	void SetInterpolation(unsigned interpolation);
	void SetMutes(const std::bitset<16> &newMutes);
#ifdef _DEBUG
	const Channel &GetChannel(std::size_t chanNum) const;
#endif
};
