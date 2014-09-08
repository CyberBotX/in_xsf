// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#pragma once

// [Dependencies - AsmJit]
#include "../core/defs.h"
#include "../core/operand.h"

#include "../x86/x86defs.h"

namespace AsmJit
{

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct GpReg;
struct GpVar;
struct Mem;
struct MmReg;
struct MmVar;
struct Var;
struct X87Reg;
struct X87Var;
struct XmmReg;
struct XmmVar;

struct SegmentReg;

//! @addtogroup AsmJit_X86
//! @{

// ============================================================================
// [AsmJit::MmData]
// ============================================================================

//! @brief Structure used for MMX specific data (64-bit).
//!
//! This structure can be used to load / store data from / to MMX register.
union MmData
{
	// --------------------------------------------------------------------------
	// [Methods]
	// --------------------------------------------------------------------------

	//! @brief Set all eight signed 8-bit integers.
	void setSB(int8_t x0, int8_t x1, int8_t x2, int8_t x3, int8_t x4, int8_t x5, int8_t x6, int8_t x7)
	{
		this->sb[0] = x0;
		this->sb[1] = x1;
		this->sb[2] = x2;
		this->sb[3] = x3;
		this->sb[4] = x4;
		this->sb[5] = x5;
		this->sb[6] = x6;
		this->sb[7] = x7;
	}

	//! @brief Set all eight unsigned 8-bit integers.
	void setUB(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7)
	{
		this->ub[0] = x0;
		this->ub[1] = x1;
		this->ub[2] = x2;
		this->ub[3] = x3;
		this->ub[4] = x4;
		this->ub[5] = x5;
		this->ub[6] = x6;
		this->ub[7] = x7;
	}

	//! @brief Set all four signed 16-bit integers.
	void setSW(int16_t x0, int16_t x1, int16_t x2, int16_t x3)
	{
		this->sw[0] = x0;
		this->sw[1] = x1;
		this->sw[2] = x2;
		this->sw[3] = x3;
	}

	//! @brief Set all four unsigned 16-bit integers.
	void setUW(uint16_t x0, uint16_t x1, uint16_t x2, uint16_t x3)
	{
		this->uw[0] = x0;
		this->uw[1] = x1;
		this->uw[2] = x2;
		this->uw[3] = x3;
	}

	//! @brief Set all two signed 32-bit integers.
	void setSD(int32_t x0, int32_t x1)
	{
		this->sd[0] = x0;
		this->sd[1] = x1;
	}

	//! @brief Set all two unsigned 32-bit integers.
	void setUD(uint32_t x0, uint32_t x1)
	{
		this->ud[0] = x0;
		this->ud[1] = x1;
	}

	//! @brief Set signed 64-bit integer.
	void setSQ(int64_t x0)
	{
		this->sq[0] = x0;
	}

	//! @brief Set unsigned 64-bit integer.
	void setUQ(uint64_t x0)
	{
		this->uq[0] = x0;
	}

	//! @brief Set all two SP-FP values.
	void setSF(float x0, float x1)
	{
		this->sf[0] = x0;
		this->sf[1] = x1;
	}

	// --------------------------------------------------------------------------
	// [Members]
	// --------------------------------------------------------------------------

	//! @brief Array of eight signed 8-bit integers.
	int8_t sb[8];
	//! @brief Array of eight unsigned 8-bit integers.
	uint8_t ub[8];
	//! @brief Array of four signed 16-bit integers.
	int16_t sw[4];
	//! @brief Array of four unsigned 16-bit integers.
	uint16_t uw[4];
	//! @brief Array of two signed 32-bit integers.
	int32_t sd[2];
	//! @brief Array of two unsigned 32-bit integers.
	uint32_t ud[2];
	//! @brief Array of one signed 64-bit integer.
	int64_t sq[1];
	//! @brief Array of one unsigned 64-bit integer.
	uint64_t uq[1];

	//! @brief Array of two SP-FP values.
	float sf[2];
};

// ============================================================================
// [AsmJit::XmmData]
// ============================================================================

//! @brief Structure used for SSE specific data (128-bit).
//!
//! This structure can be used to load / store data from / to SSE register.
//!
//! @note Always align SSE data to 16-bytes.
union XmmData
{
	// --------------------------------------------------------------------------
	// [Methods]
	// --------------------------------------------------------------------------

	//! @brief Set all sixteen signed 8-bit integers.
	void setSB(int8_t x0, int8_t x1, int8_t x2, int8_t x3, int8_t x4, int8_t x5, int8_t x6, int8_t x7, int8_t x8, int8_t x9, int8_t x10, int8_t x11, int8_t x12, int8_t x13, int8_t x14, int8_t x15)
	{
		this->sb[0] = x0;
		this->sb[1] = x1;
		this->sb[2] = x2;
		this->sb[3] = x3;
		this->sb[4] = x4;
		this->sb[5] = x5;
		this->sb[6] = x6;
		this->sb[7] = x7;
		this->sb[8] = x8;
		this->sb[9] = x9;
		this->sb[10] = x10;
		this->sb[11] = x11;
		this->sb[12] = x12;
		this->sb[13] = x13;
		this->sb[14] = x14;
		this->sb[15] = x15; 
	}

	//! @brief Set all sixteen unsigned 8-bit integers.
	void setUB(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7, uint8_t x8, uint8_t x9, uint8_t x10, uint8_t x11, uint8_t x12, uint8_t x13, uint8_t x14, uint8_t x15)
	{
		this->ub[0] = x0;
		this->ub[1] = x1;
		this->ub[2] = x2;
		this->ub[3] = x3;
		this->ub[4] = x4;
		this->ub[5] = x5;
		this->ub[6] = x6;
		this->ub[7] = x7;
		this->ub[8] = x8;
		this->ub[9] = x9;
		this->ub[10] = x10;
		this->ub[11] = x11;
		this->ub[12] = x12;
		this->ub[13] = x13;
		this->ub[14] = x14;
		this->ub[15] = x15; 
	}

	//! @brief Set all eight signed 16-bit integers.
	void setSW(int16_t x0, int16_t x1, int16_t x2, int16_t x3, int16_t x4, int16_t x5, int16_t x6, int16_t x7)
	{
		this->sw[0] = x0;
		this->sw[1] = x1;
		this->sw[2] = x2;
		this->sw[3] = x3;
		this->sw[4] = x4;
		this->sw[5] = x5;
		this->sw[6] = x6;
		this->sw[7] = x7;
	}

	//! @brief Set all eight unsigned 16-bit integers.
	void setUW(uint16_t x0, uint16_t x1, uint16_t x2, uint16_t x3, uint16_t x4, uint16_t x5, uint16_t x6, uint16_t x7)
	{
		this->uw[0] = x0;
		this->uw[1] = x1;
		this->uw[2] = x2;
		this->uw[3] = x3;
		this->uw[4] = x4;
		this->uw[5] = x5;
		this->uw[6] = x6;
		this->uw[7] = x7;
	}

