/*
 * xSF - NCSF wxWidgets App
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include "XSFApp.h"

class SoundView;
class XSFConfig_NCSF;
class XSFPlayer_NCSF;

class XSFApp_NCSF : public XSFApp
{
	SoundView *dialog;
public:
	void CreateSoundView(XSFConfig_NCSF *config, XSFPlayer_NCSF *player);
	void SyncSoundViewToConfig();
	void DestroySoundView();
};
