/*
 * xSF - 2SF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2014-09-17
 *
 * Partially based on the vio*sf framework
 */

#include <bitset>
#include "XSFPlayer.h"
#include "XSFConfig.h"
#include "convert.h"
#include "BigSString.h"
#include "desmume/NDSSystem.h"
#include "desmume/version.h"

enum
{
	idInterpolation = 1000,
	idMutes
};

class XSFConfig_2SF : public XSFConfig
{
protected:
	static unsigned initInterpolation;
	static std::wstring initMutes;

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

unsigned XSFConfig::initSampleRate = 44100;
std::wstring XSFConfig::commonName = L"2SF Decoder";
std::wstring XSFConfig::versionNumber = L"0.9b";
unsigned XSFConfig_2SF::initInterpolation = 2;
std::wstring XSFConfig_2SF::initMutes = L"0000000000000000";

XSFConfig *XSFConfig::Create()
{
	return new XSFConfig_2SF();
}

XSFConfig_2SF::XSFConfig_2SF() : XSFConfig(), interpolation(0), mutes()
{
	this->supportedSampleRates.push_back(44100);
}

void XSFConfig_2SF::LoadSpecificConfig()
{
	this->interpolation = this->configIO->GetValue(L"Interpolation", XSFConfig_2SF::initInterpolation);
	std::wstringstream mutesSS(this->configIO->GetValue(L"Mutes", XSFConfig_2SF::initMutes));
	mutesSS >> this->mutes;
}

void XSFConfig_2SF::SaveSpecificConfig()
{
	this->configIO->SetValue(L"Interpolation", this->interpolation);
	this->configIO->SetValue(L"Mutes", this->mutes.to_string<wchar_t>());
}

void XSFConfig_2SF::GenerateSpecificDialogs()
{
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Interpolation").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddComboBoxControl(DialogComboBoxBuilder().WithSize(78, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).WithID(idInterpolation).IsDropDownList().
		WithTabStop());
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Mute").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddListBoxControl(DialogListBoxBuilder().WithSize(78, 45).WithExactHeight().InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).WithID(idMutes).WithBorder().
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
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_SETCURSEL, this->interpolation, 0);
			// Mutes
			for (size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
			{
				SendMessageW(GetDlgItem(hwndDlg, idMutes), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>((L"SPU " + wstringify(x + 1)).c_str()));
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
	for (size_t x = 0, numMutes = tmpMutes.size(); x < numMutes; ++x)
		SendMessageW(GetDlgItem(hwndDlg, idMutes), LB_SETSEL, tmpMutes[x], x);
}

void XSFConfig_2SF::SaveSpecificConfigDialog(HWND hwndDlg)
{
	this->interpolation = static_cast<unsigned>(SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_GETCURSEL, 0, 0));
	for (size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		this->mutes[x] = !!SendMessageW(GetDlgItem(hwndDlg, idMutes), LB_GETSEL, x, 0);
}

void XSFConfig_2SF::CopySpecificConfigToMemory(XSFPlayer *, bool preLoad)
{
	if (!preLoad)
	{
		CommonSettings.spuInterpolationMode = static_cast<SPUInterpolationMode>(this->interpolation);
		for (size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
			CommonSettings.spu_muteChannels[x] = this->mutes[x];
	}
}

void XSFConfig_2SF::About(HWND parent)
{
	MessageBoxW(parent, (XSFConfig::commonName + L" v" + XSFConfig::versionNumber + L", using xSF Winamp plugin framework (based on the vio*sf plugins) by Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]\n\n"
		L"Utilizes modified " + String(EMU_DESMUME_NAME_AND_VERSION()).GetWStr() + L" for audio playback.").c_str(), (XSFConfig::commonName + L" v" + XSFConfig::versionNumber).c_str(), MB_OK);
}
