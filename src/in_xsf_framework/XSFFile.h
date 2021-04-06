/*
 * xSF - File structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <filesystem>
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
private:
	void ReadXSF(const std::filesystem::path &path, std::uint32_t programSizeOffset, std::uint32_t programHeaderSize, bool readTagsOnly = false);
	void ReadXSF(std::ifstream &xSF, std::uint32_t programSizeOffset, std::uint32_t programHeaderSize, bool readTagsOnly = false);
protected:
	std::uint8_t xSFType;
	bool hasFile;
	std::vector<std::uint8_t> rawData, reservedSection, programSection;
	TagList tags;
	std::filesystem::path filePath;
	std::string FormattedTitleOptionalBlock(const std::string &block, bool &hadReplacement, unsigned level) const;
public:
	XSFFile();
	XSFFile(const std::filesystem::path &path);
	XSFFile(const std::filesystem::path &path, std::uint32_t programSizeOffset, std::uint32_t programHeaderSize);
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
		return this->GetTagExists(name) ? ConvertFuncs::To<T>(this->GetTagValue(name)) : defaultValue;
	}
	unsigned long GetLengthMS(unsigned long defaultLength) const;
	unsigned long GetFadeMS(unsigned long defaultFade) const;
	double GetVolume(VolumeType preferredVolumeType, PeakType preferredPeakType) const;
	std::string GetFormattedTitle(const std::string &format) const;
	std::filesystem::path GetFilepath() const;
	std::filesystem::path GetFilenameWithoutPath() const;
	void SaveFile() const;
};
