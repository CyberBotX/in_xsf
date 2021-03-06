/*
 * xSF - Winamp plugin
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include <algorithm>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <cstddef>
#include <cstdint>
#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <wx/app.h>
#ifdef __GNUC__
# pragma GCC diagnostic pop
#elif defined(__clang__)
# pragma clang diagnostic pop
#endif
#include "windowsh_wrapper.h"
#include "XSFApp.h"
#include "XSFCommon.h"
#include "XSFConfig.h"
#include "XSFFile.h"
#include "XSFPlayer.h"
#include "convert.h"
#include "winamp/in2.h"
#include "winamp/wa_ipc.h"

extern In_Module inMod;
static const XSFFile *xSFFile = nullptr;
XSFFile *xSFFileInInfo = nullptr;
static std::unique_ptr<XSFPlayer> xSFPlayer;
std::unique_ptr<XSFApp> xSFApp;
std::unique_ptr<XSFConfig> xSFConfig;
static bool paused;
static std::atomic_int seek_needed;
static double decode_pos_ms;
static std::unique_ptr<std::thread> thread_handle;
static std::atomic_bool killThread;

static const unsigned NumChannels = 2;
static const unsigned BitsPerSample = 16;

void playThread()
{
	bool done = false;
	while (!killThread)
	{
		if (seek_needed != -1)
		{
			decode_pos_ms = seek_needed - (seek_needed % 1000);
			seek_needed = -1;
			auto dummyBuffer = std::vector<std::uint8_t>(576 * NumChannels * (BitsPerSample / 8));
			xSFPlayer->Seek(static_cast<unsigned>(decode_pos_ms), nullptr, dummyBuffer, inMod.outMod);
		}

		if (done)
		{
			inMod.outMod->CanWrite();
			if (!inMod.outMod->IsPlaying())
			{
				PostMessage(inMod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				return;
			}
			Sleep(10);
		}
		else if (static_cast<unsigned>(inMod.outMod->CanWrite()) >= ((576 * NumChannels * (BitsPerSample / 8)) << (inMod.dsp_isactive() ? 1 : 0)))
		{
			auto sampleBuffer = std::vector<std::uint8_t>(576 * NumChannels * (BitsPerSample / 8));
			unsigned samplesWritten = 0;
			done = xSFPlayer->FillBuffer(sampleBuffer, samplesWritten);
			if (samplesWritten)
			{
				inMod.SAAddPCMData(reinterpret_cast<char *>(&sampleBuffer[0]), NumChannels, BitsPerSample, static_cast<int>(decode_pos_ms));
				inMod.VSAAddPCMData(reinterpret_cast<char *>(&sampleBuffer[0]), NumChannels, BitsPerSample, static_cast<int>(decode_pos_ms));
				if (inMod.dsp_isactive())
					samplesWritten = inMod.dsp_dosamples(reinterpret_cast<short *>(&sampleBuffer[0]), samplesWritten, BitsPerSample, NumChannels, xSFPlayer->GetSampleRate());
				decode_pos_ms += samplesWritten * 1000.0 / xSFPlayer->GetSampleRate();
				inMod.outMod->Write(reinterpret_cast<char *>(&sampleBuffer[0]), samplesWritten * NumChannels * (BitsPerSample / 8));
			}
		}
		else
			Sleep(20);
	}
}

void config(HWND hwndParent)
{
	xSFConfig->CallConfigDialog(hwndParent);
	if (xSFPlayer)
		xSFConfig->CopyConfigToMemory(xSFPlayer.get(), false);
}

void about(HWND hwndParent)
{
	xSFConfig->About(hwndParent);
}

void init()
{
	wxEntryStart(inMod.hDllInstance);
	xSFApp.reset(XSFApp::Create());
	wxApp::SetInstance(xSFApp.get());
	xSFConfig.reset(XSFConfig::Create());
	xSFConfig->LoadConfig();
	xSFConfig->GenerateDialogs();
	xSFConfig->SetHInstance(inMod.hDllInstance);
}

void quit()
{
	xSFPlayer.reset();
	xSFConfig.reset();
	xSFApp.reset();
	wxEntryCleanup();
}

void getFileInfo(const in_char *file, in_char *title, int *length_in_ms)
{
	const XSFFile *xSF;
	bool toFree = false;
	if (!file || !*file)
		xSF = xSFFile;
	else
	{
		try
		{
			xSF = new XSFFile(file);
		}
		catch (const std::exception &)
		{
			if (title)
				CopyToString("", title);
			if (length_in_ms)
				*length_in_ms = -1000;
			return;
		}
		toFree = true;
	}
	if (title)
		CopyToString(xSF->GetFormattedTitle(xSFConfig->GetTitleFormat()).substr(0, GETFILEINFO_TITLE_LENGTH - 1), title);
	if (length_in_ms)
		*length_in_ms = xSF->GetLengthMS(xSFConfig->GetDefaultLength()) + xSF->GetFadeMS(xSFConfig->GetDefaultFade());
	if (toFree)
		delete xSF;
}

int infoBox(const in_char *file, HWND hwndParent)
{
	auto xSF = std::make_unique<XSFFile>();
	if (!file || !*file)
		*xSF = *xSFFile;
	else
	{
		try
		{
			auto tmpxSF = std::make_unique<XSFFile>(file);
			*xSF = *tmpxSF;
		}
		catch (const std::exception &)
		{
			return INFOBOX_UNCHANGED;
		}
	}
	// TODO: Eventually make a dialog box for editing the info
	/*xSFFileInInfo = xSF.get();
	xSFConfig->CallInfoDialog(inMod.hDllInstance, hwndParent);*/
	auto tags = xSF->GetAllTags();
	auto keys = tags.GetKeys();
	std::wstring info;
	for (unsigned x = 0, numTags = keys.size(); x < numTags; ++x)
	{
		if (x)
			info += L"\n";
		info += ConvertFuncs::StringToWString(keys[x] + "=" + tags[keys[x]]);
	}
	MessageBoxW(hwndParent, info.c_str(), xSF->GetFilenameWithoutPath().wstring().c_str(), MB_OK);
	return INFOBOX_EDITED;
}

