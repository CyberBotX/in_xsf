// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../core/logger.h"

// [Dependencies - C]
#include <stdarg.h>

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

// ============================================================================
// [AsmJit::Logger - Construction / Destruction]
// ============================================================================

Logger::Logger() : _flags(kLoggerIsEnabled | kLoggerIsUsed)
{
	memset(this->_instructionPrefix, 0, ASMJIT_ARRAY_SIZE(this->_instructionPrefix));
}

Logger::~Logger()
{
}

// ============================================================================
// [AsmJit::Logger - Logging]
// ============================================================================

void Logger::logFormat(const char *fmt, ...)
{
	char buf[1024];

	va_list ap;
	va_start(ap, fmt);
	size_t len = vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);

	this->logString(buf, len);
}

// ============================================================================
// [AsmJit::Logger - Enabled]
// ============================================================================

void Logger::setEnabled(bool enabled)
{
	if (enabled)
		this->_flags |= kLoggerIsEnabled | kLoggerIsUsed;
	else
		this->_flags &= ~(kLoggerIsEnabled | kLoggerIsUsed);
}

// ============================================================================
// [AsmJit::Logger - LogBinary]
// ============================================================================

void Logger::setLogBinary(bool value)
{
	if (value)
		this->_flags |= kLoggerOutputBinary;
	else
		this->_flags &= ~kLoggerOutputBinary;
}

// ============================================================================
// [AsmJit::Logger - HexImmediate]
// ============================================================================

void Logger::setHexImmediate(bool value)
{
	if (value)
		this->_flags |= kLoggerOutputHexImmediate;
	else
		this->_flags &= ~kLoggerOutputHexImmediate;
}

// ============================================================================
// [AsmJit::Logger - HexDisplacement]
// ============================================================================

void Logger::setHexDisplacement(bool value)
{
	if (value)
		this->_flags |= kLoggerOutputHexDisplacement;
	else
		this->_flags &= ~kLoggerOutputHexDisplacement;
}

// ============================================================================
// [AsmJit::Logger - InstructionPrefix]
// ============================================================================

void Logger::setInstructionPrefix(const char *prefix)
{
	memset(this->_instructionPrefix, 0, ASMJIT_ARRAY_SIZE(this->_instructionPrefix));

	if (!prefix)
		return;

	size_t length = strnlen(prefix, ASMJIT_ARRAY_SIZE(this->_instructionPrefix) - 1);
	memcpy(this->_instructionPrefix, prefix, length);
}

// ============================================================================
// [AsmJit::FileLogger - Construction / Destruction]
// ============================================================================

FileLogger::FileLogger(FILE *stream) : _stream(nullptr)
{
	this->setStream(stream);
}

FileLogger::~FileLogger()
{
}

// ============================================================================
// [AsmJit::FileLogger - Accessors]
// ============================================================================

//! @brief Set file stream.
void FileLogger::setStream(FILE *stream)
{
	this->_stream = stream;

	if (this->isEnabled() && this->_stream)
		this->_flags |= kLoggerIsUsed;
	else
		this->_flags &= ~kLoggerIsUsed;
}

// ============================================================================
// [AsmJit::FileLogger - Logging]
// ============================================================================

void FileLogger::logString(const char *buf, size_t len)
{
	if (!this->isUsed())
		return;

	if (len == kInvalidSize)
		len = strlen(buf);

	fwrite(buf, 1, len, this->_stream);
}

// ============================================================================
// [AsmJit::FileLogger - Enabled]
// ============================================================================

void FileLogger::setEnabled(bool enabled)
{
	if (enabled)
		this->_flags |= kLoggerIsEnabled | (this->_stream ? kLoggerIsUsed : 0);
	else
		this->_flags &= ~(kLoggerIsEnabled | kLoggerIsUsed);
}

// ============================================================================
// [AsmJit::StringLogger - Construction / Destruction]
// ============================================================================

StringLogger::StringLogger()
{
}

StringLogger::~StringLogger()
{
}

// ============================================================================
// [AsmJit::StringLogger - Logging]
// ============================================================================

void StringLogger::logString(const char *buf, size_t len)
{
	if (!this->isUsed())
		return;
	this->_stringBuilder.appendString(buf, len);
}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"
