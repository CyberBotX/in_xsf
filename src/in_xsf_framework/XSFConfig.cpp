/*
 * xSF - Core configuration handler
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-30
 *
 * Partially based on the vio*sf framework
 */

#include "XSFConfig.h"
#include "XSFPlayer.h"
#include "convert.h"
#include "BigSString.h"

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
std::wstring XSFConfig::initSkipSilenceOnStartSec = L"5";
std::wstring XSFConfig::initDetectSilenceSec = L"5";
std::wstring XSFConfig::initDefaultLength = L"1:55";
std::wstring XSFConfig::initDefaultFade = L"5";
std::wstring XSFConfig::initTitleFormat = L"%game%[ - [%disc%.]%track%] - %title%";
double XSFConfig::initVolume = 1.0;
VolumeType XSFConfig::initVolumeType = VOLUMETYPE_REPLAYGAIN_ALBUM;
PeakType XSFConfig::initPeakType = PEAKTYPE_REPLAYGAIN_TRACK;

XSFConfig::XSFConfig() : playInfinitely(false), skipSilenceOnStartSec(0), detectSilenceSec(0), defaultLength(0), defaultFade(0), volume(0.0), volumeType(VOLUMETYPE_NONE), peakType(PEAKTYPE_NONE),
	sampleRate(0), titleFormat(L""), configDialog(), configDialogProperty(), infoDialog(), supportedSampleRates(), configIO(XSFConfigIO::Create())
{
}

std::wstring XSFConfig::GetTextFromWindow(HWND hwnd)
{
	LRESULT length = SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
	std::vector<wchar_t> value(length + 1);
	length = SendMessageW(hwnd, WM_GETTEXT, length + 1, reinterpret_cast<LPARAM>(&value[0]));
	return std::wstring(value.begin(), value.begin() + length);
}

void XSFConfig::LoadConfig()
{
	this->playInfinitely = this->configIO->GetValue(L"PlayInfinitely", XSFConfig::initPlayInfinitely);
	this->skipSilenceOnStartSec = ConvertFuncs::StringToMS(this->configIO->GetValue(L"SkipSilenceOnStartSec", XSFConfig::initSkipSilenceOnStartSec));
	this->detectSilenceSec = ConvertFuncs::StringToMS(this->configIO->GetValue(L"DetectSilenceSec", XSFConfig::initDetectSilenceSec));
	this->defaultLength = ConvertFuncs::StringToMS(this->configIO->GetValue(L"DefaultLength", XSFConfig::initDefaultLength));
	this->defaultFade = ConvertFuncs::StringToMS(this->configIO->GetValue(L"DefaultFade", XSFConfig::initDefaultFade));
	this->volume = this->configIO->GetValue(L"Volume", XSFConfig::initVolume);
	this->volumeType = static_cast<VolumeType>(this->configIO->GetValue(L"VolumeType", static_cast<int>(XSFConfig::initVolumeType)));
	this->peakType = static_cast<PeakType>(this->configIO->GetValue(L"PeakType", static_cast<int>(XSFConfig::initPeakType)));
	this->sampleRate = this->configIO->GetValue(L"SampleRate", XSFConfig::initSampleRate);
	this->titleFormat = this->configIO->GetValue(L"TitleFormat", XSFConfig::initTitleFormat);

	this->LoadSpecificConfig();
}

void XSFConfig::SaveConfig()
{
	this->configIO->SetValue(L"PlayInfinitely", this->playInfinitely);
	this->configIO->SetValue(L"SkipSilenceOnStartSec", ConvertFuncs::MSToWString(this->skipSilenceOnStartSec));
	this->configIO->SetValue(L"DetectSilenceSec", ConvertFuncs::MSToWString(this->detectSilenceSec));
	this->configIO->SetValue(L"DefaultLength", ConvertFuncs::MSToWString(this->defaultLength));
	this->configIO->SetValue(L"DefaultFade", ConvertFuncs::MSToWString(this->defaultFade));
	this->configIO->SetValue(L"Volume", this->volume);
	this->configIO->SetValue(L"VolumeType", this->volumeType);
	this->configIO->SetValue(L"PeakType", this->peakType);
	this->configIO->SetValue(L"SampleRate", this->sampleRate);
	this->configIO->SetValue(L"TitleFormat", this->titleFormat);

	this->SaveSpecificConfig();
}

