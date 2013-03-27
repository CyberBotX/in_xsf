/*
 * xSF - SNSF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-21
 *
 * Partially based on the vio*sf framework
 *
 * NOTE: 16-bit sound is always enabled, the player is currently limited to
 * creating a 16-bit PCM for Winamp and can not handle 8-bit sound from
 * snes9x.
 */

#ifndef XSFCONFIG_SNSF_H
#define XSFCONFIG_SNSF_H

#include <bitset>
#include "XSFPlayer.h"
#include "XSFConfig.h"

class XSFConfig_SNSF : public XSFConfig
{
protected:
	static bool /*initSixteenBitSound, */initReverseStereo;
	static unsigned initResampler;
	static std::wstring initMutes;

	friend class XSFConfig;
	bool /*sixteenBitSound, */reverseStereo;
	std::bitset<8> mutes;

	XSFConfig_SNSF();
	void LoadSpecificConfig();
	void SaveSpecificConfig();
	void GenerateSpecificDialogs();
	INT_PTR CALLBACK ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ResetSpecificConfigDefaults(HWND hwndDlg);
	void SaveSpecificConfigDialog(HWND hwndDlg);
	void CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad);
public:
	unsigned resampler;

	void About(HWND parent);
};

#endif