	//! @brief Set all four signed 32-bit integers.
	void setSD(int32_t x0, int32_t x1, int32_t x2, int32_t x3)
	{
		this->sd[0] = x0;
		this->sd[1] = x1;
		this->sd[2] = x2;
		this->sd[3] = x3;
	}

	//! @brief Set all four unsigned 32-bit integers.
	void setUD(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3)
	{
		this->ud[0] = x0;
		this->ud[1] = x1;
		this->ud[2] = x2;
		ud[3] = x3;
	}

	//! @brief Set all two signed 64-bit integers.
	void setSQ(int64_t x0, int64_t x1)
	{
		this->sq[0] = x0;
		this->sq[1] = x1;
	}

	//! @brief Set all two unsigned 64-bit integers.
	void setUQ(uint64_t x0, uint64_t x1)
	{
		this->uq[0] = x0;
		this->uq[1] = x1;
	}

	//! @brief Set all four SP-FP floats.
	void setSF(float x0, float x1, float x2, float x3)
	{
		this->sf[0] = x0;
		this->sf[1] = x1;
		this->sf[2] = x2;
		this->sf[3] = x3;
	}

	//! @brief Set all two DP-FP floats.
	void setDF(double x0, double x1)
	{
		this->df[0] = x0;
		this->df[1] = x1;
	}

	// --------------------------------------------------------------------------
	// [Members]
	// --------------------------------------------------------------------------

	//! @brief Array of sixteen signed 8-bit integers.
	int8_t sb[16];
	//! @brief Array of sixteen unsigned 8-bit integers.
	uint8_t ub[16];
	//! @brief Array of eight signed 16-bit integers.
	int16_t sw[8];
	//! @brief Array of eight unsigned 16-bit integers.
	uint16_t uw[8];
	//! @brief Array of four signed 32-bit integers.
	int32_t sd[4];
	//! @brief Array of four unsigned 32-bit integers.
	uint32_t ud[4];
	//! @brief Array of two signed 64-bit integers.
	int64_t sq[2];
	//! @brief Array of two unsigned 64-bit integers.
	uint64_t uq[2];

	//! @brief Array of four 32-bit single precision floating points.
	float sf[4];
	//! @brief Array of two 64-bit double precision floating points.
	double df[2];
};

// ============================================================================
// [AsmJit::GpReg]
// ============================================================================

//! @brief General purpose register.
//!
//! This class is for all general purpose registers (64, 32, 16 and 8-bit).
struct GpReg : public Reg
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	//! @brief Create non-initialized general purpose register.
	GpReg() : Reg(kInvalidValue, 0) { }
	//! @brief Create a reference to @a other general purpose register.
	GpReg(const GpReg &other) : Reg(other) { }

#ifndef ASMJIT_NODOC
	GpReg(const _DontInitialize &dontInitialize) : Reg(dontInitialize) { }
	GpReg(const _Initialize &, uint32_t code) : Reg(code, static_cast<uint32_t>(1U << ((code & kRegTypeMask) >> 12))) { }
#endif // ASMJIT_NODOC

	// --------------------------------------------------------------------------
	// [Reg Specific]
	// --------------------------------------------------------------------------

	//! @brief Set register code to @a code.
	GpReg &setCode(uint32_t code)
	{
		this->_reg.code = code;
		return *this;
	}

	//! @brief Set register size to @a size.
	GpReg &setSize(uint32_t size)
	{
		this->_reg.size = static_cast<uint8_t>(size);
		return *this;
	}

	// --------------------------------------------------------------------------
	// [GpReg Specific]
	// --------------------------------------------------------------------------

	//! @brief Get whether the general purpose register is BYTE (8-bit) type.
	bool isGpb() const { return (this->_reg.code & kRegTypeMask) <= kX86RegTypeGpbHi; }
	//! @brief Get whether the general purpose register is LO-BYTE (8-bit) type.
	bool isGpbLo() const { return (this->_reg.code & kRegTypeMask) == kX86RegTypeGpbLo; }
	//! @brief Get whether the general purpose register is HI-BYTE (8-bit) type.
	bool isGpbHi() const { return (this->_reg.code & kRegTypeMask) == kX86RegTypeGpbHi; }

	//! @brief Get whether the general purpose register is WORD (16-bit) type.
	bool isGpw() const { return (this->_reg.code & kRegTypeMask) == kX86RegTypeGpw; }
	//! @brief Get whether the general purpose register is DWORD (32-bit) type.
	//!
	//! This is default type for 32-bit platforms.
	bool isGpd() const { return (this->_reg.code & kRegTypeMask) == kX86RegTypeGpd; }
	//! @brief Get whether the general purpose register is QWORD (64-bit) type.
	//!
	//! This is default type for 64-bit platforms.
	bool isGpq() const { return (this->_reg.code & kRegTypeMask) == kX86RegTypeGpq; }

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	GpReg &operator=(const GpReg &other) { this->_copy(other); return *this; }
	bool operator==(const GpReg &other) const { return this->getRegCode() == other.getRegCode(); }
	bool operator!=(const GpReg &other) const { return this->getRegCode() != other.getRegCode(); }
#endif // ASMJIT_NODOC
};

// ============================================================================
// [AsmJit::X87Reg]
// ============================================================================

//! @brief 80-bit x87 floating point register.
//!
//! To create instance of x87 register, use @c st() function.
struct X87Reg : public Reg
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	//! @brief Create non-initialized x87 register.
	X87Reg() : Reg(kInvalidValue, 10) { }
	//! @brief Create a reference to @a other x87 register.
	X87Reg(const X87Reg &other) : Reg(other) { }

#ifndef ASMJIT_NODOC
	X87Reg(const _DontInitialize &dontInitialize) : Reg(dontInitialize) { }
	X87Reg(const _Initialize &, uint32_t code) : Reg(code | kX86RegTypeX87, 10) { }
#endif // ASMJIT_NODOC

	// --------------------------------------------------------------------------
	// [Reg Specific]
	// --------------------------------------------------------------------------

	//! @brief Set register code to @a code.
	X87Reg &setCode(uint32_t code)
	{
		this->_reg.code = code;
		return *this;
	}

	//! @brief Set register size to @a size.
	X87Reg &setSize(uint32_t size)
	{
		this->_reg.size = static_cast<uint8_t>(size);
		return *this;
	}

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	X87Reg &operator=(const X87Reg &other) { this->_copy(other); return *this; }
	bool operator==(const X87Reg &other) const { return this->getRegCode() == other.getRegCode(); }
	bool operator!=(const X87Reg &other) const { return this->getRegCode() != other.getRegCode(); }