void XSFConfig::GenerateDialogs()
{
	this->infoDialog = DialogBuilder().IsPopup().WithBorder().WithDialogFrame().WithDialogModalFrame().WithSystemMenu().WithFont(L"MS Shell Dlg", 8);
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Title:").WithSize(50, 8).WithRelativePositionToParent(RelativePosition::FROM_TOPLEFT, Point<short>(7, 10)).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 14).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoTitle));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Artist:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 14).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoArtist));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Game:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 14).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoGame));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Year:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoYear));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Genre:").WithSize(25, 8).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, 3)).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(140, 14).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoGenre));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Copyright:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 4).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 14).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().WithAutoHScroll().WithBorder().
		WithTabStop().WithID(idInfoCopyright));
	this->infoDialog.AddLabelControl(DialogLabelBuilder(L"Comment:").WithSize(50, 8).WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsRightJustified());
	this->infoDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(200, 54).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().WithAutoVScroll().WithBorder().
		WithTabStop().WithID(idInfoComment).WithVerticalScrollbar().WithWantReturn().IsMultiline());

	this->configDialog = DialogBuilder().WithTitle(XSFConfig::commonName + L" v" + XSFConfig::versionNumber).IsPopup().WithBorder().WithDialogFrame().WithDialogModalFrame().WithSystemMenu().WithFont(L"MS Shell Dlg", 8);
	this->configDialog.AddGroupControl(DialogGroupBuilder(L"General").WithRelativePositionToParent(RelativePositionToParent::FROM_TOPLEFT, Point<short>(7, 7)));
	this->configDialog.AddCheckBoxControl(DialogCheckBoxBuilder(L"Play infinitely").WithSize(60, 10).InGroup(L"General").WithRelativePositionToParent(RelativePosition::FROM_TOPLEFT, Point<short>(6, 11)).WithTabStop().
		WithID(idPlayInfinitely));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Default play length (m:s)").WithSize(85, 8).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 7)).
		IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idDefaultLength));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Default fadeout length (m:s)").WithSize(85, 8).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).
		IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idDefaultFade));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Skip silence on start (sec)").WithSize(85, 8).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).
		IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idSkipSilenceOnStartSec));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Detect silence (sec)").WithSize(85, 8).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).
		IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"General").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idDetectSilenceSec));
	this->configDialog.AddGroupControl(DialogGroupBuilder(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 7)));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Volume").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToParent(RelativePosition::FROM_TOPLEFT, Point<short>(6, 14)).IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(25, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idVolume));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"ReplayGain").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddComboBoxControl(DialogComboBoxBuilder().WithSize(78, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).WithID(idReplayGain).
		IsDropDownList().WithTabStop());
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Clip Protect").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddComboBoxControl(DialogComboBoxBuilder().WithSize(78, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).WithID(idClipProtect).
		IsDropDownList().WithTabStop());
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Sample Rate").WithSize(50, 8).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 10), 2).IsLeftJustified());
	this->configDialog.AddComboBoxControl(DialogComboBoxBuilder().WithSize(50, 14).InGroup(L"Output").WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(5, -3)).WithID(idSampleRate).
		IsDropDownList().WithTabStop());
	this->configDialog.AddGroupControl(DialogGroupBuilder(L"Title Format").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 7)));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"NOTE: This is only used if Advanced Title Formatting is disabled in Winamp.").WithSize(150, 16).InGroup(L"Title Format").
		WithRelativePositionToParent(RelativePosition::FROM_TOPLEFT, Point<short>(6, 11)).IsLeftJustified());
	this->configDialog.AddEditBoxControl(DialogEditBoxBuilder().WithSize(150, 14).InGroup(L"Title Format").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 4)).IsLeftJustified().
		WithAutoHScroll().WithBorder().WithTabStop().WithID(idTitleFormat));
	this->configDialog.AddLabelControl(DialogLabelBuilder(L"Names between percent symbols (e.g. %game%, %title%) will be replaced with the respective value from the file's tags.  Using square brackets around any "
			L"items will cause them to only be displayed if there was a replacement done (e.g. [%disc%.] will display 01. if disc was in the tags as 01, but will display nothing if disc was not in the tags).  Square "
			L"bracket blocks can be nested.").WithSize(150, 72).InGroup(L"Title Format").WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMLEFT, Point<short>(0, 4)).IsLeftJustified());

	this->GenerateSpecificDialogs();

	this->infoDialog.AddButtonControl(DialogButtonBuilder(L"OK").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMRIGHT, Point<short>(-104, 7)).WithID(IDOK).IsDefault().WithTabStop());
	this->infoDialog.AddButtonControl(DialogButtonBuilder(L"Cancel").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(4, 0)).WithID(IDCANCEL).WithTabStop());
	this->infoDialog.AutoSize();

	this->configDialogProperty = this->configDialog;
	this->configDialogProperty = DialogBuilder().IsChild().IsControlWindow().WithFont(L"MS Shell Dlg", 8);
	this->configDialogProperty.AutoSize();

	this->configDialog.AddButtonControl(DialogButtonBuilder(L"Reset Defaults").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMRIGHT, Point<short>(-50, 7)).WithID(idResetDefaults).
		WithTabStop());
	this->configDialog.AddButtonControl(DialogButtonBuilder(L"OK").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::FROM_BOTTOMRIGHT, Point<short>(-104, 7)).WithID(IDOK).IsDefault().WithTabStop());
	this->configDialog.AddButtonControl(DialogButtonBuilder(L"Cancel").WithSize(50, 14).WithRelativePositionToSibling(RelativePosition::FROM_TOPRIGHT, Point<short>(4, 0)).WithID(IDCANCEL).WithTabStop());
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
			SetWindowTextW(GetDlgItem(hwndDlg, idVolume), wstringify(this->volume).c_str());
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Disabled"));
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Use Volume Tag"));
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Track"));
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Album"));
			SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_SETCURSEL, this->volumeType, 0);
			SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Disabled"));
			SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Track"));
			SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Album"));
			SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_SETCURSEL, this->peakType, 0);
			for (unsigned x = 0, rates = this->supportedSampleRates.size(); x < rates; ++x)
			{
				unsigned rate = this->supportedSampleRates[x];
				SendMessageW(GetDlgItem(hwndDlg, idSampleRate), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(wstringify(rate).c_str()));
				if (this->sampleRate == rate)
					SendMessageW(GetDlgItem(hwndDlg, idSampleRate), CB_SETCURSEL, x, 0);
			}
			SetWindowTextW(GetDlgItem(hwndDlg, idTitleFormat), this->titleFormat.c_str());
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
			SetWindowTextW(hwndDlg, xSFFileInInfo->GetFilename().GetWStrC());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoTitle), xSFFileInInfo->GetTagValue("title").GetWStrC());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoArtist), xSFFileInInfo->GetTagValue("artist").GetWStrC());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoGame), xSFFileInInfo->GetTagValue("game").GetWStrC());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoYear), xSFFileInInfo->GetTagValue("year").GetWStrC());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoGenre), xSFFileInInfo->GetTagValue("genre").GetWStrC());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoCopyright), xSFFileInInfo->GetTagValue("copyright").GetWStrC());
			SetWindowTextW(GetDlgItem(hwndDlg, idInfoComment), xSFFileInInfo->GetTagValue("comment").GetWStrC());
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
	DialogBoxIndirectParam(hInstance, this->configDialog.GenerateTemplate(), hwndParent, XSFConfig::ConfigDialogProcStatic, reinterpret_cast<LPARAM>(this));
}