int isOurFile(const in_char *)
{
	return 0;
}

int play(const in_char *fn)
{
	try
	{
		auto tmpxSFPlayer = std::unique_ptr<XSFPlayer>(XSFPlayer::Create(fn));
		xSFConfig->CopyConfigToMemory(tmpxSFPlayer.get(), true);
		if (!tmpxSFPlayer->Load())
			return 1;
		xSFConfig->CopyConfigToMemory(tmpxSFPlayer.get(), false);
		xSFFile = tmpxSFPlayer->GetXSFFile();
		paused = false;
		seek_needed = -1;
		decode_pos_ms = 0.0;

		int maxlatency = inMod.outMod->Open(tmpxSFPlayer->GetSampleRate(), NumChannels, BitsPerSample, -1, -1);
		if (maxlatency < 0)
			return 1;
		inMod.SetInfo((tmpxSFPlayer->GetSampleRate() * NumChannels * BitsPerSample) / 1000, tmpxSFPlayer->GetSampleRate() / 1000, NumChannels, 1);
		inMod.SAVSAInit(maxlatency, tmpxSFPlayer->GetSampleRate());
		inMod.VSASetInfo(tmpxSFPlayer->GetSampleRate(), NumChannels);
		inMod.outMod->SetVolume(-666);

		xSFPlayer = std::move(tmpxSFPlayer);
		killThread = false;
		thread_handle.reset(new std::thread(playThread));
		return 0;
	}
	catch (const std::exception &)
	{
		return 1;
	}
}

void pause()
{
	paused = true;
	inMod.outMod->Pause(1);
}

void unPause()
{
	paused = false;
	inMod.outMod->Pause(0);
}

int isPaused()
{
	return paused;
}

void stop()
{
	killThread = true;
	thread_handle->join();
	thread_handle.reset();
	inMod.outMod->Close();
	inMod.SAVSADeInit();
	xSFPlayer.reset();
	xSFFile = nullptr;
}

int getLength()
{
	if (!xSFFile->HasFile())
		return -1000;
	return xSFFile->GetLengthMS(xSFConfig->GetDefaultLength()) + xSFFile->GetFadeMS(xSFConfig->GetDefaultFade());
}

int getOutputTime()
{
	return inMod.outMod->GetOutputTime();
}

