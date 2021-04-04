/*
 * xSF - GSF configuration
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
#include "XSFConfig_GSF.h"
#include "XSFConfigDialog_GSF.h"
#include "convert.h"
#include "vbam/gba/Sound.h"

class wxWindow;
class XSFConfigDialog;
class XSFPlayer;

enum
{
	idLowPassFiltering = 1000,
	idMutes
};

const unsigned XSFConfig::initSampleRate = 44100;
const std::string XSFConfig::commonName = "GSF Decoder";
const std::string XSFConfig::versionNumber = "0.9b";

XSFConfig *XSFConfig::Create()
{
	return new XSFConfig_GSF();
}

XSFConfig_GSF::XSFConfig_GSF() : XSFConfig(), lowPassFiltering(false), mutes()
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

void XSFConfig_GSF::LoadSpecificConfig()
{
	this->lowPassFiltering = this->configIO->GetValue("LowPassFiltering", XSFConfig_GSF::initLowPassFiltering);
	std::stringstream mutesSS(this->configIO->GetValue("Mutes", XSFConfig_GSF::initMutes));
	mutesSS >> this->mutes;
}

void XSFConfig_GSF::SaveSpecificConfig()
{
	this->configIO->SetValue("LowPassFiltering", this->lowPassFiltering);
	this->configIO->SetValue("Mutes", this->mutes.to_string<char>());
}

void XSFConfig_GSF::InitializeSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto gsfDialog = static_cast<XSFConfigDialog_GSF *>(dialog);
	gsfDialog->lowPassFiltering = this->lowPassFiltering;
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		if (this->mutes[x])
			gsfDialog->mute.Add(x);
}

void XSFConfig_GSF::ResetSpecificConfigDefaults(XSFConfigDialog *dialog)
{
	auto gsfDialog = static_cast<XSFConfigDialog_GSF *>(dialog);
	gsfDialog->lowPassFiltering = XSFConfig_GSF::initLowPassFiltering;
	gsfDialog->mute.Clear();
	auto tmpMutes = std::bitset<6>(XSFConfig_GSF::initMutes);
	for (std::size_t x = 0, numMutes = tmpMutes.size(); x < numMutes; ++x)
		if (tmpMutes[x])
			gsfDialog->mute.Add(x);
}

void XSFConfig_GSF::SaveSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto gsfDialog = static_cast<XSFConfigDialog_GSF *>(dialog);
	this->lowPassFiltering = gsfDialog->lowPassFiltering;
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		this->mutes[x] = gsfDialog->mute.Index(x) != wxNOT_FOUND;
}

void XSFConfig_GSF::CopySpecificConfigToMemory(XSFPlayer *, bool preLoad)
{
	if (!preLoad)
	{
		soundInterpolation = this->lowPassFiltering;
		unsigned long tmpMutes = this->mutes.to_ulong();
		soundSetEnable((((tmpMutes & 0x30) << 4) | (tmpMutes & 0xF)) ^ 0x30F);
	}
}

void XSFConfig_GSF::About(HWND parent)
{
	MessageBoxW(parent, ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber + ", using xSF Winamp plugin framework (based on the vio*sf plugins) by Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]\n\n"
		"Utilizes modified VBA-M, SVN revision 1231, for audio playback.").c_str(), ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber).c_str(), MB_OK);
}

XSFConfigDialog *XSFConfig_GSF::CreateDialogBox(wxWindow *window, const std::string &title)
{
	return new XSFConfigDialog_GSF(*this, window, title);
}
