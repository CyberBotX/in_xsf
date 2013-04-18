// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

// [Guard]
#ifndef _ASMJIT_CORE_LOCK_H
#define _ASMJIT_CORE_LOCK_H

// [Dependencies - AsmJit]
#include "../core/build.h"

// [Dependencies - Windows]
#ifdef ASMJIT_WINDOWS
# include "windowsh_wrapper.h"
#endif // ASMJIT_WINDOWS

// [Dependencies - Posix]
#ifdef ASMJIT_POSIX
# include <pthread.h>
#endif // ASMJIT_POSIX

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

//! @addtogroup AsmJit_Core
//! @{

// ============================================================================
// [AsmJit::Lock]
// ============================================================================

//! @brief Lock - used in thread-safe code for locking.
struct Lock
{
	ASMJIT_NO_COPY(Lock)

	// --------------------------------------------------------------------------
	// [Windows]
	// --------------------------------------------------------------------------

#ifdef ASMJIT_WINDOWS
	typedef CRITICAL_SECTION Handle;

	//! @brief Create a new @ref Lock instance.
	Lock() { InitializeCriticalSection(&this->_handle); }
	//! @brief Destroy the @ref Lock instance.
	~Lock() { DeleteCriticalSection(&this->_handle); }

	//! @brief Lock.
	void lock() { EnterCriticalSection(&this->_handle); }
	//! @brief Unlock.
	void unlock() { LeaveCriticalSection(&this->_handle); }
#endif // ASMJIT_WINDOWS

	// --------------------------------------------------------------------------
	// [Posix]
	// --------------------------------------------------------------------------

#ifdef ASMJIT_POSIX
	typedef pthread_mutex_t Handle;

	//! @brief Create a new @ref Lock instance.
	Lock() { pthread_mutex_init(&this->_handle, nullptr); }
	//! @brief Destroy the @ref Lock instance.
	~Lock() { pthread_mutex_destroy(&this->_handle); }

	//! @brief Lock.
	void lock() { pthread_mutex_lock(&this->_handle); }
	//! @brief Unlock.
	void unlock() { pthread_mutex_unlock(&this->_handle); }
#endif // ASMJIT_POSIX

	// --------------------------------------------------------------------------
	// [Accessors]
	// --------------------------------------------------------------------------

	//! @brief Get handle.
	Handle &getHandle() { return this->_handle; }
	//! @overload
	const Handle &getHandle() const { return this->_handle; }

	// --------------------------------------------------------------------------
	// [Members]
	// --------------------------------------------------------------------------

	//! @brief Handle.
	Handle _handle;
};

// ============================================================================
// [AsmJit::AutoLock]
// ============================================================================

//! @brief Scope auto locker.
struct AutoLock
{
	ASMJIT_NO_COPY(AutoLock)

	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	//! @brief Locks @a target.
	AutoLock(Lock &target) : _target(target)
	{
		this->_target.lock();
	}

	//! @brief Unlocks target.
	~AutoLock()
	{
		this->_target.unlock();
	}

	// --------------------------------------------------------------------------
	// [Members]
	// --------------------------------------------------------------------------

	//! @brief Pointer to target (lock).
	Lock &_target;
};

//! @}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"

// [Guard]
#endif // _ASMJIT_CORE_LOCK_H