void XSFConfig::CallInfoDialog(HINSTANCE hInstance, HWND hwndParent)
{
	DialogBoxIndirectParam(hInstance, this->infoDialog.GenerateTemplate(), hwndParent, XSFConfig::InfoDialogProcStatic, reinterpret_cast<LPARAM>(this));
}

void XSFConfig::ResetConfigDefaults(HWND hwndDlg)
{
	SendMessageW(GetDlgItem(hwndDlg, idPlayInfinitely), BM_SETCHECK, XSFConfig::initPlayInfinitely ? BST_CHECKED : BST_UNCHECKED, 0);
	SetWindowTextW(GetDlgItem(hwndDlg, idDefaultLength), XSFConfig::initDefaultLength.c_str());
	SetWindowTextW(GetDlgItem(hwndDlg, idDefaultFade), XSFConfig::initDefaultFade.c_str());
	SetWindowTextW(GetDlgItem(hwndDlg, idSkipSilenceOnStartSec), XSFConfig::initSkipSilenceOnStartSec.c_str());
	SetWindowTextW(GetDlgItem(hwndDlg, idDetectSilenceSec), XSFConfig::initDetectSilenceSec.c_str());
	SetWindowTextW(GetDlgItem(hwndDlg, idVolume), wstringify(XSFConfig::initVolume).c_str());
	SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_SETCURSEL, XSFConfig::initVolumeType, 0);
	SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_SETCURSEL, XSFConfig::initPeakType, 0);
	std::vector<unsigned>::const_iterator found = std::find(this->supportedSampleRates.begin(), this->supportedSampleRates.end(), XSFConfig::initSampleRate);
	SendMessageW(GetDlgItem(hwndDlg, idSampleRate), CB_SETCURSEL, found - this->supportedSampleRates.begin(), 0);
	SetWindowTextW(GetDlgItem(hwndDlg, idTitleFormat), XSFConfig::initTitleFormat.c_str());

	this->ResetSpecificConfigDefaults(hwndDlg);
}

