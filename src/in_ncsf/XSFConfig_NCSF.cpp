/*
 * xSF - NCSF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include <bitset>
#include <sstream>
#include <string>
#include <cstddef>
#include "windowsh_wrapper.h"
#include <windowsx.h>
#include "XSFConfig.h"
#include "XSFConfig_NCSF.h"
#include "XSFConfigDialog_NCSF.h"
#include "XSFPlayer_NCSF.h"
#include "convert.h"
#ifndef NDEBUG
# include <cstdint>
# include "SSEQPlayer/common.h"
# include "SSEQPlayer/consts.h"
# include "resource.h"
# include <CommCtrl.h>
#endif

class wxWindow;
class XSFConfigDialog;
class XSFPlayer;

enum
{
	idInterpolation = 1000,
	idMutes
};

const unsigned XSFConfig::initSampleRate = 44100;
const std::string XSFConfig::commonName = "NCSF Decoder";
const std::string XSFConfig::versionNumber = "1.11.1";

XSFConfig *XSFConfig::Create()
{
	return new XSFConfig_NCSF();
}

XSFConfig_NCSF::XSFConfig_NCSF() : XSFConfig(), interpolation(0), mutes()
#ifndef NDEBUG
	, useSoundViewDialog(false), soundViewData()
#endif
{
	this->supportedSampleRates.push_back(8000);
	this->supportedSampleRates.push_back(11025);
	this->supportedSampleRates.push_back(16000);
	this->supportedSampleRates.push_back(22050);
	this->supportedSampleRates.push_back(32000);
	this->supportedSampleRates.push_back(44100);
	this->supportedSampleRates.push_back(48000);
	this->supportedSampleRates.push_back(88200);
	this->supportedSampleRates.push_back(96000);
	this->supportedSampleRates.push_back(176400);
	this->supportedSampleRates.push_back(192000);
}

void XSFConfig_NCSF::LoadSpecificConfig()
{
#ifndef NDEBUG
	this->useSoundViewDialog = this->configIO->GetValue("UseSoundViewDialog", XSFConfig_NCSF::initUseSoundViewDialog);
#endif
	this->interpolation = this->configIO->GetValue("Interpolation", XSFConfig_NCSF::initInterpolation);
	std::stringstream mutesSS(this->configIO->GetValue("Mutes", XSFConfig_NCSF::initMutes));
	mutesSS >> this->mutes;
}

void XSFConfig_NCSF::SaveSpecificConfig()
{
#ifndef NDEBUG
	this->configIO->SetValue("UseSoundViewDialog", this->useSoundViewDialog);
#endif
	this->configIO->SetValue("Interpolation", this->interpolation);
	this->configIO->SetValue("Mutes", this->mutes.to_string<char>());
}

void XSFConfig_NCSF::InitializeSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto ncsfDialog = static_cast<XSFConfigDialog_NCSF *>(dialog);
#ifndef NDEBUG
	ncsfDialog->useSoundView = this->useSoundViewDialog;
#endif
	ncsfDialog->interpolation = static_cast<int>(this->interpolation);
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		if (this->mutes[x])
			ncsfDialog->mute.Add(x);
}

void XSFConfig_NCSF::ResetSpecificConfigDefaults(XSFConfigDialog *dialog)
{
	auto ncsfDialog = static_cast<XSFConfigDialog_NCSF *>(dialog);
#ifndef NDEBUG
	ncsfDialog->useSoundView = XSFConfig_NCSF::initUseSoundViewDialog;
#endif
	ncsfDialog->interpolation = XSFConfig_NCSF::initInterpolation;
	ncsfDialog->mute.Clear();
	auto tmpMutes = std::bitset<16>(XSFConfig_NCSF::initMutes);
	for (std::size_t x = 0, numMutes = tmpMutes.size(); x < numMutes; ++x)
		if (tmpMutes[x])
			ncsfDialog->mute.Add(x);
}

void XSFConfig_NCSF::SaveSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto ncsfDialog = static_cast<XSFConfigDialog_NCSF *>(dialog);
#ifndef NDEBUG
	this->useSoundViewDialog = ncsfDialog->useSoundView;
#endif
	this->interpolation = ncsfDialog->interpolation;
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		this->mutes[x] = ncsfDialog->mute.Index(x) != wxNOT_FOUND;
}

void XSFConfig_NCSF::CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool)
{
	auto NCSFPlayer = static_cast<XSFPlayer_NCSF *>(xSFPlayer);
#ifndef NDEBUG
	NCSFPlayer->SetUseSoundViewDialog(this->useSoundViewDialog);
#endif
	NCSFPlayer->SetInterpolation(this->interpolation);
	NCSFPlayer->SetMutes(this->mutes);
}

void XSFConfig_NCSF::About(HWND parent)
{
	MessageBoxW(parent, ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber + ", using xSF Winamp plugin framework (based on the vio*sf plugins) by Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]\n\n"
		"Utilizes code adapted from the FeOS Sound System library by fincs, git revision 5204c55 on GitHub, for audio playback.").c_str(), ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber).c_str(), MB_OK);
}

XSFConfigDialog *XSFConfig_NCSF::CreateDialogBox(wxWindow *window, const std::string &title)
{
	return new XSFConfigDialog_NCSF(*this, window, title);
}

#ifndef NDEBUG
INT_PTR CALLBACK XSFConfig_NCSF::SoundViewDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto data = reinterpret_cast<SoundViewData *>(GetWindowLongW(hwndDlg, DWLP_USER));
	if (!data && uMsg != WM_INITDIALOG)
		return false;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			if (!data)
			{
				data = reinterpret_cast<SoundViewData *>(lParam);
				SetWindowLongW(hwndDlg, DWLP_USER, reinterpret_cast<LONG>(data));
			}
			data->hDlg = hwndDlg;
			for (int chanId = 0; chanId < 16; ++chanId)
			{
				SendDlgItemMessageW(hwndDlg, IDC_SOUND0VOLBAR + chanId, PBM_SETRANGE, 0, MAKELPARAM(0, 128));
				SendDlgItemMessageW(hwndDlg, IDC_SOUND0PANBAR + chanId, PBM_SETRANGE, 0, MAKELPARAM(0, 128));
				SendDlgItemMessageW(hwndDlg, IDC_SOUND0MUTE + chanId, BM_SETCHECK, data->config->mutes[chanId], 0);
			}
			break;
		case WM_CLOSE:
		case WM_DESTROY:
			data->config->CloseSoundView();
			break;
		case WM_COMMAND:
			{
				auto id = GET_WM_COMMAND_ID(wParam, lParam);
				switch (id)
				{
					case IDC_BUTTON_VOLMODE:
						data->volModeAlternative = !!IsDlgButtonChecked(hwndDlg, IDC_BUTTON_VOLMODE);
						break;
					case IDC_SOUND0MUTE:
					case IDC_SOUND1MUTE:
					case IDC_SOUND2MUTE:
					case IDC_SOUND3MUTE:
					case IDC_SOUND4MUTE:
					case IDC_SOUND5MUTE:
					case IDC_SOUND6MUTE:
					case IDC_SOUND7MUTE:
					case IDC_SOUND8MUTE:
					case IDC_SOUND9MUTE:
					case IDC_SOUND10MUTE:
					case IDC_SOUND11MUTE:
					case IDC_SOUND12MUTE:
					case IDC_SOUND13MUTE:
					case IDC_SOUND14MUTE:
					case IDC_SOUND15MUTE:
						data->config->mutes[id - IDC_SOUND0MUTE] = !!IsDlgButtonChecked(hwndDlg, id);
						data->config->SaveConfig();
						data->player->SetMutes(data->config->mutes);
						break;
					case IDC_SOUND_UNMUTE_ALL:
						for (int chanId = 0; chanId < 16; ++chanId)
							SendDlgItemMessageW(hwndDlg, IDC_SOUND0MUTE + chanId, BM_SETCHECK, false, 0);
						data->config->mutes.reset();
						data->config->SaveConfig();
						data->player->SetMutes(data->config->mutes);
						break;
					default:
						return false;
				}
			}
			break;
		default:
			return false;
	}

	return true;
}

void XSFConfig_NCSF::CallSoundView(XSFPlayer *xSFPlayer, HINSTANCE hInstance, HWND hwndParent)
{
	this->soundViewData.reset(new SoundViewData);
	this->soundViewData->config = this;
	this->soundViewData->player = static_cast<XSFPlayer_NCSF *>(xSFPlayer);

	auto hDlg = CreateDialogParamW(hInstance, MAKEINTRESOURCEW(IDD_SOUND_VIEW), hwndParent, XSFConfig_NCSF::SoundViewDialogProc, reinterpret_cast<LPARAM>(this->soundViewData.get()));
	if (!hDlg)
	{
		this->soundViewData.reset();
		return;
	}

	ShowWindow(hDlg, SW_SHOW);
	UpdateWindow(hDlg);
}

static inline void ProgressSetPosImmediate(HWND hDlg, int nIDDlgItem, int nPos)
{
	int nMin = SendDlgItemMessageW(hDlg, nIDDlgItem, PBM_GETRANGE, true, reinterpret_cast<LPARAM>(nullptr));
	int nMax = SendDlgItemMessageW(hDlg, nIDDlgItem, PBM_GETRANGE, false, reinterpret_cast<LPARAM>(nullptr));

	// get rid of fancy progress animation since Windows Vista
	// http://stackoverflow.com/questions/1061715/how-do-i-make-tprogressbar-stop-lagging
	if (nPos < nMax)
	{
		SendDlgItemMessageW(hDlg, nIDDlgItem, PBM_SETPOS, nPos + 1, 0);
		SendDlgItemMessageW(hDlg, nIDDlgItem, PBM_SETPOS, nPos, 0); // This will set Progress backwards and give an instant update
	}
	else
	{
		SendDlgItemMessageW(hDlg, nIDDlgItem, PBM_SETRANGE32, nMin, nPos + 1);
		SendDlgItemMessageW(hDlg, nIDDlgItem, PBM_SETPOS, nPos + 1, 0);
		SendDlgItemMessageW(hDlg, nIDDlgItem, PBM_SETRANGE32, nMin, nPos); // This will also set Progress backwards also so instant update
	}
}

static inline std::int32_t muldiv7(std::int32_t val, std::uint8_t mul)
{
	return mul == 127 ? val : ((val * mul) >> 7);
}

void XSFConfig_NCSF::RefreshSoundView()
{
	auto player = this->soundViewData->player;
	auto hDlg = this->soundViewData->hDlg;
	std::wstring buf;
	for (std::size_t chanId = 0; chanId < 16; ++chanId)
	{
		const auto &chn = player->GetChannel(chanId);

		if (chn.state > ChannelState::Start)
		{
			ProgressSetPosImmediate(hDlg, IDC_SOUND0PANBAR + chanId, muldiv7(128, chn.reg.panning));
			std::uint8_t datashift = chn.reg.volumeDiv;
			if (datashift == 3)
				datashift = 4;
			std::int32_t vol = muldiv7(128, chn.reg.volumeMul) >> datashift;
			ProgressSetPosImmediate(hDlg, IDC_SOUND0VOLBAR + chanId, vol);

			if (this->soundViewData->volModeAlternative)
				buf = std::to_wstring(chn.reg.volumeMul) + L"/" + std::to_wstring(1 << datashift);
			else
				buf = std::to_wstring(vol);
			SetDlgItemTextW(hDlg, IDC_SOUND0VOL + chanId, buf.c_str());

			if (!chn.reg.panning)
				buf = L"L";
			else if (chn.reg.panning == 64)
				buf = L"C";
			else if (chn.reg.panning == 127)
				buf = L"R";
			else if (chn.reg.panning < 64)
				buf = L"L" + std::to_wstring(64 - chn.reg.panning);
			else //if (chn.reg.panning > 64)
				buf = L"R" + std::to_wstring(chn.reg.panning - 64);
			SetDlgItemTextW(hDlg, IDC_SOUND0PAN + chanId, buf.c_str());

			static const std::wstring modes[] = { L"Manual", L"Loop Infinite", L"One-Shot", L"Prohibited" };
			SetDlgItemTextW(hDlg, IDC_SOUND0REPEATMODE + chanId, (std::to_wstring(chn.reg.repeatMode) + L" (" + modes[chn.reg.repeatMode] + L")").c_str());

			if (chn.reg.format != 3)
			{
				static const std::wstring formats[] = { L"PCM8", L"PCM16", L"IMA-ADPCM" };
				SetDlgItemTextW(hDlg, IDC_SOUND0FORMAT + chanId, (std::to_wstring(chn.reg.format) + L" (" + formats[chn.reg.format] + L")").c_str());
			}
			else
			{
				buf = L"3 (";
				if (chanId < 8)
					buf += L"PSG/Noise?";
				else if (chanId < 14)
					buf += ConvertFuncs::TrimDoubleString(std::to_wstring(chn.reg.waveDuty == 7 ? 0 : 12.5 * (chn.reg.waveDuty + 1))) + L"% Square";
				else
					buf += L"Noise";
				buf += L")";
				SetDlgItemTextW(hDlg, IDC_SOUND0FORMAT + chanId, buf.c_str());
			}

			static const std::wstring states[] = { L"NONE", L"START", L"ATTACK", L"DECAY", L"SUSTAIN", L"RELEASE" };
			SetDlgItemTextW(hDlg, IDC_SOUND0STATE + chanId, states[ConvertFuncs::ToIntegral(chn.state)].c_str());

			SetDlgItemTextW(hDlg, IDC_SOUND0PNT + chanId, (L"samp #" + std::to_wstring(chn.reg.loopStart)).c_str());

			std::wstring tmpBuf = ConvertFuncs::StringToWString(NumToHexString(chn.reg.timer)).substr(2);
			buf = L"$" + tmpBuf + L" (";
			tmpBuf = ConvertFuncs::TrimDoubleString(std::to_wstring((ARM7_CLOCK / 2) / static_cast<double>(0x10000 - chn.reg.timer) / 8));
			if (tmpBuf.find('.') != std::wstring::npos)
				tmpBuf = tmpBuf.substr(0, tmpBuf.find('.') + 2);
			buf += tmpBuf + L" Hz)";
			SetDlgItemTextW(hDlg, IDC_SOUND0TMR + chanId, buf.c_str());

			SetDlgItemTextW(hDlg, IDC_SOUND0POSLEN + chanId, (L"samp #" + std::to_wstring(static_cast<std::uint32_t>(chn.reg.samplePosition)) + L" / " + std::to_wstring(chn.reg.totalLength)).c_str());
		}
		else if (this->soundViewData->channelLastStates[chanId] != ChannelState::None)
		{
			ProgressSetPosImmediate(hDlg, IDC_SOUND0PANBAR + chanId, 0);
			ProgressSetPosImmediate(hDlg, IDC_SOUND0VOLBAR + chanId, 0);
			SetDlgItemTextW(hDlg, IDC_SOUND0VOL + chanId, L"---");
			SetDlgItemTextW(hDlg, IDC_SOUND0PAN + chanId, L"---");
			SetDlgItemTextW(hDlg, IDC_SOUND0REPEATMODE + chanId, L"---");
			SetDlgItemTextW(hDlg, IDC_SOUND0FORMAT + chanId, L"---");
			SetDlgItemTextW(hDlg, IDC_SOUND0STATE + chanId, L"NONE");
			SetDlgItemTextW(hDlg, IDC_SOUND0PNT + chanId, L"---");
			SetDlgItemTextW(hDlg, IDC_SOUND0TMR + chanId, L"---");
			SetDlgItemTextW(hDlg, IDC_SOUND0POSLEN + chanId, L"---");
		}

		this->soundViewData->channelLastStates[chanId] = chn.state;
	}
}

void XSFConfig_NCSF::CloseSoundView()
{
	if (this->soundViewData)
	{
		DestroyWindow(this->soundViewData->hDlg);
		this->soundViewData.reset();
	}
}
#endif
