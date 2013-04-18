// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

// [Guard]
#ifndef _ASMJIT_CORE_PODVECTOR_H
#define _ASMJIT_CORE_PODVECTOR_H

// [Dependencies - AsmJit]
#include "../core/assert.h"
#include "../core/defs.h"

namespace AsmJit
{

//! @addtogroup AsmJit_Core
//! @{

// ============================================================================
// [AsmJit::PodVector<T>]
// ============================================================================

//! @brief Template used to store and manage array of POD data.
//!
//! This template has these adventages over other vector<> templates:
//! - Non-copyable (designed to be non-copyable, we want it)
//! - No copy-on-write (some implementations of stl can use it)
//! - Optimized for working only with POD types
//! - Uses ASMJIT_... memory management macros
template <typename T> struct PodVector
{
	ASMJIT_NO_COPY(PodVector<T>)

	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	//! @brief Create new instance of PodVector template. Data will not
	//! be allocated (will be NULL).
	PodVector() : _data(nullptr), _length(0), _capacity(0) { }

	//! @brief Destroy PodVector and free all data.
	~PodVector()
	{
		if (this->_data)
			ASMJIT_FREE(this->_data);
	}

	// --------------------------------------------------------------------------
	// [Data]
	// --------------------------------------------------------------------------

	//! @brief Get data.
	T *getData() { return this->_data; }
	//! @overload
	const T *getData() const { return this->_data; }

	//! @brief Get length.
	size_t getLength() const { return this->_length; }
	//! @brief Get capacity.
	size_t getCapacity() const { return this->_capacity; }

	// --------------------------------------------------------------------------
	// [Manipulation]
	// --------------------------------------------------------------------------

	//! @brief Clear vector data, but not free internal buffer.
	void clear()
	{
		this->_length = 0;
	}

	//! @brief Clear vector data and free internal buffer.
	void reset()
	{
		if (this->_data)
		{
			ASMJIT_FREE(this->_data);
			this->_data = nullptr;
			this->_length = 0;
			this->_capacity = 0;
		}
	}

	//! @brief Prepend @a item to vector.
	bool prepend(const T &item)
	{
		if (this->_length == this->_capacity && !this->_grow())
			return false;

		memmove(this->_data + 1, this->_data, sizeof(T) * this->_length);
		memcpy(this->_data, &item, sizeof(T));

		++this->_length;
		return true;
	}

	//! @brief Insert an @a item at the @a index.
	bool insert(size_t index, const T &item)
	{
		ASMJIT_ASSERT(index <= this->_length);
		if (this->_length == this->_capacity && !this->_grow())
			return false;

		T *dst = this->_data + index;
		memmove(dst + 1, dst, this->_length - index);
		memcpy(dst, &item, sizeof(T));

		++this->_length;
		return true;
	}

	//! @brief Append @a item to vector.
	bool append(const T &item)
	{
		if (this->_length == this->_capacity && !this->_grow())
			return false;

		memcpy(this->_data + this->_length, &item, sizeof(T));

		++this->_length;
		return true;
	}

	//! @brief Get index of @a val or kInvalidSize if not found.
	size_t indexOf(const T &val) const
	{
		for (size_t i = 0, len = this->_length; i < len; ++i)
			if (this->_data[i] == val)
				return i;
		return kInvalidSize;
	}

	//! @brief Remove element at index @a i.
	void removeAt(size_t i)
	{
		ASMJIT_ASSERT(i < this->_length);

		T *dst = this->_data + i;
		--this->_length;
		memmove(dst, dst + 1, this->_length - i);
	}

	//! @brief Swap this pod-vector with @a other.
	void swap(PodVector<T> &other)
	{
		T *_tmp_data = this->_data;
		size_t _tmp_length = this->_length;
		size_t _tmp_capacity = this->_capacity;

		this->_data = other._data;
		this->_length = other._length;
		this->_capacity = other._capacity;

		other._data = _tmp_data;
		other._length = _tmp_length;
		other._capacity = _tmp_capacity;
	}

	//! @brief Get item at position @a i.
	T &operator[](size_t i)
	{
		ASMJIT_ASSERT(i < this->_length);
		return this->_data[i];
	}
	//! @brief Get item at position @a i.
	const T &operator[](size_t i) const
	{
		ASMJIT_ASSERT(i < this->_length);
		return this->_data[i];
	}

	//! @brief Append the item and return address so it can be initialized.
	T *newItem()
	{
		if (this->_length == this->_capacity && !this->_grow())
			return nullptr;
		return this->_data + this->_length++;
	}

	// --------------------------------------------------------------------------
	// [Private]
	// --------------------------------------------------------------------------

	//! @brief Called to grow internal array.
	bool _grow()
	{
		return this->_realloc(this->_capacity < 16 ? 16 : this->_capacity * 2);
	}

	//! @brief Realloc internal array to fit @a to items.
	bool _realloc(size_t to)
	{
		ASMJIT_ASSERT(to >= this->_length);

		T *p = reinterpret_cast<T *>(this->_data ? ASMJIT_REALLOC(this->_data, to * sizeof(T)) : ASMJIT_MALLOC(to * sizeof(T)));

		if (!p)
			return false;

		this->_data = p;
		this->_capacity = to;
		return true;
	}

	// --------------------------------------------------------------------------
	// [Members]
	// --------------------------------------------------------------------------

	//! @brief Items data.
	T *_data;
	//! @brief Length of buffer (count of items in array).
	size_t _length;
	//! @brief Capacity of buffer (maximum items that can fit to current array).
	size_t _capacity;
};

//! @}

} // AsmJit namespace

#endif // _ASMJIT_CORE_PODVECTOR_H
