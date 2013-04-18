// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../core/buffer.h"
#include "../core/defs.h"

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

// ============================================================================
// [AsmJit::Buffer]
// ============================================================================

void Buffer::emitData(const void *ptr, size_t len)
{
	size_t max = this->getCapacity() - this->getOffset();

	if (max < len && !this->realloc(this->getOffset() + len))
		return;

	memcpy(this->_cur, ptr, len);
	this->_cur += len;
}

bool Buffer::realloc(size_t to)
{
	if (this->getCapacity() < to)
	{
		size_t len = this->getOffset();
		uint8_t *newdata;

		if (this->_data)
			newdata = static_cast<uint8_t *>(ASMJIT_REALLOC(this->_data, to));
		else
			newdata = static_cast<uint8_t *>(ASMJIT_MALLOC(to));

		if (!newdata)
			return false;

		this->_data = newdata;
		this->_cur = newdata + len;
		this->_max = newdata + to;
		this->_max -= to >= kBufferGrow ? kBufferGrow : to;

		this->_capacity = to;
	}

	return true;
}

bool Buffer::grow()
{
	size_t to = this->_capacity;

	if (to < 512)
		to = 1024;
	else if (to > 65536)
		to += 65536;
	else
		to <<= 1;

	return this->realloc(to);
}

void Buffer::reset()
{
	if (!this->_data)
		return;
	ASMJIT_FREE(this->_data);

	this->_data = nullptr;
	this->_cur = nullptr;
	this->_max = nullptr;
	this->_capacity = 0;
}

uint8_t *Buffer::take()
{
	uint8_t *data = this->_data;

	this->_data = nullptr;
	this->_cur = nullptr;
	this->_max = nullptr;
	this->_capacity = 0;

	return data;
}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"
