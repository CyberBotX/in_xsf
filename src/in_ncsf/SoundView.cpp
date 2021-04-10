/*
 * xSF - Sound View Dialog
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 *
 * Based on the Sound View dialog box from DeSmuME
 * http://desmume.org/
 */

#include <string>
#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/gauge.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>
#ifdef __GNUC__
# pragma GCC diagnostic pop
#elif defined(__clang__)
# pragma clang diagnostic pop
#endif
#include "convert.h"
#include "SoundView.h"
#include "SSEQPlayer/common.h"
#include "SSEQPlayer/consts.h"
#include "XSFCommon.h"
#include "XSFConfig_NCSF.h"
#include "XSFPlayer_NCSF.h"

class wxWindow;

enum
{
	idVolumeMode = 1,
	idUnmuteAll,
	idChannel00Mute
};

// Idea comes from https://derekwill.com/2014/06/24/combating-the-lag-of-the-winforms-progressbar/
void SoundView::GaugeSetValueImmediate(wxGauge *gauge, int value)
{
	// This whole thing is to get around the progressive animation of progress bars since Windows Vista
	if (value == gauge->GetRange())
	{
		// When the value equals the maximum, we have to temporarily increase the maximum before setting the value
		gauge->SetRange(value + 1);
		gauge->SetValue(value);
		gauge->SetRange(value);
	}
	else
		gauge->SetValue(value + 1);
	gauge->SetValue(value);
}

