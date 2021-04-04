/*
 * xSF - GSF configuration
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

class XSFConfig_GSF : public XSFConfig
{
protected:
	static constexpr bool initLowPassFiltering = true;
	inline static const std::string initMutes = "000000";

	friend class XSFConfig;
	bool lowPassFiltering;
	std::bitset<6> mutes;

	XSFConfig_GSF();
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
