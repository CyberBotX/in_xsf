/*
 * xSF - Core configuration handler
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include "DialogBuilder.h"
#include "convert.h"
#include "windowsh_wrapper.h"

enum class PeakType;
enum class VolumeType;
class XSFPlayer;

class XSFConfigIO
{
private:
	// enum versions
	template<typename T> typename std::enable_if_t<std::is_enum_v<T>, T> GetValueInternal(const std::string &name, const T &defaultValue) const
	{
		return convertTo<T>(this->GetValueString(name, std::to_string(static_cast<std::underlying_type_t<T>>(defaultValue))));
	}
	template<typename T> typename std::enable_if_t<std::is_enum_v<T>> SetValueInternal(const std::string &name, const T &value)
	{
		this->SetValueString(name, std::to_string(static_cast<std::underlying_type_t<T>>(value)));
	}

	// non-enum versions
	template<typename T> typename std::enable_if_t<!std::is_enum_v<T> && std::is_arithmetic_v<T>, T> GetValueInternal(const std::string &name, const T &defaultValue) const
	{
		return convertTo<T>(this->GetValueString(name, std::to_string(defaultValue)));
	}
	template<typename T> typename std::enable_if_t<!std::is_enum_v<T> && std::is_arithmetic_v<T>> SetValueInternal(const std::string &name, const T &value)
	{
		this->SetValueString(name, std::to_string(value));
	}
protected:
	XSFConfigIO() { }
public:
	// This is not defined in XSFConfig.cpp, it should be defined in your own config's source and return a pointer to your config's I/O class.
	static XSFConfigIO *Create();

	virtual ~XSFConfigIO() { }
	virtual void SetValueString(const std::string &name, const std::string &value) = 0;
	template<typename T> void SetValue(const std::string &name, const T &value) { this->SetValueInternal(name, value); }
	void SetValue(const std::string &name, const std::string &value) { this->SetValueString(name, value); }
	virtual std::string GetValueString(const std::string &name, const std::string &defaultValue) const = 0;
	template<typename T> T GetValue(const std::string &name, const T &defaultValue) const { return this->GetValueInternal(name, defaultValue); }
	std::string GetValue(const std::string &name, const std::string &defaultValue) const { return this->GetValueString(name, defaultValue); }
	virtual void SetHInstance(HINSTANCE) { }
	virtual HINSTANCE GetHInstance() const { return nullptr; }
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
	std::string titleFormat;
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
	static std::string initSkipSilenceOnStartSec, initDetectSilenceSec, initDefaultLength, initDefaultFade, initTitleFormat;
	static double initVolume;
	static VolumeType initVolumeType;
	static PeakType initPeakType;
	// These are not defined in XSFConfig.cpp, they should be defined in your own config's source.
	static unsigned initSampleRate;
	static std::string commonName;
	static std::string versionNumber;
	// The Create function is not defined in XSFConfig.cpp, it should be defined in your own config's source and return a pointer to your config's class.
	static XSFConfig *Create();
	static const std::string &CommonNameWithVersion();

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
	void SetHInstance(HINSTANCE hInstance);
	HINSTANCE GetHInstance() const;

	virtual void About(HWND parent) = 0;

	bool GetPlayInfinitely() const;
	unsigned long GetSkipSilenceOnStartSec() const;
	unsigned long GetDetectSilenceSec() const;
	unsigned long GetDefaultLength() const;
	unsigned long GetDefaultFade() const;
	double GetVolume() const;
	VolumeType GetVolumeType() const;
	PeakType GetPeakType() const;
	const std::string &GetTitleFormat() const;
};