#endif // ASMJIT_NODOC
};

// ============================================================================
// [AsmJit::MmReg]
// ============================================================================

//! @brief 64-bit MMX register.
struct MmReg : public Reg
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	//! @brief Create non-initialized MM register.
	MmReg() : Reg(kInvalidValue, 8) { }
	//! @brief Create a reference to @a other MM register.
	MmReg(const MmReg &other) : Reg(other) { }

#ifndef ASMJIT_NODOC
	MmReg(const _DontInitialize &dontInitialize) : Reg(dontInitialize) { }
	MmReg(const _Initialize &, uint32_t code) : Reg(code, 8) { }
#endif // ASMJIT_NODOC

	// --------------------------------------------------------------------------
	// [Reg Specific]
	// --------------------------------------------------------------------------

	//! @brief Set register code to @a code.
	MmReg &setCode(uint32_t code)
	{
		this->_reg.code = code;
		return *this;
	}

	//! @brief Set register size to @a size.
	MmReg &setSize(uint32_t size)
	{
		this->_reg.size = static_cast<uint8_t>(size);
		return *this;
	}

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	MmReg &operator=(const MmReg &other) { this->_copy(other); return *this; }
	bool operator==(const MmReg &other) const { return this->getRegCode() == other.getRegCode(); }
	bool operator!=(const MmReg &other) const { return this->getRegCode() != other.getRegCode(); }
#endif // ASMJIT_NODOC
};

// ============================================================================
// [AsmJit::XmmReg]
// ============================================================================

//! @brief 128-bit SSE register.
struct XmmReg : public Reg
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	//! @brief Create non-initialized XMM register.
	XmmReg() : Reg(kInvalidValue, 16) { }
	//! @brief Create a reference to @a other XMM register.
	XmmReg(const _Initialize &, uint32_t code) : Reg(code, 16) { }

#ifndef ASMJIT_NODOC
	XmmReg(const _DontInitialize &dontInitialize) : Reg(dontInitialize) { }
	XmmReg(const XmmReg &other) : Reg(other) { }
#endif // ASMJIT_NODOC

	// --------------------------------------------------------------------------
	// [Reg Specific]
	// --------------------------------------------------------------------------

	//! @brief Set register code to @a code.
	XmmReg &setCode(uint32_t code)
	{
		this->_reg.code = code;
		return *this;
	}

	//! @brief Set register size to @a size.
	XmmReg &setSize(uint32_t size)
	{
		this->_reg.size = static_cast<uint8_t>(size);
		return *this;
	}

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	XmmReg &operator=(const XmmReg &other) { this->_copy(other); return *this; }
	bool operator==(const XmmReg &other) const { return this->getRegCode() == other.getRegCode(); }
	bool operator!=(const XmmReg &other) const { return this->getRegCode() != other.getRegCode(); }
#endif // ASMJIT_NODOC
};

// ============================================================================
// [AsmJit::SegmentReg]
// ============================================================================

//! @brief Segment register.
struct SegmentReg : public Reg
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	//! @brief Create non-initialized segment register.
	SegmentReg() : Reg(kInvalidValue, 2) { }
	//! @brief Create a reference to @a other segment register.
	SegmentReg(const _Initialize &, uint32_t code) : Reg(code, 2) { }

#ifndef ASMJIT_NODOC
	SegmentReg(const _DontInitialize &dontInitialize) : Reg(dontInitialize) { }
	SegmentReg(const SegmentReg &other) : Reg(other) { }
#endif // ASMJIT_NODOC

	// --------------------------------------------------------------------------
	// [Reg Specific]
	// --------------------------------------------------------------------------

	//! @brief Set register code to @a code.
	SegmentReg &setCode(uint32_t code)
	{
		this->_reg.code = code;
		return *this;
	}

	//! @brief Set register size to @a size.
	SegmentReg &setSize(uint32_t size)
	{
		this->_reg.size = static_cast<uint8_t>(size);
		return *this;
	}

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	SegmentReg &operator=(const SegmentReg &other) { this->_copy(other); return *this; }
	bool operator==(const SegmentReg &other) const { return this->getRegCode() == other.getRegCode(); }
	bool operator!=(const SegmentReg &other) const { return this->getRegCode() != other.getRegCode(); }
#endif // ASMJIT_NODOC
};

// ============================================================================
// [AsmJit::Registers - no_reg]
// ============================================================================

//! @brief No register, can be used only in @c Mem operand.
ASMJIT_VAR const GpReg no_reg;

// ============================================================================
// [AsmJit::Registers - 8-bit]
// ============================================================================

//! @brief 8-bit General purpose register.
ASMJIT_VAR const GpReg al;
//! @brief 8-bit General purpose register.
ASMJIT_VAR const GpReg cl;
//! @brief 8-bit General purpose register.
ASMJIT_VAR const GpReg dl;
//! @brief 8-bit General purpose register.
ASMJIT_VAR const GpReg bl;

#ifdef ASMJIT_X64
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg spl;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg bpl;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg sil;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg dil;

//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r8b;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r9b;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r10b;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r11b;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r12b;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r13b;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r14b;
//! @brief 8-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r15b;
#endif // ASMJIT_X64

//! @brief 8-bit General purpose register.
ASMJIT_VAR const GpReg ah;
//! @brief 8-bit General purpose register.
ASMJIT_VAR const GpReg ch;
//! @brief 8-bit General purpose register.
ASMJIT_VAR const GpReg dh;
//! @brief 8-bit General purpose register.
ASMJIT_VAR const GpReg bh;

// ============================================================================
// [AsmJit::Registers - 16-bit]
// ============================================================================

//! @brief 16-bit General purpose register.
ASMJIT_VAR const GpReg ax;
//! @brief 16-bit General purpose register.
ASMJIT_VAR const GpReg cx;
//! @brief 16-bit General purpose register.
ASMJIT_VAR const GpReg dx;
//! @brief 16-bit General purpose register.
ASMJIT_VAR const GpReg bx;
//! @brief 16-bit General purpose register.
ASMJIT_VAR const GpReg sp;
//! @brief 16-bit General purpose register.
ASMJIT_VAR const GpReg bp;
//! @brief 16-bit General purpose register.
ASMJIT_VAR const GpReg si;
//! @brief 16-bit General purpose register.
ASMJIT_VAR const GpReg di;

#ifdef ASMJIT_X64
//! @brief 16-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r8w;
//! @brief 16-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r9w;
//! @brief 16-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r10w;
//! @brief 16-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r11w;
//! @brief 16-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r12w;
//! @brief 16-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r13w;
//! @brief 16-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r14w;
//! @brief 16-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r15w;
#endif // ASMJIT_X64

// ============================================================================
// [AsmJit::Registers - 32-bit]
// ============================================================================

