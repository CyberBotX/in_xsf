/*
 * xSF - File structure
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-25
 *
 * Partially based on the vio*sf framework
 */

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <zlib.h>
#include "XSFFile.h"
#include "XSFCommon.h"
#include "convert.h"

static inline void Set32BitsLE(uint32_t input, uint8_t *output)
{
	output[0] = input & 0xFF;
	output[1] = (input >> 8) & 0xFF;
	output[2] = (input >> 16) & 0xFF;
	output[3] = (input >> 24) & 0xFF;
}

// The whitespace trimming was modified from the following answer on Stack Overflow:
// http://stackoverflow.com/a/217605

struct IsWhitespace : std::unary_function<char, bool>
{
	bool operator()(const char &x) const
	{
		return x >= 0x01 && x <= 0x20;
	}
};

static inline std::string LeftTrimWhitespace(const std::string &orig)
{
	auto first_non_space = std::find_if(orig.begin(), orig.end(), std::not1(IsWhitespace()));
	return std::string(first_non_space, orig.end());
}

static inline std::string RightTrimWhitespace(const std::string &orig)
{
	auto last_non_space = std::find_if(orig.rbegin(), orig.rend(), std::not1(IsWhitespace())).base();
	return std::string(orig.begin(), last_non_space);
}

static inline std::string TrimWhitespace(const std::string &orig)
{
	return LeftTrimWhitespace(RightTrimWhitespace(orig));
}

XSFFile::XSFFile() : xSFType(0), hasFile(false), rawData(), reservedSection(), programSection(), tags(), fileName("")
{
}

XSFFile::XSFFile(const std::string &filename) : xSFType(0), hasFile(false), rawData(), reservedSection(), programSection(), tags(), fileName(filename)
{
	this->ReadXSF(filename, 0, 0, true);
}

XSFFile::XSFFile(const std::string &filename, uint32_t programSizeOffset, uint32_t programHeaderSize) : xSFType(0), hasFile(false), rawData(), reservedSection(), programSection(), tags(), fileName(filename)
{
	this->ReadXSF(filename, programSizeOffset, programHeaderSize);
}

#ifdef _WIN32
XSFFile::XSFFile(const std::wstring &filename) : xSFType(0), hasFile(false), rawData(), reservedSection(), programSection(), tags(), fileName(filename)
{
	this->ReadXSF(filename, 0, 0, true);
}

XSFFile::XSFFile(const std::wstring &filename, uint32_t programSizeOffset, uint32_t programHeaderSize) : xSFType(0), hasFile(false), rawData(), reservedSection(), programSection(), tags(), fileName(filename)
{
	this->ReadXSF(filename, programSizeOffset, programHeaderSize);
}
#endif

void XSFFile::ReadXSF(const std::string &filename, uint32_t programSizeOffset, uint32_t programHeaderSize, bool readTagsOnly)
{
	if (!FileExists(filename))
		throw std::logic_error("File " + filename + " does not exist.");

	std::ifstream xSF;
	xSF.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	xSF.open(filename.c_str(), std::ifstream::in | std::ifstream::binary);

	this->ReadXSF(xSF, programSizeOffset, programHeaderSize, readTagsOnly);
}

#ifdef _WIN32
void XSFFile::ReadXSF(const std::wstring &filename, uint32_t programSizeOffset, uint32_t programHeaderSize, bool readTagsOnly)
{
	if (!FileExists(filename))
		throw std::logic_error("File " + String(filename).GetAnsi() + " does not exist.");

	std::ifstream xSF;
	xSF.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	xSF.open(filename.c_str(), std::ifstream::in | std::ifstream::binary);

	this->ReadXSF(xSF, programSizeOffset, programHeaderSize, readTagsOnly);
}
#endif

