/*
 * xSF - Core configuration handler
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include <algorithm>
#include <string>
#include <vector>
#include "windowsh_wrapper.h"
#include <windowsx.h>
#include "XSFConfig.h"
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

bool XSFConfig::initPlayInfinitely = false;
std::string XSFConfig::initSkipSilenceOnStartSec = "5";
std::string XSFConfig::initDetectSilenceSec = "5";
std::string XSFConfig::initDefaultLength = "1:55";
std::string XSFConfig::initDefaultFade = "5";
std::string XSFConfig::initTitleFormat = "%game%[ - [%disc%.]%track%] - %title%";
double XSFConfig::initVolume = 1.0;
VolumeType XSFConfig::initVolumeType = VolumeType::ReplayGainAlbum;
PeakType XSFConfig::initPeakType = PeakType::ReplayGainTrack;

XSFConfig::XSFConfig() : playInfinitely(false), skipSilenceOnStartSec(0), detectSilenceSec(0), defaultLength(0), defaultFade(0), volume(0.0), volumeType(VolumeType::None), peakType(PeakType::None),
	sampleRate(0), titleFormat(""), configDialog(), configDialogProperty(), infoDialog(), supportedSampleRates(), configIO(XSFConfigIO::Create())
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

void XSFConfig::GenerateDialogs()
{
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

	this->configDialog = DialogBuilder().WithTitle(ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber)).IsPopup().WithBorder().WithDialogFrame().WithDialogModalFrame().WithSystemMenu().WithFont(L"MS Shell Dlg", 8);
	this->configDialog.AddGroupControl(DialogGroupBuilder(L"General").WithRelativePositionToParent(RelativePositionToParent::PositionType::FromTopLeft, Point<short>(7, 7)));
	this->configDialog.AddCheckBoxControl(DialogCheckBoxBuilder(L"Play infinitely").WithSize(60, 10).InGroup(L"General").WithRelativePositionToParent(RelativePosition::PositionType::FromTopLeft, Point<short>(6, 11)).WithTabStop().
		WithID(idPlayInfinitely));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Default play length (m:s)").WithSize(85, 8).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 7)).
		IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idDefaultLength));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Default fadeout length (m:s)").WithSize(85, 8).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).
		IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idDefaultFade));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Skip silence on start (sec)").WithSize(85, 8).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).
		IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idSkipSilenceOnStartSec));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Detect silence (sec)").WithSize(85, 8).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).
		IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idDetectSilenceSec));
	this->configDialog.AddGroupControl(DialogGroupBuilder(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 7)));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Volume").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToParent(RelativePosition::PositionType::FromTopLeft, Point<short>(6, 14)).IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idVolume));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"ReplayGain").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddComboBoxControl(DialogComboBoxBuilder().WithSize(78, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).WithID(idReplayGain).
		IsDropDownList().WithTabStop());
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Clip Protect").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddComboBoxControl(DialogComboBoxBuilder().WithSize(78, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).WithID(idClipProtect).
		IsDropDownList().WithTabStop());
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Sample Rate").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddComboBoxControl(DialogComboBoxBuilder().WithSize(50, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(5, -3)).WithID(idSampleRate).
		IsDropDownList().WithTabStop());
	this->configDialog.AddGroupControl(DialogGroupBuilder(L"Title Format").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 7)));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"NOTE: This is only used if Advanced Title Formatting is disabled in Winamp.").WithSize(150, 16).InGroup(L"Title Format").
		WithRelativePositionToParent(RelativePosition::PositionType::FromTopLeft, Point<short>(6, 11)).IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(150, 14).InGroup(L"Title Format").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 4)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idTitleFormat));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Names between percent symbols (e.g. %game%, %title%) will be replaced with the respective value from the file's tags. Using square brackets around any "
			L"items will cause them to only be displayed if there was a replacement done (e.g. [%disc%.] will display 01. if disc was in the tags as 01, but will display nothing if disc was not in the tags). Square "
			L"bracket blocks can be nested.").WithSize(150, 72).InGroup(L"Title Format").WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomLeft, Point<short>(0, 4)).IsLeftJustified());

	this->GenerateSpecificDialogs();

	this->infoDialog.AddButtonControl(DialogButtonBuilder(L"OK").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomRight, Point<short>(-104, 7)).WithID(IDOK).IsDefault().WithTabStop());
	this->infoDialog.AddButtonControl(DialogButtonBuilder(L"Cancel").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(4, 0)).WithID(IDCANCEL).WithTabStop());
	this->infoDialog.AutoSize();

	this->configDialogProperty = this->configDialog;
	this->configDialogProperty = DialogBuilder().IsChild().IsControlWindow().WithFont(L"MS Shell Dlg", 8);
	this->configDialogProperty.AutoSize();

	this->configDialog.AddButtonControl(DialogButtonBuilder(L"Reset Defaults").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomRight, Point<short>(-50, 7)).WithID(idResetDefaults).
		WithTabStop());
	this->configDialog.AddButtonControl(DialogButtonBuilder(L"OK").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromBottomRight, Point<short>(-104, 7)).WithID(IDOK).IsDefault().WithTabStop());
	this->configDialog.AddButtonControl(DialogButtonBuilder(L"Cancel").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::PositionType::FromTopRight, Point<short>(4, 0)).WithID(IDCANCEL).WithTabStop());
	this->configDialog.AutoSize();
}

INT_PTR CALLBACK XSFConfig::ConfigDialogProcStatic(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		return thisPtr->ConfigDialogProc(hwndDlg, uMsg, wParam, lParam);
	else
		return false;
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

INT_PTR CALLBACK XSFConfig::ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	switch (uMsg)
	{
		case WM_CLOSE:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		case WM_INITDIALOG:
			if (this->playInfinitely)
				SendMessageW(GetDlgItem(hwndDlg, idPlayInfinitely), BM_SETCHECK, BST_CHECKED, 0);
			SetWindowTextW(GetDlgItem(hwndDlg, idDefaultLength), ConvertFuncs::MSToWString(this->defaultLength).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idDefaultFade), ConvertFuncs::MSToWString(this->defaultFade).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idSkipSilenceOnStartSec), ConvertFuncs::MSToWString(this->skipSilenceOnStartSec).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idDetectSilenceSec), ConvertFuncs::MSToWString(this->detectSilenceSec).c_str());
			SetWindowTextW(GetDlgItem(hwndDlg, idVolume), ConvertFuncs::TrimDoubleString(std::to_wstring(this->volume)).c_str());
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Disabled"));
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Use Volume Tag"));
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Track"));
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Album"));
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_SETCURSEL, static_cast<WPARAM>(this->volumeType), 0);
			SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Disabled"));
			SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Track"));
			SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Album"));
			SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_SETCURSEL, static_cast<WPARAM>(this->peakType), 0);
			for (unsigned x = 0, rates = this->supportedSampleRates.size(); x < rates; ++x)
			{
				unsigned rate = this->supportedSampleRates[x];
				SendMessageW(GetDlgItem(hwndDlg, idSampleRate), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(std::to_wstring(rate).c_str()));
				if (this->sampleRate == rate)
					SendMessageW(GetDlgItem(hwndDlg, idSampleRate), CB_SETCURSEL, x, 0);
			}
			SetWindowTextW(GetDlgItem(hwndDlg, idTitleFormat), ConvertFuncs::StringToWString(this->titleFormat).c_str());
			break;
		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
					this->SaveConfigDialog(hwndDlg);
					this->SaveConfig();
					EndDialog(hwndDlg, IDOK);
					break;
				case IDCANCEL:
					EndDialog(hwndDlg, IDCANCEL);
					break;
				case idResetDefaults:
					this->ResetConfigDefaults(hwndDlg);
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
	DialogBoxIndirectParamW(hInstance, this->configDialog.GenerateTemplate(), hwndParent, XSFConfig::ConfigDialogProcStatic, reinterpret_cast<LPARAM>(this));
}

void XSFConfig::CallInfoDialog(HINSTANCE hInstance, HWND hwndParent)
{
	DialogBoxIndirectParamW(hInstance, this->infoDialog.GenerateTemplate(), hwndParent, XSFConfig::InfoDialogProcStatic, reinterpret_cast<LPARAM>(this));
}

void XSFConfig::ResetConfigDefaults(HWND hwndDlg)
{
	SendMessageW(GetDlgItem(hwndDlg, idPlayInfinitely), BM_SETCHECK, XSFConfig::initPlayInfinitely ? BST_CHECKED : BST_UNCHECKED, 0);
	SetWindowTextW(GetDlgItem(hwndDlg, idDefaultLength), ConvertFuncs::StringToWString(XSFConfig::initDefaultLength).c_str());
	SetWindowTextW(GetDlgItem(hwndDlg, idDefaultFade), ConvertFuncs::StringToWString(XSFConfig::initDefaultFade).c_str());
	SetWindowTextW(GetDlgItem(hwndDlg, idSkipSilenceOnStartSec), ConvertFuncs::StringToWString(XSFConfig::initSkipSilenceOnStartSec).c_str());
	SetWindowTextW(GetDlgItem(hwndDlg, idDetectSilenceSec), ConvertFuncs::StringToWString(XSFConfig::initDetectSilenceSec).c_str());
	SetWindowTextW(GetDlgItem(hwndDlg, idVolume), ConvertFuncs::TrimDoubleString(std::to_wstring(XSFConfig::initVolume)).c_str());
	SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_SETCURSEL, static_cast<WPARAM>(XSFConfig::initVolumeType), 0);
	SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_SETCURSEL, static_cast<WPARAM>(XSFConfig::initPeakType), 0);
	auto found = std::find(this->supportedSampleRates.begin(), this->supportedSampleRates.end(), XSFConfig::initSampleRate);
	SendMessageW(GetDlgItem(hwndDlg, idSampleRate), CB_SETCURSEL, found - this->supportedSampleRates.begin(), 0);
	SetWindowTextW(GetDlgItem(hwndDlg, idTitleFormat), ConvertFuncs::StringToWString(XSFConfig::initTitleFormat).c_str());

	this->ResetSpecificConfigDefaults(hwndDlg);
}

void XSFConfig::SaveConfigDialog(HWND hwndDlg)
{
	this->playInfinitely = SendMessageW(GetDlgItem(hwndDlg, idPlayInfinitely), BM_GETCHECK, 0, 0) == BST_CHECKED;
	this->defaultLength = ConvertFuncs::StringToMS(this->GetTextFromWindow(GetDlgItem(hwndDlg, idDefaultLength)));
	this->defaultFade = ConvertFuncs::StringToMS(this->GetTextFromWindow(GetDlgItem(hwndDlg, idDefaultFade)));
	this->skipSilenceOnStartSec = ConvertFuncs::StringToMS(this->GetTextFromWindow(GetDlgItem(hwndDlg, idSkipSilenceOnStartSec)));
	this->detectSilenceSec = ConvertFuncs::StringToMS(this->GetTextFromWindow(GetDlgItem(hwndDlg, idDetectSilenceSec)));
	this->volume = convertTo<double>(this->GetTextFromWindow(GetDlgItem(hwndDlg, idVolume)));
	this->volumeType = static_cast<VolumeType>(SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_GETCURSEL, 0, 0));
	this->peakType = static_cast<PeakType>(SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_GETCURSEL, 0, 0));
	this->sampleRate = XSFConfig::supportedSampleRates[SendMessageW(GetDlgItem(hwndDlg, idSampleRate), CB_GETCURSEL, 0, 0)];
	this->titleFormat = ConvertFuncs::WStringToString(this->GetTextFromWindow(GetDlgItem(hwndDlg, idTitleFormat)));

	this->SaveSpecificConfigDialog(hwndDlg);
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
