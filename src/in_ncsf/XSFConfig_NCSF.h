/*
 * xSF - NCSF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <algorithm>
#include <bitset>
#include <memory>
#include <string>
#include <cstdint>
#include "windowsh_wrapper.h"
#include "SSEQPlayer/consts.h"
#include "XSFConfig.h"

class wxWindow;
class XSFConfig_NCSF;
class XSFConfigDialog;
class XSFPlayer;
class XSFPlayer_NCSF;

struct SoundViewData
{
	XSFConfig_NCSF *config;
	XSFPlayer_NCSF *player;
	ChannelState channelLastStates[16];
	HWND hDlg;

	bool volModeAlternative;

	SoundViewData() : config(nullptr), player(nullptr), hDlg(nullptr), volModeAlternative(false)
	{
		std::fill_n(&this->channelLastStates[0], sizeof(this->channelLastStates), ChannelState::Start);
	}
};

class XSFConfig_NCSF : public XSFConfig
{
protected:
	static constexpr bool initUseSoundViewDialog =
#ifdef NDEBUG
		false
#else
		true
#endif
	;
	static constexpr unsigned initInterpolation = 4;
	inline static const std::string initMutes = "0000000000000000";

	friend class XSFConfig;
	friend struct SoundViewData;
	unsigned interpolation;
	std::bitset<16> mutes;

	XSFConfig_NCSF();
	void LoadSpecificConfig() override;
	void SaveSpecificConfig() override;
	void InitializeSpecificConfigDialog(XSFConfigDialog *dialog) override;
	void ResetSpecificConfigDefaults(XSFConfigDialog *dialog) override;
	void SaveSpecificConfigDialog(XSFConfigDialog *dialog) override;
	void CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad) override;

	bool useSoundViewDialog;
	std::unique_ptr<SoundViewData> soundViewData;

	static INT_PTR CALLBACK SoundViewDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	void About(HWND parent) override;
	XSFConfigDialog *CreateDialogBox(wxWindow *window, const std::string &title) override;

	void CallSoundView(XSFPlayer *xSFPlayer, HINSTANCE hInstance, HWND hwndParent);
	void RefreshSoundView();
	void CloseSoundView();
};