void setOutputTime(int time_in_ms)
{
	seek_needed = time_in_ms;
}

void setVolume(int volume)
{
	inMod.outMod->SetVolume(volume);
}

void setPan(int pan)
{
	inMod.outMod->SetPan(pan);
}

void eqSet(int, char [10], int)
{
}

In_Module inMod =
{
	IN_VER,
	const_cast<char *>(XSFConfig::CommonNameWithVersion().c_str()), /* Unsafe but Winamp's SDK requires this */
	nullptr, /* Filled by Winamp */
	nullptr, /* Filled by Winamp */
	const_cast<char *>(XSFPlayer::WinampExts), /* Unsafe but Winamp's SDK requires this */
	1,
	IN_MODULE_FLAG_USES_OUTPUT_PLUGIN | IN_MODULE_FLAG_REPLAYGAIN,
	config,
	about,
	init,
	quit,
	getFileInfo,
	infoBox,
	isOurFile,
	play,
	pause,
	unPause,
	isPaused,
	stop,
	getLength,
	getOutputTime,
	setOutputTime,
	setVolume,
	setPan,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* Vis stuff, filled by Winamp */
	nullptr, nullptr, /* DSP stuff, filled by Winamp */
	eqSet,
	nullptr, /* Filled by Winamp */
	nullptr /* Filled by Winamp */
};

extern "C" __declspec(dllexport) In_Module *winampGetInModule2()
{
	return &inMod;
}

static eq_str eqstr;

template<typename T> int wrapperWinampGetExtendedFileInfo(const XSFFile &file, const char *data, T *dest, std::size_t destlen)
{
	if (eqstr(data, "type"))
	{
		dest[0] = '0';
		dest[1] = 0;
		return 1;
	}
	else if (eqstr(data, "family"))
		return 0;
	else
	{
		try
		{
			std::string tagToGet = data;
			if (eqstr(data, "album"))
				tagToGet = "game";
			std::string tag = "";
			if (!file.GetTagExists(tagToGet))
			{
				if (eqstr(tagToGet, "replaygain_track_gain"))
					return 1;
				return 0;
			}
			else if (eqstr(tagToGet, "length"))
				tag = std::to_string(file.GetLengthMS(xSFConfig->GetDefaultLength()) + file.GetFadeMS(xSFConfig->GetDefaultFade()));
			else
				tag = file.GetTagValue(tagToGet);
			CopyToString(tag.substr(0, destlen - 1), dest);
			return 1;
		}
		catch (const std::exception &)
		{
			return 0;
		}
	}
}

extern "C" __declspec(dllexport) int winampGetExtendedFileInfo(const char *fn, const char *data, char *dest, std::size_t destlen)
{
	try
	{
		auto file = XSFFile(fn);
		return wrapperWinampGetExtendedFileInfo(file, data, dest, destlen);
	}
	catch (const std::exception &)
	{
		return 0;
	}
}

extern "C" __declspec(dllexport) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, std::size_t destlen)
{
	try
	{
		auto file = XSFFile(fn);
		return wrapperWinampGetExtendedFileInfo(file, data, dest, destlen);
	}
	catch (const std::exception &)
	{
		return 0;
	}
}

static std::unique_ptr<XSFFile> extendedXSFFile;

int wrapperWinampSetExtendedFileInfo(const char *data, const wchar_t *val)
{
	extendedXSFFile->SetTag(data, val);
	return 1;
}

extern "C" __declspec(dllexport) int winampSetExtendedFileInfo(const char *fn, const char *data, const wchar_t *val)
{
	try
	{
		if (!extendedXSFFile || extendedXSFFile->GetFilepath() != fn)
			extendedXSFFile.reset(new XSFFile(fn));
		return wrapperWinampSetExtendedFileInfo(data, val);
	}
	catch (const std::exception &)
	{
		return 0;
	}
}

extern "C" __declspec(dllexport) int winampSetExtendedFileInfoW(const wchar_t *fn, const char *data, const wchar_t *val)
{
	try
	{
		if (!extendedXSFFile || extendedXSFFile->GetFilepath() != fn)
			extendedXSFFile.reset(new XSFFile(fn));
		return wrapperWinampSetExtendedFileInfo(data, val);
	}
	catch (const std::exception &)
	{
		return 0;
	}
}

