/*
 * xSF - Common functions
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Partially based on the vio*sf framework
 */

#pragma once

#include <fstream>
#include <limits>
#include <string>
#include <type_traits>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <cstring>
#include "convert.h"

// Code from http://learningcppisfun.blogspot.com/2010/04/comparing-floating-point-numbers.html
template<typename T> inline typename std::enable_if_t<std::is_floating_point_v<T>, bool> fEqual(T x, T y, int N = 1)
{
	T diff = std::abs(x - y);
	T tolerance = N * std::numeric_limits<T>::epsilon();
	return diff <= tolerance * std::abs(x) && diff <= tolerance * std::abs(y);
}

inline std::uint32_t Get32BitsLE(const std::uint8_t *input)
{
	return input[0] | (input[1] << 8) | (input[2] << 16) | (input[3] << 24);
}

inline std::uint32_t Get32BitsLE(std::ifstream &input)
{
	std::uint8_t bytes[4];
	input.read(reinterpret_cast<char *>(bytes), 4);
	return Get32BitsLE(bytes);
}

// Code from the following answer on Stack Overflow:
// http://stackoverflow.com/a/15479212
template<typename T> inline typename std::enable_if_t<std::is_integral_v<T>, T> NextHighestPowerOf2(T value)
{
	if (value < 1)
		return 1;
	--value;
	for (std::size_t i = 1; i < sizeof(T) * std::numeric_limits<unsigned char>::digits; i <<= 1)
		value |= value >> i;
	return value + 1;
}

// Clamp a value between a minimum and maximum value
template<typename T1, typename T2> inline void clamp(T1 &valueToClamp, const T2 &minValue, const T2 &maxValue)
{
	if (valueToClamp < minValue)
		valueToClamp = minValue;
	else if (valueToClamp > maxValue)
		valueToClamp = maxValue;
}

inline void CopyToString(const std::wstring &src, wchar_t *dst)
{
	std::wcscpy(dst, src.c_str());
}

inline void CopyToString(const std::string &src, wchar_t *dst)
{
	std::wcscpy(dst, ConvertFuncs::StringToWString(src).c_str());
}

inline void CopyToString(const std::string &src, char *dst)
{
	std::strcpy(dst, src.c_str());
}

inline void CopyToString(const std::wstring &src, char *dst)
{
	std::strcpy(dst, ConvertFuncs::WStringToString(src).c_str());
}
