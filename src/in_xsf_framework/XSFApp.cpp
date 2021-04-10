/*
 * xSF - Core wxWidgets App
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <wx/nativewin.h>
#ifdef __GNUC__
# pragma GCC diagnostic pop
#elif defined(__clang__)
# pragma clang diagnostic pop
#endif
#include "windowsh_wrapper.h"
#include "XSFApp.h"
#include "XSFConfig.h"
#include "XSFConfigDialog.h"

void XSFApp::ConfigDialog(HWND parent, XSFConfig *config)
{
	// NOTE: This is only good enough to make it so the dialog box isn't shown on the taskbar, it is not good enough to make the dialog modal to Winamp's preferences screen... it'll actually crash if someone changes to another section of preferences...

	auto window = wxNativeContainerWindow(parent);
	auto dialog = config->CreateDialogBox(&window, XSFConfig::commonName + " v" + XSFConfig::versionNumber);
	dialog->Finalize();
	config->InitializeConfigDialog(dialog);
	if (dialog->ShowModal() == wxID_OK)
	{
		config->SaveConfigDialog(dialog);
		config->SaveConfig();
	}
	dialog->Destroy();
	window.Destroy();
}
