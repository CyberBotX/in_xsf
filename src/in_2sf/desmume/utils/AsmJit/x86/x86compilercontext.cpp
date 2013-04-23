// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../core/intutil.h"
#include "../core/stringutil.h"

#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86compilercontext.h"
#include "../x86/x86compilerfunc.h"
#include "../x86/x86compileritem.h"
#include "../x86/x86util.h"

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

// ============================================================================
// [AsmJit::CompilerContext - Construction / Destruction]
// ============================================================================

X86CompilerContext::X86CompilerContext(X86Compiler *x86Compiler) : CompilerContext(x86Compiler)
{
	this->_state = &_x86State;

	this->_clear();
	this->_emitComments = !!x86Compiler->getLogger();
}

X86CompilerContext::~X86CompilerContext()
{
}

// ============================================================================
// [AsmJit::CompilerContext - Clear]
// ============================================================================

void X86CompilerContext::_clear()
{
	this->_zoneMemory.clear();
	this->_func = nullptr;

	this->_start = nullptr;
	this->_stop = nullptr;

	this->_x86State.clear();
	this->_active = nullptr;

	this->_forwardJumps = nullptr;

	this->_currentOffset = 0;
	this->_isUnreachable = 0;

	this->_modifiedGpRegisters = 0;
	this->_modifiedMmRegisters = 0;
	this->_modifiedXmmRegisters = 0;

	this->_allocableEBP = false;

	this->_adjustESP = 0;

	this->_argumentsBaseReg = kRegIndexInvalid; // Used by patcher.
	this->_argumentsBaseOffset = 0; // Used by patcher.
	this->_argumentsActualDisp = 0; // Used by translate().

	this->_variablesBaseReg = kRegIndexInvalid; // Used by patcher.
	this->_variablesBaseOffset = 0; // Used by patcher.
	this->_variablesActualDisp = 0; // Used by translate()

	this->_memUsed = nullptr;
	this->_memFree = nullptr;

	this->_mem4BlocksCount = 0;
	this->_mem8BlocksCount = 0;
	this->_mem16BlocksCount = 0;

	this->_memBytesTotal = 0;

	this->_backCode.clear();
	this->_backPos = 0;
}

// ============================================================================
// [AsmJit::CompilerContext - Construction / Destruction]
// ============================================================================

void X86CompilerContext::allocVar(X86CompilerVar *var, uint32_t regMask, uint32_t vflags)
{
	switch (var->getType())
	{
		case kX86VarTypeGpd:
#ifdef ASMJIT_X64
		case kX86VarTypeGpq:
#endif // ASMJIT_X64
			this->allocGpVar(var, regMask, vflags);
			break;

		case kX86VarTypeX87:
		case kX86VarTypeX87SS:
		case kX86VarTypeX87SD:
			// TODO: X87 Support.
			break;

		case kX86VarTypeMm:
			this->allocMmVar(var, regMask, vflags);
			break;

		case kX86VarTypeXmm:
		case kX86VarTypeXmmSS:
		case kX86VarTypeXmmPS:
		case kX86VarTypeXmmSD:
		case kX86VarTypeXmmPD:
			this->allocXmmVar(var, regMask, vflags);
	}

	this->_postAlloc(var, vflags);
}

void X86CompilerContext::saveVar(X86CompilerVar *var)
{
	switch (var->getType())
	{
		case kX86VarTypeGpd:
#ifdef ASMJIT_X64
		case kX86VarTypeGpq:
#endif // ASMJIT_X64
			this->saveGpVar(var);
			break;

		case kX86VarTypeX87:
		case kX86VarTypeX87SS:
		case kX86VarTypeX87SD:
			// TODO: X87 Support.
			break;

		case kX86VarTypeMm:
			this->saveMmVar(var);
			break;

		case kX86VarTypeXmm:
		case kX86VarTypeXmmSS:
		case kX86VarTypeXmmPS:
		case kX86VarTypeXmmSD:
		case kX86VarTypeXmmPD:
			this->saveXmmVar(var);
	}
}

void X86CompilerContext::spillVar(X86CompilerVar *var)
{
	switch (var->getType())
	{
		case kX86VarTypeGpd:
#ifdef ASMJIT_X64
		case kX86VarTypeGpq:
#endif // ASMJIT_X64
			this->spillGpVar(var);
			break;

		case kX86VarTypeX87:
		case kX86VarTypeX87SS:
		case kX86VarTypeX87SD:
			// TODO: X87 Support.
			break;

		case kX86VarTypeMm:
			this->spillMmVar(var);
			break;

		case kX86VarTypeXmm:
		case kX86VarTypeXmmSS:
		case kX86VarTypeXmmPS:
		case kX86VarTypeXmmSD:
		case kX86VarTypeXmmPD:
			this->spillXmmVar(var);
	}
}

void X86CompilerContext::unuseVar(X86CompilerVar *var, uint32_t toState)
{
	ASMJIT_ASSERT(toState != kVarStateReg);

	if (var->state == kVarStateReg)
	{
		uint32_t regIndex = var->regIndex;
		switch (var->getType())
		{
			case kX86VarTypeGpd:
#ifdef ASMJIT_X64
			case kX86VarTypeGpq:
#endif // ASMJIT_X64
				this->_x86State.gp[regIndex] = nullptr;
				this->_freedGpRegister(regIndex);
				break;

			case kX86VarTypeX87:
			case kX86VarTypeX87SS:
			case kX86VarTypeX87SD:
				// TODO: X87 Support.
				break;

			case kX86VarTypeMm:
				this->_x86State.mm[regIndex] = nullptr;
				this->_freedMmRegister(regIndex);
				break;

			case kX86VarTypeXmm:
			case kX86VarTypeXmmSS:
			case kX86VarTypeXmmPS:
			case kX86VarTypeXmmSD:
			case kX86VarTypeXmmPD:
				this->_x86State.xmm[regIndex] = nullptr;
				this->_freedXmmRegister(regIndex);
		}
	}

	var->state = toState;
	var->changed = false;
	var->regIndex = kRegIndexInvalid;
}

