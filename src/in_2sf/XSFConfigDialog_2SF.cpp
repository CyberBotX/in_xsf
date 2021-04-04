/*
 * xSF - 2SF configuration dialog
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#include <string>
#include <wx/arrstr.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include <wx/valgen.h>
#include "XSFConfig.h"
#include "XSFConfigDialog.h"
#include "XSFConfigDialog_2SF.h"

XSFConfigDialog_2SF::XSFConfigDialog_2SF(XSFConfig &config, wxWindow *parent, const wxString &title) : XSFConfigDialog(config, parent, title)
{
	auto interpolationLabel = new wxStaticText(this->outputPanel, wxID_ANY, "Interpolation");
	this->outputSizer->Add(interpolationLabel, { 4, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	wxArrayString interpolationChoices;
	interpolationChoices.Add("None");
	interpolationChoices.Add("Linear");
	interpolationChoices.Add("Cosine");
	interpolationChoices.Add("Sharp");
	auto interpolationChoice = new wxChoice(this->outputPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, interpolationChoices, 0, wxGenericValidator{ &this->interpolation });
	this->outputSizer->Add(interpolationChoice, { 4, 1 }, { 1, 1 }, wxALL, 5);

	auto muteLabel = new wxStaticText(this->outputPanel, wxID_ANY, "Mute");
	this->outputSizer->Add(muteLabel, { 5, 0 }, { 1, 1 }, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	wxArrayString muteChoices;
	for (int i = 0; i < 16; ++i)
		muteChoices.Add("SPU " + std::to_string(i + 1));
	auto muteListBox = new wxListBox(this->outputPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, muteChoices, wxLB_MULTIPLE, wxGenericValidator{ &this->mute });
	this->outputSizer->Add(muteListBox, { 5, 1 }, { 1, 1 }, wxALL, 5);
}
