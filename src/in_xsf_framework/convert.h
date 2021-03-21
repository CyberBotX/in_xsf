/*
 * Common conversion functions
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 */

#pragma once

#include <locale>
#include <string>
#include <type_traits>
#include <vector>
#include <cmath>
#include <cstddef>
#include "windowsh_wrapper.h"

/*
 * Originally the convert* functions came from the C++ FAQ, Miscellaneous Technical Issues:
 * https://isocpp.org/wiki/faq/misc-technical-issues#convert-string-to-any
 *
 * They have been replaced with a couple functions that use the C++11 std::enable_if
 * construct along with various other type traits constructs to use the proper
 * string conversions.
 */
template<typename T, typename S> inline typename std::enable_if_t<!std::is_enum_v<T> &&std::is_arithmetic_v<T>, T> convertTo(const std::basic_string<S> &s)
{
	if (std::is_integral_v<T>)
	{
		if (std::is_unsigned_v<T>)
		{
			if (std::is_same_v<unsigned long long, std::remove_cv_t<T>>)
				return static_cast<T>(std::stoull(s));
			else
				return static_cast<T>(std::stoul(s));
		}
		else if (std::is_same_v<long long, std::remove_cv_t<T>>)
			return static_cast<T>(std::stoll(s));
		else if (std::is_same_v<long, std::remove_cv_t<T>>)
			return static_cast<T>(std::stol(s));
		else
			return static_cast<T>(std::stoi(s));
	}
	else if (std::is_floating_point_v<T>)
	{
		if (std::is_same_v<long double, std::remove_cv_t<T>>)
			return static_cast<T>(std::stold(s));
		else if (std::is_same_v<double, std::remove_cv_t<T>>)
			return static_cast<T>(std::stod(s));
		else
			return static_cast<T>(std::stof(s));
	}
}
template<typename T, typename S> inline typename std::enable_if_t<std::is_enum_v<T>, T> convertTo(const std::basic_string<S> &s)
{
	return static_cast<T>(convertTo<std::underlying_type_t<T>>(s));
}

// Miscellaneous conversion functions
class ConvertFuncs
{
private:
	template<typename T> static bool IsDigitsOnly(const std::basic_string<T> &input, const std::locale &loc = std::locale::classic())
	{
		auto inputChars = std::vector<T>(input.begin(), input.end());
		std::size_t length = inputChars.size();
		auto masks = std::vector<typename std::ctype<T>::mask>(length);
		std::use_facet<std::ctype<T>>(loc).is(&inputChars[0], &inputChars[length], &masks[0]);
		for (std::size_t x = 0; x < length; ++x)
			if (inputChars[x] != '.' && !(masks[x] & std::ctype<T>::digit))
				return false;
		return true;
	}

public:
	static unsigned long StringToMS(const std::string &time)
	{
		unsigned long hours = 0, minutes = 0;
		double seconds = 0.0;
		std::string hoursStr, minutesStr, secondsStr;
		std::size_t firstcolon = time.find(':');
		if (firstcolon != std::string::npos)
		{
			std::size_t secondcolon = time.substr(firstcolon + 1).find(':');
			if (secondcolon != std::string::npos)
			{
				secondcolon = firstcolon + secondcolon + 1;
				hoursStr = time.substr(0, firstcolon);
				minutesStr = time.substr(firstcolon + 1, secondcolon - firstcolon - 1);
				secondsStr = time.substr(secondcolon + 1);
			}
			else
			{
				minutesStr = time.substr(0, firstcolon);
				secondsStr = time.substr(firstcolon + 1);
			}
		}
		else
			secondsStr = time;
		if (!hoursStr.empty())
		{
			if (!ConvertFuncs::IsDigitsOnly(hoursStr))
				return 0;
			hours = convertTo<unsigned long>(hoursStr);
		}
		if (!minutesStr.empty())
		{
			if (!ConvertFuncs::IsDigitsOnly(minutesStr))
				return 0;
			minutes = convertTo<unsigned long>(minutesStr);
		}
		if (!secondsStr.empty())
		{
			if (!ConvertFuncs::IsDigitsOnly(secondsStr))
				return 0;
			std::size_t comma = secondsStr.find(',');
			if (comma != std::string::npos)
				secondsStr[comma] = '.';
			seconds = convertTo<double>(secondsStr);
		}
		seconds += minutes * 60 + hours * 1440;
		return static_cast<unsigned long>(std::floor(seconds * 1000 + 0.5));
	}

	static unsigned long StringToMS(const std::wstring &time)
	{
		return ConvertFuncs::StringToMS(ConvertFuncs::WStringToString(time));
	}

	static std::string MSToString(unsigned long time)
	{
		double seconds = time / 1000.0;
		if (seconds < 60)
			return ConvertFuncs::TrimDoubleString(std::to_string(seconds));
		unsigned long minutes = static_cast<unsigned long>(seconds) / 60;
		seconds -= minutes * 60;
		if (minutes < 60)
			return std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + ConvertFuncs::TrimDoubleString(std::to_string(seconds));
		unsigned long hours = minutes / 60;
		minutes %= 60;
		return std::to_string(hours) + ":" + (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + ConvertFuncs::TrimDoubleString(std::to_string(seconds));
	}

	static std::wstring MSToWString(unsigned long time)
	{
		return ConvertFuncs::StringToWString(ConvertFuncs::MSToString(time));
	}

	// Derived from https://stackoverflow.com/a/13709929 (specifically the comments)
	template<typename T> static std::basic_string<T> TrimDoubleString(const std::basic_string<T> &str)
	{
		auto strCopy = str;
		auto lastNonZero = strCopy.find_last_not_of('0');
		strCopy.erase(lastNonZero + (lastNonZero == strCopy.find('.') ? 0 : 1));
		return strCopy;
	}

	static std::wstring StringToWString(const std::string &str)
	{
		auto strC = str.c_str();
		int bufferSize = MultiByteToWideChar(CP_UTF8, 0, strC, -1, nullptr, 0);
		auto buffer = std::vector<wchar_t>(bufferSize);
		MultiByteToWideChar(CP_UTF8, 0, strC, -1, &buffer[0], bufferSize);
		return std::wstring(buffer.begin(), buffer.begin() + bufferSize - 1);
	}

	static std::string WStringToString(const std::wstring &wstr)
	{
		auto wstrC = wstr.c_str();
		int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstrC, -1, nullptr, 0, nullptr, nullptr);
		auto buffer = std::vector<char>(bufferSize);
		WideCharToMultiByte(CP_UTF8, 0, wstrC, -1, &buffer[0], bufferSize, nullptr, nullptr);
		return std::string(buffer.begin(), buffer.begin() + bufferSize - 1);
	}
};