void X86CompilerContext::allocGpVar(X86CompilerVar *var, uint32_t regMask, uint32_t vflags)
{
	uint32_t fullMask = IntUtil::maskUpToIndex(kX86RegNumGp) & ~IntUtil::maskFromIndex(kX86RegIndexEsp);
	if (!this->_allocableEBP)
		fullMask &= ~IntUtil::maskFromIndex(kX86RegIndexEbp);

	// Fix the regMask (0 or full bit-array means that any register may be used).
	if (!regMask)
		regMask = 0xFFFFFFFF;
	regMask &= fullMask;

	// Working variables.
	uint32_t i;
	uint32_t mask;

	// Last register code (aka home).
	uint32_t home = var->homeRegisterIndex;
	// New register code.
	uint32_t idx = kRegIndexInvalid;

	// Preserved GP variables.
	uint32_t preservedGP = var->funcScope->getDecl()->getGpPreservedMask();

	// Spill candidate.
	X86CompilerVar *spillCandidate = nullptr;

	// Whether to alloc the non-preserved variables first.
	bool nonPreservedFirst = true;

	if (getFunc()->isCaller())
		nonPreservedFirst = !var->funcCall || var->funcCall->getOffset() >= var->lastItem->getOffset();

	// --------------------------------------------------------------------------
	// [Already Allocated]
	// --------------------------------------------------------------------------

	// Go away if variable is already allocated.
	if (var->state == kVarStateReg)
	{
		uint32_t oldIndex = var->regIndex;

		// Already allocated in the right register.
		if (IntUtil::maskFromIndex(oldIndex) & regMask)
			return;

		// Try to find unallocated register first.
		mask = regMask & ~_x86State.usedGP;
		if (mask)
			idx = IntUtil::findFirstBit(nonPreservedFirst && (mask & ~preservedGP) ? mask & ~preservedGP : mask);
		// Then find the allocated and exchange later.
		else
			idx = IntUtil::findFirstBit(regMask & _x86State.usedGP);
		ASMJIT_ASSERT(idx != kRegIndexInvalid);

		X86CompilerVar *other = this->_x86State.gp[idx];
		this->emitExchangeVar(var, idx, vflags, other);

		this->_x86State.gp[oldIndex] = other;
		this->_x86State.gp[idx] = var;

		if (other)
			other->regIndex = oldIndex;
		else
			this->_freedGpRegister(oldIndex);

		// Update X86CompilerVar.
		var->state = kVarStateReg;
		var->regIndex = idx;
		var->homeRegisterIndex = idx;

		this->_allocatedGpRegister(idx);
		return;
	}

	// --------------------------------------------------------------------------
	// [Find Unused GP]
	// --------------------------------------------------------------------------

	// Home register code.
	if (idx == kRegIndexInvalid && home != kRegIndexInvalid && (regMask & IntUtil::maskFromIndex(home)) && !(_x86State.usedGP & IntUtil::maskFromIndex(home)))
	{
		idx = home;
		goto _Alloc;
	}

	// We start from 1, because EAX/RAX register is sometimes explicitly
	// needed. So we trying to prevent reallocation in near future.
	if (idx == kRegIndexInvalid)
	{
		for (i = 1, mask = (1 << i); i < kX86RegNumGp; ++i, mask <<= 1)
		{
			if ((regMask & mask) && !(_x86State.usedGP & mask))
			{
				// Convenience to alloc non-preserved first or non-preserved last.
				if (nonPreservedFirst)
				{
					if (idx != kRegIndexInvalid && (preservedGP & mask))
						continue;

					idx = i;
					// If current register is preserved, we should try to find different
					// one that is not. This can save one push / pop in prolog / epilog.
					if (!(preservedGP & mask))
						break;
				}
				else
				{
					if (idx != kRegIndexInvalid && !(preservedGP & mask))
						continue;

					idx = i;
					// The opposite.
					if (preservedGP & mask)
						break;
				}
			}
		}
	}

	// If not found, try EAX/RAX.
	if (idx == kRegIndexInvalid && (regMask & IntUtil::maskFromIndex(kX86RegIndexEax)) && !(_x86State.usedGP & IntUtil::maskFromIndex(kX86RegIndexEax)))
	{
		idx = kX86RegIndexEax;
		goto _Alloc;
	}

	// If regMask contains restricted registers which may be used then everything
	// is handled inside this block.
	if (idx == kRegIndexInvalid && regMask != fullMask)
	{
		// Try to find unallocated register first.
		mask = regMask & ~this->_x86State.usedGP;
		if (mask)
		{
			idx = IntUtil::findFirstBit(nonPreservedFirst && (mask & ~preservedGP) ? (mask & ~preservedGP) : mask);
			ASMJIT_ASSERT(idx != kRegIndexInvalid);
		}
		// Then find the allocated and spill later.
		else
		{
			idx = IntUtil::findFirstBit(regMask & _x86State.usedGP);
			ASMJIT_ASSERT(idx != kRegIndexInvalid);

			// Spill register we need.
			spillCandidate = this->_x86State.gp[idx];

			// Jump to spill part of allocation.
			goto L_Spill;
		}
	}

	// --------------------------------------------------------------------------
	// [Spill]
	// --------------------------------------------------------------------------

	// If register is still not found, spill other variable.
	if (idx == kRegIndexInvalid)
	{
		if (!spillCandidate)
			spillCandidate = this->_getSpillCandidateGP();

		// Spill candidate not found?
		if (!spillCandidate)
		{
			this->_compiler->setError(kErrorNoRegisters);
			return;
		}

	L_Spill:
		// Prevented variables can't be spilled. _getSpillCandidate() never returns
		// prevented variables, but when jumping to L_Spill it could happen.
		if (spillCandidate->workOffset == this->_currentOffset)
		{
			this->_compiler->setError(kErrorOverlappedRegisters);
			return;
		}

		idx = spillCandidate->regIndex;
		this->spillGpVar(spillCandidate);
	}

	// --------------------------------------------------------------------------
	// [Alloc]
	// --------------------------------------------------------------------------

_Alloc:
	if (var->state == kVarStateMem && (vflags & kVarAllocRead))
		this->emitLoadVar(var, idx);

	// Update X86CompilerVar.
	var->state = kVarStateReg;
	var->regIndex = idx;
	var->homeRegisterIndex = idx;

	// Update CompilerState.
	this->_allocatedVariable(var);
}

void X86CompilerContext::saveGpVar(X86CompilerVar *var)
{
	// Can't save variable that isn't allocated.
	ASMJIT_ASSERT(var->state == kVarStateReg);
	ASMJIT_ASSERT(var->regIndex != kRegIndexInvalid);

	uint32_t idx = var->regIndex;
	this->emitSaveVar(var, idx);

	// Update X86CompilerVar.
	var->changed = false;
}

void X86CompilerContext::spillGpVar(X86CompilerVar *var)
{
	// Can't spill variable that isn't allocated.
	ASMJIT_ASSERT(var->state == kVarStateReg);
	ASMJIT_ASSERT(var->regIndex != kRegIndexInvalid);

	uint32_t idx = var->regIndex;

	if (var->changed)
		this->emitSaveVar(var, idx);

	// Update X86CompilerVar.
	var->regIndex = kRegIndexInvalid;
	var->state = kVarStateMem;
	var->changed = false;

	// Update CompilerState.
	this->_x86State.gp[idx] = nullptr;
	this->_freedGpRegister(idx);
}

