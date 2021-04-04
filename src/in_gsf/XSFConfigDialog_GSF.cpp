/*
 * xSF - GSF configuration dialog
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include <wx/arrstr.h>
#include <wx/checkbox.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include <wx/valgen.h>
#include "XSFConfig.h"
#include "XSFConfigDialog.h"
#include "XSFConfigDialog_GSF.h"

XSFConfigDialog_GSF::XSFConfigDialog_GSF(XSFConfig &config, wxWindow *parent, const wxString &title) : XSFConfigDialog(config, parent, title)
{
	auto lowPassFilteringCheckBox = new wxCheckBox(this->outputPanel, wxID_ANY, "Low-Pass Filtering", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator{ &this->lowPassFiltering });
	this->outputSizer->Add(lowPassFilteringCheckBox, { 4, 0 }, { 1, 2 }, wxALL, 5);

	auto muteLabel = new wxStaticText(this->outputPanel, wxID_ANY, "Mute");
	this->outputSizer->Add(muteLabel, { 5, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	wxArrayString muteChoices;
	muteChoices.Add("Square 1");
	muteChoices.Add("Square 2");
	muteChoices.Add("Wave Pattern");
	muteChoices.Add("Noise");
	muteChoices.Add("PCM A");
	muteChoices.Add("PCM B");
	auto muteListBox = new wxListBox(this->outputPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, muteChoices, wxLB_MULTIPLE, wxGenericValidator{ &this->mute });
	this->outputSizer->Add(muteListBox, { 5, 1 }, { 1, 1 }, wxALL, 5);
}
