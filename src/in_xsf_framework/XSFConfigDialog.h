/*
 * xSF - Core configuration dialog
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <wx/msw/winundef.h>
#include <wx/dialog.h>
#include <wx/string.h>
#ifdef __GNUC__
# pragma GCC diagnostic pop
#elif defined(__clang__)
# pragma clang diagnostic pop
#endif

class wxGridBagSizer;
class wxPanel;
class wxWindow;
class XSFConfig;

class XSFConfigDialog : public wxDialog
{
	inline static const std::string timeRegex = R"(^\d+(:[0-5]\d){0,2}([.,]\d+)?$)";

	wxGridBagSizer *mainSizer;
	XSFConfig &config;
public:
	XSFConfigDialog(XSFConfig &newConfig, wxWindow *parent, const wxString &title);
	void Finalize();

	wxPanel *generalPanel;
	wxGridBagSizer *generalSizer;

	wxPanel *outputPanel;
	wxGridBagSizer *outputSizer;

	bool playInfinitely = false;
	wxString defaultPlayLength;
	wxString defaultFadeoutLength;
	wxString skipSilenceOnStart;
	wxString detectSilence;
	double volume = 1;
	int replayGain;
	int clipProtect;
	int sampleRate;
	wxString titleFormat;
};
