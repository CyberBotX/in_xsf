// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../core/assembler.h"
#include "../core/compiler.h"
#include "../core/compilercontext.h"
#include "../core/compilerfunc.h"
#include "../core/compileritem.h"
#include "../core/cpuinfo.h"
#include "../core/intutil.h"
#include "../core/logger.h"

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

// ============================================================================
// [AsmJit::Compiler - Construction / Destruction]
// ============================================================================

Compiler::Compiler(Context *context) : _zoneMemory(16384 - sizeof(ZoneChunk) - 32), _linkMemory(1024 - 32), _context(context ? context : static_cast<Context *>(JitContext::getGlobal())), _logger(nullptr), _error(0),
	_properties(0), _emitOptions(0), _finished(false), _first(nullptr), _last(nullptr), _current(nullptr),  _cc(nullptr), _varNameId(0)
{
}

Compiler::~Compiler()
{
	this->reset();
}

// ============================================================================
// [AsmJit::Compiler - Logging]
// ============================================================================

void Compiler::setLogger(Logger *logger)
{
	this->_logger = logger;
}

// ============================================================================
// [AsmJit::Compiler - Error Handling]
// ============================================================================

void Compiler::setError(uint32_t error)
{
	this->_error = error;
	if (this->_error == kErrorOk)
		return;

	if (this->_logger)
		this->_logger->logFormat("*** COMPILER ERROR: %s (%u).\n", getErrorString(error), static_cast<unsigned>(error));
}

// ============================================================================
// [AsmJit::Compiler - Properties]
// ============================================================================

uint32_t Compiler::getProperty(uint32_t propertyId)
{
	if (propertyId > 31)
		return 0;

	return !!(this->_properties & IntUtil::maskFromIndex(propertyId));
}

void Compiler::setProperty(uint32_t propertyId, uint32_t value)
{
	if (propertyId > 31)
		return;

	if (value)
		this->_properties |= IntUtil::maskFromIndex(propertyId);
	else
		this->_properties &= ~IntUtil::maskFromIndex(propertyId);
}

// ============================================================================
// [AsmJit::Compiler - Clear / Reset]
// ============================================================================

void Compiler::clear()
{
	this->_purge();

	if (this->_error != kErrorOk)
		this->setError(kErrorOk);
}

void Compiler::reset()
{
	this->_purge();

	this->_zoneMemory.reset();
	this->_linkMemory.reset();

	this->_targets.reset();
	this->_vars.reset();

	if (this->_error != kErrorOk)
		this->setError(kErrorOk);
}

void Compiler::_purge()
{
	this->_zoneMemory.clear();
	this->_linkMemory.clear();

	this->_emitOptions = 0;
	this->_finished = false;

	this->_first = nullptr;
	this->_last = nullptr;
	this->_current = nullptr;
	this->_func = nullptr;

	this->_targets.clear();
	this->_vars.clear();

	this->_cc = nullptr;
	this->_varNameId = 0;
}

// ============================================================================
// [AsmJit::Compiler - Item Management]
// ============================================================================

CompilerItem *Compiler::setCurrentItem(CompilerItem *item)
{
	CompilerItem *old = this->_current;
	this->_current = item;
	return old;
}

void Compiler::addItem(CompilerItem *item)
{
	ASMJIT_ASSERT(item);
	ASMJIT_ASSERT(!item->_prev);
	ASMJIT_ASSERT(!item->_next);

	if (!this->_current)
	{
		if (!this->_first)
		{
			this->_first = item;
			this->_last = item;
		}
		else
		{
			item->_next = this->_first;
			this->_first->_prev = item;
			this->_first = item;
		}
	}
	else
	{
		CompilerItem *prev = this->_current;
		CompilerItem *next = this->_current->_next;

		item->_prev = prev;
		item->_next = next;

		prev->_next = item;
		if (next)
			next->_prev = item;
		else
			this->_last = item;
	}

	this->_current = item;
}

void Compiler::addItemAfter(CompilerItem *item, CompilerItem *ref)
{
	ASMJIT_ASSERT(item);
	ASMJIT_ASSERT(!item->_prev);
	ASMJIT_ASSERT(!item->_next);
	ASMJIT_ASSERT(ref);

	CompilerItem *prev = ref;
	CompilerItem *next = ref->_next;

	item->_prev = prev;
	item->_next = next;

	prev->_next = item;
	if (next)
		next->_prev = item;
	else
		this->_last = item;
}

void Compiler::removeItem(CompilerItem *item)
{
	CompilerItem *prev = item->_prev;
	CompilerItem *next = item->_next;

	if (this->_first == item)
		this->_first = next;
	else
		prev->_next = next;
	if (this->_last  == item)
		this->_last  = prev;
	else
		next->_prev = prev;

	item->_prev = nullptr;
	item->_next = nullptr;

	if (this->_current == item)
		this->_current = prev;
}

// ============================================================================
// [AsmJit::Compiler - Comment]
// ============================================================================

void Compiler::comment(const char *fmt, ...)
{
	char buf[128];
	char *p = buf;

	if (fmt)
	{
		*p++ = ';';
		*p++ = ' ';

		va_list ap;
		va_start(ap, fmt);
		p += vsnprintf(p, 100, fmt, ap);
		va_end(ap);
	}

	*p++ = '\n';
	*p = '\0';

	CompilerComment *item = Compiler_newItem<CompilerComment>(this, buf);
	this->addItem(item);
}

// ============================================================================
// [AsmJit::Compiler - Embed]
// ============================================================================

void Compiler::embed(const void *data, size_t len)
{
	// Align length to 16 bytes.
	size_t alignedSize = IntUtil::align(len, sizeof(uintptr_t));
	void *p = this->_zoneMemory.alloc(sizeof(CompilerEmbed) - sizeof(void *) + alignedSize);

	if (!p)
		return;

	CompilerEmbed *item = new(p) CompilerEmbed(this, data, len);
	this->addItem(item);
}

} // AsmJit namespace

// [Api-Begin]
#include "../core/apibegin.h"
