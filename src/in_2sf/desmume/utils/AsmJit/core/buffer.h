// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

// [Guard]
#ifndef _ASMJIT_CORE_BUFFER_H
#define _ASMJIT_CORE_BUFFER_H

// [Dependencies - AsmJit]
#include "../core/assert.h"
#include "../core/build.h"

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

//! @addtogroup AsmJit_Core
//! @{

// ============================================================================
// [AsmJit::Buffer]
// ============================================================================

//! @brief Buffer used to store instruction stream in AsmJit.
//! 
//! This class can be dangerous, if you don't know how it works. Assembler
//! instruction stream is usually constructed by multiple calls of emit
//! functions that emits bytes, words, dwords or qwords. But to decrease
//! AsmJit library size and improve performance, we are not checking for
//! buffer overflow for each emit operation, but only once in highler level
//! emit instruction.
//!
//! So, if you want to use this class, you need to do buffer checking yourself
//! by using @c ensureSpace() method. It's designed to grow buffer if needed.
//! Threshold for growing is named @c growThreshold() and it means count of
//! bytes for emitting single operation. Default size is set to 16 bytes,
//! because x86 and x64 instruction can't be larger (so it's space to hold 1
//! instruction).
//!
//! Example using Buffer:
//!
//! @code
//! // Buffer instance, growThreshold == 16
//! // (no memory allocated in constructor).
//! AsmJit::Buffer buf(16);
//!
//! // Begin of emit stream, ensure space can fail on out of memory error.
//! if (buf.ensureSpace()) 
//! {
//!   // here, you can emit up to 16 (growThreshold) bytes
//!   buf.emitByte(0x00);
//!   buf.emitByte(0x01);
//!   buf.emitByte(0x02);
//!   buf.emitByte(0x03);
//!   ...
//! }
//! @endcode
struct Buffer
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	Buffer() : _data(nullptr), _cur(nullptr), _max(nullptr), _capacity(0) { }

	~Buffer()
	{
		if (this->_data)
			ASMJIT_FREE(this->_data);
	}

	//! @brief Get start of buffer.
	uint8_t *getData() const { return this->_data; }

	//! @brief Get current pointer in code buffer.
	uint8_t *getCur() const { return this->_cur; }

	//! @brief Get maximum pointer in code buffer for growing.
	uint8_t *getMax() const { return this->_max; }

	//! @brief Get current offset in buffer.
	size_t getOffset() const { return static_cast<size_t>(this->_cur - this->_data); }

	//! @brief Get capacity of buffer.
	size_t getCapacity() const { return this->_capacity; }

	//! @brief Ensure space for next instruction
	bool ensureSpace() { return this->_cur >= this->_max ? this->grow() : true; }

	//! @brief Sets offset to @a o and returns previous offset.
	//!
	//! This method can be used to truncate buffer or it's used to
	//! overwrite specific position in buffer by Assembler.
	size_t toOffset(size_t offset)
	{
		ASMJIT_ASSERT(offset < this->_capacity);

		size_t prev = static_cast<size_t>(this->_cur - this->_data);
		this->_cur = this->_data + offset;
		return prev;
	}

	//! @brief Reallocate buffer.
	//!
	//! It's only used for growing, buffer is never reallocated to smaller 
	//! number than current capacity() is.
	ASMJIT_API bool realloc(size_t to);

	//! @brief Used to grow the buffer.
	//!
	//! It will typically realloc to twice size of capacity(), but if capacity()
	//! is large, it will use smaller steps.
	ASMJIT_API bool grow();

	//! @brief Clear everything, but not deallocate buffer.
	void clear() { this->_cur = this->_data; }

	//! @brief Free buffer and NULL all pointers.
	ASMJIT_API void reset();

	//! @brief Take ownership of the buffer data and purge @c Buffer instance.
	ASMJIT_API uint8_t *take();

	// --------------------------------------------------------------------------
	// [Emit]
	// --------------------------------------------------------------------------

	//! @brief Emit Byte.
	void emitByte(uint8_t x)
	{
		ASMJIT_ASSERT(this->getOffset() + 1 <= this->_capacity);

		*this->_cur++ = x;
	}

	//! @brief Emit Word (2 bytes).
	void emitWord(uint16_t x)
	{
		ASMJIT_ASSERT(this->getOffset() + 2 <= this->_capacity);

		*reinterpret_cast<uint16_t *>(this->_cur) = x;
		this->_cur += 2;
	}

	//! @brief Emit DWord (4 bytes).
	void emitDWord(uint32_t x)
	{
		ASMJIT_ASSERT(this->getOffset() + 4 <= this->_capacity);

		*reinterpret_cast<uint32_t *>(this->_cur) = x;
		this->_cur += 4;
	}

	//! @brief Emit QWord (8 bytes).
	void emitQWord(uint64_t x)
	{
		ASMJIT_ASSERT(this->getOffset() + 8 <= this->_capacity);

		*reinterpret_cast<uint64_t *>(this->_cur) = x;
		this->_cur += 8;
	}

	//! @brief Emit intptr_t (4 or 8 bytes).
	void emitIntPtrT(intptr_t x)
	{
		ASMJIT_ASSERT(this->getOffset() + sizeof(intptr_t) <= this->_capacity);

		*reinterpret_cast<intptr_t *>(this->_cur) = x;
		this->_cur += sizeof(intptr_t);
	}

	//! @brief Emit uintptr_t (4 or 8 bytes).
	void emitUIntPtrT(uintptr_t x)
	{
		ASMJIT_ASSERT(this->getOffset() + sizeof(uintptr_t) <= this->_capacity);

		*reinterpret_cast<uintptr_t *>(this->_cur) = x;
		this->_cur += sizeof(uintptr_t);
	}

	//! @brief Emit size_t (4 or 8 bytes).
	void emitSizeT(size_t x)
	{
		ASMJIT_ASSERT(this->getOffset() + sizeof(size_t) <= this->_capacity);

		*reinterpret_cast<size_t *>(this->_cur) = x;
		this->_cur += sizeof(size_t);
	}

	//! @brief Emit custom data. 
	ASMJIT_API void emitData(const void *ptr, size_t len);

	// --------------------------------------------------------------------------
	// [Get / Set]
	// --------------------------------------------------------------------------

	//! @brief Set byte at position @a pos.
	uint8_t getByteAt(size_t pos) const
	{
		ASMJIT_ASSERT(pos + 1 <= this->_capacity);

		return *(this->_data + pos);
	}

	//! @brief Set word at position @a pos.
	uint16_t getWordAt(size_t pos) const
	{
		ASMJIT_ASSERT(pos + 2 <= this->_capacity);

		return *reinterpret_cast<const uint16_t *>(this->_data + pos);
	}

	//! @brief Set dword at position @a pos.
	uint32_t getDWordAt(size_t pos) const
	{
		ASMJIT_ASSERT(pos + 4 <= this->_capacity);

		return *reinterpret_cast<const uint32_t *>(this->_data + pos);
	}

	//! @brief Set qword at position @a pos.
	uint64_t getQWordAt(size_t pos) const
	{
		ASMJIT_ASSERT(pos + 8 <= this->_capacity);

		return *reinterpret_cast<const uint64_t *>(this->_data + pos);
	}

	//! @brief Set intptr_t at position @a pos.
	intptr_t getIntPtrTAt(size_t pos) const
	{
		ASMJIT_ASSERT(pos + sizeof(intptr_t) <= this->_capacity);

		return *reinterpret_cast<const intptr_t *>(this->_data + pos);
	}

	//! @brief Set uintptr_t at position @a pos.
	uintptr_t getUIntPtrTAt(size_t pos) const
	{
		ASMJIT_ASSERT(pos + sizeof(uintptr_t) <= this->_capacity);

		return *reinterpret_cast<const uintptr_t *>(this->_data + pos);
	}

	//! @brief Set size_t at position @a pos.
	uintptr_t getSizeTAt(size_t pos) const
	{
		ASMJIT_ASSERT(pos + sizeof(size_t) <= this->_capacity);

		return *reinterpret_cast<const size_t *>(this->_data + pos);
	}

	//! @brief Set byte at position @a pos.
	void setByteAt(size_t pos, uint8_t x)
	{
		ASMJIT_ASSERT(pos + 1 <= this->_capacity);

		*(this->_data + pos) = x;
	}

	//! @brief Set word at position @a pos.
	void setWordAt(size_t pos, uint16_t x)
	{
		ASMJIT_ASSERT(pos + 2 <= this->_capacity);

		*reinterpret_cast<uint16_t *>(this->_data + pos) = x;
	}

	//! @brief Set dword at position @a pos.
	void setDWordAt(size_t pos, uint32_t x)
	{
		ASMJIT_ASSERT(pos + 4 <= this->_capacity);

		*reinterpret_cast<uint32_t *>(this->_data + pos) = x;
	}

	//! @brief Set qword at position @a pos.
	void setQWordAt(size_t pos, uint64_t x)
	{
		ASMJIT_ASSERT(pos + 8 <= this->_capacity);

		*reinterpret_cast<uint64_t *>(this->_data + pos) = x;
	}

	//! @brief Set intptr_t at position @a pos.
	void setIntPtrTAt(size_t pos, intptr_t x)
	{
		ASMJIT_ASSERT(pos + sizeof(intptr_t) <= this->_capacity);

		*reinterpret_cast<intptr_t *>(this->_data + pos) = x;
	}

	//! @brief Set uintptr_t at position @a pos.
	void setUIntPtrTAt(size_t pos, uintptr_t x)
	{
		ASMJIT_ASSERT(pos + sizeof(uintptr_t) <= this->_capacity);

		*reinterpret_cast<uintptr_t *>(this->_data + pos) = x;
	}

	//! @brief Set size_t at position @a pos.
	void setSizeTAt(size_t pos, size_t x)
	{
		ASMJIT_ASSERT(pos + sizeof(size_t) <= this->_capacity);

		*reinterpret_cast<size_t *>(this->_data + pos) = x;
	}

	// --------------------------------------------------------------------------
	// [Members]
	// --------------------------------------------------------------------------

	// All members are public, because they can be accessed and modified by 
	// Assembler/Compiler directly.

	//! @brief Beginning position of buffer.
	uint8_t *_data;
	//! @brief Current position in buffer.
	uint8_t *_cur;
	//! @brief Maximum position in buffer for realloc.
	uint8_t *_max;

	//! @brief Buffer capacity (in bytes).
	size_t _capacity;
};

//! @}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"

#endif // _ASMJIT_CORE_BUFFER_H
