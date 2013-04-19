// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../core/intutil.h"
#include "../core/stringutil.h"

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
// [AsmJit::X86CompilerAlign - Construction / Destruction]
// ============================================================================

X86CompilerAlign::X86CompilerAlign(X86Compiler *x86Compiler, uint32_t size) : CompilerAlign(x86Compiler, size)
{
}

X86CompilerAlign::~X86CompilerAlign()
{
}

// ============================================================================
// [AsmJit::X86CompilerAlign - Interface]
// ============================================================================

void X86CompilerAlign::emit(Assembler &a)
{
	X86Assembler &x86Asm = static_cast<X86Assembler &>(a);

	x86Asm.align(_size);
}

// ============================================================================
// [AsmJit::X86CompilerTarget - Construction / Destruction]
// ============================================================================

X86CompilerTarget::X86CompilerTarget(X86Compiler *c, const Label &label) : CompilerTarget(c, label)
{
}

X86CompilerTarget::~X86CompilerTarget()
{
}

// ============================================================================
// [AsmJit::X86CompilerTarget - Interface]
// ============================================================================

static X86CompilerTarget *X86CompilerTarget_removeUnreachableItems(X86CompilerTarget *target)
{
	CompilerItem *prev = target->getPrev();
	CompilerItem *item = target->getNext();

	ASMJIT_ASSERT(prev);
	ASMJIT_ASSERT(item);

	for (;;)
	{
		CompilerItem *next = item->getNext();
		ASMJIT_ASSERT(next);

		if (item->getType() == kCompilerItemTarget)
			break;

		item->_prev = nullptr;
		item->_next = nullptr;
		item->_isUnreachable = true;

		item = next;
	}

	target->_prev = nullptr;
	target->_next = nullptr;
	target->_isTranslated = true;

	prev->_next = item;
	item->_prev = prev;

	return static_cast<X86CompilerTarget *>(item);
}

void X86CompilerTarget::prepare(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	this->_offset = x86Context._currentOffset++;
}

CompilerItem *X86CompilerTarget::translate(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);

	// If this X86CompilerTarget was already translated, it's needed to change
	// the current state and return NULL to tell CompilerContext to process next
	// untranslated item.
	if (this->_isTranslated)
	{
		x86Context._restoreState(this->getState());
		return nullptr;
	}

	if (x86Context._isUnreachable)
	{
		// If the context has "isUnreachable" flag set and there is no state then
		// it means that this code will be never called. This is a problem, because
		// we are unable to assign a state to current location so we can't allocate
		// registers for variables used inside. So instead of doing anything wrong
		// we remove the unreachable code.
		if (!this->_state)
			return X86CompilerTarget_removeUnreachableItems(this);

		// Assign state to the compiler context. 
		x86Context._isUnreachable = 0;
		x86Context._assignState(this->getState());
	}
	else
		this->_state = x86Context._saveState();

	return this->translated();
}

void X86CompilerTarget::emit(Assembler &a)
{
	X86Assembler &x86Asm = static_cast<X86Assembler &>(a);
	x86Asm.bind(this->_label);
}

// ============================================================================
// [AsmJit::X86CompilerHint - Construction / Destruction]
// ============================================================================

X86CompilerHint::X86CompilerHint(X86Compiler *compiler, X86CompilerVar *var, uint32_t hintId, uint32_t hintValue) : CompilerHint(compiler, var, hintId, hintValue)
{
}

X86CompilerHint::~X86CompilerHint()
{
}

// ============================================================================
// [AsmJit::X86CompilerHint - Interface]
// ============================================================================

void X86CompilerHint::prepare(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86CompilerVar *var = this->getVar();

	this->_offset = x86Context._currentOffset;

	// First item (begin of variable scope).
	if (!var->firstItem)
		var->firstItem = this;

	// Last item (end of variable scope).
	CompilerItem *oldLast = var->lastItem;
	var->lastItem = this;

	switch (this->_hintId)
	{
		case kVarHintAlloc:
		case kVarHintSpill:
		case kVarHintSave:
			if (!x86Context._isActive(var))
				x86Context._addActive(var);
			break;

		case kVarHintSaveAndUnuse:
			if (!x86Context._isActive(var))
				x86Context._addActive(var);
		break;

		case kVarHintUnuse:
			if (oldLast)
				oldLast->_tryUnuseVar(var);
	}
}

CompilerItem *X86CompilerHint::translate(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86CompilerVar *var = this->getVar();

	switch (this->_hintId)
	{
		case kVarHintAlloc:
			x86Context.allocVar(var, this->_hintValue, kVarAllocRead);
			break;

		case kVarHintSpill:
			if (var->state == kVarStateReg)
				x86Context.spillVar(var);
			break;

		case kVarHintSave:
		case kVarHintSaveAndUnuse:
			if (var->state == kVarStateReg && var->changed)
			{
				x86Context.emitSaveVar(var, var->regIndex);
				var->changed = false;
			}
			if (this->_hintId == kVarHintSaveAndUnuse)
				goto _Unuse;
			break;

		case kVarHintUnuse:
		_Unuse:
			x86Context.unuseVar(var, kVarStateUnused);
				goto _End;
	}

	x86Context._unuseVarOnEndOfScope(this, var);

_End:
	return this->translated();
}

// ============================================================================
// [AsmJit::X86CompilerHint - Misc]
// ============================================================================

int X86CompilerHint::getMaxSize() const
{
	// Compiler hint is NOP, but it can generate other items which can do 
	// something - in such more items are added into the stream so we don't need
	// to worry about this.
	return 0;
}

// ============================================================================
// [AsmJit::X86CompilerInst - Construction / Destruction]
// ============================================================================