//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg eax;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg ecx;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg edx;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg ebx;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg esp;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg ebp;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg esi;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg edi;

#ifdef ASMJIT_X64
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg r8d;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg r9d;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg r10d;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg r11d;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg r12d;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg r13d;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg r14d;
//! @brief 32-bit General purpose register.
ASMJIT_VAR const GpReg r15d;
#endif // ASMJIT_X64

// ============================================================================
// [AsmJit::Registers - 64-bit]
// ============================================================================

#ifdef ASMJIT_X64
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg rax;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg rcx;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg rdx;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg rbx;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg rsp;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg rbp;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg rsi;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg rdi;

//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r8;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r9;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r10;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r11;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r12;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r13;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r14;
//! @brief 64-bit General purpose register (64-bit mode only).
ASMJIT_VAR const GpReg r15;
#endif // ASMJIT_X64

// ============================================================================
// [AsmJit::Registers - Native (AsmJit extension)]
// ============================================================================

//! @brief 32-bit or 64-bit General purpose register.
ASMJIT_VAR const GpReg zax;
//! @brief 32-bit or 64-bit General purpose register.
ASMJIT_VAR const GpReg zcx;
//! @brief 32-bit or 64-bit General purpose register.
ASMJIT_VAR const GpReg zdx;
//! @brief 32-bit or 64-bit General purpose register.
ASMJIT_VAR const GpReg zbx;
//! @brief 32-bit or 64-bit General purpose register.
ASMJIT_VAR const GpReg zsp;
//! @brief 32-bit or 64-bit General purpose register.
ASMJIT_VAR const GpReg zbp;
//! @brief 32-bit or 64-bit General purpose register.
ASMJIT_VAR const GpReg zsi;
//! @brief 32-bit or 64-bit General purpose register.
ASMJIT_VAR const GpReg zdi;

// ============================================================================
// [AsmJit::Registers - MM]
// ============================================================================

//! @brief 64-bit MM register.
ASMJIT_VAR const MmReg mm0;
//! @brief 64-bit MM register.
ASMJIT_VAR const MmReg mm1;
//! @brief 64-bit MM register.
ASMJIT_VAR const MmReg mm2;
//! @brief 64-bit MM register.
ASMJIT_VAR const MmReg mm3;
//! @brief 64-bit MM register.
ASMJIT_VAR const MmReg mm4;
//! @brief 64-bit MM register.
ASMJIT_VAR const MmReg mm5;
//! @brief 64-bit MM register.
ASMJIT_VAR const MmReg mm6;
//! @brief 64-bit MM register.
ASMJIT_VAR const MmReg mm7;

// ============================================================================
// [AsmJit::Registers - XMM]
// ============================================================================

//! @brief 128-bit XMM register.
ASMJIT_VAR const XmmReg xmm0;
//! @brief 128-bit XMM register.
ASMJIT_VAR const XmmReg xmm1;
//! @brief 128-bit XMM register.
ASMJIT_VAR const XmmReg xmm2;
//! @brief 128-bit XMM register.
ASMJIT_VAR const XmmReg xmm3;
//! @brief 128-bit XMM register.
ASMJIT_VAR const XmmReg xmm4;
//! @brief 128-bit XMM register.
ASMJIT_VAR const XmmReg xmm5;
//! @brief 128-bit XMM register.
ASMJIT_VAR const XmmReg xmm6;
//! @brief 128-bit XMM register.
ASMJIT_VAR const XmmReg xmm7;

#ifdef ASMJIT_X64
//! @brief 128-bit XMM register (64-bit mode only).
ASMJIT_VAR const XmmReg xmm8;
//! @brief 128-bit XMM register (64-bit mode only).
ASMJIT_VAR const XmmReg xmm9;
//! @brief 128-bit XMM register (64-bit mode only).
ASMJIT_VAR const XmmReg xmm10;
//! @brief 128-bit XMM register (64-bit mode only).
ASMJIT_VAR const XmmReg xmm11;
//! @brief 128-bit XMM register (64-bit mode only).
ASMJIT_VAR const XmmReg xmm12;
//! @brief 128-bit XMM register (64-bit mode only).
ASMJIT_VAR const XmmReg xmm13;
//! @brief 128-bit XMM register (64-bit mode only).
ASMJIT_VAR const XmmReg xmm14;
//! @brief 128-bit XMM register (64-bit mode only).
ASMJIT_VAR const XmmReg xmm15;
#endif // ASMJIT_X64

// ============================================================================
// [AsmJit::Registers - Segment]
// ============================================================================

//! @brief CS segment register.
ASMJIT_VAR const SegmentReg cs;
//! @brief SS segment register.
ASMJIT_VAR const SegmentReg ss;
//! @brief DS segment register.
ASMJIT_VAR const SegmentReg ds;
//! @brief ES segment register.
ASMJIT_VAR const SegmentReg es;
//! @brief FS segment register.
ASMJIT_VAR const SegmentReg fs;
//! @brief GS segment register.
ASMJIT_VAR const SegmentReg gs;

// ============================================================================
// [AsmJit::Registers - Register From Index]
// ============================================================================

//! @brief Get general purpose register of byte size.
inline GpReg gpb_lo(uint32_t index) { return GpReg(_Initialize(), index | kX86RegTypeGpbLo); }

//! @brief Get general purpose register of byte size.
inline GpReg gpb_hi(uint32_t index) { return GpReg(_Initialize(), index | kX86RegTypeGpbHi); }

//! @brief Get general purpose register of word size.
inline GpReg gpw(uint32_t index) { return GpReg(_Initialize(), index | kX86RegTypeGpw); }

//! @brief Get general purpose register of dword size.
inline GpReg gpd(uint32_t index) { return GpReg(_Initialize(), index | kX86RegTypeGpd); }

#ifdef ASMJIT_X64
//! @brief Get general purpose register of qword size (64-bit only).
inline GpReg gpq(uint32_t index) { return GpReg(_Initialize(), index | kX86RegTypeGpq); }
#endif

//! @brief Get general purpose dword/qword register (depending to architecture).
inline GpReg gpz(uint32_t index) { return GpReg(_Initialize(), index | kX86RegTypeGpz); }

//! @brief Get MMX (MM) register .
inline MmReg mm(uint32_t index) { return MmReg(_Initialize(), index | kX86RegTypeMm); }

//! @brief Get SSE (XMM) register.
inline XmmReg xmm(uint32_t index) { return XmmReg(_Initialize(), index | kX86RegTypeXmm); }

//! @brief Get x87 register with index @a i.
inline X87Reg st(uint32_t i)
{
	ASMJIT_ASSERT(i < 8);
	return X87Reg(_Initialize(), i);
}

// ============================================================================
// [AsmJit::Mem]
// ============================================================================