void XSFFile::ReadXSF(std::ifstream &xSF, uint32_t programSizeOffset, uint32_t programHeaderSize, bool readTagsOnly)
{
	xSF.seekg(0, std::ifstream::end);
	auto filesize = xSF.tellg();
	xSF.seekg(0, std::ifstream::beg);

	if (filesize < 4)
		throw std::runtime_error("File is too small.");

	char PSFHeader[4];
	xSF.read(PSFHeader, 4);

	if (PSFHeader[0] != 'P' || PSFHeader[1] != 'S' || PSFHeader[2] != 'F')
		throw std::runtime_error("Not a PSF file.");

	this->xSFType = PSFHeader[3];

	this->rawData.resize(4);
	memcpy(&this->rawData[0], PSFHeader, 4);

	if (filesize < 16)
		throw std::runtime_error("File is too small.");

	uint32_t reservedSize = Get32BitsLE(xSF), programCompressedSize = Get32BitsLE(xSF);
	this->rawData.resize(reservedSize + programCompressedSize + 16);
	Set32BitsLE(reservedSize, &this->rawData[4]);
	Set32BitsLE(programCompressedSize, &this->rawData[8]);
	xSF.read(reinterpret_cast<char *>(&this->rawData[12]), 4);

	if (reservedSize)
	{
		if (filesize < reservedSize + 16)
			throw std::runtime_error("File is too small.");

		if (readTagsOnly)
			xSF.read(reinterpret_cast<char *>(&this->rawData[16]), reservedSize);
		else
		{
			this->reservedSection.resize(reservedSize);
			xSF.read(reinterpret_cast<char *>(&this->reservedSection[0]), reservedSize);
			memcpy(&this->rawData[16], &this->reservedSection[0], reservedSize);
		}
	}

	if (programCompressedSize)
	{
		if (filesize < reservedSize + programCompressedSize + 16)
			throw std::runtime_error("File is too small.");

		if (readTagsOnly)
			xSF.read(reinterpret_cast<char *>(&this->rawData[reservedSize + 16]), programCompressedSize);
		else
		{
			auto programSectionCompressed = std::vector<uint8_t>(programCompressedSize);
			xSF.read(reinterpret_cast<char *>(&programSectionCompressed[0]), programCompressedSize);
			memcpy(&this->rawData[reservedSize + 16], &programSectionCompressed[0], programCompressedSize);

			auto programSectionUncompressed = std::vector<uint8_t>(programHeaderSize);
			unsigned long programUncompressedSize = programHeaderSize;
			uncompress(&programSectionUncompressed[0], &programUncompressedSize, &programSectionCompressed[0], programCompressedSize);
			programUncompressedSize = Get32BitsLE(&programSectionUncompressed[programSizeOffset]) + programHeaderSize;
			this->programSection.resize(programUncompressedSize);
			uncompress(&this->programSection[0], &programUncompressedSize, &programSectionCompressed[0], programCompressedSize);
		}
	}

	if (xSF.tellg() != filesize && filesize >= reservedSize + programCompressedSize + 21)
	{
		char tagheader[6] = "";
		xSF.read(tagheader, 5);
		if (std::string(tagheader) == "[TAG]")
		{
			auto startOfTags = xSF.tellg();
			unsigned lengthOfTags = static_cast<unsigned>(filesize - startOfTags);
			if (lengthOfTags)
			{
				auto rawtags = std::vector<char>(lengthOfTags);
				xSF.read(&rawtags[0], lengthOfTags);
				std::string name, value;
				bool onName = true;
				for (unsigned x = 0; x < lengthOfTags; ++x)
				{
					char curr = rawtags[x];
					if (curr == 0x0A)
					{
						if (!name.empty() && !value.empty())
						{
							name = TrimWhitespace(name);
							value = TrimWhitespace(value);
							if (this->tags.Exists(name))
								this->tags[name] += "\n" + value;
							else
								this->tags[name] = value;
						}
						name = value = "";
						onName = true;
						continue;
					}
					if (curr == '=')
					{
						onName = false;
						continue;
					}
					if (onName)
						name += curr;
					else
						value += curr;
				}
			}
		}
	}

	xSF.close();

	this->hasFile = true;
}

