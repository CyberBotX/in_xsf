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
#include <cstdint>
#include "windowsh_wrapper.h"
#include "XSFApp.h"
#include "XSFApp_NCSF.h"
#include "XSFConfig.h"
#include "XSFConfig_NCSF.h"
#include "XSFConfigDialog_NCSF.h"
#include "XSFPlayer_NCSF.h"
#include "convert.h"

class wxWindow;
class XSFConfigDialog;
class XSFPlayer;

const unsigned XSFConfig::initSampleRate = 44100;
const std::string XSFConfig::commonName = "NCSF Decoder";
const std::string XSFConfig::versionNumber = "1.11.1";

XSFApp *XSFApp::Create()
{
	return new XSFApp_NCSF();
}

XSFConfig *XSFConfig::Create()
{
	return new XSFConfig_NCSF();
}

XSFConfig_NCSF::XSFConfig_NCSF() : XSFConfig(), interpolation(0), mutes(), useSoundViewDialog(false)
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
	this->useSoundViewDialog = this->configIO->GetValue("UseSoundViewDialog", XSFConfig_NCSF::initUseSoundViewDialog);
	this->interpolation = this->configIO->GetValue("Interpolation", XSFConfig_NCSF::initInterpolation);
	std::stringstream mutesSS(this->configIO->GetValue("Mutes", XSFConfig_NCSF::initMutes));
	mutesSS >> this->mutes;
}

void XSFConfig_NCSF::SaveSpecificConfig()
{
	this->configIO->SetValue("UseSoundViewDialog", this->useSoundViewDialog);
	this->configIO->SetValue("Interpolation", this->interpolation);
	this->configIO->SetValue("Mutes", this->mutes.to_string<char>());
}

void XSFConfig_NCSF::InitializeSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto ncsfDialog = static_cast<XSFConfigDialog_NCSF *>(dialog);
	ncsfDialog->useSoundView = this->useSoundViewDialog;
	ncsfDialog->interpolation = static_cast<int>(this->interpolation);
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		if (this->mutes[x])
			ncsfDialog->mute.Add(x);
}

void XSFConfig_NCSF::ResetSpecificConfigDefaults(XSFConfigDialog *dialog)
{
	auto ncsfDialog = static_cast<XSFConfigDialog_NCSF *>(dialog);
	ncsfDialog->useSoundView = XSFConfig_NCSF::initUseSoundViewDialog;
	ncsfDialog->interpolation = XSFConfig_NCSF::initInterpolation;
	ncsfDialog->mute.Clear();
	auto tmpMutes = std::bitset<16>(XSFConfig_NCSF::initMutes);
	for (std::size_t x = 0, numMutes = tmpMutes.size(); x < numMutes; ++x)
		if (tmpMutes[x])
			ncsfDialog->mute.Add(x);
}

extern std::unique_ptr<XSFApp> xSFApp;

void XSFConfig_NCSF::SaveSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto ncsfDialog = static_cast<XSFConfigDialog_NCSF *>(dialog);
	this->useSoundViewDialog = ncsfDialog->useSoundView;
	this->interpolation = ncsfDialog->interpolation;
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		this->mutes[x] = ncsfDialog->mute.Index(x) != wxNOT_FOUND;
	static_cast<XSFApp_NCSF *>(xSFApp.get())->SyncSoundViewToConfig();
}

void XSFConfig_NCSF::CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool)
{
	auto NCSFPlayer = static_cast<XSFPlayer_NCSF *>(xSFPlayer);
	NCSFPlayer->SetUseSoundViewDialog(this->useSoundViewDialog);
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
