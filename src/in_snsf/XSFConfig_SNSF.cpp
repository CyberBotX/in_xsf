/*
 * xSF - SNSF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 *
 * NOTE: 16-bit sound is always enabled, the player is currently limited to
 * creating a 16-bit PCM for Winamp and can not handle 8-bit sound from
 * snes9x.
 */

#include <bitset>
#include <sstream>
#include <string>
#include <cstddef>
#include <cstdint>
#include "windowsh_wrapper.h"
#include "XSFApp.h"
#include "XSFConfig.h"
#include "XSFConfig_SNSF.h"
#include "XSFConfigDialog_SNSF.h"
#include "convert.h"
#include "snes9x/apu/apu.h"

class wxWindow;
class XSFConfigDialog;
class XSFPlayer;

const unsigned XSFConfig::initSampleRate = 44100;
const std::string XSFConfig::commonName = "SNSF Decoder";
const std::string XSFConfig::versionNumber = "0.9b";

XSFApp *XSFApp::Create()
{
	return new XSFApp();
}

XSFConfig *XSFConfig::Create()
{
	return new XSFConfig_SNSF();
}

XSFConfig_SNSF::XSFConfig_SNSF() : XSFConfig(), /*sixteenBitSound(false), */reverseStereo(false), mutes(), resampler(0)
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

void XSFConfig_SNSF::LoadSpecificConfig()
{
	//this->sixteenBitSound = this->configIO->GetValue("SixteenBitSound", XSFConfig_SNSF::initSixteenBitSound);
	this->reverseStereo = this->configIO->GetValue("ReverseStereo", XSFConfig_SNSF::initReverseStereo);
	this->resampler = this->configIO->GetValue("Resampler", XSFConfig_SNSF::initResampler);
	std::stringstream mutesSS(this->configIO->GetValue("Mutes", XSFConfig_SNSF::initMutes));
	mutesSS >> this->mutes;
}

void XSFConfig_SNSF::SaveSpecificConfig()
{
	//this->configIO->SetValue("SixteenBitSound", this->sixteenBitSound);
	this->configIO->SetValue("ReverseStereo", this->reverseStereo);
	this->configIO->SetValue("Resampler", this->resampler);
	this->configIO->SetValue("Mutes", this->mutes.to_string<char>());
}

void XSFConfig_SNSF::InitializeSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto snsfDialog = static_cast<XSFConfigDialog_SNSF *>(dialog);
	snsfDialog->reverseStereo = this->reverseStereo;
	snsfDialog->resampler = static_cast<int>(this->resampler);
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		if (this->mutes[x])
			snsfDialog->mute.Add(x);
}

void XSFConfig_SNSF::ResetSpecificConfigDefaults(XSFConfigDialog *dialog)
{
	auto snsfDialog = static_cast<XSFConfigDialog_SNSF *>(dialog);
	snsfDialog->reverseStereo = XSFConfig_SNSF::initReverseStereo;
	snsfDialog->resampler = XSFConfig_SNSF::initResampler;
	snsfDialog->mute.Clear();
	auto tmpMutes = std::bitset<8>(XSFConfig_SNSF::initMutes);
	for (std::size_t x = 0, numMutes = tmpMutes.size(); x < numMutes; ++x)
		if (tmpMutes[x])
			snsfDialog->mute.Add(x);
}

void XSFConfig_SNSF::SaveSpecificConfigDialog(XSFConfigDialog *dialog)
{
	auto snsfDialog = static_cast<XSFConfigDialog_SNSF *>(dialog);
	this->reverseStereo = snsfDialog->reverseStereo;
	this->resampler = snsfDialog->resampler;
	for (std::size_t x = 0, numMutes = this->mutes.size(); x < numMutes; ++x)
		this->mutes[x] = snsfDialog->mute.Index(x) != wxNOT_FOUND;
}

void XSFConfig_SNSF::CopySpecificConfigToMemory(XSFPlayer *, bool preLoad)
{
	if (preLoad)
	{
		memset(&Settings, 0, sizeof(Settings));
		//Settings.SixteenBitSound = this->sixteenBitSound;
		Settings.ReverseStereo = this->reverseStereo;
	}
	else
		S9xSetSoundControl(static_cast<std::uint8_t>(this->mutes.to_ulong()) ^ 0xFF);
}

void XSFConfig_SNSF::About(HWND parent)
{
	MessageBoxW(parent, ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber + ", using xSF Winamp plugin framework (based on the vio*sf plugins) by Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]\n\n"
		"Utilizes modified snes9x v1.53 for audio playback.").c_str(), ConvertFuncs::StringToWString(XSFConfig::commonName + " v" + XSFConfig::versionNumber).c_str(), MB_OK);
}

XSFConfigDialog *XSFConfig_SNSF::CreateDialogBox(wxWindow *window, const std::string &title)
{
	return new XSFConfigDialog_SNSF(*this, window, title);
}
