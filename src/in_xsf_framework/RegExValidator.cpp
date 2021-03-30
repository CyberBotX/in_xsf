#include <wx/object.h>
#include <wx/string.h>
#include "RegExValidator.h"

RegExValidator::RegExValidator(const wxString &regExpString, wxString *valPtr, int regExpFlags) : wxTextValidator(wxFILTER_EMPTY, valPtr), regEx(regExpString, regExpFlags), regExString(regExpString), regExFlags(regExpFlags)
{
}

wxObject *RegExValidator::Clone() const
{
	return new RegExValidator(this->regExString, this->m_stringValue, this->regExFlags);
}

wxString RegExValidator::IsValid(const wxString &str) const
{
	if (!this->regEx.Matches(str))
		return "Invalid format.";

	return wxTextValidator::IsValid(str);
}
