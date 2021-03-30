#pragma once

#include <wx/object.h>
#include <wx/regex.h>
#include <wx/string.h>
#include <wx/valtext.h>

class RegExValidator : public wxTextValidator
{
protected:
	wxRegEx regEx;
	wxString regExString;
	int regExFlags;
public:
	RegExValidator(wxString regExpString, wxString *valPtr = nullptr, int regExpFlags = wxRE_DEFAULT);
	wxObject *Clone() const override;
	wxString IsValid(const wxString &str) const override;
};
