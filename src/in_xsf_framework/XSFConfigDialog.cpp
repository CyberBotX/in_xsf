/*
 * xSF - Core configuration dialog
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include <string>
#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/gbsizer.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>
#include <wx/valnum.h>
#include <wx/window.h>
#ifdef __GNUC__
# pragma GCC diagnostic pop
#elif defined(__clang__)
# pragma clang diagnostic pop
#endif
#include "RegExValidator.h"
#include "XSFConfig.h"
#include "XSFConfigDialog.h"

enum
{
	idResetDefaults = 1
};

// This just sets up the base dialog box's controls, it does not do the auto-sizing at this stage.
XSFConfigDialog::XSFConfigDialog(XSFConfig &newConfig, wxWindow *parent, const wxString &title) : wxDialog(parent, wxID_ANY, title), config(newConfig)
{
	this->mainSizer = new wxGridBagSizer;

	auto notebook = new wxNotebook(this, wxID_ANY);

	this->generalPanel = new wxPanel(notebook, wxID_ANY);
	this->generalSizer = new wxGridBagSizer;

	auto playInfinitelyCheckBox = new wxCheckBox(this->generalPanel, wxID_ANY, "Play Infinitely", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator{ &this->playInfinitely });
	this->generalSizer->Add(playInfinitelyCheckBox, { 0, 0 }, { 1, 2 }, wxALL, 5);

	auto defaultPlayLengthLabel = new wxStaticText(this->generalPanel, wxID_ANY, "Default play length (m:s)");
	this->generalSizer->Add(defaultPlayLengthLabel, { 1, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	auto defaultPlayLengthTextCtrl = new wxTextCtrl(this->generalPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, { 38, -1 }, 0, RegExValidator{ XSFConfigDialog::timeRegex, &this->defaultPlayLength, wxRE_ADVANCED });
	this->generalSizer->Add(defaultPlayLengthTextCtrl, { 1, 1 }, { 1, 1 }, wxALL, 5);

	auto defaultFadeoutLengthLabel = new wxStaticText(this->generalPanel, wxID_ANY, "Default fadeout length (m:s)");
	this->generalSizer->Add(defaultFadeoutLengthLabel, { 2, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	auto defaultFadeoutLengthTextCtrl = new wxTextCtrl(this->generalPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, { 38, -1 }, 0, RegExValidator{ XSFConfigDialog::timeRegex, &this->defaultFadeoutLength, wxRE_ADVANCED });
	this->generalSizer->Add(defaultFadeoutLengthTextCtrl, { 2, 1 }, { 1, 1 }, wxALL, 5);

	auto skipSilenceOnStartLabel = new wxStaticText(this->generalPanel, wxID_ANY, "Skip silence on start (s)");
	this->generalSizer->Add(skipSilenceOnStartLabel, { 3, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	auto skipSilenceOnStartTextCtrl = new wxTextCtrl(this->generalPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, { 38, -1 }, 0, RegExValidator{ XSFConfigDialog::timeRegex, &this->skipSilenceOnStart, wxRE_ADVANCED });
	this->generalSizer->Add(skipSilenceOnStartTextCtrl, { 3, 1 }, { 1, 1 }, wxALL, 5);

	auto detectSilenceLabel = new wxStaticText(this->generalPanel, wxID_ANY, "Detect silence (s)");
	this->generalSizer->Add(detectSilenceLabel, { 4, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	auto detectSilenceTextCtrl = new wxTextCtrl(this->generalPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, { 38, -1 }, 0, RegExValidator{ XSFConfigDialog::timeRegex, &this->detectSilence, wxRE_ADVANCED });
	this->generalSizer->Add(detectSilenceTextCtrl, { 4, 1 }, { 1, 1 }, wxALL, 5);

	notebook->AddPage(this->generalPanel, "General");

	this->outputPanel = new wxPanel(notebook, wxID_ANY);
	this->outputSizer = new wxGridBagSizer;

	auto volumeLabel = new wxStaticText(this->outputPanel, wxID_ANY, "Volume");
	this->outputSizer->Add(volumeLabel, { 0, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	auto volumeTextCtrl = new wxTextCtrl(this->outputPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, { 38, -1 }, 0, wxFloatingPointValidator<double>{ &this->volume, wxNUM_VAL_NO_TRAILING_ZEROES });
	this->outputSizer->Add(volumeTextCtrl, { 0, 1 }, { 1, 1 }, wxALL, 5);

	auto replayGainLabel = new wxStaticText(this->outputPanel, wxID_ANY, "ReplayGain");
	this->outputSizer->Add(replayGainLabel, { 1, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	wxArrayString replayGainChoices;
	replayGainChoices.Add("Disabled");
	replayGainChoices.Add("Use Volume Tag");
	replayGainChoices.Add("Track");
	replayGainChoices.Add("Album");
	auto replayGainChoice = new wxChoice(this->outputPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, replayGainChoices, 0, wxGenericValidator{ &this->replayGain });
	this->outputSizer->Add(replayGainChoice, { 1, 1 }, { 1, 1 }, wxALL, 5);

	auto clipProtectLabel = new wxStaticText(this->outputPanel, wxID_ANY, "Clip Protect");
	this->outputSizer->Add(clipProtectLabel, { 2, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	wxArrayString clipProtectChoices;
	clipProtectChoices.Add("Disabled");
	clipProtectChoices.Add("Track");
	clipProtectChoices.Add("Album");
	auto clipProtectChoice = new wxChoice(this->outputPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, clipProtectChoices, 0, wxGenericValidator{ &this->clipProtect });
	this->outputSizer->Add(clipProtectChoice, { 2, 1 }, { 1, 1 }, wxALL, 5);

	auto sampleRateLabel = new wxStaticText(this->outputPanel, wxID_ANY, "Sample Rate");
	this->outputSizer->Add(sampleRateLabel, { 3, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	wxArrayString sampleRateChoices;
	for (auto supportedSampleRate : this->config.GetSupportedSampleRates())
		sampleRateChoices.Add(std::to_string(supportedSampleRate));
	auto sampleRateChoice = new wxChoice(this->outputPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, sampleRateChoices, 0, wxGenericValidator{ &this->sampleRate });
	this->outputSizer->Add(sampleRateChoice, { 3, 1 }, { 1, 1 }, wxALL, 5);

	notebook->AddPage(this->outputPanel, "Output");

	auto titleFormatPanel = new wxPanel(notebook, wxID_ANY);
	auto titleFormatSizer = new wxBoxSizer(wxVERTICAL);

	auto titleFormatPreText = new wxStaticText(titleFormatPanel, wxID_ANY, "NOTE: This is only used if Advanced Title Formatting is disabled in Winamp.");
	titleFormatPreText->Wrap(300);
	titleFormatSizer->Add(titleFormatPreText, 0, wxALL, 5);

	auto titleFormatTextCtrl = new wxTextCtrl(titleFormatPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, { 225, -1 }, 0, wxGenericValidator{ &this->titleFormat });
	titleFormatSizer->Add(titleFormatTextCtrl, 0, wxALL, 5);

	auto titleFormatPostText = new wxStaticText(titleFormatPanel, wxID_ANY, "Names between percent symbols (e.g. %game%, %title%) will be replaced with the respective value from the file's tags. "
		"Using square brackets around any items will cause them to only be displayed if there was a replacement done (e.g. [%disc%.] will display 01. if disc was in the tags as 01, but will "
		"display nothing if disc was not in the tags). Square bracket blocks can be nested.");
	titleFormatPostText->Wrap(300);
	titleFormatSizer->Add(titleFormatPostText, 0, wxALL, 5);

	titleFormatPanel->SetSizer(titleFormatSizer);
	titleFormatPanel->Layout();
	titleFormatSizer->Fit(titleFormatPanel);
	notebook->AddPage(titleFormatPanel, "Title Format");

	this->mainSizer->Add(notebook, { 0, 0 }, { 1, 2 }, wxALL | wxEXPAND, 5);

	auto resetDefaults = new wxButton(this, idResetDefaults, "Reset Defaults");
	this->mainSizer->Add(resetDefaults, { 1, 1 }, { 1, 1 }, wxALIGN_RIGHT | wxALL, 5);
	this->Bind(wxEVT_BUTTON, [&](wxCommandEvent &)
	{
		this->config.ResetConfigDefaults(this);
		this->TransferDataToWindow();
	}, idResetDefaults);

	auto standardButtons = new wxStdDialogButtonSizer;
	auto standardButtonOK = new wxButton(this, wxID_OK);
	standardButtons->AddButton(standardButtonOK);
	auto standardButtonCancel = new wxButton(this, wxID_CANCEL);
	standardButtons->AddButton(standardButtonCancel);
	standardButtons->Realize();
	this->mainSizer->Add(standardButtons, { 2, 0 }, { 1, 2 }, wxBOTTOM | wxEXPAND | wxTOP, 5);
}

void XSFConfigDialog::Finalize()
{
	this->generalPanel->SetSizer(this->generalSizer);
	this->generalPanel->Layout();
	this->generalSizer->Fit(this->generalPanel);

	this->outputPanel->SetSizer(this->outputSizer);
	this->outputPanel->Layout();
	this->outputSizer->Fit(this->outputPanel);

	this->SetSizer(this->mainSizer);
	this->Layout();
	this->mainSizer->Fit(this);
}
