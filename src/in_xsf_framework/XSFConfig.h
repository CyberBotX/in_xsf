/*
 * xSF - Core configuration handler
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-30
 *
 * Partially based on the vio*sf framework
 */

#ifndef XSFCONFIG_H
#define XSFCONFIG_H

#include <memory>
#include "XSFPlayer.h"
#include "DialogBuilder.h"
#include "convert.h"
#include "windowsh_wrapper.h"
#include <windowsx.h>

class XSFConfigIO
{
protected:
	XSFConfigIO() { }
public:
	// This is not defined in XSFConfig.cpp, it should be defined in your own config's source and return a pointer to your config's I/O class.
	static XSFConfigIO *Create();

	virtual ~XSFConfigIO() { }
	virtual void SetValueString(const std::wstring &name, const std::wstring &value) = 0;
	template<typename T> void SetValue(const std::wstring &name, const T &value) { this->SetValueString(name, wstringify(value)); }
	void SetValue(const std::wstring &name, const std::wstring &value) { this->SetValueString(name, value); }
	virtual std::wstring GetValueString(const std::wstring &name, const std::wstring &defaultValue) = 0;
	template<typename T> T GetValue(const std::wstring &name, const T &defaultValue) { return convertTo<T>(this->GetValueString(name, wstringify(defaultValue))); }
	std::wstring GetValue(const std::wstring &name, const std::wstring &defaultValue) { return this->GetValueString(name, defaultValue); }
};

class XSFConfig
{
protected:
	bool playInfinitely;
	unsigned long skipSilenceOnStartSec, detectSilenceSec, defaultLength, defaultFade;
	double volume;
	VolumeType volumeType;
	PeakType peakType;
	unsigned sampleRate;
	std::wstring titleFormat;
	DialogTemplate configDialog, configDialogProperty, infoDialog;
	std::vector<unsigned> supportedSampleRates;
	std::unique_ptr<XSFConfigIO> configIO;

	XSFConfig();
	std::wstring GetTextFromWindow(HWND hwnd);
	virtual void LoadSpecificConfig() = 0;
	virtual void GenerateSpecificDialogs() = 0;
	virtual void SaveSpecificConfig() = 0;
	virtual INT_PTR CALLBACK ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR CALLBACK InfoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void ResetSpecificConfigDefaults(HWND hwndDlg) = 0;
	virtual void SaveSpecificConfigDialog(HWND hwndDlg) = 0;
	virtual void CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad) = 0;
public:
	static bool initPlayInfinitely;
	static std::wstring initSkipSilenceOnStartSec, initDetectSilenceSec, initDefaultLength, initDefaultFade, initTitleFormat;
	static double initVolume;
	static VolumeType initVolumeType;
	static PeakType initPeakType;
	// These are not defined in XSFConfig.cpp, they should be defined in your own config's source.
	static unsigned initSampleRate;
	static std::wstring commonName;
	static std::wstring versionNumber;
	// The Create function is not defined in XSFConfig.cpp, it should be defined in your own config's source and return a pointer to your config's class.
	static XSFConfig *Create();

	virtual ~XSFConfig() { }
	void LoadConfig();
	void SaveConfig();
	void GenerateDialogs();
	static INT_PTR CALLBACK ConfigDialogProcStatic(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK InfoDialogProcStatic(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void CallConfigDialog(HINSTANCE hInstance, HWND hwndParent);
	void CallInfoDialog(HINSTANCE hInstance, HWND hwndParent);
	void ResetConfigDefaults(HWND hwndDlg);
	void SaveConfigDialog(HWND hwndDlg);
	void CopyConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad);

	virtual void About(HWND parent) = 0;

	bool GetPlayInfinitely() const;
	unsigned long GetSkipSilenceOnStartSec() const;
	unsigned long GetDetectSilenceSec() const;
	unsigned long GetDefaultLength() const;
	unsigned long GetDefaultFade() const;
	double GetVolume() const;
	VolumeType GetVolumeType() const;
	PeakType GetPeakType() const;
	const std::wstring &GetTitleFormat() const;
};

#endif
