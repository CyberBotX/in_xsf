/*
 * xSF - 2SF configuration dialog
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

class XSFConfigDialog_2SF : public XSFConfigDialog
{
public:
	XSFConfigDialog_2SF(XSFConfig &config, wxWindow *parent, const wxString &title);

	int interpolation;
	wxArrayInt mute;
};
