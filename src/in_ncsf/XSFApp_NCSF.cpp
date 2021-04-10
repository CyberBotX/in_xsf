/*
 * xSF - NCSF wxWidgets App
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include "SoundView.h"
#include "XSFApp_NCSF.h"

class XSFConfig_NCSF;
class XSFPlayer_NCSF;

void XSFApp_NCSF::CreateSoundView(XSFConfig_NCSF *config, XSFPlayer_NCSF *player)
{
	this->dialog = new SoundView(nullptr, config, player);
	this->dialog->ShowModal();
}

void XSFApp_NCSF::SyncSoundViewToConfig()
{
	if (this->dialog)
		this->dialog->SyncToConfig();
}

void XSFApp_NCSF::DestroySoundView()
{
	this->dialog->Destroy();
	this->dialog = nullptr;
}
