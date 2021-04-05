#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <wx/object.h>
#include <wx/string.h>
#ifdef __GNUC__
# pragma GCC diagnostic pop
#elif defined(__clang__)
# pragma clang diagnostic pop
#endif
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
