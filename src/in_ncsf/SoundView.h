/*
 * xSF - Sound View Dialog
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 *
 * Based on the Sound View dialog box from DeSmuME
 * http://desmume.org/
 */

#pragma once

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <wx/dialog.h>
#ifdef __GNUC__
# pragma GCC diagnostic pop
#elif defined(__clang__)
# pragma clang diagnostic pop
#endif
#include "SSEQPlayer/consts.h"

class wxCheckBox;
class wxGauge;
class wxIdleEvent;
class wxStaticText;
class wxWindow;
class XSFConfig_NCSF;
class XSFPlayer_NCSF;

struct SoundViewData
{
	wxGauge *volumeGauge = nullptr;
	wxStaticText *volumeLabel = nullptr;
	wxStaticText *repeatModeLabel = nullptr;
	wxStaticText *stateLabel = nullptr;
	wxStaticText *timerValueLabel = nullptr;
	wxCheckBox *muteCheckBox = nullptr;
	wxGauge *panGauge = nullptr;
	wxStaticText *panLabel = nullptr;
	wxStaticText *formatLabel = nullptr;
	wxStaticText *loopStartLabel = nullptr;
	wxStaticText *soundPosLenLabel = nullptr;
	ChannelState lastState = ChannelState::Start;
};

class SoundView : public wxDialog
{
	void GaugeSetValueImmediate(wxGauge *gauge, int value);
public:
	SoundView(wxWindow *parent, XSFConfig_NCSF *ncsfConfig, XSFPlayer_NCSF *ncsfPlayer);
	void OnIdle(wxIdleEvent &evt);
	void SyncToConfig();
	void Update();

	XSFConfig_NCSF *config;
	XSFPlayer_NCSF *player;
	SoundViewData channelData[16];
	bool volumeModeAlternative;
};
