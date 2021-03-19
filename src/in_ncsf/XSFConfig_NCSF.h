/*
 * xSF - NCSF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <bitset>
#include <memory>
#include "XSFConfig.h"
#include "XSFPlayer_NCSF.h"
#include "windowsh_wrapper.h"

class XSFConfig_NCSF;

#ifdef _DEBUG
struct SoundViewData
{
	XSFConfig_NCSF *config;
	XSFPlayer_NCSF *player;
	uint8_t channelLastStates[16];
	HWND hDlg;

	bool volModeAlternative;

	SoundViewData() : config(nullptr), player(nullptr), hDlg(nullptr), volModeAlternative(false)
	{
		std::fill_n(&this->channelLastStates[0], sizeof(this->channelLastStates), CS_START);
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
	void LoadSpecificConfig();
	void SaveSpecificConfig();
	void GenerateSpecificDialogs();
	INT_PTR CALLBACK ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ResetSpecificConfigDefaults(HWND hwndDlg);
	void SaveSpecificConfigDialog(HWND hwndDlg);
	void CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad);

#ifdef _DEBUG
	std::unique_ptr<SoundViewData> soundViewData;

	static INT_PTR CALLBACK SoundViewDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
public:
	void About(HWND parent);

#ifdef _DEBUG
	void CallSoundView(XSFPlayer *xSFPlayer, HINSTANCE hInstance, HWND hwndParent);
	void RefreshSoundView();
	void CloseSoundView();
#endif
};
