/*
 * xSF - NCSF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <bitset>
#include <string>
#ifndef NDEBUG
# include <algorithm>
# include <memory>
# include <cstdint>
# include "SSEQPlayer/consts.h"
#endif
#include "windowsh_wrapper.h"
#include "XSFConfig.h"

class wxWindow;
class XSFConfig_NCSF;
class XSFConfigDialog;
class XSFPlayer;
#ifndef DEBUG
class XSFPlayer_NCSF;
#endif

#ifndef NDEBUG
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
#endif

class XSFConfig_NCSF : public XSFConfig
{
protected:
#ifndef NDEBUG
	static constexpr bool initUseSoundViewDialog = true;
#endif
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

#ifndef NDEBUG
	bool useSoundViewDialog;
	std::unique_ptr<SoundViewData> soundViewData;

	static INT_PTR CALLBACK SoundViewDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
public:
	void About(HWND parent) override;
	XSFConfigDialog *CreateDialogBox(wxWindow *window, const std::string &title) override;

#ifndef NDEBUG
	void CallSoundView(XSFPlayer *xSFPlayer, HINSTANCE hInstance, HWND hwndParent);
	void RefreshSoundView();
	void CloseSoundView();
#endif
};
