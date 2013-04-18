// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../core/assembler.h"
#include "../core/memorymanager.h"
#include "../core/intutil.h"

// [Dependenceis - C]
#include <stdarg.h>

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

// ============================================================================
// [AsmJit::Assembler - Construction / Destruction]
// ============================================================================

Assembler::Assembler(Context *context) : _zoneMemory(16384 - sizeof(ZoneChunk) - 32), _buffer(), _context(context ? context : static_cast<Context *>(JitContext::getGlobal())), _logger(nullptr), _error(kErrorOk),
	_properties(0), _emitOptions(0), _trampolineSize(0), _inlineComment(nullptr), _unusedLinks(nullptr)
{
}

Assembler::~Assembler()
{
}

// ============================================================================
// [AsmJit::Assembler - Logging]
// ============================================================================

void Assembler::setLogger(Logger *logger)
{
	this->_logger = logger;
}

// ============================================================================
// [AsmJit::Assembler - Error Handling]
// ============================================================================

void Assembler::setError(uint32_t error)
{
	this->_error = error;
	if (this->_error == kErrorOk)
		return;

	if (this->_logger)
		this->_logger->logFormat("*** ASSEMBLER ERROR: %s (%u).\n", getErrorString(error), static_cast<unsigned>(error));
}

// ============================================================================
// [AsmJit::Assembler - Properties]
// ============================================================================

uint32_t Assembler::getProperty(uint32_t propertyId) const
{
	if (propertyId > 31)
		return 0;

	return !!(this->_properties & IntUtil::maskFromIndex(propertyId));
}

void Assembler::setProperty(uint32_t propertyId, uint32_t value)
{
	if (propertyId > 31)
		return;

	if (value)
		this->_properties |= IntUtil::maskFromIndex(propertyId);
	else
		this->_properties &= ~IntUtil::maskFromIndex(propertyId);
}

// ============================================================================
// [AsmJit::Assembler - TakeCode]
// ============================================================================

uint8_t *Assembler::takeCode()
{
	uint8_t *code = this->_buffer.take();
	this->_relocData.clear();
	this->_zoneMemory.clear();

	if (this->_error != kErrorOk)
		this->setError(kErrorOk);

	return code;
}

// ============================================================================
// [AsmJit::Assembler - Clear / Reset]
// ============================================================================

void Assembler::clear()
{
	this->_purge();

	if (this->_error != kErrorOk)
		this->setError(kErrorOk);
}

void Assembler::reset()
{
	this->_purge();

	this->_zoneMemory.reset();
	this->_buffer.reset();

	this->_labels.reset();
	this->_relocData.reset();

	if (this->_error != kErrorOk)
		this->setError(kErrorOk);
}

void Assembler::_purge()
{
	this->_zoneMemory.clear();
	this->_buffer.clear();
 
	this->_emitOptions = 0;
	this->_trampolineSize = 0;

	this->_inlineComment = nullptr;
	this->_unusedLinks = nullptr;

	this->_labels.clear();
	this->_relocData.clear();
}

// ============================================================================
// [AsmJit::Assembler - Emit]
// ============================================================================

void Assembler::embed(const void *data, size_t len)
{
	if (!this->canEmit())
		return;

	if (this->_logger)
	{
		char buf[128];
		const char dot[] = ".data ";

		memcpy(buf, dot, ASMJIT_ARRAY_SIZE(dot) - 1);

		for (size_t i = 0; i < len; i += 16)
		{
			size_t max = len - i < 16 ? len - i : 16;
			char *p = buf + ASMJIT_ARRAY_SIZE(dot) - 1;

			for (size_t j = 0; j < max; ++j)
				p += sprintf(p, "%02X", reinterpret_cast<const uint8_t *>(data)[i + j]);

			*p++ = '\n';
			*p = '\0';

			this->_logger->logString(buf);
		}
	}

	this->_buffer.emitData(data, len);
}

// ============================================================================
// [AsmJit::Assembler - Helpers]
// ============================================================================

auto Assembler::_newLabelLink() -> LabelLink *
{
	LabelLink *link = this->_unusedLinks;

	if (link)
		this->_unusedLinks = link->prev;
	else
	{
		link = static_cast<LabelLink *>(this->_zoneMemory.alloc(sizeof(LabelLink)));
		if (!link)
			return nullptr;
	}

	// clean link
	link->prev = nullptr;
	link->offset = 0;
	link->displacement = 0;
	link->relocId = -1;

	return link;
}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"