//! @brief Memory operand.
struct Mem : public Operand
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	Mem() : Operand(_DontInitialize())
	{
		this->_mem.op = kOperandMem;
		this->_mem.size = 0;
		this->_mem.type = kOperandMemNative;
		this->_mem.segment = kX86SegNone;
		this->_mem.sizePrefix = 0;
		this->_mem.shift = 0;

		this->_mem.id = kInvalidValue;
		this->_mem.base = kInvalidValue;
		this->_mem.index = kInvalidValue;

		this->_mem.target = nullptr;
		this->_mem.displacement = 0;
	}

	Mem(const Label &label, sysint_t displacement, uint32_t size = 0) : Operand(_DontInitialize())
	{
		this->_mem.op = kOperandMem;
		this->_mem.size = static_cast<uint8_t>(size);
		this->_mem.type = kOperandMemLabel;
		this->_mem.segment = kX86SegNone;
		this->_mem.sizePrefix = 0;
		this->_mem.shift = 0;

		this->_mem.id = kInvalidValue;
		this->_mem.base = reinterpret_cast<const Operand &>(label)._base.id;
		_mem.index = kInvalidValue;

		_mem.target = nullptr;
		_mem.displacement = displacement;
	}

	Mem(const GpReg &base, sysint_t displacement, uint32_t size = 0) : Operand(_DontInitialize())
	{
		this->_mem.op = kOperandMem;
		this->_mem.size = static_cast<uint8_t>(size);
		this->_mem.type = kOperandMemNative;
		this->_mem.segment = kX86SegNone;

#ifdef ASMJIT_X86
		this->_mem.sizePrefix = base.getSize() != 4;
#else
		this->_mem.sizePrefix = base.getSize() != 8;
#endif

		this->_mem.shift = 0;

		this->_mem.id = kInvalidValue;
		this->_mem.base = base.getRegCode() & kRegIndexMask;
		this->_mem.index = kInvalidValue;

		this->_mem.target = nullptr;
		this->_mem.displacement = displacement;
	}

	Mem(const GpVar &base, sysint_t displacement, uint32_t size = 0) : Operand(_DontInitialize())
	{
		this->_mem.op = kOperandMem;
		this->_mem.size = static_cast<uint8_t>(size);
		this->_mem.type = kOperandMemNative;
		this->_mem.segment = kX86SegNone;

#ifdef ASMJIT_X86
		this->_mem.sizePrefix = (reinterpret_cast<const Operand &>(base)._var.size) != 4;
#else
		this->_mem.sizePrefix = (reinterpret_cast<const Operand &>(base)._var.size) != 8;
#endif

		this->_mem.shift = 0;

		this->_mem.id = kInvalidValue;
		this->_mem.base = reinterpret_cast<const Operand &>(base).getId();
		this->_mem.index = kInvalidValue;

		this->_mem.target = nullptr;
		this->_mem.displacement = displacement;
	}

	Mem(const GpReg &base, const GpReg &index, uint32_t shift, sysint_t displacement, uint32_t size = 0) : Operand(_DontInitialize())
	{
		ASMJIT_ASSERT(shift <= 3);

		this->_mem.op = kOperandMem;
		this->_mem.size = static_cast<uint8_t>(size);
		this->_mem.type = kOperandMemNative;
		this->_mem.segment = kX86SegNone;

#ifdef ASMJIT_X86
		this->_mem.sizePrefix = (base.getSize() | index.getSize()) != 4;
#else
		this->_mem.sizePrefix = (base.getSize() | index.getSize()) != 8;
#endif

		this->_mem.shift = static_cast<uint8_t>(shift);

		this->_mem.id = kInvalidValue;
		this->_mem.base = base.getRegIndex();
		this->_mem.index = index.getRegIndex();

		this->_mem.target = nullptr;
		this->_mem.displacement = displacement;
	}

	Mem(const GpVar &base, const GpVar &index, uint32_t shift, sysint_t displacement, uint32_t size = 0) : Operand(_DontInitialize())
	{
		ASMJIT_ASSERT(shift <= 3);

		this->_mem.op = kOperandMem;
		this->_mem.size = static_cast<uint8_t>(size);
		this->_mem.type = kOperandMemNative;
		this->_mem.segment = kX86SegNone;

#ifdef ASMJIT_X86
		this->_mem.sizePrefix = (reinterpret_cast<const Operand &>(base)._var.size | reinterpret_cast<const Operand &>(index)._var.size) != 4;
#else
		this->_mem.sizePrefix = (reinterpret_cast<const Operand &>(base)._var.size | reinterpret_cast<const Operand &>(index)._var.size) != 8;
#endif

		this->_mem.shift = static_cast<uint8_t>(shift);

		this->_mem.id = kInvalidValue;
		this->_mem.base = reinterpret_cast<const Operand &>(base).getId();
		this->_mem.index = reinterpret_cast<const Operand &>(index).getId();

		this->_mem.target = nullptr;
		this->_mem.displacement = displacement;
	}

	Mem(const Mem &other) : Operand(other) { }

	Mem(const _DontInitialize &dontInitialize) : Operand(dontInitialize) { }

	// --------------------------------------------------------------------------
	// [Mem Specific]
	// --------------------------------------------------------------------------

	//! @brief Get type of memory operand, see @c kOperandMemType.
	uint32_t getMemType() const { return this->_mem.type; }

	//! @brief Get memory operand segment, see @c kX86Seg.
	uint32_t getSegment() const { return this->_mem.segment; }

	//! @brief Set memory operand segment, see @c kX86Seg.
	Mem &setSegment(uint32_t seg)
	{
		this->_mem.segment = static_cast<uint8_t>(seg);
		return *this;
	}

	//! @brief Set memory operand segment, see @c kX86Seg.
	Mem &setSegment(const SegmentReg &seg)
	{
		this->_mem.segment = static_cast<uint8_t>(seg.getRegIndex());
		return *this;
	}

	//! @brief Get whether the memory operand has segment override prefix.
	bool hasSegment() const { return this->_mem.segment >= kX86SegCount; }

	//! @brief Get whether the memory operand has base register.
	bool hasBase() const { return this->_mem.base != kInvalidValue; }

	//! @brief Get whether the memory operand has index.
	bool hasIndex() const { return this->_mem.index != kInvalidValue; }

	//! @brief Get whether the memory operand has shift used.
	bool hasShift() const { return !!this->_mem.shift; }

	//! @brief Get memory operand base register or @c kInvalidValue.
	uint32_t getBase() const { return this->_mem.base; }

	//! @brief Get memory operand index register or @c kInvalidValue.
	uint32_t getIndex() const { return this->_mem.index; }

	//! @brief Get memory operand index scale (0, 1, 2 or 3).
	uint32_t getShift() const { return this->_mem.shift; }

	//! @brief Get whether to use size-override prefix.
	//!
	//! @note This is useful only for MOV and LEA type of instructions.
	bool getSizePrefix() const { return !!this->_mem.sizePrefix; }

	//! @brief Set whether to use size-override prefix.
	Mem &setSizePrefix(bool b)
	{
		this->_mem.sizePrefix = b;
		return *this;
	}

	//! @brief Get absolute target address.
	//!
	//! @note You should always check if operand contains address by @c getMemType().
	void *getTarget() const { return this->_mem.target; }

	//! @brief Set absolute target address.
	Mem &setTarget(void *target)
	{
		this->_mem.target = target;
		return *this;
	}

	//! @brief Set memory operand size.
	Mem &setSize(uint32_t size)
	{
		this->_mem.size = size;
		return *this;
	}

	//! @brief Get memory operand relative displacement.
	sysint_t getDisplacement() const { return this->_mem.displacement; }

	//! @brief Set memory operand relative displacement.
	Mem &setDisplacement(sysint_t displacement)
	{
		this->_mem.displacement = displacement;
		return *this;
	}

	//! @brief Adjust memory operand relative displacement by @a displacement.
	Mem &adjust(sysint_t displacement)
	{
		this->_mem.displacement += displacement;
		return *this;
	}

	//! @brief Get new memory operand adjusted by @a displacement.
	Mem adjusted(sysint_t displacement) const
	{
		Mem result(*this);
		result.adjust(displacement);
		return result;
	}

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	Mem &operator=(const Mem &other) { this->_copy(other); return *this; }

	bool operator==(const Mem &other) const
	{
		return this->_bin.u32[0] == other._bin.u32[0] && this->_bin.u32[1] == other._bin.u32[1] && this->_bin.u32[2] == other._bin.u32[2] && this->_bin.u32[3] == other._bin.u32[3] &&
			this->_bin.uptr[0] == other._bin.uptr[0] && this->_bin.uptr[1] == other._bin.uptr[1];
	}

	bool operator!=(const Mem &other) const
	{
		return !(*this == other);
	}