void XSFConfig::SaveConfigDialog(HWND hwndDlg)
{
	this->playInfinitely = SendMessageW(GetDlgItem(hwndDlg, idPlayInfinitely), BM_GETCHECK, 0, 0) == BST_CHECKED;
	this->defaultLength = ConvertFuncs::StringToMS(this->GetTextFromWindow(GetDlgItem(hwndDlg, idDefaultLength)));
	this->defaultFade = ConvertFuncs::StringToMS(this->GetTextFromWindow(GetDlgItem(hwndDlg, idDefaultFade)));
	this->skipSilenceOnStartSec = ConvertFuncs::StringToMS(this->GetTextFromWindow(GetDlgItem(hwndDlg, idSkipSilenceOnStartSec)));
	this->detectSilenceSec = ConvertFuncs::StringToMS(this->GetTextFromWindow(GetDlgItem(hwndDlg, idDetectSilenceSec)));
	this->volume = convertTo<double>(this->GetTextFromWindow(GetDlgItem(hwndDlg, idVolume)), false);
	this->volumeType = static_cast<VolumeType>(SendMessageW(GetDlgItem(hwndDlg, idReplayGain), CB_GETCURSEL, 0, 0));
	this->peakType = static_cast<PeakType>(SendMessageW(GetDlgItem(hwndDlg, idClipProtect), CB_GETCURSEL, 0, 0));
	this->sampleRate = XSFConfig::supportedSampleRates[SendMessageW(GetDlgItem(hwndDlg, idSampleRate), CB_GETCURSEL, 0, 0)];
	this->titleFormat = this->GetTextFromWindow(GetDlgItem(hwndDlg, idTitleFormat));

	this->SaveSpecificConfigDialog(hwndDlg);
}

void XSFConfig::CopyConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad)
{
	if (preLoad)
		xSFPlayer->SetSampleRate(this->sampleRate);

	this->CopySpecificConfigToMemory(xSFPlayer, preLoad);
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

const std::wstring &XSFConfig::GetTitleFormat() const
{
	return this->titleFormat;
}
