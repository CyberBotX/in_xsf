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
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#ifndef NDEBUG
# include <cstddef>
#endif
#include <cstdint>
#include "XSFPlayer.h"
#include "SSEQPlayer/SDAT.h"
#include "SSEQPlayer/Player.h"

class XSFFile;

class XSFPlayer_NCSF : public XSFPlayer
{
	std::uint32_t sseq;
	std::vector<std::uint8_t> sdatData;
	std::unique_ptr<SDAT> sdat;
	Player player;
	double secondsPerSample, secondsIntoPlayback, secondsUntilNextClock;
	std::bitset<16> mutes;
#ifndef NDEBUG
	bool useSoundViewDialog;
#endif

	void MapNCSFSection(const std::vector<std::uint8_t> &section);
	bool MapNCSF(XSFFile *xSFToLoad);
	bool RecursiveLoadNCSF(XSFFile *xSFToLoad, int level);
	bool LoadNCSF();
public:
	XSFPlayer_NCSF(const std::filesystem::path &path);
	~XSFPlayer_NCSF() override;
	bool Load() override;
	void GenerateSamples(std::vector<std::uint8_t> &buf, unsigned offset, unsigned samples) override;
	void Terminate() override;

#ifndef NDEBUG
	void SetUseSoundViewDialog(bool newUseSoundViewDialog);
#endif
	void SetInterpolation(unsigned interpolation);
	void SetMutes(const std::bitset<16> &newMutes);
#ifndef NDEBUG
	const Channel &GetChannel(std::size_t chanNum) const;
#endif
};