#endif // ASMJIT_NODOC
};

// ============================================================================
// [AsmJit::Var]
// ============================================================================

ASMJIT_API Mem _BaseVarMem(const Var &var, uint32_t size, sysint_t disp = 0);
ASMJIT_API Mem _BaseVarMem(const Var &var, uint32_t size, const GpVar &index, uint32_t shift, sysint_t disp);

//! @brief Base class for all variables.
struct Var : public Operand
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	Var(const _DontInitialize &dontInitialize) : Operand(dontInitialize) { }
#endif // ASMJIT_NODOC

	Var() : Operand(_DontInitialize())
	{
		this->_var.op = kOperandVar;
		this->_var.size = 0;
		this->_var.regCode = kInvalidValue;
		this->_var.varType = kInvalidValue;
		this->_var.id = kInvalidValue;
	}

	Var(const Var &other) : Operand(other) { }

	// --------------------------------------------------------------------------
	// [Type]
	// --------------------------------------------------------------------------

	uint32_t getVarType() const { return this->_var.varType; }

	bool isGpVar() const { return this->_var.varType <= kX86VarTypeGpq; }

	bool isX87Var() const { return this->_var.varType >= kX86VarTypeX87 && this->_var.varType <= kX86VarTypeX87SD; }

	bool isMmVar() const { return this->_var.varType == kX86VarTypeMm; }

	bool isXmmVar() const { return this->_var.varType >= kX86VarTypeXmm && this->_var.varType <= kX86VarTypeXmmPD; }

	// --------------------------------------------------------------------------
	// [Memory Cast]
	// --------------------------------------------------------------------------

	//! @brief Cast this variable to memory operand.
	//!
	//! @note Size of operand depends on native variable type, you can use other
	//! variants if you want specific one.
	Mem m() const { return _BaseVarMem(*this, kInvalidValue); }

	//! @overload.
	Mem m(sysint_t disp) const { return _BaseVarMem(*this, kInvalidValue, disp); }

	//! @overload.
	Mem m(const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) const { return _BaseVarMem(*this, kInvalidValue, index, shift, disp); }

	//! @brief Cast this variable to 8-bit memory operand.
	Mem m8() const { return _BaseVarMem(*this, 1); }

	//! @overload.
	Mem m8(sysint_t disp) const { return _BaseVarMem(*this, 1, disp); }

	//! @overload.
	Mem m8(const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) const { return _BaseVarMem(*this, 1, index, shift, disp); }

	//! @brief Cast this variable to 16-bit memory operand.
	Mem m16() const { return _BaseVarMem(*this, 2); }

	//! @overload.
	Mem m16(sysint_t disp) const { return _BaseVarMem(*this, 2, disp); }

	//! @overload.
	Mem m16(const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) const { return _BaseVarMem(*this, 2, index, shift, disp); }

	//! @brief Cast this variable to 32-bit memory operand.
	Mem m32() const { return _BaseVarMem(*this, 4); }

	//! @overload.
	Mem m32(sysint_t disp) const { return _BaseVarMem(*this, 4, disp); }

	//! @overload.
	Mem m32(const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) const { return _BaseVarMem(*this, 4, index, shift, disp); }

	//! @brief Cast this variable to 64-bit memory operand.
	Mem m64() const { return _BaseVarMem(*this, 8); }

	//! @overload.
	Mem m64(sysint_t disp) const { return _BaseVarMem(*this, 8, disp); }

	//! @overload.
	Mem m64(const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) const { return _BaseVarMem(*this, 8, index, shift, disp); }

	//! @brief Cast this variable to 80-bit memory operand (long double).
	Mem m80() const { return _BaseVarMem(*this, 10); }

	//! @overload.
	Mem m80(sysint_t disp) const { return _BaseVarMem(*this, 10, disp); }

	//! @overload.
	Mem m80(const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) const { return _BaseVarMem(*this, 10, index, shift, disp); }

	//! @brief Cast this variable to 128-bit memory operand.
	Mem m128() const { return _BaseVarMem(*this, 16); }

	//! @overload.
	Mem m128(sysint_t disp) const { return _BaseVarMem(*this, 16, disp); }

	//! @overload.
	Mem m128(const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) const { return _BaseVarMem(*this, 16, index, shift, disp); }

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	Var &operator=(const Var &other) { this->_copy(other); return *this; }

	bool operator==(const Var &other) const { return this->_base.id == other._base.id && this->_var.regCode == other._var.regCode; }
	bool operator!=(const Var &other) const { return this->_base.id != other._base.id || this->_var.regCode != other._var.regCode; }
#endif // ASMJIT_NODOC

	// --------------------------------------------------------------------------
	// [Private]
	// --------------------------------------------------------------------------