X86CompilerInst::X86CompilerInst(X86Compiler *x86Compiler, uint32_t code, Operand *opData, uint32_t opCount) : CompilerInst(x86Compiler, code, opData, opCount)
{
	this->_memOp = nullptr;
	this->_vars = nullptr;
	this->_variablesCount = 0;

	for (uint32_t i = 0; i < opCount; ++i)
	{
		if (this->_operands[i].isMem())
		{
			this->_memOp = reinterpret_cast<Mem *>(&this->_operands[i]);
			break;
		}
	}

	const X86InstInfo *info = &x86InstInfo[this->_code];

	if (info->isSpecial())
		this->setInstFlag(kX86CompilerInstFlagIsSpecial);
	if (info->isFpu())
		this->setInstFlag(kX86CompilerInstFlagIsFpu);

	if (this->isSpecial())
	{
		// ${SPECIAL_INSTRUCTION_HANDLING_BEGIN}
		switch (this->_code)
		{
			case kX86InstCpuId:
				// Special...
				break;

			case kX86InstCbw:
			case kX86InstCdq:
			case kX86InstCdqe:
			case kX86InstCwd:
			case kX86InstCwde:
			case kX86InstCqo:
				// Special...
				break;

			case kX86InstCmpXCHG:
			case kX86InstCmpXCHG8B:
#ifdef ASMJIT_X64
			case kX86InstCmpXCHG16B:
#endif // ASMJIT_X64
				// Special...
				break;

#ifdef ASMJIT_X86
			case kX86InstDaa:
			case kX86InstDas:
				// Special...
				break;
#endif // ASMJIT_X86

			case kX86InstIMul:
				switch (this->_operandsCount)
				{
					case 2:
						// IMUL dst, src is not special instruction.
						this->clearInstFlag(kX86CompilerInstFlagIsSpecial);
						break;
					case 3:
						// Only IMUL dst_hi, dst_lo, reg/mem is special, all others don't.
						if (!(this->_operands[0].isVar() && this->_operands[1].isVar() && this->_operands[2].isVarMem()))
							this->clearInstFlag(kX86CompilerInstFlagIsSpecial);
				}
				break;
			case kX86InstMul:
			case kX86InstIDiv:
			case kX86InstDiv:
				// Special...
				break;

			case kX86InstMovPtr:
				// Special...
				break;

			case kX86InstLahf:
			case kX86InstSahf:
				// Special...
				break;

			case kX86InstMaskMovQ:
			case kX86InstMaskMovDQU:
				// Special...
				break;

			case kX86InstEnter:
			case kX86InstLeave:
				// Special...
				break;

			case kX86InstRet:
				// Special...
				break;

			case kX86InstMonitor:
			case kX86InstMWait:
				// Special...
				break;

			case kX86InstPop:
			case kX86InstPopAD:
			case kX86InstPopFD:
			case kX86InstPopFQ:
				// Special...
				break;

			case kX86InstPush:
			case kX86InstPushAD:
			case kX86InstPushFD:
			case kX86InstPushFQ:
				// Special...
				break;

			case kX86InstRcl:
			case kX86InstRcr:
			case kX86InstRol:
			case kX86InstRor:
			case kX86InstSal:
			case kX86InstSar:
			case kX86InstShl:
			case kX86InstShr:
				// Rot instruction is special only if last operand is variable (register).
				if (!this->_operands[1].isVar())
					this->clearInstFlag(kX86CompilerInstFlagIsSpecial);
				break;

			case kX86InstShld:
			case kX86InstShrd:
				// Shld/Shrd instruction is special only if last operand is variable (register).
				if (!this->_operands[2].isVar())
					this->clearInstFlag(kX86CompilerInstFlagIsSpecial);
				break;

			case kX86InstRdtsc:
			case kX86InstRdtscP:
				// Special...
				break;

			case kX86InstRepLodSB:
			case kX86InstRepLodSD:
			case kX86InstRepLodSQ:
			case kX86InstRepLodSW:
			case kX86InstRepMovSB:
			case kX86InstRepMovSD:
			case kX86InstRepMovSQ:
			case kX86InstRepMovSW:
			case kX86InstRepStoSB:
			case kX86InstRepStoSD:
			case kX86InstRepStoSQ:
			case kX86InstRepStoSW:
			case kX86InstRepECmpSB:
			case kX86InstRepECmpSD:
			case kX86InstRepECmpSQ:
			case kX86InstRepECmpSW:
			case kX86InstRepEScaSB:
			case kX86InstRepEScaSD:
			case kX86InstRepEScaSQ:
			case kX86InstRepEScaSW:
			case kX86InstRepNECmpSB:
			case kX86InstRepNECmpSD:
			case kX86InstRepNECmpSQ:
			case kX86InstRepNECmpSW:
			case kX86InstRepNEScaSB:
			case kX86InstRepNEScaSD:
			case kX86InstRepNEScaSQ:
			case kX86InstRepNEScaSW:
				// Special...
				break;

			default:
				ASMJIT_ASSERT(0);
		}
		// ${SPECIAL_INSTRUCTION_HANDLING_END}
	}
}

X86CompilerInst::~X86CompilerInst()
{
}

// ============================================================================
// [AsmJit::X86CompilerInst - Interface]
// ============================================================================

