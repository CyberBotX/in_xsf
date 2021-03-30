#pragma once

#include <wx/regex.h>
#include <wx/string.h>
#include <wx/valtext.h>

class wxObject;

class RegExValidator : public wxTextValidator
{
protected:
	wxRegEx regEx;
	wxString regExString;
	int regExFlags;
public:
	RegExValidator(const wxString &regExpString, wxString *valPtr = nullptr, int regExpFlags = wxRE_DEFAULT);
	wxObject *Clone() const override;
	wxString IsValid(const wxString &str) const override;
};
