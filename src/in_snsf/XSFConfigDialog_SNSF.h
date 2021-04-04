/*
 * xSF - SNSF configuration dialog
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <wx/dynarray.h>
#include "XSFConfigDialog.h"

class wxString;
class wxWindow;
class XSFConfig;

class XSFConfigDialog_SNSF : public XSFConfigDialog
{
public:
	XSFConfigDialog_SNSF(XSFConfig &config, wxWindow *parent, const wxString &title);

	bool reverseStereo;
	int resampler;
	wxArrayInt mute;
};
