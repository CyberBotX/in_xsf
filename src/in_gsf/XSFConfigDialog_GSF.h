/*
 * xSF - GSF configuration dialog
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

class XSFConfigDialog_GSF : public XSFConfigDialog
{
public:
	XSFConfigDialog_GSF(XSFConfig &config, wxWindow *parent, const wxString &title);

	bool lowPassFiltering;
	wxArrayInt mute;
};
