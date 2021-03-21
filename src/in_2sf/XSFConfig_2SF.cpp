/*
 * xSF - 2SF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include <bitset>
#include <sstream>
#include <string>
#include <cstddef>
#include "windowsh_wrapper.h"
#include "XSFConfig.h"
#include "convert.h"
#include "desmume/NDSSystem.h"
#include "desmume/version.h"

class XSFPlayer;

enum
{
	idInterpolation = 1000,
	idMutes
};

class XSFConfig_2SF : public XSFConfig
{
protected:
	static unsigned initInterpolation;
	static std::string initMutes;

	friend class XSFConfig;
	unsigned interpolation;
	std::bitset<16> mutes;

	XSFConfig_2SF();
	void LoadSpecificConfig();
	void SaveSpecificConfig();
	void GenerateSpecificDialogs();
	INT_PTR CALLBACK ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ResetSpecificConfigDefaults(HWND hwndDlg);
	void SaveSpecificConfigDialog(HWND hwndDlg);
	void CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad);
public:
	void About(HWND parent);
};

unsigned XSFConfig::initSampleRate = DESMUME_SAMPLE_RATE;
std::string XSFConfig::commonName = "2SF Decoder";
std::string XSFConfig::versionNumber = "0.9b";
unsigned XSFConfig_2SF::initInterpolation = 2;
std::string XSFConfig_2SF::initMutes = "0000000000000000";

XSFConfig *XSFConfig::Create()
{
	return new XSFConfig_2SF();
}

XSFConfig_2SF::XSFConfig_2SF() : XSFConfig(), interpolation(0), mutes()
{
	this->supportedSampleRates.push_back(DESMUME_SAMPLE_RATE);
}

void XSFConfig_2SF::LoadSpecificConfig()
{
	this->interpolation = this->configIO->GetValue("Interpolation", XSFConfig_2SF::initInterpolation);
	std::stringstream mutesSS(this->configIO->GetValue("Mutes", XSFConfig_2SF::initMutes));
	mutesSS >> this->mutes;
}

void XSFConfig_2SF::SaveSpecificConfig()
{
	this->configIO->SetValue("Interpolation", this->interpolation);
	this->configIO->SetValue("Mutes", this->mutes.to_string<char>());
}

void XSFConfig_2SF::GenerateSpecificDialogs()
{
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Interpolation").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddComboBoxControl(DialogComboBoxBuilder().WithSize(78, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).WithID(idInterpolation).IsDropDownList().
		WithTabStop());
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Mute").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddListBoxControl(DialogListBoxBuilder().WithSize(78, 45).WithExactHeight().InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).WithID(idMutes).WithBorder().
		WithVerticalScrollbar().WithMultipleSelect().WithTabStop());
}

INT_PTR CALLBACK XSFConfig_2SF::ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			// Interpolation
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"No Interpolation"));
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Linear Interpolation"));
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Cosine Interpolation"));
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Sharp Interpolation"));
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_SETCURSEL, this->interpolation, 0);
			// Mutes
			for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
			{
				SendMessageW(GetDlgItem(hwndDlg, idMutes), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>((L"SPU " + std::to_wstring(x + 1)).c_str()));
				SendMessageW(GetDlgItem(hwndDlg, idMutes), LB_SETSEL, this->mutes[x], x);
			}
			break;
		case WM_COMMAND:
			break;
	}

	return XSFConfig::ConfigDialogProc(hwndDlg, uMsg, wParam, lParam);
}

void XSFConfig_2SF::ResetSpecificConfigDefaults(HWND hwndDlg)
{
	SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_SETCURSEL, XSFConfig_2SF::initInterpolation, 0);
	auto tmpMutes = std::bitset<16>(XSFConfig_2SF::initMutes);
	for (std::size_t x = 0, numMutes = tmpMutes.size(); x < numMutes; ++x)
		SendMessageW(GetDlgItem(hwndDlg, idMutes), LB_SETSEL, tmpMutes[x], x);
}

void XSFConfig_2SF::SaveSpecificConfigDialog(HWND hwndDlg)
{
	this->interpolation = static_cast<unsigned>(SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_GETCURSEL, 0, 0));
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		this->mutes[x] = !!SendMessageW(GetDlgItem(hwndDlg, idMutes), LB_GETSEL, x, 0);
}

void XSFConfig_2SF::CopySpecificConfigToMemory(XSFPlayer *, bool preLoad)
{
	if (!preLoad)
	{
		CommonSettings.spuInterpolationMode = static_cast<SPUInterpolationMode>(this->interpolation);
		for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
			CommonSettings.spu_muteChannels[x] = this->mutes[x];
	}
}

void XSFConfig_2SF::About(HWND parent)
{
	MessageBoxW(parent, ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber + ", using xSF Winamp plugin framework (based on the vio*sf plugins) by Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]\n\n"
		"Utilizes modified " + EMU_DESMUME_NAME_AND_VERSION() + " for audio playback.").c_str(), ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber).c_str(), MB_OK);
}
