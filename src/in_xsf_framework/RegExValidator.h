#pragma once

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <wx/regex.h>
#include <wx/string.h>
#include <wx/valtext.h>
#ifdef __GNUC__
# pragma GCC diagnostic pop
#elif defined(__clang__)
# pragma clang diagnostic pop
#endif

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