bool XSFFile::IsValidType(uint8_t type) const
{
	return this->xSFType == type;
}

void XSFFile::Clear()
{
	this->xSFType = 0;
	this->hasFile = false;
	this->reservedSection.clear();
	this->programSection.clear();
	this->tags.Clear();
}

bool XSFFile::HasFile() const
{
	return this->hasFile;
}

std::vector<uint8_t> &XSFFile::GetReservedSection()
{
	return this->reservedSection;
}

std::vector<uint8_t> XSFFile::GetReservedSection() const
{
	return this->reservedSection;
}

std::vector<uint8_t> &XSFFile::GetProgramSection()
{
	return this->programSection;
}

std::vector<uint8_t> XSFFile::GetProgramSection() const
{
	return this->programSection;
}

const TagList &XSFFile::GetAllTags() const
{
	return this->tags;
}

void XSFFile::SetAllTags(const TagList &newTags)
{
	this->tags = newTags;
}

void XSFFile::SetTag(const std::string &name, const String &value)
{
	this->tags[name] = value.GetStr();
}

bool XSFFile::GetTagExists(const std::string &name) const
{
	return this->tags.Exists(name);
}

String XSFFile::GetTagValue(const std::string &name) const
{
	return String(this->GetTagExists(name) ? this->tags[name] : "", this->GetTagExists("utf8"));
}

unsigned long XSFFile::GetLengthMS(unsigned long defaultLength) const
{
	unsigned long length = 0;
	String value = this->GetTagValue("length");
	if (!value.empty())
		length = ConvertFuncs::StringToMS(value.GetAnsi());
	if (!length)
		length = defaultLength;
	return length;
}

unsigned long XSFFile::GetFadeMS(unsigned long defaultFade) const
{
	unsigned long fade = defaultFade;
	String value = this->GetTagValue("fade");
	if (!value.empty())
		fade = ConvertFuncs::StringToMS(value.GetAnsi());
	return fade;
}

double XSFFile::GetVolume(VolumeType preferredVolumeType, PeakType preferredPeakType) const
{
	if (preferredVolumeType == VOLUMETYPE_NONE)
		return 1.0;
	String replaygain_album_gain = this->GetTagValue("replaygain_album_gain"), replaygain_album_peak = this->GetTagValue("replaygain_album_peak");
	String replaygain_track_gain = this->GetTagValue("replaygain_track_gain"), replaygain_track_peak = this->GetTagValue("replaygain_track_peak");
	String volume = this->GetTagValue("volume");
	double gain = 0.0;
	bool hadReplayGain = false;
	if (preferredVolumeType == VOLUMETYPE_REPLAYGAIN_ALBUM && !replaygain_album_gain.empty())
	{
		gain = convertTo<double>(replaygain_album_gain.GetAnsi(), false);
		hadReplayGain = true;
	}
	if (!hadReplayGain && preferredVolumeType != VOLUMETYPE_VOLUME && !replaygain_track_gain.empty())
	{
		gain = convertTo<double>(replaygain_track_gain.GetAnsi(), false);
		hadReplayGain = true;
	}
	if (hadReplayGain)
	{
		double vol = std::pow(10.0, gain / 20.0), peak = 1.0;
		if (preferredPeakType == PEAKTYPE_REPLAYGAIN_ALBUM && !replaygain_album_peak.empty())
			peak = convertTo<double>(replaygain_album_peak.GetAnsi(), false);
		else if (preferredPeakType != PEAKTYPE_NONE && !replaygain_track_peak.empty())
			peak = convertTo<double>(replaygain_track_peak.GetAnsi(), false);
		return !fEqual(peak, 1.0) ? std::min(vol, 1.0 / peak) : vol;
	}
	return volume.empty() ? 1.0 : convertTo<double>(volume.GetAnsi(), false);
}