SoundView::SoundView(wxWindow *parent, XSFConfig_NCSF *ncsfConfig, XSFPlayer_NCSF *ncsfPlayer) : wxDialog(parent, wxID_ANY, "Sound View"), config(ncsfConfig), player(ncsfPlayer), channelData(), volumeModeAlternative(false)
{
	auto outerSizer = new wxBoxSizer(wxVERTICAL);

	auto mainSizer = new wxGridBagSizer;

	auto volumeModeButton = new wxToggleButton(this, idVolumeMode, "Vol. Mode");
	mainSizer->Add(volumeModeButton, { 0 ,0 }, { 1, 2 }, wxALIGN_LEFT | wxBOTTOM | wxLEFT, 5);
	this->Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent &)
	{
		this->volumeModeAlternative = !this->volumeModeAlternative;
	}, idVolumeMode);

	auto unmuteAllButton = new wxButton(this, idUnmuteAll, "Unmute All");
	mainSizer->Add(unmuteAllButton, { 0, 5 }, { 1, 1 }, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, 5);
	this->Bind(wxEVT_BUTTON, [&](wxCommandEvent &)
	{
		for (int i = 0; i < 16; ++i)
			this->channelData[i].muteCheckBox->SetValue(false);
		this->config->mutes.reset();
		this->config->SaveConfig();
		this->player->SetMutes(this->config->mutes);
	}, idUnmuteAll);

	outerSizer->Add(mainSizer, 1, wxALL | wxEXPAND, 10);

	for (int i = 0; i < 2; ++i)
	{
		auto volumeLabel = new wxStaticText(this, wxID_ANY, "Volume", wxDefaultPosition, { 150, -1 }, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC);
		mainSizer->Add(volumeLabel, { 1, 7 * i + 1 }, { 1, 2 }, wxALL, 1);

		auto repeatModeLabel = new wxStaticText(this, wxID_ANY, "Repeat Mode", wxDefaultPosition, { 108, -1 }, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC);
		mainSizer->Add(repeatModeLabel, { 1, 7 * i + 3 }, { 1, 1 }, wxALL, 1);

		auto stateLabel = new wxStaticText(this, wxID_ANY, "State", wxDefaultPosition, { 90, -1 }, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC);
		mainSizer->Add(stateLabel, { 1, 7 * i + 4 }, { 1, 1 }, wxALL, 1);

		auto timerValueLabel = new wxStaticText(this, wxID_ANY, "Timer Value", wxDefaultPosition, { 132, -1 }, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC);
		mainSizer->Add(timerValueLabel, { 1, 7 * i + 5 }, { 1, 1 }, wxALL, 1);

		auto panLabel = new wxStaticText(this, wxID_ANY, "Pan", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC);
		mainSizer->Add(panLabel, { 2, 7 * i + 1 }, { 1, 2 }, wxALL | wxEXPAND, 1);

		auto formatLabel = new wxStaticText(this, wxID_ANY, "Format", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC);
		mainSizer->Add(formatLabel, { 2, 7 * i + 3 }, { 1, 1 }, wxALL | wxEXPAND, 1);

		auto loopStartLabel = new wxStaticText(this, wxID_ANY, "Loop Start", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC);
		mainSizer->Add(loopStartLabel, { 2, 7 * i + 4 }, { 1, 1 }, wxALL | wxEXPAND, 1);

		auto soundPosLenLabel = new wxStaticText(this, wxID_ANY, "Sound Pos / Len", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC);
		mainSizer->Add(soundPosLenLabel, { 2, 7 * i + 5 }, { 1, 1 }, wxALL | wxEXPAND, 1);
	}

	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 2; ++j)
		{
			int channelNum = 8 * j + i;
			auto channelLabel = new wxStaticText(this, wxID_ANY, std::string("#") + (channelNum < 10 ? "0" : "") + std::to_string(channelNum));
			mainSizer->Add(channelLabel, { 2 * i + 3, 7 * j }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 1);

			auto &currentChannelData = this->channelData[channelNum];

			currentChannelData.volumeGauge = new wxGauge(this, wxID_ANY, 128, wxDefaultPosition, { 102, 18 }, wxGA_SMOOTH);
			mainSizer->Add(currentChannelData.volumeGauge, { 2 * i + 3, 7 * j + 1 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 1);

			currentChannelData.volumeLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC | wxST_NO_AUTORESIZE);
			mainSizer->Add(currentChannelData.volumeLabel, { 2 * i + 3, 7 * j + 2 }, { 1, 1 }, wxALL | wxEXPAND, 1);

			currentChannelData.repeatModeLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC | wxST_NO_AUTORESIZE);
			mainSizer->Add(currentChannelData.repeatModeLabel, { 2 * i + 3, 7 * j + 3 }, { 1, 1 }, wxALL | wxEXPAND, 1);

			currentChannelData.stateLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC | wxST_NO_AUTORESIZE);
			mainSizer->Add(currentChannelData.stateLabel, { 2 * i + 3, 7 * j + 4 }, { 1, 1 }, wxALL | wxEXPAND, 1);

			currentChannelData.timerValueLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC | wxST_NO_AUTORESIZE);
			mainSizer->Add(currentChannelData.timerValueLabel, { 2 * i + 3, 7 * j + 5 }, { 1, 1 }, wxALL | wxEXPAND, 1);

			currentChannelData.muteCheckBox = new wxCheckBox(this, idChannel00Mute + channelNum, wxEmptyString);
			mainSizer->Add(currentChannelData.muteCheckBox, { 2 * i + 4, 7 * j }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 1);
			this->Bind(wxEVT_CHECKBOX, [this, currentChannelData, channelNum](wxCommandEvent &)
			{
				this->config->mutes[channelNum] = currentChannelData.muteCheckBox->GetValue();
				this->config->SaveConfig();
				this->player->SetMutes(this->config->mutes);
			}, idChannel00Mute + channelNum);

			currentChannelData.panGauge = new wxGauge(this, wxID_ANY, 128, wxDefaultPosition, { 102, 18 }, wxGA_SMOOTH);
			mainSizer->Add(currentChannelData.panGauge, { 2 * i + 4, 7 * j + 1 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 1);

			currentChannelData.panLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC | wxST_NO_AUTORESIZE);
			mainSizer->Add(currentChannelData.panLabel, { 2 * i + 4, 7 * j + 2 }, { 1, 1 }, wxALL | wxEXPAND, 1);

			currentChannelData.formatLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC | wxST_NO_AUTORESIZE);
			mainSizer->Add(currentChannelData.formatLabel, { 2 * i + 4, 7 * j + 3 }, { 1, 1 }, wxALL | wxEXPAND, 1);

			currentChannelData.loopStartLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC | wxST_NO_AUTORESIZE);
			mainSizer->Add(currentChannelData.loopStartLabel, { 2 * i + 4, 7 * j + 4 }, { 1, 1 }, wxALL | wxEXPAND, 1);

			currentChannelData.soundPosLenLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_STATIC | wxST_NO_AUTORESIZE);
			mainSizer->Add(currentChannelData.soundPosLenLabel, { 2 * i + 4, 7 * j + 5 }, { 1, 1 }, wxALL | wxEXPAND, 1);
		}

	this->SetSizer(outerSizer);
	this->Layout();
	outerSizer->Fit(this);

	this->Center();

	this->SyncToConfig();
	this->Bind(wxEVT_IDLE, &SoundView::OnIdle, this);
}

