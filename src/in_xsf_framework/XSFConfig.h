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
#include "XSFFile.h"
#include "convert.h"
#include "windowsh_wrapper.h"

class wxWindow;
class XSFConfigDialog;
class XSFPlayer;

class XSFConfigIO
{
private:
	// enum versions
	template<typename T> typename std::enable_if_t<std::is_enum_v<T>, T> GetValueInternal(const std::string &name, const T &defaultValue) const
	{
		return ConvertFuncs::To<T>(this->GetValueString(name, std::to_string(ConvertFuncs::ToIntegral(defaultValue))));
	}
	template<typename T> typename std::enable_if_t<std::is_enum_v<T>> SetValueInternal(const std::string &name, const T &value)
	{
		this->SetValueString(name, std::to_string(ConvertFuncs::ToIntegral(value)));
	}

	// non-enum versions
	template<typename T> typename std::enable_if_t<!std::is_enum_v<T> && std::is_arithmetic_v<T>, T> GetValueInternal(const std::string &name, const T &defaultValue) const
	{
		return ConvertFuncs::To<T>(this->GetValueString(name, std::to_string(defaultValue)));
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
	DialogTemplate infoDialog;
	std::vector<unsigned> supportedSampleRates;
	std::unique_ptr<XSFConfigIO> configIO;

	XSFConfig();
	std::wstring GetTextFromWindow(HWND hwnd);
	virtual void LoadSpecificConfig() = 0;
	virtual void SaveSpecificConfig() = 0;
	virtual INT_PTR CALLBACK InfoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void InitializeSpecificConfigDialog(XSFConfigDialog *dialog) = 0;
	virtual void ResetSpecificConfigDefaults(XSFConfigDialog *dialog) = 0;
	virtual void SaveSpecificConfigDialog(XSFConfigDialog *dialog) = 0;
	virtual void CopySpecificConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad) = 0;
public:
	static constexpr bool initPlayInfinitely = false;
	inline static const std::string initSkipSilenceOnStartSec = "5", initDetectSilenceSec = "5", initDefaultLength = "1:55", initDefaultFade = "5", initTitleFormat = "%game%[ - [%disc%.]%track%] - %title%";
	static constexpr double initVolume = 1.0;
	static constexpr VolumeType initVolumeType = VolumeType::ReplayGainAlbum;
	static constexpr PeakType initPeakType = PeakType::ReplayGainTrack;
	// These are not defined in XSFConfig.cpp, they should be defined in your own config's source.
	static const unsigned initSampleRate;
	static const std::string commonName;
	static const std::string versionNumber;
	// The Create function is not defined in XSFConfig.cpp, it should be defined in your own config's source and return a pointer to your config's class.
	static XSFConfig *Create();
	static const std::string &CommonNameWithVersion();

	virtual ~XSFConfig() { }
	void LoadConfig();
	void SaveConfig();
	void GenerateDialogs();
	static INT_PTR CALLBACK InfoDialogProcStatic(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void CallConfigDialog(HINSTANCE hInstance, HWND hwndParent);
	void CallInfoDialog(HINSTANCE hInstance, HWND hwndParent);
	void InitializeConfigDialog(XSFConfigDialog *dialog);
	void ResetConfigDefaults(XSFConfigDialog *dialog);
	void SaveConfigDialog(XSFConfigDialog *dialog);
	void CopyConfigToMemory(XSFPlayer *xSFPlayer, bool preLoad);
	void SetHInstance(HINSTANCE hInstance);
	HINSTANCE GetHInstance() const;

	virtual void About(HWND parent) = 0;
	virtual XSFConfigDialog *CreateDialogBox(wxWindow *window, const std::string &title) = 0;

	const std::vector<unsigned> &GetSupportedSampleRates() const;

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