protected:
	Var(const Var &other, uint32_t regCode, uint32_t size) : Operand(_DontInitialize())
	{
		this->_var.op = kOperandVar;
		this->_var.size = static_cast<uint8_t>(size);
		this->_var.id = other._base.id;
		this->_var.regCode = regCode;
		this->_var.varType = other._var.varType;
	}
};

// ============================================================================
// [AsmJit::X87Var]
// ============================================================================

//! @brief X87 Variable operand.
struct X87Var : public Var
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	X87Var(const _DontInitialize &dontInitialize) : Var(dontInitialize) { }

	X87Var() : Var(_DontInitialize())
	{
		this->_var.op = kOperandVar;
		this->_var.size = 12;
		this->_var.id = kInvalidValue;

		this->_var.regCode = kX86RegTypeX87;
		this->_var.varType = kX86VarTypeX87;
	}

	X87Var(const X87Var &other) : Var(other) { }

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	X87Var &operator=(const X87Var &other) { this->_copy(other); return *this; }

	bool operator==(const X87Var &other) const { return this->_base.id == other._base.id; }
	bool operator!=(const X87Var &other) const { return this->_base.id != other._base.id; }
#endif // ASMJIT_NODOC
};

// ============================================================================
// [AsmJit::GpVar]
// ============================================================================

//! @brief GP variable operand.
struct GpVar : public Var
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	//! @brief Create new uninitialized @c GpVar instance (internal constructor).
	GpVar(const _DontInitialize &dontInitialize) : Var(dontInitialize) { }

	//! @brief Create new uninitialized @c GpVar instance.
	GpVar() : Var(_DontInitialize())
	{
		this->_var.op = kOperandVar;
		this->_var.size = sizeof(sysint_t);
		this->_var.id = kInvalidValue;

		this->_var.regCode = kX86RegTypeGpz;
		this->_var.varType = kX86VarTypeGpz;
	}

	//! @brief Create new @c GpVar instance using @a other.
	//!
	//! Note this will not create a different variable, use @c Compiler::newGpVar()
	//! if you want to do so. This is only copy-constructor that allows to store
	//! the same variable in different places.
	GpVar(const GpVar &other) : Var(other) { }

	// --------------------------------------------------------------------------
	// [GpVar Specific]
	// --------------------------------------------------------------------------

	//! @brief Get whether this variable is general purpose BYTE register.
	bool isGpb() const { return (this->_var.regCode & kRegTypeMask) <= kX86RegTypeGpbHi; }
	//! @brief Get whether this variable is general purpose BYTE.LO register.
	bool isGpbLo() const { return (this->_var.regCode & kRegTypeMask) == kX86RegTypeGpbLo; }
	//! @brief Get whether this variable is general purpose BYTE.HI register.
	bool isGpbHi() const { return (this->_var.regCode & kRegTypeMask) == kX86RegTypeGpbHi; }

	//! @brief Get whether this variable is general purpose WORD register.
	bool isGpw() const { return (this->_var.regCode & kRegTypeMask) == kX86RegTypeGpw; }
	//! @brief Get whether this variable is general purpose DWORD register.
	bool isGpd() const { return (this->_var.regCode & kRegTypeMask) == kX86RegTypeGpd; }
	//! @brief Get whether this variable is general purpose QWORD (only 64-bit) register.
	bool isGpq() const { return (this->_var.regCode & kRegTypeMask) == kX86RegTypeGpq; }

	// --------------------------------------------------------------------------
	// [GpVar Cast]
	// --------------------------------------------------------------------------

	//! @brief Cast this variable to 8-bit (LO) part of variable
	GpVar r8() const { return GpVar(*this, kX86RegTypeGpbLo, 1); }
	//! @brief Cast this variable to 8-bit (LO) part of variable
	GpVar r8Lo() const { return GpVar(*this, kX86RegTypeGpbLo, 1); }
	//! @brief Cast this variable to 8-bit (HI) part of variable
	GpVar r8Hi() const { return GpVar(*this, kX86RegTypeGpbHi, 1); }

	//! @brief Cast this variable to 16-bit part of variable
	GpVar r16() const { return GpVar(*this, kX86RegTypeGpw, 2); }
	//! @brief Cast this variable to 32-bit part of variable
	GpVar r32() const { return GpVar(*this, kX86RegTypeGpd, 4); }
#ifdef ASMJIT_X64
	//! @brief Cast this variable to 64-bit part of variable
	GpVar r64() const { return GpVar(*this, kX86RegTypeGpq, 8); }
#endif // ASMJIT_X64

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	GpVar &operator=(const GpVar &other) { this->_copy(other); return *this; }

	bool operator==(const GpVar &other) const { return this->_base.id == other._base.id && this->_var.regCode == other._var.regCode; }
	bool operator!=(const GpVar &other) const { return this->_base.id != other._base.id || this->_var.regCode != other._var.regCode; }
#endif // ASMJIT_NODOC

	// --------------------------------------------------------------------------
	// [Private]
	// --------------------------------------------------------------------------

protected:
	GpVar(const GpVar &other, uint32_t regCode, uint32_t size) : Var(other, regCode, size) { }
};

// ============================================================================
// [AsmJit::MmVar]
// ============================================================================

//! @brief MM variable operand.
struct MmVar : public Var
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	//! @brief Create new uninitialized @c MmVar instance (internal constructor).
	MmVar(const _DontInitialize &dontInitialize) : Var(dontInitialize) { }

	//! @brief Create new uninitialized @c MmVar instance.
	MmVar() : Var(_DontInitialize())
	{
		this->_var.op = kOperandVar;
		this->_var.size = 8;
		this->_var.id = kInvalidValue;

		this->_var.regCode = kX86RegTypeMm;
		this->_var.varType = kX86VarTypeMm;
	}

	//! @brief Create new @c MmVar instance using @a other.
	//!
	//! Note this will not create a different variable, use @c Compiler::newMmVar()
	//! if you want to do so. This is only copy-constructor that allows to store
	//! the same variable in different places.
	MmVar(const MmVar &other) : Var(other) { }

	// --------------------------------------------------------------------------
	// [MmVar Cast]
	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	MmVar &operator=(const MmVar &other) { this->_copy(other); return *this; }

	bool operator==(const MmVar &other) const { return this->_base.id == other._base.id; }
	bool operator!=(const MmVar &other) const { return this->_base.id != other._base.id; }
#endif // ASMJIT_NODOC
};

// ============================================================================
// [AsmJit::XmmVar]
// ============================================================================

//! @brief XMM Variable operand.
struct XmmVar : public Var
{
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	XmmVar(const _DontInitialize &dontInitialize) : Var(dontInitialize) { }