void X86CompilerContext::allocMmVar(X86CompilerVar *var, uint32_t regMask, uint32_t vflags)
{
	// Fix the regMask (0 or full bit-array means that any register may be used).
	if (!regMask)
		regMask = IntUtil::maskUpToIndex(kX86RegNumMm);
	regMask &= IntUtil::maskUpToIndex(kX86RegNumMm);

	// Working variables.
	uint32_t i;
	uint32_t mask;

	// Last register code (aka home).
	uint32_t home = var->homeRegisterIndex;
	// New register code.
	uint32_t idx = kRegIndexInvalid;

	// Preserved MM variables.
	//
	// NOTE: Currently MM variables are not preserved and there is no calling
	// convention known to me that does that. But on the other side it's possible
	// to write such calling convention.
	uint32_t preservedMM = var->funcScope->getDecl()->getMmPreservedMask();

	// Spill candidate.
	X86CompilerVar *spillCandidate = nullptr;

	// Whether to alloc non-preserved first or last.
	bool nonPreservedFirst = true;
	if (this->getFunc()->isCaller())
		nonPreservedFirst = !var->funcCall || var->funcCall->getOffset() >= var->lastItem->getOffset();

	// --------------------------------------------------------------------------
	// [Already Allocated]
	// --------------------------------------------------------------------------

	// Go away if variable is already allocated.
	if (var->state == kVarStateReg)
	{
		uint32_t oldIndex = var->regIndex;

		// Already allocated in the right register.
		if (IntUtil::maskFromIndex(oldIndex) & regMask)
			return;

		// Try to find unallocated register first.
		mask = regMask & ~this->_x86State.usedMM;
		if (mask)
			idx = IntUtil::findFirstBit(nonPreservedFirst && (mask & ~preservedMM) ? mask & ~preservedMM : mask);
		// Then find the allocated and exchange later.
		else
			idx = IntUtil::findFirstBit(regMask & this->_x86State.usedMM);
		ASMJIT_ASSERT(idx != kRegIndexInvalid);

		X86CompilerVar *other = this->_x86State.mm[idx];
		if (other)
			this->spillMmVar(other);

		this->emitMoveVar(var, idx, vflags);
		this->_freedMmRegister(oldIndex);
		this->_x86State.mm[idx] = var;

		// Update X86CompilerVar.
		var->state = kVarStateReg;
		var->regIndex = idx;
		var->homeRegisterIndex = idx;

		this->_allocatedMmRegister(idx);
		return;
	}

	// --------------------------------------------------------------------------
	// [Find Unused MM]
	// --------------------------------------------------------------------------

	// If regMask contains restricted registers which may be used then everything
	// is handled in this block.
	if (regMask != IntUtil::maskUpToIndex(kX86RegNumMm))
	{
		// Try to find unallocated register first.
		mask = regMask & ~this->_x86State.usedMM;
		if (mask)
		{
			idx = IntUtil::findFirstBit(nonPreservedFirst && (mask & ~preservedMM) ? mask & ~preservedMM : mask);
			ASMJIT_ASSERT(idx != kRegIndexInvalid);
		}
		// Then find the allocated and spill later.
		else
		{
			idx = IntUtil::findFirstBit(regMask & this->_x86State.usedMM);
			ASMJIT_ASSERT(idx != kRegIndexInvalid);

			// Spill register we need.
			spillCandidate = this->_x86State.mm[idx];

			// Jump to spill part of allocation.
			goto L_Spill;
		}
	}

	// Home register code.
	if (idx == kRegIndexInvalid && home != kRegIndexInvalid)
	{
		if (!(_x86State.usedMM & (1U << home)))
			idx = home;
	}

	if (idx == kRegIndexInvalid)
	{
		for (i = 0, mask = (1 << i); i < kX86RegNumMm; ++i, mask <<= 1)
		{
			if (!(this->_x86State.usedMM & mask))
			{
				// Convenience to alloc non-preserved first or non-preserved last.
				if (nonPreservedFirst)
				{
					if (idx != kRegIndexInvalid && (preservedMM & mask))
						continue;
					idx = i;
					// If current register is preserved, we should try to find different
					// one that is not. This can save one push / pop in prolog / epilog.
					if (!(preservedMM & mask))
						break;
				}
				else
				{
					if (idx != kRegIndexInvalid && !(preservedMM & mask))
						continue;
					idx = i;
					// The opposite.
					if (preservedMM & mask)
						break;
				}
			}
		}
	}

	// --------------------------------------------------------------------------
	// [Spill]
	// --------------------------------------------------------------------------

	// If register is still not found, spill other variable.
	if (idx == kRegIndexInvalid)
	{
		if (!spillCandidate)
			spillCandidate = this->_getSpillCandidateMM();

		// Spill candidate not found?
		if (!spillCandidate)
		{
			this->_compiler->setError(kErrorNoRegisters);
			return;
		}

	L_Spill:

		// Prevented variables can't be spilled. _getSpillCandidate() never returns
		// prevented variables, but when jumping to L_spill it can happen.
		if (spillCandidate->workOffset == this->_currentOffset)
		{
			this->_compiler->setError(kErrorOverlappedRegisters);
			return;
		}

		idx = spillCandidate->regIndex;
		this->spillMmVar(spillCandidate);
	}

	// --------------------------------------------------------------------------
	// [Alloc]
	// --------------------------------------------------------------------------

	if (var->state == kVarStateMem && (vflags & kVarAllocRead))
		this->emitLoadVar(var, idx);

	// Update X86CompilerVar.
	var->state = kVarStateReg;
	var->regIndex = idx;
	var->homeRegisterIndex = idx;

	// Update CompilerState.
	this->_allocatedVariable(var);
}

void X86CompilerContext::saveMmVar(X86CompilerVar *var)
{
	// Can't save variable that isn't allocated.
	ASMJIT_ASSERT(var->state == kVarStateReg);
	ASMJIT_ASSERT(var->regIndex != kRegIndexInvalid);

	uint32_t idx = var->regIndex;
	this->emitSaveVar(var, idx);

	// Update X86CompilerVar.
	var->changed = false;
}

void X86CompilerContext::spillMmVar(X86CompilerVar *var)
{
	// Can't spill variable that isn't allocated.
	ASMJIT_ASSERT(var->state == kVarStateReg);
	ASMJIT_ASSERT(var->regIndex != kRegIndexInvalid);

	uint32_t idx = var->regIndex;

	if (var->changed)
		this->emitSaveVar(var, idx);

	// Update X86CompilerVar.
	var->regIndex = kRegIndexInvalid;
	var->state = kVarStateMem;
	var->changed = false;

	// Update CompilerState.
	this->_x86State.mm[idx] = nullptr;
	this->_freedMmRegister(idx);
}