void X86CompilerInst::prepare(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = this->getCompiler();

#define __GET_VARIABLE(__vardata__) \
{ \
	X86CompilerVar *_candidate = __vardata__; \
\
	for (var = cur; ; ) \
	{ \
		if (var == this->_vars) \
		{ \
			var = cur++; \
			var->vdata = _candidate; \
			var->vflags = 0; \
			var->regMask = 0xFFFFFFFF; \
			break; \
		} \
\
		--var; \
\
		if (var->vdata == _candidate) \
			break; \
	} \
\
	ASMJIT_ASSERT(var); \
}

	this->_offset = x86Context._currentOffset;

	const X86InstInfo *id = &x86InstInfo[this->_code];

	uint32_t i, len = _operandsCount;
	uint32_t variablesCount = 0;

	for (i = 0; i < len; ++i)
	{
		Operand &o = this->_operands[i];

		if (o.isVar())
		{
			ASMJIT_ASSERT(o.getId() != kInvalidValue);
			X86CompilerVar *vdata = x86Compiler->_getVar(o.getId());
			ASMJIT_ASSERT(vdata);

			if (reinterpret_cast<Var *>(&o)->isGpVar())
			{
				if (reinterpret_cast<GpVar *>(&o)->isGpbLo())
				{
					this->setInstFlag(kX86CompilerInstFlagIsGpbLoUsed);
					++vdata->regGpbLoCount;
				}
				if (reinterpret_cast<GpVar *>(&o)->isGpbHi())
				{
					this->setInstFlag(kX86CompilerInstFlagIsGpbHiUsed);
					++vdata->regGpbHiCount;
				}
			}

			if (vdata->workOffset != this->_offset)
			{
				if (!x86Context._isActive(vdata))
					x86Context._addActive(vdata);

				vdata->workOffset = this->_offset;
				++variablesCount;
			}
		}
		else if (o.isMem())
		{
			if ((o.getId() & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *vdata = x86Compiler->_getVar(o.getId());
				ASMJIT_ASSERT(vdata);

				x86Context._markMemoryUsed(vdata);

				if (vdata->workOffset != this->_offset)
				{
					if (!x86Context._isActive(vdata))
						x86Context._addActive(vdata);

					vdata->workOffset = this->_offset;
					++variablesCount;
				}
			}
			else if ((o._mem.base & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *vdata = x86Compiler->_getVar(o._mem.base);
				ASMJIT_ASSERT(vdata);

				if (vdata->workOffset != this->_offset)
				{
					if (!x86Context._isActive(vdata))
						x86Context._addActive(vdata);

					vdata->workOffset = this->_offset;
					++variablesCount;
				}
			}

			if ((o._mem.index & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *vdata = x86Compiler->_getVar(o._mem.index);
				ASMJIT_ASSERT(vdata);

				if (vdata->workOffset != this->_offset)
				{
					if (!x86Context._isActive(vdata))
						x86Context._addActive(vdata);

					vdata->workOffset = this->_offset;
					++variablesCount;
				}
			}
		}
	}

	if (!variablesCount)
	{
		++x86Context._currentOffset;
		return;
	}

	this->_vars = reinterpret_cast<VarAllocRecord *>(x86Compiler->getZoneMemory().alloc(sizeof(VarAllocRecord) * variablesCount));
	if (!this->_vars)
	{
		x86Compiler->setError(kErrorNoHeapMemory);
		++x86Context._currentOffset;
		return;
	}

	this->_variablesCount = variablesCount;

	VarAllocRecord *cur = this->_vars;
	VarAllocRecord *var = nullptr;

	bool _isGpbUsed = hasInstFlag(kX86CompilerInstFlagIsGpbLoUsed) | hasInstFlag(kX86CompilerInstFlagIsGpbHiUsed);
	uint32_t gpRestrictMask = IntUtil::maskUpToIndex(kX86RegNumGp);

#ifdef ASMJIT_X64
	if (hasInstFlag(kX86CompilerInstFlagIsGpbHiUsed))
		gpRestrictMask &= IntUtil::maskFromIndex(kX86RegIndexEax) | IntUtil::maskFromIndex(kX86RegIndexEbx) | IntUtil::maskFromIndex(kX86RegIndexEcx) | IntUtil::maskFromIndex(kX86RegIndexEdx) |
			IntUtil::maskFromIndex(kX86RegIndexEbp) | IntUtil::maskFromIndex(kX86RegIndexEsi) | IntUtil::maskFromIndex(kX86RegIndexEdi);
#endif // ASMJIT_X64

	for (i = 0; i < len; ++i)
	{
		Operand &o = this->_operands[i];

		if (o.isVar())
		{
			X86CompilerVar *vdata = x86Compiler->_getVar(o.getId());
			ASMJIT_ASSERT(vdata);

			__GET_VARIABLE(vdata)
			var->vflags |= kVarAllocRegister;

			if (_isGpbUsed)
			{
#ifdef ASMJIT_X86
				if (reinterpret_cast<GpVar *>(&o)->isGpb())
					var->regMask &= IntUtil::maskFromIndex(kX86RegIndexEax) | IntUtil::maskFromIndex(kX86RegIndexEbx) | IntUtil::maskFromIndex(kX86RegIndexEcx) | IntUtil::maskFromIndex(kX86RegIndexEdx);
#else
				// Restrict all BYTE registers to RAX/RBX/RCX/RDX if HI BYTE register
				// is used (REX prefix makes HI BYTE addressing unencodable).
				if (hasInstFlag(kX86CompilerInstFlagIsGpbHiUsed))
				{
					if (reinterpret_cast<GpVar *>(&o)->isGpb())
						var->regMask &= IntUtil::maskFromIndex(kX86RegIndexEax) | IntUtil::maskFromIndex(kX86RegIndexEbx) | IntUtil::maskFromIndex(kX86RegIndexEcx) | IntUtil::maskFromIndex(kX86RegIndexEdx);
				}
#endif // ASMJIT_X86/X64
			}

			if (this->isSpecial())
			{
				// ${SPECIAL_INSTRUCTION_HANDLING_BEGIN}
				switch (this->_code)
				{
					case kX86InstCpuId:
						switch (i)
						{
							case 0:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEbx);
								gpRestrictMask &= ~var->regMask;
								break;
							case 2:
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEcx);
								gpRestrictMask &= ~var->regMask;
								break;
							case 3:
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEdx);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstCbw:
					case kX86InstCdqe:
					case kX86InstCwde:
						switch (i)
						{
							case 0:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstCdq:
					case kX86InstCwd:
					case kX86InstCqo:
						switch (i)
						{
							case 0:
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEdx);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstCmpXCHG:
						switch (i)
						{
							case 0:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite;
								break;
							case 2:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstCmpXCHG8B:
#ifdef ASMJIT_X64
					case kX86InstCmpXCHG16B:
#endif // ASMJIT_X64
						switch (i)
						{
							case 0:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEdx);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;
							case 2:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEcx);
								gpRestrictMask &= ~var->regMask;
								break;
							case 3:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEbx);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

#ifdef ASMJIT_X86
					case kX86InstDaa:
					case kX86InstDas:
						ASMJIT_ASSERT(!i);
						++vdata->regRwCount;
						var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
						var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
						gpRestrictMask &= ~var->regMask;
						break;
#endif // ASMJIT_X86

					case kX86InstIMul:
					case kX86InstMul:
					case kX86InstIDiv:
					case kX86InstDiv:
						switch (i)
						{
							case 0:
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEdx);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;
							case 2:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstMovPtr:
						switch (i)
						{
							case 0:
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstLahf:
						ASMJIT_ASSERT(!i);
						++vdata->regWriteCount;
						var->vflags |= kVarAllocWrite | kVarAllocSpecial;
						var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
						gpRestrictMask &= ~var->regMask;
						break;

					case kX86InstSahf:
						ASMJIT_ASSERT(!i);
						++vdata->regReadCount;
						var->vflags |= kVarAllocRead | kVarAllocSpecial;
						var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
						gpRestrictMask &= ~var->regMask;
						break;

					case kX86InstMaskMovQ:
					case kX86InstMaskMovDQU:
						switch (i)
						{
							case 0:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEdi);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
							case 2:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead;
						}
						break;

					case kX86InstEnter:
					case kX86InstLeave:
						// TODO: SPECIAL INSTRUCTION.
						break;

					case kX86InstRet:
						// TODO: SPECIAL INSTRUCTION.
						break;

					case kX86InstMonitor:
					case kX86InstMWait:
						// TODO: MONITOR/MWAIT (COMPILER).
						break;

					case kX86InstPop:
						// TODO: SPECIAL INSTRUCTION.
						break;

					case kX86InstPopAD:
					case kX86InstPopFD:
					case kX86InstPopFQ:
						// TODO: SPECIAL INSTRUCTION.
						break;

					case kX86InstPush:
						// TODO: SPECIAL INSTRUCTION.
						break;

					case kX86InstPushAD:
					case kX86InstPushFD:
					case kX86InstPushFQ:
						// TODO: SPECIAL INSTRUCTION.
						break;

					case kX86InstRcl:
					case kX86InstRcr:
					case kX86InstRol:
					case kX86InstRor:
					case kX86InstSal:
					case kX86InstSar:
					case kX86InstShl:
					case kX86InstShr:
						switch (i)
						{
							case 0:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite;
								break;
							case 1:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEcx);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstShld:
					case kX86InstShrd:
						switch (i)
						{
							case 0:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite;
								break;
							case 1:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead;
								break;
							case 2:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEcx);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstRdtsc:
					case kX86InstRdtscP:
						switch (i)
						{
							case 0:
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEdx);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;
							case 2:
								ASMJIT_ASSERT(this->_code == kX86InstRdtscP);
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEcx);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstRepLodSB:
					case kX86InstRepLodSD:
					case kX86InstRepLodSQ:
					case kX86InstRepLodSW:
						switch (i)
						{
							case 0:
								++vdata->regWriteCount;
								var->vflags |= kVarAllocWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEsi);
								gpRestrictMask &= ~var->regMask;
								break;
							case 2:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEcx);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstRepMovSB:
					case kX86InstRepMovSD:
					case kX86InstRepMovSQ:
					case kX86InstRepMovSW:
					case kX86InstRepECmpSB:
					case kX86InstRepECmpSD:
					case kX86InstRepECmpSQ:
					case kX86InstRepECmpSW:
					case kX86InstRepNECmpSB:
					case kX86InstRepNECmpSD:
					case kX86InstRepNECmpSQ:
					case kX86InstRepNECmpSW:
						switch (i)
						{
							case 0:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEdi);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEsi);
								gpRestrictMask &= ~var->regMask;
								break;
							case 2:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEcx);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstRepStoSB:
					case kX86InstRepStoSD:
					case kX86InstRepStoSQ:
					case kX86InstRepStoSW:
						switch (i)
						{
							case 0:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEdi);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;
							case 2:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEcx);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					case kX86InstRepEScaSB:
					case kX86InstRepEScaSD:
					case kX86InstRepEScaSQ:
					case kX86InstRepEScaSW:
					case kX86InstRepNEScaSB:
					case kX86InstRepNEScaSD:
					case kX86InstRepNEScaSQ:
					case kX86InstRepNEScaSW:
						switch (i)
						{
							case 0:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEdi);
								gpRestrictMask &= ~var->regMask;
								break;
							case 1:
								++vdata->regReadCount;
								var->vflags |= kVarAllocRead | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEax);
								gpRestrictMask &= ~var->regMask;
								break;
							case 2:
								++vdata->regRwCount;
								var->vflags |= kVarAllocReadWrite | kVarAllocSpecial;
								var->regMask = IntUtil::maskFromIndex(kX86RegIndexEcx);
								gpRestrictMask &= ~var->regMask;
								break;

							default:
								ASMJIT_ASSERT(0);
						}
						break;

					default:
						ASMJIT_ASSERT(0);
				}
				// ${SPECIAL_INSTRUCTION_HANDLING_END}
			}
			else
			{
				if (!i)
				{
					// CMP/TEST instruction.
					if (id->getCode() == kX86InstCmp || id->getCode() == kX86InstTest)
					{
						// Read-only case.
						++vdata->regReadCount;
						var->vflags |= kVarAllocRead;
					}
					// CVTTSD2SI/CVTTSS2SI instructions.
					else if (id->getCode() == kX86InstCvttSD2SI || id->getCode() == kX86InstCvttSS2SI)
					{
						// In 32-bit mode the whole destination is replaced. In 64-bit mode
						// we need to check whether the destination operand size is 64-bits.
#ifdef ASMJIT_X64
						if (this->_operands[0].isRegType(kX86RegTypeGpq))
						{
#endif // ASMJIT_X64
							// Write-only case.
							++vdata->regWriteCount;
							var->vflags |= kVarAllocWrite;
#ifdef ASMJIT_X64
						}
						else
						{
							// Read/Write.
							++vdata->regRwCount;
							var->vflags |= kVarAllocReadWrite;
						}
#endif // ASMJIT_X64
					}
					// MOV/MOVSS/MOVSD instructions.
					//
					// If instruction is MOV (source replaces the destination) or 
					// MOVSS/MOVSD and source operand is memory location then register
					// allocator should know that previous destination value is lost 
					// (write only operation).
					else if ((id->isMov()) || ((id->getCode() == kX86InstMovSS || id->getCode() == kX86InstMovSD) /* && _operands[1].isMem() */) ||
						(id->getCode() == kX86InstIMul && this->_operandsCount == 3 && !isSpecial()))
					{
						// Write-only case.
						++vdata->regWriteCount;
						var->vflags |= kVarAllocWrite;
					}
					else if (id->getCode() == kX86InstLea)
					{
						// Write.
						++vdata->regWriteCount;
						var->vflags |= kVarAllocWrite;
					}
					else
					{
						// Read/Write.
						++vdata->regRwCount;
						var->vflags |= kVarAllocReadWrite;
					}
				}
				else
				{
					// Second, third, ... operands are read-only.
					++vdata->regReadCount;
					var->vflags |= kVarAllocRead;
				}

				if (!this->_memOp && i < 2 && (id->_opFlags[i] & kX86InstOpMem))
					var->vflags |= kVarAllocMem;
			}

			// If variable must be in specific register we could add some hint to allocator.
			if (var->vflags & kVarAllocSpecial)
			{
				vdata->prefRegisterMask |= var->regMask;
				x86Context._newRegisterHomeIndex(vdata, IntUtil::findFirstBit(var->regMask));
			}
		}
		else if (o.isMem())
		{
			if ((o.getId() & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *vdata = x86Compiler->_getVar(o.getId());
				ASMJIT_ASSERT(vdata);

				__GET_VARIABLE(vdata)

				if (!i)
				{
					// If variable is MOV instruction type (source replaces the destination)
					// or variable is MOVSS/MOVSD instruction then register allocator should
					// know that previous destination value is lost (write only operation).
					if (id->isMov() || (id->getCode() == kX86InstMovSS || id->getCode() == kX86InstMovSD))
						// Write only case.
						++vdata->memWriteCount;
					else
						++vdata->memRwCount;
				}
				else
					++vdata->memReadCount;
			}
			else if ((o._mem.base & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *vdata = x86Compiler->_getVar(reinterpret_cast<Mem &>(o).getBase());
				ASMJIT_ASSERT(vdata);

				__GET_VARIABLE(vdata)
				++vdata->regReadCount;
				var->vflags |= kVarAllocRegister | kVarAllocRead;
				var->regMask &= gpRestrictMask;
			}

			if ((o._mem.index & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *vdata = x86Compiler->_getVar(reinterpret_cast<Mem &>(o).getIndex());
				ASMJIT_ASSERT(vdata);

				__GET_VARIABLE(vdata)
				++vdata->regReadCount;
				var->vflags |= kVarAllocRegister | kVarAllocRead;
				var->regMask &= gpRestrictMask;
			}
		}
	}

	// Traverse all variables and update firstItem / lastItem. This
	// function is called from iterator that scans items using forward
	// direction so we can use this knowledge to optimize the process.
	//
	// Similar to X86CompilerFuncCall::prepare().
	for (i = 0; i < this->_variablesCount; ++i)
	{
		X86CompilerVar *v = this->_vars[i].vdata;

		// Update GP register allocator restrictions.
		if (X86Util::isVarTypeInt(v->getType()) && this->_vars[i].regMask == 0xFFFFFFFF)
			this->_vars[i].regMask &= gpRestrictMask;

		// Update first/last item (begin of variable scope).
		if (!v->firstItem)
			v->firstItem = this;
		v->lastItem = this;
	}

	// There are some instructions that can be used to clear or to set all bits
	// in a register:
	//
	// - andn reg, reg        ; Set all bits in reg to 0.
	// - xor/pxor reg, reg    ; Set all bits in reg to 0.
	// - sub/psub reg, reg    ; Set all bits in reg to 0.
	// - pcmpgt reg, reg      ; Set all bits in reg to 0.
	// - pcmpeq reg, reg      ; Set all bits in reg to 1.
	//
	// There are also combinations which do nothing:
	//
	// - and reg, reg         ; Nop.
	// - or reg, reg          ; Nop.
	// - xchg reg, reg        ; Nop.

	if (this->_variablesCount == 1 && this->_operandsCount > 1 && this->_operands[0].isVar() && this->_operands[1].isVar() && !this->_memOp)
	{
		switch (this->_code)
		{
			// ----------------------------------------------------------------------
			// [Zeros/Ones]
			// ----------------------------------------------------------------------

			// ANDN Instructions.
			case kX86InstPAndN:
			// XOR Instructions.
			case kX86InstXor:
			case kX86InstXorPD:
			case kX86InstXorPS:
			case kX86InstPXor:
			// SUB Instructions.
			case kX86InstSub:
			case kX86InstPSubB:
			case kX86InstPSubW:
			case kX86InstPSubD:
			case kX86InstPSubQ:
			case kX86InstPSubSB:
			case kX86InstPSubSW:
			case kX86InstPSubUSB:
			case kX86InstPSubUSW:
			// PCMPEQ Instructions.
			case kX86InstPCmpEqB:
			case kX86InstPCmpEqW:
			case kX86InstPCmpEqD:
			case kX86InstPCmpEqQ:
			// PCMPGT Instructions.
			case kX86InstPCmpGtB:
			case kX86InstPCmpGtW:
			case kX86InstPCmpGtD:
			case kX86InstPCmpGtQ:
				// Clear the read flag. This prevents variable alloc/spill.
				this->_vars[0].vflags = kVarAllocWrite;
				--this->_vars[0].vdata->regReadCount;
				break;

			// ----------------------------------------------------------------------
			// [Nop]
			// ----------------------------------------------------------------------

			// AND Instructions.
			case kX86InstAnd:
			case kX86InstAndPD:
			case kX86InstAndPS:
			case kX86InstPAnd:
			// OR Instructions.
			case kX86InstOr:
			case kX86InstOrPD:
			case kX86InstOrPS:
			case kX86InstPOr:
			// XCHG Instruction.
			case kX86InstXchg:
				// Clear the write flag.
				this->_vars[0].vflags = kVarAllocRead;
				--this->_vars[0].vdata->regWriteCount;
				break;
		}
	}
	++x86Context._currentOffset;

#undef __GET_VARIABLE
}

CompilerItem *X86CompilerInst::translate(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = this->getCompiler();

	uint32_t i;
	uint32_t variablesCount = this->_variablesCount;

	if (variablesCount > 0)
	{
		// These variables are used by the instruction and we set current offset
		// to their work offsets -> getSpillCandidate never return the variable
		// used this instruction.
		for (i = 0; i < variablesCount; ++i)
			this->_vars[i].vdata->workOffset = x86Context._currentOffset;

		// Alloc variables used by the instruction (special first).
		for (i = 0; i < variablesCount; ++i)
		{
			VarAllocRecord &r = this->_vars[i];
			// Alloc variables with specific register first.
			if (r.vflags & kVarAllocSpecial)
				x86Context.allocVar(r.vdata, r.regMask, r.vflags);
		}

		for (i = 0; i < variablesCount; ++i)
		{
			VarAllocRecord &r = this->_vars[i];
			// Alloc variables without specific register last.
			if (!(r.vflags & kVarAllocSpecial))
				x86Context.allocVar(r.vdata, r.regMask, r.vflags);
		}

		x86Context.translateOperands(this->_operands, this->_operandsCount);
	}

	if (this->_memOp && (this->_memOp->getId() & kOperandIdTypeMask) == kOperandIdTypeVar)
	{
		X86CompilerVar *cv = x86Compiler->_getVar(this->_memOp->getId());
		ASMJIT_ASSERT(cv);

		switch (cv->state)
		{
			case kVarStateUnused:
				cv->state = kVarStateMem;
				break;
			case kVarStateReg:
				cv->changed = false;
				x86Context.unuseVar(cv, kVarStateMem);
		}
	}

	for (i = 0; i < variablesCount; ++i)
		x86Context._unuseVarOnEndOfScope(this, &this->_vars[i]);

	return this->translated();
}

void X86CompilerInst::emit(Assembler &a)
{
	X86Assembler &x86Asm = static_cast<X86Assembler &>(a);

	x86Asm._inlineComment = this->_comment;
	x86Asm._emitOptions = this->_emitOptions;

	if (this->isSpecial())
	{
		// ${SPECIAL_INSTRUCTION_HANDLING_BEGIN}
		switch (this->_code)
		{
			case kX86InstCpuId:
				x86Asm._emitInstruction(this->_code);
				return;

			case kX86InstCbw:
			case kX86InstCdq:
			case kX86InstCdqe:
			case kX86InstCwd:
			case kX86InstCwde:
			case kX86InstCqo:
				x86Asm._emitInstruction(this->_code);
				return;

			case kX86InstCmpXCHG:
				x86Asm._emitInstruction(this->_code, &this->_operands[1], &this->_operands[2]);
				return;

			case kX86InstCmpXCHG8B:
#ifdef ASMJIT_X64
			case kX86InstCmpXCHG16B:
#endif // ASMJIT_X64
				x86Asm._emitInstruction(this->_code, &this->_operands[4]);
				return;

#ifdef ASMJIT_X86
			case kX86InstDaa:
			case kX86InstDas:
				x86Asm._emitInstruction(this->_code);
				return;
#endif // ASMJIT_X86

			case kX86InstIMul:
			case kX86InstMul:
			case kX86InstIDiv:
			case kX86InstDiv:
				// INST dst_lo (implicit), dst_hi (implicit), src (explicit)
				ASMJIT_ASSERT(this->_operandsCount == 3);
				x86Asm._emitInstruction(this->_code, &this->_operands[2]);
				return;

			case kX86InstMovPtr:
				break;

			case kX86InstLahf:
			case kX86InstSahf:
				x86Asm._emitInstruction(this->_code);
				return;

			case kX86InstMaskMovQ:
			case kX86InstMaskMovDQU:
				x86Asm._emitInstruction(this->_code, &this->_operands[1], &this->_operands[2]);
				return;

			case kX86InstEnter:
			case kX86InstLeave:
				// TODO: SPECIAL INSTRUCTION.
				break;

			case kX86InstRet:
				// TODO: SPECIAL INSTRUCTION.
				break;

			case kX86InstMonitor:
			case kX86InstMWait:
				// TODO: MONITOR/MWAIT (COMPILER).
				break;

			case kX86InstPop:
			case kX86InstPopAD:
			case kX86InstPopFD:
			case kX86InstPopFQ:
				// TODO: SPECIAL INSTRUCTION.
				break;

			case kX86InstPush:
			case kX86InstPushAD:
			case kX86InstPushFD:
			case kX86InstPushFQ:
				// TODO: SPECIAL INSTRUCTION.
				break;

			case kX86InstRcl:
			case kX86InstRcr:
			case kX86InstRol:
			case kX86InstRor:
			case kX86InstSal:
			case kX86InstSar:
			case kX86InstShl:
			case kX86InstShr:
				x86Asm._emitInstruction(this->_code, &this->_operands[0], &cl);
				return;

			case kX86InstShld:
			case kX86InstShrd:
				x86Asm._emitInstruction(this->_code, &this->_operands[0], &this->_operands[1], &cl);
				return;

			case kX86InstRdtsc:
			case kX86InstRdtscP:
				x86Asm._emitInstruction(this->_code);
				return;

			case kX86InstRepLodSB:
			case kX86InstRepLodSD:
			case kX86InstRepLodSQ:
			case kX86InstRepLodSW:
			case kX86InstRepMovSB:
			case kX86InstRepMovSD:
			case kX86InstRepMovSQ:
			case kX86InstRepMovSW:
			case kX86InstRepStoSB:
			case kX86InstRepStoSD:
			case kX86InstRepStoSQ:
			case kX86InstRepStoSW:
			case kX86InstRepECmpSB:
			case kX86InstRepECmpSD:
			case kX86InstRepECmpSQ:
			case kX86InstRepECmpSW:
			case kX86InstRepEScaSB:
			case kX86InstRepEScaSD:
			case kX86InstRepEScaSQ:
			case kX86InstRepEScaSW:
			case kX86InstRepNECmpSB:
			case kX86InstRepNECmpSD:
			case kX86InstRepNECmpSQ:
			case kX86InstRepNECmpSW:
			case kX86InstRepNEScaSB:
			case kX86InstRepNEScaSD:
			case kX86InstRepNEScaSQ:
			case kX86InstRepNEScaSW:
				x86Asm._emitInstruction(this->_code);
				return;

			default:
				ASMJIT_ASSERT(0);
		}
		// ${SPECIAL_INSTRUCTION_HANDLING_END}
	}

	switch (this->_operandsCount)
	{
		case 0:
			x86Asm._emitInstruction(this->_code);
			break;
		case 1:
			x86Asm._emitInstruction(this->_code, &this->_operands[0]);
			break;
		case 2:
			x86Asm._emitInstruction(this->_code, &this->_operands[0], &this->_operands[1]);
			break;
		case 3:
			x86Asm._emitInstruction(this->_code, &this->_operands[0], &this->_operands[1], &this->_operands[2]);
			break;
		default:
			ASMJIT_ASSERT(0);
	}
}

// ============================================================================
// [AsmJit::X86CompilerInst - Misc]
// ============================================================================

int X86CompilerInst::getMaxSize() const
{
	// TODO: Instruction max size.
	return 15;
}

bool X86CompilerInst::_tryUnuseVar(CompilerVar *_v)
{
	X86CompilerVar *cv = static_cast<X86CompilerVar *>(_v);

	for (uint32_t i = 0; i < this->_variablesCount; ++i)
		if (this->_vars[i].vdata == cv)
		{
			this->_vars[i].vflags |= kVarAllocUnuseAfterUse;
			return true;
		}

	return false;
}

// ============================================================================
// [AsmJit::X86CompilerJmpInst - Construction / Destruction]
// ============================================================================

X86CompilerJmpInst::X86CompilerJmpInst(X86Compiler *x86Compiler, uint32_t code, Operand *opData, uint32_t opCount) : X86CompilerInst(x86Compiler, code, opData, opCount)
{
	this->_jumpTarget = x86Compiler->_getTarget(this->_operands[0].getId());
	++this->_jumpTarget->_jumpsCount;

	this->_jumpNext = static_cast<X86CompilerJmpInst *>(this->_jumpTarget->_from);
	this->_jumpTarget->_from = this;

	// The 'jmp' is always taken, conditional jump can contain hint, we detect it.
	if (this->getCode() == kX86InstJmp)
		this->setInstFlag(kX86CompilerInstFlagIsTaken);
	else if (opCount > 1 && opData[1].isImm() && reinterpret_cast<Imm *>(&opData[1])->getValue() == kCondHintLikely)
		this->setInstFlag(kX86CompilerInstFlagIsTaken);
}

X86CompilerJmpInst::~X86CompilerJmpInst()
{
}

// ============================================================================
// [AsmJit::X86CompilerJmpInst - Interface]
// ============================================================================

void X86CompilerJmpInst::prepare(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	this->_offset = x86Context._currentOffset;

	// Update _isTaken to true if this is conditional backward jump. This behavior
	// can be overridden by using kCondHintUnlikely when using the instruction.
	if (this->getCode() != kX86InstJmp && this->_operandsCount == 1 && this->_jumpTarget->getOffset() < getOffset())
		this->setInstFlag(kX86CompilerInstFlagIsTaken);

	// Now patch all variables where jump location is in the active range.
	if (this->_jumpTarget->getOffset() != kInvalidValue && x86Context._active)
	{
		X86CompilerVar *first = static_cast<X86CompilerVar *>(x86Context._active);
		X86CompilerVar *var = first;
		uint32_t jumpOffset = this->_jumpTarget->getOffset();

		do
		{
			if (var->firstItem)
			{
				ASMJIT_ASSERT(var->lastItem);
				uint32_t start = var->firstItem->getOffset();
				uint32_t end = var->lastItem->getOffset();

				if (jumpOffset >= start && jumpOffset <= end)
					var->lastItem = this;
			}
			var = var->nextActive;
		} while (var != first);
	}

	++x86Context._currentOffset;
}

CompilerItem* X86CompilerJmpInst::translate(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);

	// Translate using X86CompilerInst.
	CompilerItem *ret = X86CompilerInst::translate(cc);

	// We jump with item if its kX86InstJUMP (not based on condiiton) and it
	// points into yet unknown location.
	if (this->_code == kX86InstJmp && !this->_jumpTarget->isTranslated())
	{
		x86Context.addBackwardCode(this);
		ret = this->_jumpTarget;
	}
	else
	{
		this->_state = x86Context._saveState();
		if (this->_jumpTarget->isTranslated())
			this->doJump(cc);
		else
		{
			// State is not known, so we need to call doJump() later. Compiler will
			// do it for us.
			x86Context.addForwardJump(this);
			this->_jumpTarget->_state = this->_state;
		}

		// Mark next code as unreachable, cleared by a next label (CompilerTarget).
		if (this->_code == kX86InstJmp)
			x86Context._isUnreachable = 1;
	}

	// Need to traverse over all active variables and unuse them if their scope ends
	// here. 
	if (x86Context._active)
	{
		X86CompilerVar *first = static_cast<X86CompilerVar *>(x86Context._active);
		X86CompilerVar *var = first;

		do
		{
			x86Context._unuseVarOnEndOfScope(this, var);
			var = var->nextActive;
		} while (var != first);
	}

	return ret;
}

