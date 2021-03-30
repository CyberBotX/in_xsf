/*
 * xSF - NCSF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <bitset>
#include <string>
#ifdef _DEBUG
# include <algorithm>
# include <memory>
# include <cstdint>
# include "SSEQPlayer/consts.h"
#endif
#include "windowsh_wrapper.h"
#include "XSFConfig.h"

class XSFConfig_NCSF;
class XSFPlayer_NCSF;

#ifdef _DEBUG
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
	static unsigned initInterpolation;
	static std::string initMutes;

	friend class XSFConfig;
	friend struct SoundViewData;
	unsigned interpolation;
	std::bitset<16> mutes;

	XSFConfig_NCSF();
	void LoadSpecificConfig() override;
	void SaveSpecificConfig() override;
	void GenerateSpecificDialogs() override;
	INT_PTR CALLBACK ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void ResetSpecificConfigDefaults(HWND hwndDlg) override;
	void SaveSpecificConfigDialog(HWND hwndDlg) override;
	void CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad) override;

#ifdef _DEBUG
	std::unique_ptr<SoundViewData> soundViewData;

	static INT_PTR CALLBACK SoundViewDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
public:
	void About(HWND parent) override;

#ifdef _DEBUG
	void CallSoundView(XSFPlayer *xSFPlayer, HINSTANCE hInstance, HWND hwndParent);
	void RefreshSoundView();
	void CloseSoundView();
#endif
};