void X86CompilerContext::allocXmmVar(X86CompilerVar *var, uint32_t regMask, uint32_t vflags)
{
	// Fix the regMask (0 or full bit-array means that any register may be used).
	if (!regMask)
		regMask = IntUtil::maskUpToIndex(kX86RegNumXmm);
	regMask &= IntUtil::maskUpToIndex(kX86RegNumXmm);

	// Working variables.
	uint32_t i;
	uint32_t mask;

	// Last register code (aka home).
	uint32_t home = var->homeRegisterIndex;
	// New register code.
	uint32_t idx = kRegIndexInvalid;

	// Preserved XMM variables.
	uint32_t preservedXMM = var->funcScope->getDecl()->getXmmPreservedMask();

	// Spill candidate.
	X86CompilerVar *spillCandidate = nullptr;

	// Whether to alloc non-preserved first or last.
	bool nonPreservedFirst = true;

	if (this->getFunc()->isCaller())
		nonPreservedFirst = !var->funcCall || var->funcCall->getOffset() >= var->lastItem->getOffset();

	// --------------------------------------------------------------------------
	// [Already Allocated]
	// --------------------------------------------------------------------------

	// Go away if variable is already allocated.
	if (var->state == kVarStateReg)
	{
		uint32_t oldIndex = var->regIndex;

		// Already allocated in the right register.
		if (IntUtil::maskFromIndex(oldIndex) & regMask)
			return;

		// Try to find unallocated register first.
		mask = regMask & ~this->_x86State.usedXMM;
		if (mask)
			idx = IntUtil::findFirstBit(nonPreservedFirst && (mask & ~preservedXMM) ? mask & ~preservedXMM : mask);
		// Then find the allocated and exchange later.
		else
			idx = IntUtil::findFirstBit(regMask & this->_x86State.usedXMM);
		ASMJIT_ASSERT(idx != kRegIndexInvalid);

		X86CompilerVar *other = this->_x86State.xmm[idx];
		if (other)
			this->spillXmmVar(other);

		this->emitMoveVar(var, idx, vflags);
		this->_freedXmmRegister(oldIndex);
		this->_x86State.xmm[idx] = var;

		// Update X86CompilerVar.
		var->state = kVarStateReg;
		var->regIndex = idx;
		var->homeRegisterIndex = idx;

		this->_allocatedXmmRegister(idx);
		return;
	}

	// --------------------------------------------------------------------------
	// [Find Unused XMM]
	// --------------------------------------------------------------------------

	// If regMask contains restricted registers which may be used then everything
	// is handled in this block.
	if (regMask != IntUtil::maskUpToIndex(kX86RegNumXmm))
	{
		// Try to find unallocated register first.
		mask = regMask & ~this->_x86State.usedXMM;
		if (mask)
		{
			idx = IntUtil::findFirstBit(nonPreservedFirst && (mask & ~preservedXMM) ? mask & ~preservedXMM : mask);
			ASMJIT_ASSERT(idx != kRegIndexInvalid);
		}
		// Then find the allocated and spill later.
		else
		{
			idx = IntUtil::findFirstBit(regMask & this->_x86State.usedXMM);
			ASMJIT_ASSERT(idx != kRegIndexInvalid);

			// Spill register we need.
			spillCandidate = this->_x86State.xmm[idx];

			// Jump to spill part of allocation.
			goto L_Spill;
		}
	}

	// Home register code.
	if (idx == kRegIndexInvalid && home != kRegIndexInvalid)
	{
		if (!(_x86State.usedXMM & (1U << home)))
			idx = home;
	}

	if (idx == kRegIndexInvalid)
	{
		for (i = 0, mask = (1 << i); i < kX86RegNumXmm; ++i, mask <<= 1)
		{
			if (!(this->_x86State.usedXMM & mask))
			{
				// Convenience to alloc non-preserved first or non-preserved last.
				if (nonPreservedFirst)
				{
					if (idx != kRegIndexInvalid && (preservedXMM & mask))
						continue;
					idx = i;
					// If current register is preserved, we should try to find different
					// one that is not. This can save one push / pop in prolog / epilog.
					if (!(preservedXMM & mask))
						break;
				}
				else
				{
					if (idx != kRegIndexInvalid && !(preservedXMM & mask))
						continue;
					idx = i;
					// The opposite.
					if (preservedXMM & mask)
						break;
				}
			}
		}
	}

	// --------------------------------------------------------------------------
	// [Spill]
	// --------------------------------------------------------------------------

	// If register is still not found, spill other variable.
	if (idx == kRegIndexInvalid)
	{
		if (!spillCandidate)
			spillCandidate = this->_getSpillCandidateXMM();

		// Spill candidate not found?
		if (!spillCandidate)
		{
			this->_compiler->setError(kErrorNoRegisters);
			return;
		}

	L_Spill:

		// Prevented variables can't be spilled. _getSpillCandidate() never returns
		// prevented variables, but when jumping to L_spill it can happen.
		if (spillCandidate->workOffset == this->_currentOffset)
		{
			this->_compiler->setError(kErrorOverlappedRegisters);
			return;
		}

		idx = spillCandidate->regIndex;
		this->spillXmmVar(spillCandidate);
	}

	// --------------------------------------------------------------------------
	// [Alloc]
	// --------------------------------------------------------------------------

	if (var->state == kVarStateMem && (vflags & kVarAllocRead))
		this->emitLoadVar(var, idx);

	// Update X86CompilerVar.
	var->state = kVarStateReg;
	var->regIndex = idx;
	var->homeRegisterIndex = idx;

	// Update CompilerState.
	this->_allocatedVariable(var);
}

void X86CompilerContext::saveXmmVar(X86CompilerVar *var)
{
	// Can't save variable that isn't allocated.
	ASMJIT_ASSERT(var->state == kVarStateReg);
	ASMJIT_ASSERT(var->regIndex != kRegIndexInvalid);

	uint32_t idx = var->regIndex;
	this->emitSaveVar(var, idx);

	// Update X86CompilerVar.
	var->changed = false;
}

void X86CompilerContext::spillXmmVar(X86CompilerVar *var)
{
	// Can't spill variable that isn't allocated.
	ASMJIT_ASSERT(var->state == kVarStateReg);
	ASMJIT_ASSERT(var->regIndex != kRegIndexInvalid);

	uint32_t idx = var->regIndex;

	if (var->changed)
		this->emitSaveVar(var, idx);

	// Update CompilerVar.
	var->regIndex = kRegIndexInvalid;
	var->state = kVarStateMem;
	var->changed = false;

	// Update CompilerState.
	this->_x86State.xmm[idx] = nullptr;
	this->_freedXmmRegister(idx);
}

