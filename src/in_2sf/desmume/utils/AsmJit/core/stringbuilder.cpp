// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../core/defs.h"
#include "../core/intutil.h"
#include "../core/stringbuilder.h"

// [Dependencies - C]
#include <cstdarg>

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

// Should be placed in read-only memory.
static const char StringBuilder_empty[4] = { 0 };

// ============================================================================
// [AsmJit::StringBuilder - Construction / Destruction]
// ============================================================================

StringBuilder::StringBuilder() : _data(const_cast<char *>(StringBuilder_empty)), _length(0), _capacity(0), _canFree(false)
{
}

StringBuilder::~StringBuilder()
{
	if (this->_canFree)
		ASMJIT_FREE(this->_data);
}

// ============================================================================
// [AsmJit::StringBuilder - Prepare / Reserve]
// ============================================================================

char *StringBuilder::prepare(uint32_t op, size_t len)
{
	// --------------------------------------------------------------------------
	// [Set]
	// --------------------------------------------------------------------------

	if (op == kStringBuilderOpSet)
	{
		// We don't care here, but we can't return a NULL pointer since it indicates
		// failure in memory allocation.
		if (!len)
		{
			if (this->_data != StringBuilder_empty)
				this->_data[0] = 0;

			this->_length = 0;
			return this->_data;
		}

		if (this->_capacity < len)
		{
			if (len >= IntUtil::maxValue<size_t>() - sizeof(uintptr_t) * 2)
				return nullptr;

			size_t to = IntUtil::align(len, sizeof(uintptr_t));
			if (to < 256 - sizeof(uintptr_t))
				to = 256 - sizeof(uintptr_t);

			char *newData = static_cast<char *>(ASMJIT_MALLOC(to + sizeof(uintptr_t)));
			if (!newData)
			{
				this->clear();
				return nullptr;
			}

			if (this->_canFree)
				ASMJIT_FREE(this->_data);

			this->_data = newData;
			this->_capacity = to + sizeof(uintptr_t) - 1;
			this->_canFree = true;
		}

		this->_data[len] = 0;
		this->_length = len;

		ASMJIT_ASSERT(this->_length <= this->_capacity);
		return this->_data;
	}

	// --------------------------------------------------------------------------
	// [Append]
	// --------------------------------------------------------------------------

	else
	{
		// We don't care here, but we can't return a NULL pointer since it indicates
		// failure in memory allocation.
		if (!len)
			return this->_data + this->_length;

		// Overflow.
		if (IntUtil::maxValue<size_t>() - sizeof(uintptr_t) * 2 - this->_length < len)
			return nullptr;

		size_t after = this->_length + len;
		if (this->_capacity < after)
		{
			size_t to = this->_capacity;

			if (to < 256)
				to = 256;

			while (to < 1024 * 1024 && to < after)
				to *= 2;

			if (to < after)
			{
				to = after;
				if (to < IntUtil::maxValue<size_t>() - 1024 * 32)
					to = IntUtil::align<size_t>(to, 1024 * 32);
			}

			to = IntUtil::align(to, sizeof(uintptr_t));
			char *newData = static_cast<char *>(ASMJIT_MALLOC(to + sizeof(uintptr_t)));

			if (!newData)
				return nullptr;

			::memcpy(newData, this->_data, this->_length);

			if (this->_canFree)
				ASMJIT_FREE(this->_data);

			this->_data = newData;
			this->_capacity = to + sizeof(uintptr_t) - 1;
			this->_canFree = true;
		}

		char *ret = this->_data + this->_length;
		this->_data[after] = 0;
		this->_length = after;

		ASMJIT_ASSERT(this->_length <= this->_capacity);
		return ret;
	}
}

bool StringBuilder::reserve(size_t to)
{
	if (this->_capacity >= to)
		return true;

	if (to >= IntUtil::maxValue<size_t>() - sizeof(uintptr_t) * 2)
		return false;

	to = IntUtil::align(to, sizeof(uintptr_t));

	char *newData = static_cast<char *>(ASMJIT_MALLOC(to + sizeof(uintptr_t)));
	if (!newData)
		return false;

	::memcpy(newData, this->_data, this->_length + 1);
	if (this->_canFree)
		ASMJIT_FREE(this->_data);

	this->_data = newData;
	this->_capacity = to + sizeof(uintptr_t) - 1;
	this->_canFree = true;
	return true;
}

// ============================================================================
// [AsmJit::StringBuilder - Clear]
// ============================================================================

void StringBuilder::clear()
{
	if (this->_data != StringBuilder_empty)
		this->_data[0] = 0;
	this->_length = 0;
}