	XmmVar() : Var(_DontInitialize())
	{
		this->_var.op = kOperandVar;
		this->_var.size = 16;
		this->_var.id = kInvalidValue;

		this->_var.regCode = kX86RegTypeXmm;
		this->_var.varType = kX86VarTypeXmm;
	}

	XmmVar(const XmmVar &other) : Var(other) { }

	// --------------------------------------------------------------------------
	// [XmmVar Access]
	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------
	// [Operator Overload]
	// --------------------------------------------------------------------------

#ifndef ASMJIT_NODOC
	XmmVar &operator=(const XmmVar &other) { this->_copy(other); return *this; }

	bool operator==(const XmmVar &other) const { return this->_base.id == other._base.id; }
	bool operator!=(const XmmVar &other) const { return this->_base.id != other._base.id; }
#endif // ASMJIT_NODOC
};

// ============================================================================
// [AsmJit::Mem - [label + displacement]]
// ============================================================================

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr(const Label &label, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr(const Label &label, sysint_t disp = 0) { return ptr(label, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr(const Label &label, sysint_t disp = 0) { return ptr(label, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr(const Label &label, sysint_t disp = 0) { return ptr(label, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr(const Label &label, sysint_t disp = 0) { return ptr(label, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
inline Mem tword_ptr(const Label &label, sysint_t disp = 0) { return ptr(label, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr(const Label &label, sysint_t disp = 0) { return ptr(label, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr(const Label &label, sysint_t disp = 0) { return ptr(label, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr(const Label &label, sysint_t disp = 0) { return ptr(label, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr(const Label &label, sysint_t disp = 0) { return ptr(label, disp, sizeof(sysint_t)); }

// ============================================================================
// [AsmJit::Mem - [label + index << shift + displacement]]
// ============================================================================

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
inline Mem tword_ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr(const Label &label, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, sizeof(sysint_t)); }

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
inline Mem tword_ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr(const Label &label, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr(label, index, shift, disp, sizeof(sysint_t)); }

// ============================================================================
// [AsmJit::Mem - segment[target + displacement]
// ============================================================================

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr_abs(void *target, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr_abs(void *target, sysint_t disp = 0) { return ptr_abs(target, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr_abs(void *target, sysint_t disp = 0) { return ptr_abs(target, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr_abs(void *target, sysint_t disp = 0) { return ptr_abs(target, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr_abs(void *target, sysint_t disp = 0) { return ptr_abs(target, disp, kSizeQWord); }
//! @brief Create a tword pointer operand (used for 80-bit floating points).
inline Mem tword_ptr_abs(void *target, sysint_t disp = 0) { return ptr_abs(target, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr_abs(void *target, sysint_t disp = 0) { return ptr_abs(target, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr_abs(void *target, sysint_t disp = 0) { return ptr_abs(target, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr_abs(void *target, sysint_t disp = 0) { return ptr_abs(target, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr_abs(void *target, sysint_t disp = 0) { return ptr_abs(target, disp, sizeof(sysint_t)); }

// ============================================================================
// [AsmJit::Mem - segment[target + index << shift + displacement]
// ============================================================================

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
inline Mem tword_ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr_abs(void *target, const GpReg &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, sizeof(sysint_t)); }

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
inline Mem tword_ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr_abs(void *target, const GpVar &index, uint32_t shift, sysint_t disp = 0) { return ptr_abs(target, index, shift, disp, sizeof(sysint_t)); }

// ============================================================================
// [AsmJit::Mem - ptr[base + displacement]]
// ============================================================================

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr(const GpReg &base, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr(const GpReg &base, sysint_t disp = 0) { return ptr(base, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr(const GpReg &base, sysint_t disp = 0) { return ptr(base, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr(const GpReg &base, sysint_t disp = 0) { return ptr(base, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr(const GpReg &base, sysint_t disp = 0) { return ptr(base, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
inline Mem tword_ptr(const GpReg &base, sysint_t disp = 0) { return ptr(base, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr(const GpReg &base, sysint_t disp = 0) { return ptr(base, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr(const GpReg &base, sysint_t disp = 0) { return ptr(base, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr(const GpReg &base, sysint_t disp = 0) { return ptr(base, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr(const GpReg &base, sysint_t disp = 0) { return ptr(base, disp, sizeof(sysint_t)); }

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr(const GpVar &base, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr(const GpVar &base, sysint_t disp = 0) { return ptr(base, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr(const GpVar &base, sysint_t disp = 0) { return ptr(base, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr(const GpVar &base, sysint_t disp = 0) { return ptr(base, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr(const GpVar &base, sysint_t disp = 0) { return ptr(base, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
inline Mem tword_ptr(const GpVar &base, sysint_t disp = 0) { return ptr(base, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr(const GpVar &base, sysint_t disp = 0) { return ptr(base, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr(const GpVar &base, sysint_t disp = 0) { return ptr(base, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr(const GpVar &base, sysint_t disp = 0) { return ptr(base, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr(const GpVar &base, sysint_t disp = 0) { return ptr(base, disp, sizeof(sysint_t)); }

// ============================================================================
// [AsmJit::Mem - ptr[base + (index << shift) + displacement]]
// ============================================================================

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
inline Mem tword_ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr(const GpReg &base, const GpReg &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, sizeof(sysint_t)); }

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0, uint32_t size = 0);
//! @brief Create a byte pointer operand.
inline Mem byte_ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
inline Mem word_ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
inline Mem dword_ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
inline Mem qword_ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
inline Mem tword_ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! @brief Create a dqword pointer operand.
inline Mem dqword_ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeDQWord); }
//! @brief Create a mmword pointer operand.
inline Mem mmword_ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a xmmword pointer operand.
inline Mem xmmword_ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, kSizeDQWord); }
//! @brief Create an intptr_t pointer operand.
inline Mem sysint_ptr(const GpVar &base, const GpVar &index, uint32_t shift = 0, sysint_t disp = 0) { return ptr(base, index, shift, disp, sizeof(sysint_t)); }

// ============================================================================
// [AsmJit::Macros]
// ============================================================================

//! @brief Create Shuffle Constant for MMX/SSE shuffle instrutions.
//! @param z First component position, number at interval [0, 3] inclusive.
//! @param x Second component position, number at interval [0, 3] inclusive.
//! @param y Third component position, number at interval [0, 3] inclusive.
//! @param w Fourth component position, number at interval [0, 3] inclusive.
//!
//! Shuffle constants can be used to make immediate value for these intrinsics:
//! - @ref X86Assembler::pshufw()
//! - @ref X86Assembler::pshufd()
//! - @ref X86Assembler::pshufhw()
//! - @ref X86Assembler::pshuflw()
//! - @ref X86Assembler::shufps()
inline uint8_t mm_shuffle(uint8_t z, uint8_t y, uint8_t x, uint8_t w) { return (z << 6) | (y << 4) | (x << 2) | w; }

//! @}

} // AsmJit namespace