void X86CompilerContext::emitLoadVar(X86CompilerVar *var, uint32_t regIndex)
{
	X86Compiler *x86Compiler = this->getCompiler();
	Mem m = this->_getVarMem(var);

	switch (var->getType())
	{
		case kX86VarTypeGpd:
			x86Compiler->emit(kX86InstMov, gpd(regIndex), m);
			if (this->_emitComments)
				goto _AddComment;
			break;

#ifdef ASMJIT_X64
		case kX86VarTypeGpq:
			x86Compiler->emit(kX86InstMov, gpq(regIndex), m);
			if (this->_emitComments)
				goto _AddComment;
			break;
#endif // ASMJIT_X64

		case kX86VarTypeX87:
		case kX86VarTypeX87SS:
		case kX86VarTypeX87SD:
			// TODO: X87 Support.
			break;

		case kX86VarTypeMm:
			x86Compiler->emit(kX86InstMovQ, mm(regIndex), m);
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmm:
			x86Compiler->emit(kX86InstMovDQA, xmm(regIndex), m);
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmmSS:
			x86Compiler->emit(kX86InstMovSS, xmm(regIndex), m);
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmmSD:
			x86Compiler->emit(kX86InstMovSD, xmm(regIndex), m);
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmmPS:
			x86Compiler->emit(kX86InstMovAPS, xmm(regIndex), m);
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmmPD:
			x86Compiler->emit(kX86InstMovAPD, xmm(regIndex), m);
			if (this->_emitComments)
				goto _AddComment;
	}
	return;

_AddComment:
	x86Compiler->getCurrentItem()->formatComment("Alloc %s", var->getName());
}

void X86CompilerContext::emitSaveVar(X86CompilerVar *var, uint32_t regIndex)
{
	// Caller must ensure that variable is allocated.
	ASMJIT_ASSERT(regIndex != kRegIndexInvalid);

	X86Compiler *x86Compiler = this->getCompiler();
	Mem m = this->_getVarMem(var);

	switch (var->getType())
	{
		case kX86VarTypeGpd:
			x86Compiler->emit(kX86InstMov, m, gpd(regIndex));
			if (this->_emitComments)
				goto _AddComment;
			break;

#ifdef ASMJIT_X64
		case kX86VarTypeGpq:
			x86Compiler->emit(kX86InstMov, m, gpq(regIndex));
			if (this->_emitComments)
				goto _AddComment;
			break;
#endif // ASMJIT_X64

		case kX86VarTypeX87:
		case kX86VarTypeX87SS:
		case kX86VarTypeX87SD:
			// TODO: X87 Support.
			break;

		case kX86VarTypeMm:
			x86Compiler->emit(kX86InstMovQ, m, mm(regIndex));
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmm:
			x86Compiler->emit(kX86InstMovDQA, m, xmm(regIndex));
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmmSS:
			x86Compiler->emit(kX86InstMovSS, m, xmm(regIndex));
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmmSD:
			x86Compiler->emit(kX86InstMovSD, m, xmm(regIndex));
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmmPS:
			x86Compiler->emit(kX86InstMovAPS, m, xmm(regIndex));
			if (this->_emitComments)
				goto _AddComment;
			break;

		case kX86VarTypeXmmPD:
			x86Compiler->emit(kX86InstMovAPD, m, xmm(regIndex));
			if (this->_emitComments)
				goto _AddComment;
	}
	return;

_AddComment:
	x86Compiler->getCurrentItem()->formatComment("Spill %s", var->getName());
}

void X86CompilerContext::emitMoveVar(X86CompilerVar *var, uint32_t regIndex, uint32_t vflags)
{
	// Caller must ensure that the given variable is allocated.
	ASMJIT_ASSERT(var->regIndex != kRegIndexInvalid);

	X86Compiler *x86Compiler = this->getCompiler();
	if (!(vflags & kVarAllocRead))
		return;

	switch (var->getType())
	{
		case kX86VarTypeGpd:
			x86Compiler->emit(kX86InstMov, gpd(regIndex), gpd(var->regIndex));
			break;

#ifdef ASMJIT_X64
		case kX86VarTypeGpq:
			x86Compiler->emit(kX86InstMov, gpq(regIndex), gpq(var->regIndex));
			break;
#endif // ASMJIT_X64

		case kX86VarTypeX87:
		case kX86VarTypeX87SS:
		case kX86VarTypeX87SD:
			// TODO: X87 Support.
			break;

		case kX86VarTypeMm:
			x86Compiler->emit(kX86InstMovQ, mm(regIndex), mm(var->regIndex));
			break;

		case kX86VarTypeXmm:
			x86Compiler->emit(kX86InstMovDQA, xmm(regIndex), xmm(var->regIndex));
			break;

		case kX86VarTypeXmmSS:
			x86Compiler->emit(kX86InstMovSS, xmm(regIndex), xmm(var->regIndex));
			break;

		case kX86VarTypeXmmSD:
			x86Compiler->emit(kX86InstMovSD, xmm(regIndex), xmm(var->regIndex));
			break;

		case kX86VarTypeXmmPS:
			x86Compiler->emit(kX86InstMovAPS, xmm(regIndex), xmm(var->regIndex));
			break;

		case kX86VarTypeXmmPD:
			x86Compiler->emit(kX86InstMovAPD, xmm(regIndex), xmm(var->regIndex));
	}
}

void X86CompilerContext::emitExchangeVar(X86CompilerVar *var, uint32_t regIndex, uint32_t vflags, X86CompilerVar *other)
{
	// Caller must ensure that the given variable is allocated.
	ASMJIT_ASSERT(var->regIndex != kRegIndexInvalid);

	X86Compiler *x86Compiler = this->getCompiler();

	// If other is not valid then we can just emit MOV (or other similar instruction).
	if (!other)
	{
		this->emitMoveVar(var, regIndex, vflags);
		return;
	}

	// If we need to alloc for write-only operation then we can move other
	// variable away instead of exchanging them.
	if (!(vflags & kVarAllocRead))
	{
		this->emitMoveVar(other, var->regIndex, kVarAllocRead);
		return;
	}

	switch (var->getType())
	{
		case kX86VarTypeGpd:
			x86Compiler->emit(kX86InstXchg, gpd(regIndex), gpd(var->regIndex));
			break;

#ifdef ASMJIT_X64
		case kX86VarTypeGpq:
			x86Compiler->emit(kX86InstXchg, gpq(regIndex), gpq(var->regIndex));
			break;
#endif // ASMJIT_X64

		case kX86VarTypeX87:
		case kX86VarTypeX87SS:
		case kX86VarTypeX87SD:
			// TODO: X87 Support.
			break;

		// NOTE: MM and XMM registers shoudln't be exchanged using this way, it's
		// correct, but instead of using one instruction we need three.

		case kX86VarTypeMm:
		{
			MmReg a = mm(regIndex);
			MmReg b = mm(var->regIndex);

			x86Compiler->emit(kX86InstPXor, a, b);
			x86Compiler->emit(kX86InstPXor, b, a);
			x86Compiler->emit(kX86InstPXor, a, b);
			break;
		}

		case kX86VarTypeXmmSS:
		case kX86VarTypeXmmPS:
		{
			XmmReg a = xmm(regIndex);
			XmmReg b = xmm(var->regIndex);

			x86Compiler->emit(kX86InstXorPS, a, b);
			x86Compiler->emit(kX86InstXorPS, b, a);
			x86Compiler->emit(kX86InstXorPS, a, b);
			break;
		}

		case kX86VarTypeXmmSD:
		case kX86VarTypeXmmPD:
		{
			XmmReg a = xmm(regIndex);
			XmmReg b = xmm(var->regIndex);

			x86Compiler->emit(kX86InstXorPD, a, b);
			x86Compiler->emit(kX86InstXorPD, b, a);
			x86Compiler->emit(kX86InstXorPD, a, b);
			break;
		}

		case kX86VarTypeXmm:
		{
			XmmReg a = xmm(regIndex);
			XmmReg b = xmm(var->regIndex);

			x86Compiler->emit(kX86InstPXor, a, b);
			x86Compiler->emit(kX86InstPXor, b, a);
			x86Compiler->emit(kX86InstPXor, a, b);
		}
	}
}

