////////////////////////////////////////////////////////////////////////////////
///
/// General FIR digital filter routines with MMX optimization.
///
/// Note : MMX optimized functions reside in a separate, platform-specific file,
/// e.g. 'mmx_win.cpp' or 'mmx_gcc.cpp'
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
/// SoundTouch WWW: http://www.surina.net/soundtouch
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2006/02/05 16:44:06 $
// File revision : $Revision: 1.16 $
//
// $Id: FIRFilter.cpp,v 1.16 2006/02/05 16:44:06 Olli Exp $
//
////////////////////////////////////////////////////////////////////////////////
//
// License :
//
//  SoundTouch audio processing library
//  Copyright (c) Olli Parviainen
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>
#include <cstring>
#include <cassert>
#include <cmath>

#include "FIRFilter.h"
#include "cpu_detect.h"

using namespace soundtouch;

/*****************************************************************************
 *
 * Implementation of the class 'FIRFilter'
 *
 *****************************************************************************/

FIRFilter::FIRFilter()
{
	resultDivFactor = 0;
	length = 0;
	lengthDiv8 = 0;
	filterCoeffs = NULL;
}

FIRFilter::~FIRFilter()
{
	if (filterCoeffs)
		delete[] filterCoeffs;
}

// Usual C-version of the filter routine for stereo sound
uint32_t FIRFilter::evaluateFilterStereo(SAMPLETYPE *dest, const SAMPLETYPE *src, uint32_t numSamples) const
{
#ifdef FLOAT_SAMPLES
	// when using floating point samples, use a scaler instead of a divider
	// because division is much slower operation than multiplying.
	double dScaler = 1.0 / resultDivider;
#endif

	assert(length);

	uint32_t end = 2 * (numSamples - length);

	for (uint32_t j = 0; j < end; j += 2)
	{
		LONG_SAMPLETYPE suml = 0, sumr = 0;
		const SAMPLETYPE *ptr = src + j;

		for (uint32_t i = 0; i < length; i += 4)
		{
			// loop is unrolled by factor of 4 here for efficiency
			suml += ptr[2 * i] * filterCoeffs[i] +
                    ptr[2 * i + 2] * filterCoeffs[i + 1] +
                    ptr[2 * i + 4] * filterCoeffs[i + 2] +
                    ptr[2 * i + 6] * filterCoeffs[i + 3];
			sumr += ptr[2 * i + 1] * filterCoeffs[i] +
                    ptr[2 * i + 3] * filterCoeffs[i + 1] +
                    ptr[2 * i + 5] * filterCoeffs[i + 2] +
                    ptr[2 * i + 7] * filterCoeffs[i + 3];
		}

#ifdef INTEGER_SAMPLES
		suml >>= resultDivFactor;
		sumr >>= resultDivFactor;
		// saturate to 16 bit integer limits
		suml = suml < -32768 ? -32768 : (suml > 32767 ? 32767 : suml);
		// saturate to 16 bit integer limits
		sumr = sumr < -32768 ? -32768 : (sumr > 32767 ? 32767 : sumr);
#else
		suml *= dScaler;
		sumr *= dScaler;
#endif // INTEGER_SAMPLES
		dest[j] = static_cast<SAMPLETYPE>(suml);
		dest[j + 1] = static_cast<SAMPLETYPE>(sumr);
	}
	return numSamples - length;
}

