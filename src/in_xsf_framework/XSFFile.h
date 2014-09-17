/*
 * xSF - File structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2014-09-17
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <cstdint>
#include "convert.h"
#include "TagList.h"
#include "BigSString.h"

enum VolumeType
{
	VOLUMETYPE_NONE,
	VOLUMETYPE_VOLUME,
	VOLUMETYPE_REPLAYGAIN_TRACK,
	VOLUMETYPE_REPLAYGAIN_ALBUM
};

enum PeakType
{
	PEAKTYPE_NONE,
	PEAKTYPE_REPLAYGAIN_TRACK,
	PEAKTYPE_REPLAYGAIN_ALBUM
};

class XSFFile
{
protected:
	uint8_t xSFType;
	bool hasFile;
	std::vector<uint8_t> rawData, reservedSection, programSection;
	TagList tags;
	String fileName;
	void ReadXSF(const std::string &filename, uint32_t programSizeOffset, uint32_t programHeaderSize, bool readTagsOnly = false);
#ifdef _MSC_VER
	void ReadXSF(const std::wstring &filename, uint32_t programSizeOffset, uint32_t programHeaderSize, bool readTagsOnly = false);
#endif
	void ReadXSF(std::ifstream &xSF, uint32_t programSizeOffset, uint32_t programHeaderSize, bool readTagsOnly = false);
	String FormattedTitleOptionalBlock(const std::wstring &block, bool &hadReplacement, unsigned level) const;
public:
	XSFFile();
	XSFFile(const std::string &filename);
	XSFFile(const std::string &filename, uint32_t programSizeOffset, uint32_t programHeaderSize);
#ifdef _MSC_VER
	XSFFile(const std::wstring &filename);
	XSFFile(const std::wstring &filename, uint32_t programSizeOffset, uint32_t programHeaderSize);
#endif
	bool IsValidType(uint8_t type) const;
	void Clear();
	bool HasFile() const;
	std::vector<uint8_t> &GetReservedSection();
	std::vector<uint8_t> GetReservedSection() const;
	std::vector<uint8_t> &GetProgramSection();
	std::vector<uint8_t> GetProgramSection() const;
	const TagList &GetAllTags() const;
	void SetAllTags(const TagList &newTags);
	void SetTag(const std::string &name, const String &value);
	bool GetTagExists(const std::string &name) const;
	String GetTagValue(const std::string &name) const;
	template<typename T> T GetTagValue(const std::string &name, const T &defaultValue) const
	{
		return this->GetTagExists(name) ? convertTo<T>(this->GetTagValue(name).GetAnsi(), false) : defaultValue;
	}
	unsigned long GetLengthMS(unsigned long defaultLength) const;
	unsigned long GetFadeMS(unsigned long defaultFade) const;
	double GetVolume(VolumeType preferredVolumeType, PeakType preferredPeakType) const;
	String GetFormattedTitle(const std::wstring &format) const;
	String GetFilename() const;
	String GetFilenameWithoutPath() const;
	void SaveFile() const;
};
