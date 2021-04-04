/*
 * xSF - 2SF configuration
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <bitset>
#include <string>
#include "windowsh_wrapper.h"
#include "XSFConfig.h"

class wxWindow;
class XSFConfigDialog;
class XSFPlayer;

class XSFConfig_2SF : public XSFConfig
{
protected:
	static constexpr unsigned initInterpolation = 2;
	inline static const std::string initMutes = "0000000000000000";

	friend class XSFConfig;
	unsigned interpolation;
	std::bitset<16> mutes;

	XSFConfig_2SF();
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
