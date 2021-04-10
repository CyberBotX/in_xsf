/*
 * xSF - Core wxWidgets App
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
#include <wx/app.h>
#ifdef __GNUC__
# pragma GCC diagnostic pop
#elif defined(__clang__)
# pragma clang diagnostic pop
#endif
#include "windowsh_wrapper.h"

class XSFConfig;

class XSFApp : public wxApp
{
public:
	// Even though most of the plugins are just going to create an instance of XSFApp, I am using this to allow the NCSF plugin to make a derived instance instead.
	static XSFApp *Create();

	void ConfigDialog(HWND parent, XSFConfig *config);
};
