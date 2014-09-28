/*
 * xSF - NCSF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2014-09-24
 *
 * Partially based on the vio*sf framework
 */

#include <bitset>
#include "XSFPlayer_NCSF.h"
#include "XSFConfig.h"
#include "convert.h"

enum
{
	idInterpolation = 1000,
	idMutes
};

class XSFConfig_NCSF : public XSFConfig
{
protected:
	static unsigned initInterpolation;
	static std::string initMutes;

	friend class XSFConfig;
	unsigned interpolation;
	std::bitset<16> mutes;

	XSFConfig_NCSF();
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
std::string XSFConfig::commonName = "NCSF Decoder";
std::string XSFConfig::versionNumber = "1.8.1";
unsigned XSFConfig_NCSF::initInterpolation = 4;
std::string XSFConfig_NCSF::initMutes = "0000000000000000";

XSFConfig *XSFConfig::Create()
{
	return new XSFConfig_NCSF();
}

XSFConfig_NCSF::XSFConfig_NCSF() : XSFConfig(), interpolation(0), mutes()
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
	this->interpolation = this->configIO->GetValue("Interpolation", XSFConfig_NCSF::initInterpolation);
	std::stringstream mutesSS(this->configIO->GetValue("Mutes", XSFConfig_NCSF::initMutes));
	mutesSS >> this->mutes;
}

void XSFConfig_NCSF::SaveSpecificConfig()
{
	this->configIO->SetValue("Interpolation", this->interpolation);
	this->configIO->SetValue("Mutes", this->mutes.to_string<char>());
}

void XSFConfig_NCSF::GenerateSpecificDialogs()
{
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Interpolation").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddComboBoxControl(DialogComboBoxBuilder().WithSize(110, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).WithID(idInterpolation).
		IsDropDownList().WithTabStop());
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Mute").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddListBoxControl(DialogListBoxBuilder().WithSize(78, 45).WithExactHeight().InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).WithID(idMutes).
		WithBorder().WithVerticalScrollbar().WithMultipleSelect().WithTabStop());
}

INT_PTR CALLBACK XSFConfig_NCSF::ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			// Interpolation
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None"));
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Linear"));
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"4-point, 3rd-order Legrange"));
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"6-point, 5th-order Legrange"));
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"16-point Sinc (Lanczos Window)"));
			SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_SETCURSEL, this->interpolation, 0);
			// Mutes
			for (int x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
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

void XSFConfig_NCSF::ResetSpecificConfigDefaults(HWND hwndDlg)
{
	SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_SETCURSEL, XSFConfig_NCSF::initInterpolation, 0);
	auto tmpMutes = std::bitset<16>(XSFConfig_NCSF::initMutes);
	for (int x = 0, numMutes = tmpMutes.size(); x < numMutes; ++x)
		SendMessageW(GetDlgItem(hwndDlg, idMutes), LB_SETSEL, tmpMutes[x], x);
}

void XSFConfig_NCSF::SaveSpecificConfigDialog(HWND hwndDlg)
{
	this->interpolation = static_cast<unsigned>(SendMessageW(GetDlgItem(hwndDlg, idInterpolation), CB_GETCURSEL, 0, 0));
	for (int x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		this->mutes[x] = !!SendMessageW(GetDlgItem(hwndDlg, idMutes), LB_GETSEL, x, 0);
}

void XSFConfig_NCSF::CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool)
{
	XSFPlayer_NCSF *NCSFPlayer = static_cast<XSFPlayer_NCSF *>(xSFPlayer);
	NCSFPlayer->SetInterpolation(this->interpolation);
	NCSFPlayer->SetMutes(this->mutes);
}

void XSFConfig_NCSF::About(HWND parent)
{
	MessageBoxW(parent, ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber + ", using xSF Winamp plugin framework (based on the vio*sf plugins) by Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]\n\n"
		"Utilizes code adapted from the FeOS Sound System library by fincs, git revision 5204c55 on GitHub, for audio playback.").c_str(), ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber).c_str(), MB_OK);
}