extern "C" __declspec(dllexport) int winampWriteExtendedFileInfo()
{
	if (!extendedXSFFile || extendedXSFFile->GetFilepath().empty())
		return 0;
	extendedXSFFile->SaveFile();
	return 1;
}

extern "C" __declspec(dllexport) int winampClearExtendedFileInfoW(const wchar_t *)
{
	return 0;
}

std::intptr_t wrapperWinampGetExtendedRead_open(std::unique_ptr<XSFPlayer> &&tmpxSFPlayer, int *size, int *bps, int *nch, int *srate)
{
	xSFConfig->CopyConfigToMemory(tmpxSFPlayer.get(), true);
	if (!tmpxSFPlayer->Load())
		return 0;
	tmpxSFPlayer->IgnoreVolume();
	xSFConfig->CopyConfigToMemory(tmpxSFPlayer.get(), false);
	if (size)
		*size = tmpxSFPlayer->GetLengthInSamples() * NumChannels * (BitsPerSample / 8);
	if (bps)
		*bps = BitsPerSample;
	if (nch)
		*nch = NumChannels;
	if (srate)
		*srate = tmpxSFPlayer->GetSampleRate();
	return reinterpret_cast<std::intptr_t>(tmpxSFPlayer.release());
}

extern "C" __declspec(dllexport) std::intptr_t winampGetExtendedRead_open(const char *fn, int *size, int *bps, int *nch, int *srate)
{
	try
	{
		auto tmpxSFPlayer = std::unique_ptr<XSFPlayer>(XSFPlayer::Create(fn));
		return wrapperWinampGetExtendedRead_open(std::move(tmpxSFPlayer), size, bps, nch, srate);
	}
	catch (const std::exception &)
	{
		return 0;
	}
}

extern "C" __declspec(dllexport) std::intptr_t winampGetExtendedRead_openW(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
{
	try
	{
		auto tmpxSFPlayer = std::unique_ptr<XSFPlayer>(XSFPlayer::Create(fn));
		return wrapperWinampGetExtendedRead_open(std::move(tmpxSFPlayer), size, bps, nch, srate);
	}
	catch (const std::exception &)
	{
		return 0;
	}
}

static int extendedSeekNeeded = -1;

extern "C" __declspec(dllexport) std::size_t winampGetExtendedRead_getData(std::intptr_t handle, char *dest, std::size_t len, int *killswitch)
{
	XSFPlayer *tmpxSFPlayer = reinterpret_cast<XSFPlayer *>(handle);
	if (!tmpxSFPlayer)
		return 0;
	if (extendedSeekNeeded != -1)
	{
		auto dummyBuffer = std::vector<std::uint8_t>(576 * NumChannels * (BitsPerSample / 8));
		if (tmpxSFPlayer->Seek(static_cast<unsigned>(extendedSeekNeeded), killswitch, dummyBuffer, nullptr))
			return 0;
		extendedSeekNeeded = -1;
	}
	unsigned copied = 0;
	bool done = false;
	while (copied + (576 * NumChannels * (BitsPerSample / 8)) < len && !done)
	{
		auto sampleBuffer = std::vector<std::uint8_t>(576 * NumChannels * (BitsPerSample / 8));
		unsigned samplesWritten = 0;
		done = tmpxSFPlayer->FillBuffer(sampleBuffer, samplesWritten);
		std::copy_n(&sampleBuffer[0], samplesWritten * NumChannels * (BitsPerSample / 8), &dest[copied]);
		copied += samplesWritten * NumChannels * (BitsPerSample / 8);
		if (killswitch && *killswitch)
			break;
	}
	return copied;
}

extern "C" __declspec(dllexport) int winampGetExtendedRead_setTime(std::intptr_t, int millisecs)
{
	extendedSeekNeeded = millisecs;
	return 1;
}

extern "C" __declspec(dllexport) void winampGetExtendedRead_close(std::intptr_t handle)
{
	XSFPlayer *tmpxSFPlayer = reinterpret_cast<XSFPlayer *>(handle);
	if (tmpxSFPlayer)
		delete tmpxSFPlayer;
}
