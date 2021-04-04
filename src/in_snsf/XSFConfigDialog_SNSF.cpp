/*
 * xSF - SNSF configuration dialog
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include <string>
#include <wx/arrstr.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include <wx/valgen.h>
#include "XSFConfig.h"
#include "XSFConfigDialog.h"
#include "XSFConfigDialog_SNSF.h"

XSFConfigDialog_SNSF::XSFConfigDialog_SNSF(XSFConfig &config, wxWindow *parent, const wxString &title) : XSFConfigDialog(config, parent, title)
{
	auto reverseStereoCheckBox = new wxCheckBox(this->outputPanel, wxID_ANY, "Reverse Stereo", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator{ &this->reverseStereo });
	this->outputSizer->Add(reverseStereoCheckBox, { 4, 0 }, { 1, 2 }, wxALL, 5);

	auto resamplerLabel = new wxStaticText(this->outputPanel, wxID_ANY, "Resampler");
	this->outputSizer->Add(resamplerLabel, { 5, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	wxArrayString resamplerChoices;
	resamplerChoices.Add("Linear Resampler");
	resamplerChoices.Add("Hermite Resampler");
	resamplerChoices.Add("Bspline Resampler");
	resamplerChoices.Add("Osculating Resampler");
	resamplerChoices.Add("Sinc Resampler");
	auto resamplerChoice = new wxChoice(this->outputPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, resamplerChoices, 0, wxGenericValidator{ &this->resampler });
	this->outputSizer->Add(resamplerChoice, { 5, 1 }, { 1, 1 }, wxALL, 5);

	auto muteLabel = new wxStaticText(this->outputPanel, wxID_ANY, "Mute");
	this->outputSizer->Add(muteLabel, { 6, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	wxArrayString muteChoices;
	for (int i = 0; i < 16; ++i)
		muteChoices.Add("BRRPCM " + std::to_string(i + 1));
	auto muteListBox = new wxListBox(this->outputPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, muteChoices, wxLB_MULTIPLE, wxGenericValidator{ &this->mute });
	this->outputSizer->Add(muteListBox, { 6, 1 }, { 1, 1 }, wxALL, 5);
}
