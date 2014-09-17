/*
 * xSF - Winamp plugin
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2014-09-17
 *
 * Partially based on the vio*sf framework
 */

#include "XSFPlayer.h"
#include "XSFConfig.h"
#include "windowsh_wrapper.h"
#include <winamp/in2.h>
#include <winamp/wa_ipc.h>

extern In_Module inMod;
const XSFFile *xSFFile = nullptr;
XSFFile *xSFFileInInfo = nullptr;
XSFPlayer *xSFPlayer = nullptr;
XSFConfig *xSFConfig = nullptr;
bool paused;
int seek_needed;
double decode_pos_ms;
HANDLE thread_handle = INVALID_HANDLE_VALUE;
bool killThread = false;

static const unsigned NumChannels = 2;
static const unsigned BitsPerSample = 16;

DWORD WINAPI playThread(void *b)
{
	bool done = false;
	while (!*static_cast<bool *>(b))
	{
		if (seek_needed != -1)
		{
			decode_pos_ms = seek_needed - (seek_needed % 1000);
			seek_needed = -1;
			auto dummyBuffer = std::vector<uint8_t>(576 * NumChannels * (BitsPerSample / 8));
			xSFPlayer->Seek(static_cast<unsigned>(decode_pos_ms), nullptr, dummyBuffer, inMod.outMod);
		}

		if (done)
		{
			inMod.outMod->CanWrite();
			if (!inMod.outMod->IsPlaying())
			{
				PostMessage(inMod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				return 0;
			}
			Sleep(10);
		}
		else if (static_cast<unsigned>(inMod.outMod->CanWrite()) >= ((576 * NumChannels * (BitsPerSample / 8)) << (inMod.dsp_isactive() ? 1 : 0)))
		{
			auto sampleBuffer = std::vector<uint8_t>(576 * NumChannels * (BitsPerSample / 8));
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
	return 0;
}

void config(HWND hwndParent)
{
	xSFConfig->CallConfigDialog(inMod.hDllInstance, hwndParent);
	if (xSFPlayer)
		xSFConfig->CopyConfigToMemory(xSFPlayer, false);
}

void about(HWND hwndParent)
{
	xSFConfig->About(hwndParent);
}

void init()
{
	xSFConfig = XSFConfig::Create();
	xSFConfig->LoadConfig();
	xSFConfig->GenerateDialogs();
}

void quit()
{
	delete xSFPlayer;
	delete xSFConfig;
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
				String("").CopyToString(title);
			if (length_in_ms)
				*length_in_ms = -1000;
			return;
		}
		toFree = true;
	}
	if (title)
	{
		String formattedTitle = xSF->GetFormattedTitle(xSFConfig->GetTitleFormat()).Substring(0, GETFILEINFO_TITLE_LENGTH - 1);
		formattedTitle.CopyToString(title);
	}
	if (length_in_ms)
		*length_in_ms = xSF->GetLengthMS(xSFConfig->GetDefaultLength()) + xSF->GetFadeMS(xSFConfig->GetDefaultFade());
	if (toFree)
		delete xSF;
}

int infoBox(const in_char *file, HWND hwndParent)
{
	auto xSF = std::unique_ptr<XSFFile>(new XSFFile());
	if (!file || !*file)
		*xSF = *xSFFile;
	else
	{
		try
		{
			auto tmpxSF = std::unique_ptr<XSFFile>(new XSFFile(file));
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
	bool utf8 = xSF->GetTagExists("utf8");
	auto tags = xSF->GetAllTags();
	auto keys = tags.GetKeys();
	String info;
	for (unsigned x = 0, numTags = keys.size(); x < numTags; ++x)
	{
		if (x)
			info += "\n";
		info += String(keys[x] + "=") + String(tags[keys[x]], utf8);
	}
	MessageBoxW(hwndParent, info.GetWStrC(), xSF->GetFilenameWithoutPath().GetWStrC(), MB_OK);
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

		xSFPlayer = tmpxSFPlayer.release();
		killThread = false;
		thread_handle = CreateThread(nullptr, 0, playThread, &killThread, 0, nullptr);
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
	if (thread_handle != INVALID_HANDLE_VALUE)
	{
		killThread = true;
		if (WaitForSingleObject(thread_handle, 2000) == WAIT_TIMEOUT)
		{
			MessageBoxW(inMod.hMainWindow, L"error asking thread to die!", L"error killing decode thread", 0);
			TerminateThread(thread_handle, 0);
		}
		CloseHandle(thread_handle);
		thread_handle = INVALID_HANDLE_VALUE;
	}
	inMod.outMod->Close();
	inMod.SAVSADeInit();
	delete xSFPlayer;
	xSFPlayer = nullptr;
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
	const_cast<char *>(XSFConfig::CommonNameWithVersion().GetStrC()), /* Unsafe but Winamp's SDK requires this */
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

template<typename T> int wrapperWinampGetExtendedFileInfo(const XSFFile &file, const char *data, T *dest, size_t destlen)
{
	if (eqstr(data, "type"))
	{
		dest[0] = '0';
		dest[1] = 0;
		return 1;
	}
	else if (eqstr(data, "family"))
	{
		return 0;
	}
	else
	{
		try
		{
			std::string tagToGet = data;
			if (eqstr(data, "album"))
				tagToGet = "game";
			if (!file.GetTagExists(tagToGet))
			{
				if (eqstr(tagToGet, "replaygain_track_gain"))
					return 1;
				return 0;
			}
			else if (eqstr(tagToGet, "length"))
			{
				String(stringify(file.GetLengthMS(xSFConfig->GetDefaultLength()) + file.GetFadeMS(xSFConfig->GetDefaultFade()))).Substring(0, destlen - 1).CopyToString(dest, true);
				return 1;
			}
			file.GetTagValue(tagToGet).Substring(0, destlen - 1).CopyToString(dest, true);
			return 1;
		}
		catch (const std::exception &)
		{
			return 0;
		}
	}
}

extern "C" __declspec(dllexport) int winampGetExtendedFileInfo(const char *fn, const char *data, char *dest, size_t destlen)
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

#ifdef _MSC_VER
extern "C" __declspec(dllexport) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, size_t destlen)
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
#endif

std::unique_ptr<XSFFile> extendedXSFFile;

int wrapperWinampSetExtendedFileInfo(const char *data, const wchar_t *val)
{
	extendedXSFFile->SetTag(data, val);
	return 1;
}

extern "C" __declspec(dllexport) int winampSetExtendedFileInfo(const char *fn, const char *data, const wchar_t *val)
{
	try
	{
		if (!extendedXSFFile || extendedXSFFile->GetFilename().GetStr() != fn)
			extendedXSFFile.reset(new XSFFile(fn));
		return wrapperWinampSetExtendedFileInfo(data, val);
	}
	catch (const std::exception &)
	{
		return 0;
	}
}

#ifdef _MSC_VER
extern "C" __declspec(dllexport) int winampSetExtendedFileInfoW(const wchar_t *fn, const char *data, const wchar_t *val)
{
	try
	{
		if (!extendedXSFFile || extendedXSFFile->GetFilename().GetWStr() != fn)
			extendedXSFFile.reset(new XSFFile(fn));
		return wrapperWinampSetExtendedFileInfo(data, val);
	}
	catch (const std::exception &)
	{
		return 0;
	}
}
#endif

extern "C" __declspec(dllexport) int winampWriteExtendedFileInfo()
{
	if (!extendedXSFFile || extendedXSFFile->GetFilename().empty())
		return 0;
	extendedXSFFile->SaveFile();
	return 1;
}

extern "C" __declspec(dllexport) int winampClearExtendedFileInfoW(const wchar_t *)
{
	return 0;
}

intptr_t wrapperWinampGetExtendedRead_open(std::unique_ptr<XSFPlayer> tmpxSFPlayer, int *size, int *bps, int *nch, int *srate)
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
	return reinterpret_cast<intptr_t>(tmpxSFPlayer.release());
}

extern "C" __declspec(dllexport) intptr_t winampGetExtendedRead_open(const char *fn, int *size, int *bps, int *nch, int *srate)
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

#ifdef _MSC_VER
extern "C" __declspec(dllexport) intptr_t winampGetExtendedRead_openW(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
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
#endif

int extendedSeekNeeded = -1;

extern "C" __declspec(dllexport) size_t winampGetExtendedRead_getData(intptr_t handle, char *dest, size_t len, int *killswitch)
{
	XSFPlayer *tmpxSFPlayer = reinterpret_cast<XSFPlayer *>(handle);
	if (!tmpxSFPlayer)
		return 0;
	if (extendedSeekNeeded != -1)
	{
		auto dummyBuffer = std::vector<uint8_t>(576 * NumChannels * (BitsPerSample / 8));
		if (tmpxSFPlayer->Seek(static_cast<unsigned>(extendedSeekNeeded), killswitch, dummyBuffer, nullptr))
			return 0;
		extendedSeekNeeded = -1;
	}
	unsigned copied = 0;
	bool done = false;
	while (copied + (576 * NumChannels * (BitsPerSample / 8)) < len && !done)
	{
		auto sampleBuffer = std::vector<uint8_t>(576 * NumChannels * (BitsPerSample / 8));
		unsigned samplesWritten = 0;
		done = tmpxSFPlayer->FillBuffer(sampleBuffer, samplesWritten);
		memcpy(&dest[copied], &sampleBuffer[0], samplesWritten * NumChannels * (BitsPerSample / 8));
		copied += samplesWritten * NumChannels * (BitsPerSample / 8);
		if (killswitch && *killswitch)
			break;
	}
	return copied;
}

extern "C" __declspec(dllexport) int winampGetExtendedRead_setTime(intptr_t, int millisecs)
{
	extendedSeekNeeded = millisecs;
	return 1;
}

extern "C" __declspec(dllexport) void winampGetExtendedRead_close(intptr_t handle)
{
	XSFPlayer *tmpxSFPlayer = reinterpret_cast<XSFPlayer *>(handle);
	if (tmpxSFPlayer)
		delete tmpxSFPlayer;
}
