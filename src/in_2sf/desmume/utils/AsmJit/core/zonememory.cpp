// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../core/defs.h"
#include "../core/intutil.h"
#include "../core/zonememory.h"

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

// ============================================================================
// [AsmJit::ZoneMemory]
// ============================================================================

ZoneMemory::ZoneMemory(size_t chunkSize)
{
	this->_chunks = nullptr;
	this->_total = 0;
	this->_chunkSize = chunkSize;
}

ZoneMemory::~ZoneMemory()
{
	this->reset();
}

void *ZoneMemory::alloc(size_t size)
{
	ZoneChunk *cur = this->_chunks;

	// Align to 4 or 8 bytes.
	size = IntUtil::align(size, sizeof(size_t));

	if (!cur || cur->getRemainingBytes() < size)
	{
		size_t chSize = this->_chunkSize;
 
		if (chSize < size)
			chSize = size;

		cur = static_cast<ZoneChunk *>(ASMJIT_MALLOC(sizeof(ZoneChunk) - sizeof(void *) + chSize));
		if (!cur)
			return nullptr;

		cur->prev = this->_chunks;
		cur->pos = 0;
		cur->size = chSize;

		this->_chunks = cur;
	}

	uint8_t *p = cur->data + cur->pos;
	cur->pos += size;
	this->_total += size;

	ASMJIT_ASSERT(cur->pos <= cur->size);
	return static_cast<void *>(p);
}

char *ZoneMemory::sdup(const char *str)
{
	if (!str)
		return nullptr;

	size_t len = strlen(str);
	if (!len)
		return nullptr;

	// Include NULL terminator and limit string length.
	if (++len > 256)
		len = 256;

	char *m = static_cast<char *>(alloc(IntUtil::align<size_t>(len, 16)));
	if (!m)
		return nullptr;

	memcpy(m, str, len);
	m[len - 1] = 0;
	return m;
}

void ZoneMemory::clear()
{
	ZoneChunk *cur = this->_chunks;

	if (!cur)
		return;

	cur = cur->prev;
	while (cur)
	{
		ZoneChunk *prev = cur->prev;
		ASMJIT_FREE(cur);
		cur = prev;
	}

	this->_chunks->pos = 0;
	this->_chunks->prev = nullptr;
	this->_total = 0;
}

void ZoneMemory::reset()
{
	ZoneChunk *cur = this->_chunks;

	this->_chunks = nullptr;
	this->_total = 0;

	while (cur)
	{
		ZoneChunk *prev = cur->prev;
		ASMJIT_FREE(cur);
		cur = prev;
	}
}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"