void X86CompilerJmpInst::emit(Assembler &a)
{
	static const unsigned MAXIMUM_SHORT_JMP_SIZE = 127;

	// Try to minimize size of jump using SHORT jump (8-bit displacement) by 
	// traversing into the target and calculating the maximum code size. We
	// end when code size reaches MAXIMUM_SHORT_JMP_SIZE.
	if (!(this->_emitOptions & kX86EmitOptionShortJump) && this->getJumpTarget()->getOffset() > this->getOffset())
	{
		// Calculate the code size.
		unsigned codeSize = 0;
		CompilerItem *cur = this->getNext();
		CompilerItem *target = this->getJumpTarget();

		while (cur)
		{
			if (cur == target)
			{
				// Target found, we can tell assembler to generate short form of jump.
				this->_emitOptions |= kX86EmitOptionShortJump;
				goto _End;
			}

			int s = cur->getMaxSize();
			if (s == -1)
				break;

			codeSize += s;
			if (codeSize > MAXIMUM_SHORT_JMP_SIZE)
				break;

			cur = cur->getNext();
		}
	}

_End:
	X86CompilerInst::emit(a);
}

// ============================================================================
// [AsmJit::X86CompilerJmpInst - DoJump]
// ============================================================================

void X86CompilerJmpInst::doJump(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = this->getCompiler();

	// The state have to be already known. The doJump() method is called by
	// translate() or by Compiler in case that it's forward jump.
	ASMJIT_ASSERT(this->_jumpTarget->getState());

	if (this->getCode() == kX86InstJmp || (this->isTaken() && this->_jumpTarget->getOffset() < this->getOffset()))
		// Instruction type is JMP or conditional jump that should be taken (likely).
		// We can set state here instead of jumping out, setting state and jumping
		// to _jumpTarget.
		//
		// NOTE: We can't use this technique if instruction is forward conditional
		// jump. The reason is that when generating code we can't change state here,
		// because the next instruction depends on it.
		x86Context._restoreState(this->_jumpTarget->getState(), this->_jumpTarget->getOffset());
	else
	{
		// Instruction type is JMP or conditional jump that should be not normally
		// taken. If we need add code that will switch between different states we
		// add it after the end of function body (after epilog, using 'ExtraBlock').
		CompilerItem *ext = x86Context.getExtraBlock();
		CompilerItem *old = x86Compiler->setCurrentItem(ext);

		x86Context._restoreState(this->_jumpTarget->getState(), this->_jumpTarget->getOffset());

		if (x86Compiler->getCurrentItem() != ext)
		{
			// Add the jump to the target.
			x86Compiler->jmp(this->_jumpTarget->_label);
			ext = x86Compiler->getCurrentItem();

			// The x86Context._restoreState() method emitted some instructions so we need to
			// patch the jump.
			Label L = x86Compiler->newLabel();
			x86Compiler->setCurrentItem(x86Context.getExtraBlock());
			x86Compiler->bind(L);

			// Finally, patch the jump target.
			ASMJIT_ASSERT(this->_operandsCount > 0);
			this->_operands[0] = L; // Operand part (Label).
			this->_jumpTarget = x86Compiler->_getTarget(L.getId()); // Compiler part (CompilerTarget).
		}

		x86Context.setExtraBlock(ext);
		x86Compiler->setCurrentItem(old);

		// Assign state back.
		x86Context._assignState(static_cast<X86CompilerState *>(this->_state));
	}
}

// ============================================================================
// [AsmJit::X86CompilerJmpInst - GetJumpTarget]
// ============================================================================

CompilerTarget *X86CompilerJmpInst::getJumpTarget() const
{
	return this->_jumpTarget;
}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"
