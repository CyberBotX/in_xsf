/*
 * xSF - GSF Player
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Based on a modified viogsf v0.08
 *
 * Partially based on the vio*sf framework
 *
 * Utilizes a modified VBA-M, SVN revision 1102, for playback
 * http://vba-m.com/
 */

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <zlib.h>
#include "XSFCommon.h"
#include "XSFPlayer.h"
#include "vbam/gba/Globals.h"
#include "vbam/gba/Sound.h"
#include "vbam/common/SoundDriver.h"

class XSFPlayer_GSF : public XSFPlayer
{
public:
	XSFPlayer_GSF(const std::filesystem::path &path);
	~XSFPlayer_GSF() override { this->Terminate(); }
	bool Load() override;
	void GenerateSamples(std::vector<std::uint8_t> &buf, unsigned offset, unsigned samples) override;
	void Terminate() override;
};

const char *XSFPlayer::WinampDescription = "GSF Decoder";
const char *XSFPlayer::WinampExts = "gsf;minigsf\0Game Boy Advance Sound Format files (*.gsf;*.minigsf)\0";

XSFPlayer *XSFPlayer::Create(const std::filesystem::path &path)
{
	return new XSFPlayer_GSF(path);
}

static struct
{
	std::vector<std::uint8_t> rom;
	unsigned entry;
} loaderwork = { std::vector<std::uint8_t>(), 0 };

int mapgsf(std::uint8_t *d, int l, int &s)
{
	if (static_cast<std::size_t>(l) > loaderwork.rom.size())
		l = loaderwork.rom.size();
	if (l)
		std::copy_n(&loaderwork.rom[0], l, d);
	s = l;
	return l;
}

static struct
{
	std::vector<std::uint8_t> buf;
	std::uint32_t len, fil, cur;
} buffer = { std::vector<std::uint8_t>(), 0, 0, 0 };

class GSFSoundDriver : public SoundDriver
{
	void freebuffer()
	{
		buffer.buf.clear();
		buffer.len = buffer.fil = buffer.cur = 0;
	}
public:
	bool init(long sampleRate)
	{
		freebuffer();
		std::int32_t len = (sampleRate / 10) << 2;
		buffer.buf.resize(len);
		buffer.len = len;
		return true;
	}

	void pause()
	{
	}

	void reset()
	{
	}

	void resume()
	{
	}

	void write(std::uint16_t *finalWave, int length)
	{
		if (static_cast<std::uint32_t>(length) > buffer.len - buffer.fil)
			length = buffer.len - buffer.fil;
		if (length > 0)
		{
			std::copy_n(reinterpret_cast<std::uint8_t *>(finalWave), length, &buffer.buf[buffer.fil]);
			buffer.fil += length;
		}
	}

	void setThrottle(unsigned short)
	{
	}
};

SoundDriver *systemSoundInit()
{
	return new GSFSoundDriver();
}

static void MapGSFSection(const std::vector<std::uint8_t> &section, int level)
{
	auto &data = loaderwork.rom;

	std::uint32_t entry = Get32BitsLE(&section[0]), offset = Get32BitsLE(&section[4]) & 0x1FFFFFF, size = Get32BitsLE(&section[8]), finalSize = size + offset;
	if (level == 1)
		loaderwork.entry = entry;
	finalSize = NextHighestPowerOf2(finalSize);
	if (data.empty())
		data.resize(finalSize + 10, 0);
	else if (data.size() < size + offset)
		data.resize(offset + finalSize + 10);
	std::copy_n(&section[12], size, &data[offset]);
}

static bool MapGSF(XSFFile *xSF, int level)
{
	if (!xSF->IsValidType(0x22))
		return false;

	auto &programSection = xSF->GetProgramSection();

	if (!programSection.empty())
		MapGSFSection(programSection, level);

	return true;
}

static bool RecursiveLoadGSF(XSFFile *xSF, int level)
{
	if (level <= 10 && xSF->GetTagExists("_lib"))
	{
		auto libxSF = std::make_unique<XSFFile>(xSF->GetFilepath().parent_path() / xSF->GetTagValue("_lib"), 8, 12);
		if (!RecursiveLoadGSF(libxSF.get(), level + 1))
			return false;
	}

	if (!MapGSF(xSF, level))
		return false;

	unsigned n = 2;
	bool found;
	do
	{
		found = false;
		std::string libTag = "_lib" + std::to_string(n++);
		if (xSF->GetTagExists(libTag))
		{
			found = true;
			auto libxSF = std::make_unique<XSFFile>(xSF->GetFilepath().parent_path() / xSF->GetTagValue(libTag), 8, 12);
			if (!RecursiveLoadGSF(libxSF.get(), level + 1))
				return false;
		}
	} while (found);

	return true;
}

static bool LoadGSF(XSFFile *xSF)
{
	loaderwork.rom.clear();
	loaderwork.entry = 0;

	return RecursiveLoadGSF(xSF, 1);
}

XSFPlayer_GSF::XSFPlayer_GSF(const std::filesystem::path &path) : XSFPlayer()
{
	this->xSF.reset(new XSFFile(path, 8, 12));
}

bool XSFPlayer_GSF::Load()
{
	if (!LoadGSF(this->xSF.get()))
		return false;

	cpuIsMultiBoot = (loaderwork.entry >> 24) == 2;

	CPULoadRom();

	soundSetSampleRate(this->sampleRate);
	soundInit();
	soundReset();
	soundSetEnable(0x30F);

	CPUInit();
	CPUReset();

	return XSFPlayer::Load();
}

void XSFPlayer_GSF::GenerateSamples(std::vector<std::uint8_t> &buf, unsigned offset, unsigned samples)
{
	unsigned bytes = samples << 2;
	while (bytes)
	{
		unsigned remainbytes = buffer.fil - buffer.cur;
		while (!remainbytes)
		{
			buffer.cur = buffer.fil = 0;
			CPULoop(250000);

			remainbytes = buffer.fil - buffer.cur;
		}
		unsigned len = remainbytes;
		if (len > bytes)
			len = bytes;
		std::copy_n(&buffer.buf[buffer.cur], len, &buf[offset]);
		bytes -= len;
		offset += len;
		buffer.cur += len;
	}
}

void XSFPlayer_GSF::Terminate()
{
	soundShutdown();

	loaderwork.rom.clear();
	loaderwork.entry = 0;
}
