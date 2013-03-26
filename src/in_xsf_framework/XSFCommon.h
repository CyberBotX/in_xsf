/*
 * xSF - Common functions
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-21
 *
 * Partially based on the vio*sf framework
 */

#ifndef XSFCOMMON_H
#define XSFCOMMON_H

#include <limits>
#include <fstream>
#include <string>
#define _USE_MATH_DEFINES
#include <cmath>
#include "pstdint.h"

// Code from http://learningcppisfun.blogspot.com/2010/04/comparing-floating-point-numbers.html
template<typename T> inline bool fEqual(T x, T y, int N = 1)
{
	T diff = std::abs(x - y);
	T tolerance = N * std::numeric_limits<T>::epsilon();
	return diff <= tolerance * std::abs(x) && diff <= tolerance * std::abs(y);
}

inline uint32_t Get32BitsLE(const uint8_t *input)
{
	return input[0] | (input[1] << 8) | (input[2] << 16) | (input[3] << 24);
}

inline uint32_t Get32BitsLE(std::ifstream &input)
{
	uint8_t bytes[4];
	input.read(reinterpret_cast<char *>(bytes), 4);
	return Get32BitsLE(bytes);
}

// This gets the directory for the path, including the final forward/backward slash
template<typename T> inline std::basic_string<T> ExtractDirectoryFromPath(const std::basic_string<T> &fullPath)
{
	size_t lastSlash = fullPath.rfind('\\');
	if (lastSlash == std::basic_string<T>::npos)
		lastSlash = fullPath.rfind('/');
	return lastSlash != std::basic_string<T>::npos ? fullPath.substr(0, lastSlash + 1) : std::basic_string<T>();
}

// This gets the filename for the path
template<typename T> inline std::basic_string<T> ExtractFilenameFromPath(const std::basic_string<T> &fullPath)
{
	size_t lastSlash = fullPath.rfind('\\');
	if (lastSlash == std::basic_string<T>::npos)
		lastSlash = fullPath.rfind('/');
	return lastSlash != std::basic_string<T>::npos ? fullPath.substr(lastSlash + 1) : std::basic_string<T>();
}

// Code from the following answer on Stack Overflow:
// http://stackoverflow.com/a/15479212
template<typename T> static inline T NextHighestPowerOf2(T value)
{
	if (value < 1)
		return 1;
	--value;
	for (size_t i = 1; i < sizeof(T) * CHAR_BIT; i <<= 1)
		value |= value >> i;
	return value + 1;
}

inline bool FileExists(const std::string &filename)
{
	std::ifstream file((filename.c_str()));
	return !!file;
}

#ifdef _WIN32
inline bool FileExists(const std::wstring &filename)
{
	std::ifstream file((filename.c_str()));
	return !!file;
}
#endif

#endif
