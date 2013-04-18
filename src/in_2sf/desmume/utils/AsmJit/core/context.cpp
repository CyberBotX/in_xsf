// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../core/assembler.h"
#include "../core/context.h"
#include "../core/defs.h"
#include "../core/memorymanager.h"
#include "../core/memorymarker.h"

namespace AsmJit
{

// ============================================================================
// [AsmJit::Context - Construction / Destruction]
// ============================================================================

Context::Context() { }
Context::~Context() { }

// ============================================================================
// [AsmJit::JitContext - Construction / Destruction]
// ============================================================================

JitContext::JitContext() : _memoryManager(nullptr), _memoryMarker(nullptr), _allocType(kMemAllocFreeable)
{
}

JitContext::~JitContext()
{
}

// ============================================================================
// [AsmJit::JitContext - Generate]
// ============================================================================

uint32_t JitContext::generate(void **dest, Assembler *assembler)
{
	// Disallow empty code generation.
	size_t codeSize = assembler->getCodeSize();
	if (!codeSize)
	{
		*dest = nullptr;
		return kErrorNoFunction;
	}

	// Switch to global memory manager if not provided.
	MemoryManager *memmgr = this->getMemoryManager();

	if (!memmgr)
		memmgr = MemoryManager::getGlobal();

	void *p = memmgr->alloc(codeSize, getAllocType());
	if (!p)
	{
		*dest = nullptr;
		return kErrorNoVirtualMemory;
	}

	// Relocate the code.
	size_t relocatedSize = assembler->relocCode(p);

	// Return unused memory to MemoryManager.
	if (relocatedSize < codeSize)
		memmgr->shrink(p, relocatedSize);

	// Mark memory if MemoryMarker provided.
	if (this->_memoryMarker)
		this->_memoryMarker->mark(p, relocatedSize);

	// Return the code.
	*dest = p;
	return kErrorOk;
}

// ============================================================================
// [AsmJit::JitContext - GetGlobal]
// ============================================================================

JitContext *JitContext::getGlobal()
{
	static JitContext global;
	return &global;
}

} // AsmJit namespace