void SoundView::OnIdle(wxIdleEvent &evt)
{
	this->Refresh();
	evt.RequestMore();
}

void SoundView::Refresh()
{
	auto ncsfPlayer = this->player;
	std::string buf;
	for (std::size_t chanId = 0; chanId < 16; ++chanId)
	{
		const auto &chn = ncsfPlayer->GetChannel(chanId);
		auto &chnData = this->channelData[chanId];

		if (chn.state > ChannelState::Start)
		{
			this->GaugeSetValueImmediate(chnData.panGauge, muldiv7(128, chn.reg.panning));
			std::uint8_t datashift = chn.reg.volumeDiv;
			if (datashift == 3)
				datashift = 4;
			std::int32_t vol = muldiv7(128, chn.reg.volumeMul) >> datashift;
			this->GaugeSetValueImmediate(chnData.volumeGauge, vol);

			if (this->volumeModeAlternative)
				buf = std::to_string(chn.reg.volumeMul) + "/" + std::to_string(1 << datashift);
			else
				buf = std::to_string(vol);
			chnData.volumeLabel->SetLabel(buf);

			if (!chn.reg.panning)
				buf = "L";
			else if (chn.reg.panning == 64)
				buf = "C";
			else if (chn.reg.panning == 127)
				buf = "R";
			else if (chn.reg.panning < 64)
				buf = "L" + std::to_string(64 - chn.reg.panning);
			else //if (chn.reg.panning > 64)
				buf = "R" + std::to_string(chn.reg.panning - 64);
			chnData.panLabel->SetLabel(buf);

			static const std::string modes[] = { "Manual", "Loop Infinite", "One-Shot", "Prohibited" };
			chnData.repeatModeLabel->SetLabel(std::to_string(chn.reg.repeatMode) + " (" + modes[chn.reg.repeatMode] + ")");

			if (chn.reg.format != 3)
			{
				static const std::string formats[] = { "PCM8", "PCM16", "IMA-ADPCM" };
				chnData.formatLabel->SetLabel(std::to_string(chn.reg.format) + " (" + formats[chn.reg.format] + ")");
			}
			else
			{
				buf = "3 (";
				if (chanId < 8)
					buf += "PSG/Noise?";
				else if (chanId < 14)
					buf += ConvertFuncs::TrimDoubleString(std::to_string(chn.reg.waveDuty == 7 ? 0 : 12.5 * (chn.reg.waveDuty + 1))) + "% Square";
				else
					buf += "Noise";
				buf += ")";
				chnData.formatLabel->SetLabel(buf);
			}

			static const std::string states[] = { "NONE", "START", "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
			chnData.stateLabel->SetLabel(states[ConvertFuncs::ToIntegral(chn.state)]);

			chnData.loopStartLabel->SetLabel("samp #" + std::to_string(chn.reg.loopStart));

			std::string tmpBuf = NumToHexString(chn.reg.timer).substr(2);
			buf = "$" + tmpBuf + " (";
			tmpBuf = ConvertFuncs::TrimDoubleString(std::to_string((ARM7_CLOCK / 2) / static_cast<double>(0x10000 - chn.reg.timer) / 8));
			if (tmpBuf.find('.') != std::string::npos)
				tmpBuf = tmpBuf.substr(0, tmpBuf.find('.') + 2);
			buf += tmpBuf + " Hz)";
			chnData.timerValueLabel->SetLabel(buf);

			chnData.soundPosLenLabel->SetLabel("samp #" + std::to_string(static_cast<std::uint32_t>(chn.reg.samplePosition)) + " / " + std::to_string(chn.reg.totalLength));
		}
		else if (chnData.lastState != ChannelState::None)
		{
			chnData.panGauge->SetValue(0);
			chnData.volumeGauge->SetValue(0);
			chnData.volumeLabel->SetLabel("---");
			chnData.panLabel->SetLabel("---");
			chnData.repeatModeLabel->SetLabel("---");
			chnData.formatLabel->SetLabel("---");
			chnData.stateLabel->SetLabel("NONE");
			chnData.loopStartLabel->SetLabel("---");
			chnData.timerValueLabel->SetLabel("---");
			chnData.soundPosLenLabel->SetLabel("---");
		}

		chnData.lastState = chn.state;
	}
}

void SoundView::SyncToConfig()
{
	for (int i = 0; i < 16; ++i)
		this->channelData[i].muteCheckBox->SetValue(this->config->mutes[i]);
}
