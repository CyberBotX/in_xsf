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

class SoundView;
class wxWindow;
class XSFConfig_NCSF;
class XSFConfigDialog;
class XSFPlayer;

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
	friend struct SoundViewDataOLD;
	friend class SoundView;
	unsigned interpolation;
	std::bitset<16> mutes;
	bool useSoundViewDialog;

	XSFConfig_NCSF();
	void LoadSpecificConfig() override;
	void SaveSpecificConfig() override;
	void InitializeSpecificConfigDialog(XSFConfigDialog *dialog) override;
	void ResetSpecificConfigDefaults(XSFConfigDialog *dialog) override;
	void SaveSpecificConfigDialog(XSFConfigDialog *dialog) override;
	void CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad) override;
public:
	void About(HWND parent) override;
	XSFConfigDialog *CreateDialogBox(wxWindow *window, const std::string &title) override;
};
