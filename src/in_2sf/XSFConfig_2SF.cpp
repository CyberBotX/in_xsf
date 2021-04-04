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
#include "XSFConfig_2SF.h"
#include "XSFConfigDialog_2SF.h"
#include "convert.h"
#include "desmume/NDSSystem.h"
#include "desmume/version.h"

class wxWindow;
class XSFConfigDialog;
class XSFPlayer;

enum
{
	idInterpolation = 1000,
	idMutes
};

const unsigned XSFConfig::initSampleRate = static_cast<unsigned>(DESMUME_SAMPLE_RATE);
const std::string XSFConfig::commonName = "2SF Decoder";
const std::string XSFConfig::versionNumber = "0.9b";

XSFConfig *XSFConfig::Create()
{
	return new XSFConfig_2SF();
}

XSFConfig_2SF::XSFConfig_2SF() : XSFConfig(), interpolation(0), mutes()
{
	this->supportedSampleRates.push_back(static_cast<unsigned>(DESMUME_SAMPLE_RATE));
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

void XSFConfig_2SF::InitializeSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto twosfDialog = static_cast<XSFConfigDialog_2SF *>(dialog);
	twosfDialog->interpolation = static_cast<int>(this->interpolation);
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		if (this->mutes[x])
			twosfDialog->mute.Add(x);
}

void XSFConfig_2SF::ResetSpecificConfigDefaults(XSFConfigDialog *dialog)
{
	auto twosfDialog = static_cast<XSFConfigDialog_2SF *>(dialog);
	twosfDialog->interpolation = XSFConfig_2SF::initInterpolation;
	twosfDialog->mute.Clear();
	auto tmpMutes = std::bitset<16>(XSFConfig_2SF::initMutes);
	for (std::size_t x = 0, numMutes = tmpMutes.size(); x < numMutes; ++x)
		if (tmpMutes[x])
			twosfDialog->mute.Add(x);
}

void XSFConfig_2SF::SaveSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto twosfDialog = static_cast<XSFConfigDialog_2SF *>(dialog);
	this->interpolation = twosfDialog->interpolation;
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		this->mutes[x] = twosfDialog->mute.Index(x) != wxNOT_FOUND;
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

XSFConfigDialog *XSFConfig_2SF::CreateDialogBox(wxWindow *window, const std::string &title)
{
	return new XSFConfigDialog_2SF(*this, window, title);
}
