/*
 * xSF - NCSF configuration dialog
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

class XSFConfigDialog_NCSF : public XSFConfigDialog
{
public:
	XSFConfigDialog_NCSF(XSFConfig &config, wxWindow *parent, const wxString &title);

#ifndef NDEBUG
	bool useSoundView;
#endif
	int interpolation;
	wxArrayInt mute;
};