void X86CompilerContext::_postAlloc(X86CompilerVar *var, uint32_t vflags)
{
	if (vflags & kVarAllocWrite)
		var->changed = true;
}

void X86CompilerContext::_markMemoryUsed(X86CompilerVar *var)
{
	if (var->homeMemoryData)
		return;

	VarMemBlock *mem = this->_allocMemBlock(var->getSize());
	if (!mem)
		return;

	var->homeMemoryData = mem;
}

Mem X86CompilerContext::_getVarMem(X86CompilerVar *var)
{
	Mem m;
	m._mem.id = var->getId();

	if (!var->isMemArgument())
		m._mem.displacement = this->_adjustESP;

	this->_markMemoryUsed(var);
	return m;
}

static int32_t getSpillScore(X86CompilerVar *var, uint32_t currentOffset)
{
	int32_t score = 0;

	ASMJIT_ASSERT(var->lastItem);
	uint32_t lastOffset = var->lastItem->getOffset();

	if (lastOffset >= currentOffset)
		score += static_cast<int32_t>(lastOffset - currentOffset);

	// Each write access decreases probability of spill.
	score -= static_cast<int32_t>(var->regWriteCount) + static_cast<int32_t>(var->regRwCount);
	// Each read-only access increases probability of spill.
	score += static_cast<int32_t>(var->regReadCount);

	// Each memory access increases probability of spill.
	score += static_cast<int32_t>(var->memWriteCount) + static_cast<int32_t>(var->memRwCount);
	score += static_cast<int32_t>(var->memReadCount);

	return score;
}

X86CompilerVar *X86CompilerContext::_getSpillCandidateGP()
{
	return this->_getSpillCandidateGeneric(_x86State.gp, kX86RegNumGp);
}

X86CompilerVar *X86CompilerContext::_getSpillCandidateMM()
{
	return this->_getSpillCandidateGeneric(_x86State.mm, kX86RegNumMm);
}

X86CompilerVar *X86CompilerContext::_getSpillCandidateXMM()
{
	return this->_getSpillCandidateGeneric(_x86State.xmm, kX86RegNumXmm);
}

X86CompilerVar *X86CompilerContext::_getSpillCandidateGeneric(X86CompilerVar **varArray, uint32_t count)
{
	uint32_t i;

	X86CompilerVar *candidate = nullptr;
	uint32_t candidatePriority = 0;
	int32_t candidateScore = 0;

	uint32_t currentOffset = this->_compiler->getCurrentItem()->getOffset();

	for (i = 0; i < count; ++i)
	{
		// Get variable.
		X86CompilerVar *cv = varArray[i];

		// Never spill variables needed for next instruction.
		if (!cv || cv->workOffset == this->_currentOffset)
			continue;

		uint32_t variablePriority = cv->getPriority();
		int32_t variableScore = getSpillScore(cv, currentOffset);

		if (!candidate || variablePriority > candidatePriority || (variablePriority == candidatePriority && variableScore > candidateScore))
		{
			candidate = cv;
			candidatePriority = variablePriority;
			candidateScore = variableScore;
		}
	}

	return candidate;
}

void X86CompilerContext::_addActive(X86CompilerVar *var)
{
	// Never call with variable that is already in active list.
	ASMJIT_ASSERT(!var->nextActive);
	ASMJIT_ASSERT(!var->prevActive);

	if (!this->_active)
	{
		var->nextActive = var;
		var->prevActive = var;

		this->_active = var;
	}
	else
	{
		X86CompilerVar *vlast = static_cast<X86CompilerVar *>(this->_active)->prevActive;

		vlast->nextActive = var;
		static_cast<X86CompilerVar *>(this->_active)->prevActive = var;

		var->nextActive = static_cast<X86CompilerVar *>(this->_active);
		var->prevActive = vlast;
	}
}

void X86CompilerContext::_freeActive(X86CompilerVar *var)
{
	X86CompilerVar *next = var->nextActive;
	X86CompilerVar *prev = var->prevActive;

	if (prev == next)
		this->_active = nullptr;
	else
	{
		if (this->_active == var)
			this->_active = next;

		prev->nextActive = next;
		next->prevActive = prev;
	}

	var->nextActive = nullptr;
	var->prevActive = nullptr;
}

void X86CompilerContext::_freeAllActive()
{
	if (!this->_active)
		return;

	X86CompilerVar *cur = static_cast<X86CompilerVar *>(this->_active);
	for (;;)
	{
		X86CompilerVar *next = cur->nextActive;

		cur->nextActive = nullptr;
		cur->prevActive = nullptr;

		if (next == this->_active)
			break;
	}

	this->_active = nullptr;
}

void X86CompilerContext::_allocatedVariable(X86CompilerVar *var)
{
	uint32_t idx = var->regIndex;

	switch (var->getType())
	{
		case kX86VarTypeGpd:
		case kX86VarTypeGpq:
			this->_x86State.gp[idx] = var;
			this->_allocatedGpRegister(idx);
			break;

		case kX86VarTypeMm:
			this->_x86State.mm[idx] = var;
			this->_allocatedMmRegister(idx);
			break;

		case kX86VarTypeXmm:
		case kX86VarTypeXmmSS:
		case kX86VarTypeXmmPS:
		case kX86VarTypeXmmSD:
		case kX86VarTypeXmmPD:
			this->_x86State.xmm[idx] = var;
			this->_allocatedXmmRegister(idx);
			break;

		default:
			ASMJIT_ASSERT(0);
	}
}