// Usual C-version of the filter routine for mono sound
uint32_t FIRFilter::evaluateFilterMono(SAMPLETYPE *dest, const SAMPLETYPE *src, uint32_t numSamples) const
{
#ifdef FLOAT_SAMPLES
	// when using floating point samples, use a scaler instead of a divider
	// because division is much slower operation than multiplying.
	double dScaler = 1.0 / resultDivider;
#endif

	assert(length);

	uint32_t end = numSamples - length;
	for (uint32_t j = 0; j < end; ++j)
	{
		LONG_SAMPLETYPE sum = 0;
		for (uint32_t i = 0; i < length; i += 4)
		{
			// loop is unrolled by factor of 4 here for efficiency
			sum += src[i] * filterCoeffs[i] +
                   src[i + 1] * filterCoeffs[i + 1] +
                   src[i + 2] * filterCoeffs[i + 2] +
                   src[i + 3] * filterCoeffs[i + 3];
		}
#ifdef INTEGER_SAMPLES
		sum >>= resultDivFactor;
		// saturate to 16 bit integer limits
		sum = sum < -32768 ? -32768 : (sum > 32767 ? 32767 : sum);
#else
		sum *= dScaler;
#endif // INTEGER_SAMPLES
		dest[j] = static_cast<SAMPLETYPE>(sum);
		++src;
	}
	return end;
}

// Set filter coeffiecients and length.
//
// Throws an exception if filter length isn't divisible by 8
void FIRFilter::setCoefficients(const SAMPLETYPE *coeffs, uint32_t newLength, uint32_t uResultDivFactor)
{
	assert(newLength > 0);
	if (newLength % 8)
		throw std::runtime_error("FIR filter length not divisible by 8");

	lengthDiv8 = newLength / 8;
	length = lengthDiv8 * 8;
	assert(length == newLength);

	resultDivFactor = uResultDivFactor;
#ifdef INTEGER_SAMPLES
	resultDivider = static_cast<SAMPLETYPE>(1 << resultDivFactor);
#else
	resultDivider = static_cast<SAMPLETYPE>(pow(2, static_cast<SAMPLETYPE>(resultDivFactor)));
#endif

	delete[] filterCoeffs;
	filterCoeffs = new SAMPLETYPE[length];
	memcpy(filterCoeffs, coeffs, length * sizeof(SAMPLETYPE));
}

uint32_t FIRFilter::getLength() const
{
	return length;
}

// Applies the filter to the given sequence of samples.
//
// Note : The amount of outputted samples is by value of 'filter_length'
// smaller than the amount of input samples.
uint32_t FIRFilter::evaluate(SAMPLETYPE *dest, const SAMPLETYPE *src, uint32_t numSamples, uint32_t numChannels) const
{
	assert(numChannels == 1 || numChannels == 2);

	assert(length > 0);
	assert(lengthDiv8 * 8 == length);
	if (numSamples < length)
		return 0;
	//assert(resultDivFactor >= 0);
	if (numChannels == 2)
		return evaluateFilterStereo(dest, src, numSamples);
	else
		return evaluateFilterMono(dest, src, numSamples);
}

// Operator 'new' is overloaded so that it automatically creates a suitable instance
// depending on if we've a MMX-capable CPU available or not.
void *FIRFilter::operator new(size_t)
{
	// Notice! don't use "new FIRFilter" directly, use "newInstance" to create a new instance instead!
	throw std::runtime_error("Don't use 'new FIRFilter', use 'newInstance' member instead!");
}

FIRFilter *FIRFilter::newInstance()
{
	uint32_t uExtensions = 0;

#if !defined(_MSC_VER) || !defined(__x86_64__)
	uExtensions = detectCPUextensions();
#endif
	// Check if MMX/SSE/3DNow! instruction set extensions supported by CPU

#ifdef ALLOW_MMX
	// MMX routines available only with integer sample types
	if (uExtensions & SUPPORT_MMX)
		return ::new FIRFilterMMX;
	else
#endif // ALLOW_MMX
#ifdef __SSE__
	if (uExtensions & SUPPORT_SSE)
		// SSE support
		return ::new FIRFilterSSE;
	else
#endif // ALLOW_SSE
#ifdef ALLOW_3DNOW
	if (uExtensions & SUPPORT_3DNOW)
		// 3DNow! support
		return ::new FIRFilter3DNow;
	else
#endif // ALLOW_3DNOW
		// ISA optimizations not supported, use plain C version
		return ::new FIRFilter;
}
