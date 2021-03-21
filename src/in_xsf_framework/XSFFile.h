/*
 * xSF - File structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include "convert.h"
#include "TagList.h"

enum class VolumeType
{
	None,
	Volume,
	ReplayGainTrack,
	ReplayGainAlbum
};

enum class PeakType
{
	None,
	ReplayGainTrack,
	ReplayGainAlbum
};

class XSFFile
{
protected:
	std::uint8_t xSFType;
	bool hasFile;
	std::vector<std::uint8_t> rawData, reservedSection, programSection;
	TagList tags;
	std::string fileName;
	void ReadXSF(const std::string &filename, std::uint32_t programSizeOffset, std::uint32_t programHeaderSize, bool readTagsOnly = false);
#ifdef _WIN32
	void ReadXSF(const std::wstring &filename, std::uint32_t programSizeOffset, std::uint32_t programHeaderSize, bool readTagsOnly = false);
#endif
	void ReadXSF(std::ifstream &xSF, std::uint32_t programSizeOffset, std::uint32_t programHeaderSize, bool readTagsOnly = false);
	std::string FormattedTitleOptionalBlock(const std::string &block, bool &hadReplacement, unsigned level) const;
public:
	XSFFile();
	XSFFile(const std::string &filename);
	XSFFile(const std::string &filename, std::uint32_t programSizeOffset, std::uint32_t programHeaderSize);
#ifdef _WIN32
	XSFFile(const std::wstring &filename);
	XSFFile(const std::wstring &filename, std::uint32_t programSizeOffset, std::uint32_t programHeaderSize);
#endif
	bool IsValidType(std::uint8_t type) const;
	void Clear();
	bool HasFile() const;
	std::vector<std::uint8_t> &GetReservedSection();
	std::vector<std::uint8_t> GetReservedSection() const;
	std::vector<std::uint8_t> &GetProgramSection();
	std::vector<std::uint8_t> GetProgramSection() const;
	const TagList &GetAllTags() const;
	void SetAllTags(const TagList &newTags);
	void SetTag(const std::string &name, const std::string &value);
	void SetTag(const std::string &name, const std::wstring &value);
	bool GetTagExists(const std::string &name) const;
	std::string GetTagValue(const std::string &name) const;
	template<typename T> T GetTagValue(const std::string &name, const T &defaultValue) const
	{
		return this->GetTagExists(name) ? convertTo<T>(this->GetTagValue(name)) : defaultValue;
	}
	unsigned long GetLengthMS(unsigned long defaultLength) const;
	unsigned long GetFadeMS(unsigned long defaultFade) const;
	double GetVolume(VolumeType preferredVolumeType, PeakType preferredPeakType) const;
	std::string GetFormattedTitle(const std::string &format) const;
	std::string GetFilename() const;
	std::string GetFilenameWithoutPath() const;
	void SaveFile() const;
};