void X86CompilerContext::translateOperands(Operand *operands, uint32_t count)
{
	X86Compiler *x86Compiler = this->getCompiler();
	uint32_t i;

	// Translate variables to registers.
	for (i = 0; i < count; ++i)
	{
		Operand &o = operands[i];

		if (o.isVar())
		{
			X86CompilerVar *cv = x86Compiler->_getVar(o.getId());
			ASMJIT_ASSERT(cv);

			o._reg.op = kOperandReg;
			o._reg.code |= cv->regIndex;
		}
		else if (o.isMem())
		{
			if ((o.getId() & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				// Memory access. We just increment here actual displacement.
				X86CompilerVar *cv = x86Compiler->_getVar(o.getId());
				ASMJIT_ASSERT(cv);

				o._mem.displacement += cv->isMemArgument() ? _argumentsActualDisp : _variablesActualDisp;
				// NOTE: This is not enough, variable position will be patched later
				// by X86CompilerContext::_patchMemoryOperands().
			}
			else if ((o._mem.base & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *cv = x86Compiler->_getVar(o._mem.base);
				ASMJIT_ASSERT(cv);

				o._mem.base = cv->regIndex;
			}

			if ((o._mem.index & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *cv = x86Compiler->_getVar(o._mem.index);
				ASMJIT_ASSERT(cv);

				o._mem.index = cv->regIndex;
			}
		}
	}
}

void X86CompilerContext::addBackwardCode(X86CompilerJmpInst *from)
{
	this->_backCode.append(from);
}

void X86CompilerContext::addForwardJump(X86CompilerJmpInst *inst)
{
	ForwardJumpData *j = reinterpret_cast<ForwardJumpData *>(this->_zoneMemory.alloc(sizeof(ForwardJumpData)));
	if (!j)
	{
		this->_compiler->setError(kErrorNoHeapMemory);
		return;
	}

	j->inst = inst;
	j->state = this->_saveState();
	j->next = this->_forwardJumps;
	this->_forwardJumps = j;
}

X86CompilerState *X86CompilerContext::_saveState()
{
	X86Compiler *x86Compiler = this->getCompiler();

	// Get count of variables stored in memory.
	uint32_t memVarsCount = 0;
	X86CompilerVar *cur = static_cast<X86CompilerVar *>(this->_active);

	if (cur)
	{
		do
		{
			if (cur->state == kVarStateMem)
				++memVarsCount;
			cur = cur->nextActive;
		} while (cur != this->_active);
	}

	// Alloc X86CompilerState structure (using zone allocator) and copy current
	// state into it.
	X86CompilerState *state = x86Compiler->_newState(memVarsCount);
	memcpy(state, &this->_x86State, sizeof(X86CompilerState));

	// Clear changed flags.
	state->changedGP = 0;
	state->changedMM = 0;
	state->changedXMM = 0;

	unsigned i;
	unsigned mask;

	// Save variables stored in REGISTERs and CHANGE flag.
	for (i = 0, mask = 1; i < kX86RegNumGp; ++i, mask <<= 1)
		if (state->gp[i] && state->gp[i]->changed)
			state->changedGP |= mask;

	for (i = 0, mask = 1; i < kX86RegNumMm; ++i, mask <<= 1)
		if (state->mm[i] && state->mm[i]->changed)
			state->changedMM |= mask;

	for (i = 0, mask = 1; i < kX86RegNumXmm; ++i, mask <<= 1)
		if (state->xmm[i] && state->xmm[i]->changed)
			state->changedXMM |= mask;

	// Save variables stored in MEMORY.
	state->memVarsCount = memVarsCount;
	memVarsCount = 0;

	cur = static_cast<X86CompilerVar *>(this->_active);
	if (cur)
	{
		do
		{
			if (cur->state == kVarStateMem)
				state->memVarsData[memVarsCount++] = cur;
			cur = cur->nextActive;
		} while (cur != this->_active);
	}

	// Finished.
	return state;
}

void X86CompilerContext::_assignState(X86CompilerState *state)
{
	memcpy(&this->_x86State, state, sizeof(X86CompilerState));
	this->_x86State.memVarsCount = 0;

	unsigned i, mask;

	// Unuse all variables first.
	X86CompilerVar *cv = static_cast<X86CompilerVar *>(this->_active);
	if (cv)
	{
		do
		{
			cv->state = kVarStateUnused;
			cv = cv->nextActive;
		} while (cv != this->_active);
	}

	// Assign variables stored in memory which are not unused.
	for (i = 0; i < state->memVarsCount; ++i)
		state->memVarsData[i]->state = kVarStateMem;

	// Assign allocated variables.
	for (i = 0, mask = 1; i < kX86RegNumGp; ++i, mask <<= 1)
	{
		if ((cv = this->_x86State.gp[i]))
		{
			cv->state = kVarStateReg;
			cv->regIndex = i;
			cv->changed = !!(this->_x86State.changedGP & mask);
		}
	}

	for (i = 0, mask = 1; i < kX86RegNumMm; ++i, mask <<= 1)
	{
		if ((cv = this->_x86State.mm[i]))
		{
			cv->state = kVarStateReg;
			cv->regIndex = i;
			cv->changed = !!(this->_x86State.changedMM & mask);
		}
	}

	for (i = 0, mask = 1; i < kX86RegNumXmm; ++i, mask <<= 1)
	{
		if ((cv = this->_x86State.xmm[i]))
		{
			cv->state = kVarStateReg;
			cv->regIndex = i;
			cv->changed = !!(this->_x86State.changedXMM & mask);
		}
	}
}

void X86CompilerContext::_restoreState(X86CompilerState *state, uint32_t targetOffset)
{
	X86CompilerState *fromState = &this->_x86State;
	X86CompilerState *toState = state;

	// No change, rare...
	if (fromState == toState)
		return;

	unsigned base;
	unsigned i;

	// --------------------------------------------------------------------------
	// Set target state to all variables. cv->tInt is target state in this func.
	// --------------------------------------------------------------------------

	// UNUSED.
	X86CompilerVar *cv = static_cast<X86CompilerVar *>(this->_active);
	if (cv)
	{
		do
		{
			cv->tInt = kVarStateUnused;
			cv = cv->nextActive;
		} while (cv != this->_active);
	}

	// MEMORY.
	for (i = 0; i < toState->memVarsCount; ++i)
		toState->memVarsData[i]->tInt = kVarStateMem;

	// REGISTER.
	for (i = 0; i < X86CompilerState::kStateRegCount; ++i)
		if ((cv = toState->regs[i]))
			cv->tInt = kVarStateReg;

	// --------------------------------------------------------------------------
	// [GP-Registers Switch]
	// --------------------------------------------------------------------------

	// TODO.
#if 0
	for (i = 0; i < kX86RegNumGp; ++i)
	{
		X86CompilerVar *fromVar = fromState->gp[i];
		X86CompilerVar *toVar = toState->gp[i];

		if (fromVar != toVar)
		{
			if (fromVar)
			{
				if (toVar)
				{
					if (fromState->gp[to
				}
				else
				{
					// It is possible that variable that was saved in state currently not
					// exists (tInt is target scope!).
					if (fromVar->tInt == kVarStateUnused)
						this->unuseVar(fromVar, kVarStateUnused);
					else
						this->spillVar(fromVar);
				}
			}
		}
		else if (fromVar)
		{
			uint32_t mask = IntUtil::maskFromIndex(i);
			// Variables are the same, we just need to compare changed flags.
			if ((fromState->changedGP & mask) && !(toState->changedGP & mask))
				this->saveVar(fromVar);
		}
	}
#endif

	// Spill.
	for (base = 0, i = 0; i < X86CompilerState::kStateRegCount; ++i)
	{
		// Change the base offset (from base offset so the register index can be calculated).
		if (i == X86CompilerState::kStateRegMmBase || i == X86CompilerState::kStateRegXmmBase)
			base = i;

		uint32_t regIndex = i - base;
		X86CompilerVar *fromVar = fromState->regs[i];
		X86CompilerVar *toVar = toState->regs[i];

		if (fromVar != toVar)
		{
			// Spill the register.
			if (fromVar)
			{
				// It is possible that variable that was saved in state currently not
				// exists (tInt is target scope!).
				if (fromVar->tInt == kVarStateUnused)
					this->unuseVar(fromVar, kVarStateUnused);
				else
					this->spillVar(fromVar);
			}
		}
		else if (fromVar)
		{
			// Variables are the same, we just need to compare changed flags.
			uint32_t mask = IntUtil::maskFromIndex(regIndex);

			if ((fromState->changedGP & mask) && !(toState->changedGP & mask))
				this->saveVar(fromVar);
		}
	}

	// Alloc.
	for (base = 0, i = 0; i < X86CompilerState::kStateRegCount; ++i)
	{
		// Change the base offset (from base offset so the register index can be calculated).
		if (i == X86CompilerState::kStateRegMmBase || i == X86CompilerState::kStateRegXmmBase)
			base = i;

		X86CompilerVar *fromVar = fromState->regs[i];
		X86CompilerVar *toVar = toState->regs[i];

		if (fromVar != toVar)
		{
			// Alloc register.
			uint32_t regIndex = i - base;

			if (toVar)
				this->allocVar(toVar, IntUtil::maskFromIndex(regIndex), kVarAllocRead);
		}

		// TODO:
		//if (toVar)
			//toVar->changed = to->changed;
	}

	// --------------------------------------------------------------------------
	// Update used masks.
	// --------------------------------------------------------------------------

	this->_x86State.usedGP = state->usedGP;
	this->_x86State.usedMM = state->usedMM;
	this->_x86State.usedXMM = state->usedXMM;

	// --------------------------------------------------------------------------
	// Update changed masks and cleanup.
	// --------------------------------------------------------------------------

	cv = static_cast<X86CompilerVar *>(this->_active);
	if (cv)
	{
		do
		{
			if (cv->tInt != kVarStateReg)
			{
				cv->state = static_cast<int>(cv->tInt);
				cv->changed = false;
			}

			cv->tInt = 0;
			cv = cv->nextActive;
		} while (cv != this->_active);
	}
}

VarMemBlock *X86CompilerContext::_allocMemBlock(uint32_t size)
{
	ASMJIT_ASSERT(size);

	// First try to find mem blocks.
	VarMemBlock *mem = this->_memFree;
	VarMemBlock *prev = nullptr;

	while (mem)
	{
		VarMemBlock *next = mem->nextFree;

		if (mem->size == size)
		{
			if (prev)
				prev->nextFree = next;
			else
				this->_memFree = next;

			mem->nextFree = nullptr;
			return mem;
		}

		prev = mem;
		mem = next;
	}

	// Never mind, create new.
	mem = reinterpret_cast<VarMemBlock *>(this->_zoneMemory.alloc(sizeof(VarMemBlock)));
	if (!mem)
	{
		this->_compiler->setError(kErrorNoHeapMemory);
		return nullptr;
	}

	mem->offset = 0;
	mem->size = size;

	mem->nextUsed = this->_memUsed;
	mem->nextFree = nullptr;

	this->_memUsed = mem;

	switch (size)
	{
		case 16:
			++this->_mem16BlocksCount;
			break;
		case 8:
			++this->_mem8BlocksCount;
			break;
		case 4:
			++this->_mem4BlocksCount;
	}

	return mem;
}

void X86CompilerContext::_freeMemBlock(VarMemBlock *mem)
{
	// Add mem to free blocks.
	mem->nextFree = this->_memFree;
	this->_memFree = mem;
}

void X86CompilerContext::_allocMemoryOperands()
{
	VarMemBlock *mem;

	// Variables are allocated in this order:
	// 1. 16-byte variables.
	// 2. 8-byte variables.
	// 3. 4-byte variables.
	// 4. All others.

	uint32_t start16 = 0;
	uint32_t start8 = start16 + this->_mem16BlocksCount * 16;
	uint32_t start4 = start8 + this->_mem8BlocksCount * 8;
	uint32_t startX = IntUtil::align<uint32_t>(start4 + this->_mem4BlocksCount * 4, 16);

	for (mem = this->_memUsed; mem; mem = mem->nextUsed)
	{
		uint32_t size = mem->size;
		uint32_t offset;

		switch (size)
		{
			case 16:
				offset = start16;
				start16 += 16;
				break;

			case 8:
				offset = start8;
				start8 += 8;
				break;

			case 4:
				offset = start4;
				start4 += 4;
				break;

			default:
				// Align to 16 bytes if size is 16 or more.
				if (size >= 16)
				{
					size = IntUtil::align(size, 16u);
					startX = IntUtil::align(startX, 16u);
				}

				offset = startX;
				startX += size;
		}

		mem->offset = static_cast<int32_t>(offset);
		this->_memBytesTotal += size;
	}
}

void X86CompilerContext::_patchMemoryOperands(CompilerItem *start, CompilerItem *stop)
{
	CompilerItem *cur;

	for (cur = start; ; cur = cur->getNext())
	{
		if (cur->getType() == kCompilerItemInst)
		{
			Mem *mem = reinterpret_cast<X86CompilerInst *>(cur)->_memOp;

			if (mem && (mem->_mem.id & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *cv = this->getCompiler()->_getVar(mem->_mem.id);
				ASMJIT_ASSERT(cv);

				if (cv->isMemArgument())
				{
					mem->_mem.base = this->_argumentsBaseReg;
					mem->_mem.displacement += cv->homeMemoryOffset;
					mem->_mem.displacement += this->_argumentsBaseOffset;
				}
				else
				{
					VarMemBlock *mb = reinterpret_cast<VarMemBlock *>(cv->homeMemoryData);
					ASMJIT_ASSERT(mb);

					mem->_mem.base = this->_variablesBaseReg;
					mem->_mem.displacement += mb->offset;
					mem->_mem.displacement += this->_variablesBaseOffset;
				}
			}
		}
		if (cur == stop)
			break;
	}
}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"
