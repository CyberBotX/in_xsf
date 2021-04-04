/*
 * xSF - Core configuration handler
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>
#include <wx/app.h>
#include <wx/nativewin.h>
#include "windowsh_wrapper.h"
#include <windowsx.h>
#include "XSFConfig.h"
#include "XSFConfigDialog.h"
#include "XSFFile.h"
#include "XSFPlayer.h"
#include "convert.h"

enum
{
	idPlayInfinitely = 500,
	idDefaultLength,
	idDefaultFade,
	idSkipSilenceOnStartSec,
	idDetectSilenceSec,
	idVolume,
	idReplayGain,
	idClipProtect,
	idSampleRate,
	idTitleFormat,
	idResetDefaults,
	idInfoTitle = 600,
	idInfoArtist,
	idInfoGame,
	idInfoYear,
	idInfoGenre,
	idInfoCopyright,
	idInfoComment
};

XSFConfig::XSFConfig() : playInfinitely(false), skipSilenceOnStartSec(0), detectSilenceSec(0), defaultLength(0), defaultFade(0), volume(0.0), volumeType(VolumeType::None), peakType(PeakType::None),
	sampleRate(0), titleFormat(""), infoDialog(), supportedSampleRates(), configIO(XSFConfigIO::Create())
{
}

const std::string &XSFConfig::CommonNameWithVersion()
{
	static const auto commonNameWithVersion = XSFConfig::commonName + " v" + XSFConfig::versionNumber;
	return commonNameWithVersion;
}

std::wstring XSFConfig::GetTextFromWindow(HWND hwnd)
{
	auto length = SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
	auto value = std::vector<wchar_t>(length + 1);
	length = SendMessageW(hwnd, WM_GETTEXT, length + 1, reinterpret_cast<LPARAM>(&value[0]));
	return std::wstring(value.begin(), value.begin() + length);
}

void XSFConfig::LoadConfig()
{
	this->playInfinitely = this->configIO->GetValue("PlayInfinitely", XSFConfig::initPlayInfinitely);
	this->skipSilenceOnStartSec = ConvertFuncs::StringToMS(this->configIO->GetValue("SkipSilenceOnStartSec", XSFConfig::initSkipSilenceOnStartSec));
	this->detectSilenceSec = ConvertFuncs::StringToMS(this->configIO->GetValue("DetectSilenceSec", XSFConfig::initDetectSilenceSec));
	this->defaultLength = ConvertFuncs::StringToMS(this->configIO->GetValue("DefaultLength", XSFConfig::initDefaultLength));
	this->defaultFade = ConvertFuncs::StringToMS(this->configIO->GetValue("DefaultFade", XSFConfig::initDefaultFade));
	this->volume = this->configIO->GetValue("Volume", XSFConfig::initVolume);
	this->volumeType = this->configIO->GetValue("VolumeType", XSFConfig::initVolumeType);
	this->peakType = this->configIO->GetValue("PeakType", XSFConfig::initPeakType);
	this->sampleRate = this->configIO->GetValue("SampleRate", XSFConfig::initSampleRate);
	this->titleFormat = this->configIO->GetValue("TitleFormat", XSFConfig::initTitleFormat);

	this->LoadSpecificConfig();
}

void XSFConfig::SaveConfig()
{
	this->configIO->SetValue("PlayInfinitely", this->playInfinitely);
	this->configIO->SetValue("SkipSilenceOnStartSec", ConvertFuncs::MSToString(this->skipSilenceOnStartSec));
	this->configIO->SetValue("DetectSilenceSec", ConvertFuncs::MSToString(this->detectSilenceSec));
	this->configIO->SetValue("DefaultLength", ConvertFuncs::MSToString(this->defaultLength));
	this->configIO->SetValue("DefaultFade", ConvertFuncs::MSToString(this->defaultFade));
	this->configIO->SetValue("Volume", this->volume);
	this->configIO->SetValue("VolumeType", this->volumeType);
	this->configIO->SetValue("PeakType", this->peakType);
	this->configIO->SetValue("SampleRate", this->sampleRate);
	this->configIO->SetValue("TitleFormat", this->titleFormat);

	this->SaveSpecificConfig();
}

class XSFConfigApp : public wxApp
{
	HWND parent;
	XSFConfig *config;
public:
	XSFConfigApp() : parent(nullptr), config(nullptr)
	{
	}

	XSFConfigApp(HWND parent, XSFConfig *config) : parent(parent), config(config)
	{
	}

	bool OnInit() override
	{
		// NOTE: This is only good enough to make it so the dialog box isn't shown on the taskbar, it is not good enough to make the dialog modal to Winamp's preferences screen... it'll actually crash if someone changes to another section of preferences...

		auto window = new wxNativeContainerWindow(this->parent);
		auto dialog = this->config->CreateDialogBox(window, XSFConfig::commonName + " v" + XSFConfig::versionNumber);
		dialog->Finalize();
		this->config->InitializeConfigDialog(dialog);
		if (dialog->ShowModal() == wxID_OK)
		{
			this->config->SaveConfigDialog(dialog);
			this->config->SaveConfig();
		}
		dialog->Destroy();
		window->Destroy();
		return true;
	}
};

wxIMPLEMENT_APP_NO_MAIN(XSFConfigApp);

void XSFConfig::GenerateDialogs()
{
	// NOTE: Eventually this will be done away with when I decide to make an actual info dialog box using wxWidgets.

	this->infoDialog = DialogBuilder().IsPopup().WithBorder().WithDialogFrame().WithDialogModalFrame().WithSystemMenu().WithFont(L"MS Shell Dlg", 8);
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Title:").WithSize(50, 8).WithRelativePositionToParent(RelativePosition::PositionType::FromTopLeft, Point<short>(7, 10)).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoTitle));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Artist:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoArtist));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Game:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoGame));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Year:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoYear));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Genre:").WithSize(25, 8).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, 3)).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(140, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoGenre));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Copyright:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 4).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoCopyright));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Comment:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 54).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().WithAutoVScroll().WithBorder().
		WithTabStop().WithID(idInfoComment).WithVerticalScrollbar().WithWantReturn().IsMultiline());

	this->infoDialog.AddButtonControl(DialogButtonBuilder(L"OK").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomRight, Point<short>(-104, 7)).WithID(IDOK).IsDefault().WithTabStop());
	this->infoDialog.AddButtonControl(DialogButtonBuilder(L"Cancel").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(4, 0)).WithID(IDCANCEL).WithTabStop());
	this->infoDialog.AutoSize();
}

INT_PTR CALLBACK XSFConfig::InfoDialogProcStatic(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	XSFConfig *thisPtr = nullptr;

	if (uMsg == WM_INITDIALOG)
	{
		thisPtr = reinterpret_cast<XSFConfig *>(lParam);

		SetWindowLongPtr(hwndDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(thisPtr));
	}
	else
		thisPtr = reinterpret_cast<XSFConfig *>(GetWindowLongPtr(hwndDlg, DWLP_USER));

	if (thisPtr)
		return thisPtr->InfoDialogProc(hwndDlg, uMsg, wParam, lParam);
	else
		return false;
}

extern XSFFile *xSFFileInInfo;

INT_PTR CALLBACK XSFConfig::InfoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	switch (uMsg)
	{
		case WM_CLOSE:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		case WM_INITDIALOG:
			SetWindowTextW(hwndDlg, ConvertFuncs::StringToWString(xSFFileInInfo->GetFilename()).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoTitle), ConvertFuncs::StringToWString(xSFFileInInfo->GetTagValue("title")).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoArtist), ConvertFuncs::StringToWString(xSFFileInInfo->GetTagValue("artist")).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoGame), ConvertFuncs::StringToWString(xSFFileInInfo->GetTagValue("game")).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoYear), ConvertFuncs::StringToWString(xSFFileInInfo->GetTagValue("year")).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoGenre), ConvertFuncs::StringToWString(xSFFileInInfo->GetTagValue("genre")).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoCopyright), ConvertFuncs::StringToWString(xSFFileInInfo->GetTagValue("copyright")).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoComment), ConvertFuncs::StringToWString(xSFFileInInfo->GetTagValue("comment")).c_str());
			break;
		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
					xSFFileInInfo->SetTag("title", this->GetTextFromWindow(GetDlgItem(hwndDlg, idInfoTitle)));
					xSFFileInInfo->SetTag("artist", this->GetTextFromWindow(GetDlgItem(hwndDlg, idInfoArtist)));
					xSFFileInInfo->SetTag("game", this->GetTextFromWindow(GetDlgItem(hwndDlg, idInfoGame)));
					xSFFileInInfo->SetTag("year", this->GetTextFromWindow(GetDlgItem(hwndDlg, idInfoYear)));
					xSFFileInInfo->SetTag("genre", this->GetTextFromWindow(GetDlgItem(hwndDlg, idInfoGenre)));
					xSFFileInInfo->SetTag("copyright", this->GetTextFromWindow(GetDlgItem(hwndDlg, idInfoCopyright)));
					xSFFileInInfo->SetTag("comment", this->GetTextFromWindow(GetDlgItem(hwndDlg, idInfoComment)));
					if (!xSFFileInInfo->GetTagExists("utf8"))
						xSFFileInInfo->SetTag("utf8", "1");
					EndDialog(hwndDlg, IDOK);
					break;
				case IDCANCEL:
					EndDialog(hwndDlg, IDCANCEL);
					break;
				default:
					return false;
			}
			break;
		default:
			return false;
	}
	return true;
}

void XSFConfig::CallConfigDialog(HINSTANCE hInstance, HWND hwndParent)
{
	auto app = new XSFConfigApp(hwndParent, this);
	wxApp::SetInstance(app);
	wxEntryStart(hInstance);
	app->OnInit();
	wxEntryCleanup();
}

void XSFConfig::CallInfoDialog(HINSTANCE hInstance, HWND hwndParent)
{
	DialogBoxIndirectParamW(hInstance, this->infoDialog.GenerateTemplate(), hwndParent, XSFConfig::InfoDialogProcStatic, reinterpret_cast<LPARAM>(this));
}

void XSFConfig::InitializeConfigDialog(XSFConfigDialog *dialog)
{
	dialog->playInfinitely = this->playInfinitely;
	dialog->defaultPlayLength = ConvertFuncs::MSToString(this->defaultLength);
	dialog->defaultFadeoutLength = ConvertFuncs::MSToString(this->defaultFade);
	dialog->skipSilenceOnStart = ConvertFuncs::MSToString(this->skipSilenceOnStartSec);
	dialog->detectSilence = ConvertFuncs::MSToString(this->detectSilenceSec);
	dialog->volume = this->volume;
	dialog->replayGain = static_cast<std::underlying_type_t<VolumeType>>(this->volumeType);
	dialog->clipProtect = static_cast<std::underlying_type_t<PeakType>>(this->peakType);
	auto found = std::find(this->supportedSampleRates.begin(), this->supportedSampleRates.end(), this->sampleRate);
	dialog->sampleRate = found == this->supportedSampleRates.end() ? 0 : found - this->supportedSampleRates.begin();
	dialog->titleFormat = this->titleFormat;

	this->InitializeSpecificConfigDialog(dialog);
}

void XSFConfig::ResetConfigDefaults(XSFConfigDialog *dialog)
{
	dialog->playInfinitely = XSFConfig::initPlayInfinitely;
	dialog->defaultPlayLength = XSFConfig::initDefaultLength;
	dialog->defaultFadeoutLength = XSFConfig::initDefaultFade;
	dialog->skipSilenceOnStart = XSFConfig::initSkipSilenceOnStartSec;
	dialog->detectSilence = XSFConfig::initDetectSilenceSec;
	dialog->volume = XSFConfig::initVolume;
	dialog->replayGain = static_cast<std::underlying_type_t<VolumeType>>(XSFConfig::initVolumeType);
	dialog->clipProtect = static_cast<std::underlying_type_t<PeakType>>(XSFConfig::initPeakType);
	auto found = std::find(this->supportedSampleRates.begin(), this->supportedSampleRates.end(), XSFConfig::initSampleRate);
	dialog->sampleRate = found == this->supportedSampleRates.end() ? 0 : found - this->supportedSampleRates.begin();
	dialog->titleFormat = XSFConfig::initTitleFormat;

	this->ResetSpecificConfigDefaults(dialog);
}

void XSFConfig::SaveConfigDialog(XSFConfigDialog *dialog)
{
	this->playInfinitely = dialog->playInfinitely;
	this->defaultLength = ConvertFuncs::StringToMS(dialog->defaultPlayLength);
	this->defaultFade = ConvertFuncs::StringToMS(dialog->defaultFadeoutLength);
	this->skipSilenceOnStartSec = ConvertFuncs::StringToMS(dialog->skipSilenceOnStart);
	this->detectSilenceSec = ConvertFuncs::StringToMS(dialog->detectSilence);
	this->volume = dialog->volume;
	this->volumeType = static_cast<VolumeType>(dialog->replayGain);
	this->peakType = static_cast<PeakType>(dialog->clipProtect);
	this->sampleRate = this->supportedSampleRates[dialog->sampleRate];
	this->titleFormat = dialog->titleFormat.ToStdString();

	this->SaveSpecificConfigDialog(dialog);
}

void XSFConfig::CopyConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad)
{
	if (preLoad)
		xSFPlayer->SetSampleRate(this->sampleRate);

	this->CopySpecificConfigToMemory(xSFPlayer, preLoad);
}

void XSFConfig::SetHInstance(HINSTANCE hInstance)
{
	this->configIO->SetHInstance(hInstance);
}

HINSTANCE XSFConfig::GetHInstance() const
{
	return this->configIO->GetHInstance();
}

const std::vector<unsigned> &XSFConfig::GetSupportedSampleRates() const
{
	return this->supportedSampleRates;
}

bool XSFConfig::GetPlayInfinitely() const
{
	return this->playInfinitely;
}

unsigned long XSFConfig::GetSkipSilenceOnStartSec() const
{
	return this->skipSilenceOnStartSec;
}

unsigned long XSFConfig::GetDetectSilenceSec() const
{
	return this->detectSilenceSec;
}

unsigned long XSFConfig::GetDefaultLength() const
{
	return this->defaultLength;
}

unsigned long XSFConfig::GetDefaultFade() const
{
	return this->defaultFade;
}

double XSFConfig::GetVolume() const
{
	return this->volume;
}

VolumeType XSFConfig::GetVolumeType() const
{
	return this->volumeType;
}

PeakType XSFConfig::GetPeakType() const
{
	return this->peakType;
}

const std::string &XSFConfig::GetTitleFormat() const
{
	return this->titleFormat;
}