String XSFFile::FormattedTitleOptionalBlock(const std::wstring &block, bool &hadReplacement, unsigned level) const
{
	String formattedBlock;
	for (size_t x = 0, len = block.length(); x < len; ++x)
	{
		wchar_t c = block[x];
		if (c == L'%')
		{
			size_t origX = x;
			for (++x; x < len; ++x)
				if (block[x] == L'%')
					break;
			if (x != len)
			{
				std::wstring tagname = block.substr(origX + 1, x - origX - 1);
				String value = this->GetTagValue(String(tagname).GetAnsi());
				if (!value.empty())
				{
					formattedBlock += value;
					hadReplacement = true;
				}
			}
			continue;
		}
		if (c == L'[' && level + 1 < 10)
		{
			size_t origX = x;
			unsigned nests = 0;
			for (++x; x < len; ++x)
			{
				if (block[x] == L'[')
					++nests;
				else if (block[x] == L']')
				{
					if (!nests)
						break;
					--nests;
				}
			}
			if (x != len)
			{
				std::wstring innerBlock = block.substr(origX + 1, x - origX - 1);
				bool hadInnerReplacement = false;
				String replacedBlock = this->FormattedTitleOptionalBlock(innerBlock, hadInnerReplacement, level + 1);
				if (hadInnerReplacement)
					formattedBlock += replacedBlock;
			}
			continue;
		}
		formattedBlock += c;
	}
	return formattedBlock;
}

String XSFFile::GetFormattedTitle(const std::wstring &format) const
{
	String formattedTitle;
	for (size_t x = 0, len = format.length(); x < len; ++x)
	{
		wchar_t c = format[x];
		if (c == L'%')
		{
			size_t origX = x;
			for (++x; x < len; ++x)
				if (format[x] == L'%')
					break;
			if (x != len)
			{
				std::wstring tagname = format.substr(origX + 1, x - origX - 1);
				String value = this->GetTagValue(String(tagname).GetAnsi());
				if (value.empty())
					formattedTitle += "???";
				else
					formattedTitle += value;
			}
		}
		else if (c == L'[')
		{
			size_t origX = x;
			unsigned nests = 0;
			for (++x; x < len; ++x)
			{
				if (format[x] == L'[')
					++nests;
				else if (format[x] == L']')
				{
					if (!nests)
						break;
					--nests;
				}
			}
			if (x != len)
			{
				std::wstring block = format.substr(origX + 1, x - origX - 1);
				bool hadReplacement = false;
				String replacedBlock = this->FormattedTitleOptionalBlock(block, hadReplacement, 1);
				if (hadReplacement)
					formattedTitle += replacedBlock;
			}
		}
		else
			formattedTitle += c;
	}
	return formattedTitle;
}

String XSFFile::GetFilename() const
{
	return this->fileName;
}

String XSFFile::GetFilenameWithoutPath() const
{
	return ExtractDirectoryFromPath(this->fileName.GetStr());
}

void XSFFile::SaveFile() const
{
	std::ofstream xSF;
	xSF.exceptions(std::ofstream::failbit);
#ifdef _WIN32
	xSF.open(this->fileName.GetWStrC(), std::ofstream::out | std::ofstream::binary);
#else
	xSF.open(this->fileName.GetStrC(), std::ofstream::out | std::ofstream::binary);
#endif

	xSF.write(reinterpret_cast<const char *>(&this->rawData[0]), this->rawData.size());

	auto allTags = this->tags.GetTags();
	if (!allTags.empty())
	{
		xSF.write("[TAG]", 5);
		for (auto curr = allTags.begin(), end = allTags.end(); curr != end; ++curr)
		{
			xSF.write(curr->c_str(), curr->length());
			xSF.write("\n", 1);
		}
	}
}