// ============================================================================
// [AsmJit::StringBuilder - Methods]
// ============================================================================

bool StringBuilder::_opString(uint32_t op, const char *str, size_t len)
{
	if (len == kInvalidSize)
		len = ::strlen(str);

	char *p = this->prepare(op, len);
	if (!p)
		return false;

	::memcpy(p, str, len);
	return true;
}

bool StringBuilder::_opChars(uint32_t op, char c, size_t len)
{
	char *p = this->prepare(op, len);
	if (!p)
		return false;

	::memset(p, c, len);
	return true;
}

static const char StringBuilder_numbers[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

bool StringBuilder::_opNumber(uint32_t op, uint64_t i, uint32_t base, size_t width, uint32_t flags)
{
	if (base < 2 || base > 36)
		base = 10;

	char buf[128];
	char *p = buf + ASMJIT_ARRAY_SIZE(buf);

	uint64_t orig = i;
	char sign = 0;

	// --------------------------------------------------------------------------
	// [Sign]
	// --------------------------------------------------------------------------

	if ((flags & kStringBuilderNumSigned) && static_cast<int64_t>(i) < 0)
	{
		i = static_cast<uint64_t>(-static_cast<int64_t>(i));
		sign = '-';
	}
	else if (flags & kStringBuilderNumShowSign)
		sign = '+';
	else if (flags & kStringBuilderNumShowSpace)
		sign = ' ';

	// --------------------------------------------------------------------------
	// [Number]
	// --------------------------------------------------------------------------

	do
	{
		uint64_t d = i / base;
		uint64_t r = i % base;

		*--p = StringBuilder_numbers[r];
		i = d;
	} while (i);

	size_t numberLength = static_cast<size_t>(buf + ASMJIT_ARRAY_SIZE(buf) - p);

	// --------------------------------------------------------------------------
	// [Alternate Form]
	// --------------------------------------------------------------------------

	if (flags & kStringBuilderNumAlternate)
	{
		if (base == 8 && orig)
			*--p = '0';
		if (base == 16)
		{
			*--p = 'x';
			*--p = '0';
		}
	}

	// --------------------------------------------------------------------------
	// [Width]
	// --------------------------------------------------------------------------

	if (sign)
		*--p = sign;

	if (width > 256)
		width = 256;

	if (width <= numberLength)
		width = 0;
	else
		width -= numberLength;

	// --------------------------------------------------------------------------
	// [Write]
	// --------------------------------------------------------------------------

	size_t prefixLength = static_cast<size_t>(buf + ASMJIT_ARRAY_SIZE(buf) - p) - numberLength;
	char *data = this->prepare(op, prefixLength + width + numberLength);

	if (!data)
		return false;

	::memcpy(data, p, prefixLength);
	data += prefixLength;

	::memset(data, '0', width);
	data += width;

	::memcpy(data, p + prefixLength, numberLength);
	return true;
}

bool StringBuilder::_opHex(uint32_t op, const void *data, size_t len)
{
	if (len >= IntUtil::maxValue<size_t>() / 2)
		return false;

	char *dst = this->prepare(op, len);
	if (!dst)
		return false;

	const char *src = static_cast<const char *>(data);
	for (size_t i = 0; i < len; ++i, dst += 2, ++src)
	{
		dst[0] = StringBuilder_numbers[(src[0] >> 4) & 0xF];
		dst[1] = StringBuilder_numbers[src[0] & 0xF];
	}

	return true;
}

bool StringBuilder::_opVFormat(uint32_t op, const char *fmt, va_list ap)
{
	char buf[1024];

	vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf), fmt, ap);
	buf[ASMJIT_ARRAY_SIZE(buf) - 1] = 0;

	return this->_opString(op, buf);
}

bool StringBuilder::setFormat(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	bool result = this->_opVFormat(kStringBuilderOpSet, fmt, ap);
	va_end(ap);

	return result;
}

bool StringBuilder::appendFormat(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	bool result = _opVFormat(kStringBuilderOpAppend, fmt, ap);
	va_end(ap);

	return result;
}

bool StringBuilder::eq(const char *str, size_t len) const
{
	const char *aData = this->_data;
	const char *bData = str;

	size_t aLength = this->_length;
	size_t bLength = len;

	if (bLength == kInvalidSize)
	{
		size_t i;
		for (i = 0; i < aLength; ++i)
			if (aData[i] != bData[i] || !bData[i])
				return false;

		return !bData[i];
	}
	else
	{
		if (aLength != bLength)
			return false;

		return !::memcmp(aData, bData, aLength);
	}
}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"
