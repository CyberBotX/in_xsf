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
#include "../x86/x86cpuinfo.h"
#include "../x86/x86util.h"

// [Api-Begin]
#include "../core/apibegin.h"

namespace AsmJit
{

// ============================================================================
// [AsmJit::X86Assembler - Logging]
// ============================================================================

// Defined in AsmJit/X86/X86Assembler.cpp.
char *X86Assembler_dumpRegister(char *buf, uint32_t type, uint32_t index);
char *X86Assembler_dumpOperand(char *buf, const Operand *op, uint32_t memRegType, uint32_t loggerFlags);

// ============================================================================
// [AsmJit::X86CompilerFuncDecl - Construction / Destructioin]
// ============================================================================

X86CompilerFuncDecl::X86CompilerFuncDecl(X86Compiler *x86Compiler) : CompilerFuncDecl(x86Compiler), _gpModifiedAndPreserved(0), _mmModifiedAndPreserved(0), _xmmModifiedAndPreserved(0), _movDqInstCode(kInstNone),
	_pePushPopStackSize(0), _peMovStackSize(0), _peAdjustStackSize(0), _memStackSize(0), _memStackSize16(0)
{
	this->_decl = &_x86Decl;

	// Just clear to safe defaults.
	this->_funcHints |= IntUtil::maskFromIndex(kX86FuncHintPushPop);

	// Stack is always aligned to 16-bytes when using 64-bit OS.
	if (CompilerUtil::isStack16ByteAligned())
		this->_funcHints |= IntUtil::maskFromIndex(kX86FuncHintAssume16ByteAlignment);

	this->_entryLabel = x86Compiler->newLabel();
	this->_exitLabel = x86Compiler->newLabel();

	this->_entryTarget = x86Compiler->_getTarget(this->_entryLabel.getId());
	this->_exitTarget = x86Compiler->_getTarget(this->_exitLabel.getId());

	this->_end = Compiler_newItem<X86CompilerFuncEnd>(x86Compiler, this);
}

X86CompilerFuncDecl::~X86CompilerFuncDecl()
{
}

// ============================================================================
// [AsmJit::X86CompilerFuncDecl - Interface]
// ============================================================================

void X86CompilerFuncDecl::prepare(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	this->_offset = x86Context._currentOffset++;

	this->_prepareVariables(this);
}

CompilerItem *X86CompilerFuncDecl::translate(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);

	this->_allocVariables(x86Context);
	return this->translated();
}

// ============================================================================
// [AsmJit::X86CompilerFuncDecl - Misc]
// ============================================================================

int X86CompilerFuncDecl::getMaxSize() const
{
	// NOP.
	return 0;
}

// ============================================================================
// [AsmJit::X86CompilerFuncDecl - Prototype]
// ============================================================================

void X86CompilerFuncDecl::setPrototype(uint32_t convention, uint32_t returnType, const uint32_t *arguments, uint32_t argumentsCount)
{
	this->_x86Decl.setPrototype(convention, returnType, arguments, argumentsCount);
}

// ============================================================================
// [AsmJit::X86CompilerFuncDecl - Helpers]
// ============================================================================

void X86CompilerFuncDecl::_createVariables()
{
	X86Compiler *x86Compiler = this->getCompiler();

	uint32_t i, count = this->_x86Decl.getArgumentsCount();
	if (!count)
		return;

	this->_vars = reinterpret_cast<CompilerVar **>(x86Compiler->getZoneMemory().alloc(count * sizeof(void*)));
	if (!this->_vars)
	{
		x86Compiler->setError(kErrorNoHeapMemory);
		return;
	}

	char argNameStorage[64];
	char *argName = nullptr;

	bool debug = !!x86Compiler->getLogger();
	if (debug)
		argName = argNameStorage;

	for (i = 0; i < count; ++i)
	{
		FuncArg &arg = this->_x86Decl.getArgument(i);

		if (debug)
			snprintf(argName, ASMJIT_ARRAY_SIZE(argNameStorage), "arg_%u", i);

		uint32_t size = X86Util::getVarSizeFromVarType(arg.getVarType());
		X86CompilerVar *cv = x86Compiler->_newVar(argName, arg.getVarType(), size);

		if (arg.getRegIndex() != kRegIndexInvalid)
		{
			cv->_isRegArgument = true;
			cv->regIndex = arg.getRegIndex();
		}

		if (arg.getStackOffset() != kFuncStackInvalid)
		{
			cv->_isMemArgument = true;
			cv->homeMemoryOffset = arg.getStackOffset();
		}

		this->_vars[i] = cv;
	}
}

void X86CompilerFuncDecl::_prepareVariables(CompilerItem *first)
{
	uint32_t count = this->_x86Decl.getArgumentsCount();
	if (!count)
		return;

	for (uint32_t i = 0; i < count; ++i)
	{
		X86CompilerVar *cv = this->getVar(i);

		// This is where variable scope starts.
		cv->firstItem = first;
		// If this will not be changed then it will be deallocated immediately.
		cv->lastItem = first;
	}
}

void X86CompilerFuncDecl::_allocVariables(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	uint32_t count = this->getDecl()->getArgumentsCount();

	if (!count)
		return;

	for (uint32_t i = 0; i < count; ++i)
	{
		X86CompilerVar *cv = this->getVar(i);

		if (cv->firstItem || cv->isArgument())
		{
			// Variable is used.
			if (cv->regIndex != kRegIndexInvalid)
			{
				cv->state = kVarStateReg;
				// If variable is in register -> mark it as changed so it will not be
				// lost by first spill.
				cv->changed = true;
				x86Context._allocatedVariable(cv);
			}
			else if (cv->isMemArgument())
				cv->state = kVarStateMem;
		}
		else
			// Variable is not used.
			cv->regIndex = kRegIndexInvalid;
	}
}

void X86CompilerFuncDecl::_preparePrologEpilog(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);

	this->clearFuncFlag(kX86FuncFlagPushPop | kX86FuncFlagEmitEmms | kX86FuncFlagEmitSFence | kX86FuncFlagEmitLFence | kX86FuncFlagAssume16ByteAlignment | kX86FuncFlagPerform16ByteAlignment);

	uint32_t accessibleMemoryBelowStack = 0;
	if (this->getDecl()->getConvention() == kX86FuncConvX64U) 
		accessibleMemoryBelowStack = 128;

	if (this->getHint(kX86FuncHintAssume16ByteAlignment))
		this->setFuncFlag(kX86FuncFlagAssume16ByteAlignment);

	if (this->getHint(kX86FuncHintPerform16ByteAlignment))
		this->setFuncFlag(kX86FuncFlagPerform16ByteAlignment);

	if (this->getHint(kFuncHintNaked))
		this->setFuncFlag(kFuncFlagIsNaked);

	if (this->isCaller() && (x86Context._memBytesTotal > 0 || this->isAssumed16ByteAlignment()))
		this->setFuncFlag(kX86FuncFlagIsEspAdjusted);

	if (x86Context._memBytesTotal > accessibleMemoryBelowStack)
		this->setFuncFlag(kX86FuncFlagIsEspAdjusted);

	if (this->getHint(kX86FuncHintPushPop))
		this->setFuncFlag(kX86FuncFlagPushPop);

	if (this->getHint(kX86FuncHintEmms))
		this->setFuncFlag(kX86FuncFlagEmitEmms);

	if (this->getHint(kX86FuncHintSFence))
		this->setFuncFlag(kX86FuncFlagEmitSFence);

	if (this->getHint(kX86FuncHintLFence))
		this->setFuncFlag(kX86FuncFlagEmitLFence);

	// Updated to respect comment from issue #47, align also when using MMX code.
	if (!this->isAssumed16ByteAlignment() && !this->isNaked() && (x86Context._mem16BlocksCount + (x86Context._mem8BlocksCount > 0)))
		// Have to align stack to 16-bytes.
		this->setFuncFlag(kX86FuncFlagIsEspAdjusted | kX86FuncFlagPerform16ByteAlignment);

	this->_gpModifiedAndPreserved = x86Context._modifiedGpRegisters & this->_x86Decl.getGpPreservedMask() & (~IntUtil::maskFromIndex(kX86RegIndexEsp));
	this->_mmModifiedAndPreserved = x86Context._modifiedMmRegisters & this->_x86Decl.getMmPreservedMask();
	this->_xmmModifiedAndPreserved = x86Context._modifiedXmmRegisters & this->_x86Decl.getXmmPreservedMask();
	this->_movDqInstCode = this->isAssumed16ByteAlignment() || this->isPerformed16ByteAlignment() ? kX86InstMovDQA : kX86InstMovDQU;

	// Prolog & Epilog stack size.
	int32_t memGpSize = IntUtil::bitCount(this->_gpModifiedAndPreserved) * sizeof(intptr_t);
	int32_t memMmSize = IntUtil::bitCount(this->_mmModifiedAndPreserved) * 8;
	int32_t memXmmSize = IntUtil::bitCount(this->_xmmModifiedAndPreserved) * 16;

	if (this->hasFuncFlag(kX86FuncFlagPushPop))
	{
		this->_pePushPopStackSize = memGpSize;
		this->_peMovStackSize = memXmmSize + IntUtil::align(memMmSize, 16);
	}
	else
	{
		this->_pePushPopStackSize = 0;
		this->_peMovStackSize = memXmmSize + IntUtil::align(memMmSize + memGpSize, 16);
	}

	if (this->isPerformed16ByteAlignment())
		this->_peAdjustStackSize += IntUtil::delta(this->_pePushPopStackSize, 16);
	else
	{
		int32_t v = 16 - sizeof(uintptr_t);

		if (!this->isNaked())
			v -= sizeof(uintptr_t);

		v -= this->_pePushPopStackSize & 15;

		if (v < 0)
			v += 16;

		this->_peAdjustStackSize = v;

		//this->_peAdjustStackSize += IntUtil::delta(this->_pePushPopStackSize + v, 16);
	}

	// Memory stack size.
	this->_memStackSize = x86Context._memBytesTotal;
	this->_memStackSize16 = IntUtil::align(this->_memStackSize, 16);

	if (this->isNaked())
	{
		x86Context._argumentsBaseReg = kX86RegIndexEsp;
		x86Context._argumentsBaseOffset = this->hasFuncFlag(kX86FuncFlagIsEspAdjusted) ?
			this->_funcCallStackSize + this->_memStackSize16 + this->_peMovStackSize + this->_pePushPopStackSize + this->_peAdjustStackSize : this->_pePushPopStackSize;
	}
	else
	{
		x86Context._argumentsBaseReg = kX86RegIndexEbp;
		x86Context._argumentsBaseOffset = sizeof(sysint_t);
	}

	x86Context._variablesBaseReg = kX86RegIndexEsp;
	x86Context._variablesBaseOffset = this->_funcCallStackSize;

	if (!this->hasFuncFlag(kX86FuncFlagIsEspAdjusted))
		x86Context._variablesBaseOffset = -this->_memStackSize16 - this->_peMovStackSize - this->_peAdjustStackSize;
}

void X86CompilerFuncDecl::_dumpFunction(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = this->getCompiler();

	Logger *logger = x86Compiler->getLogger();
	ASMJIT_ASSERT(logger);

	uint32_t i;
	char _buf[1024];
	char *p;

	// Log function prototype.
	uint32_t argumentsCount = this->_x86Decl.getArgumentsCount();
	bool first = true;

	logger->logString("; Function Prototype:\n");
	logger->logString(";\n");

	for (i = 0; i < argumentsCount; ++i)
	{
		const FuncArg &a = this->_x86Decl.getArgument(i);
		X86CompilerVar *cv = this->getVar(i);

		if (first)
		{
			logger->logString("; IDX| Type     | Sz | Home           |\n");
			logger->logString("; ---+----------+----+----------------+\n");
		}

		char *memHome = _buf;

		if (a.hasRegIndex())
		{
			Reg regOp(a.getRegIndex() | kX86RegTypeGpz, 0);
			X86Assembler_dumpOperand(memHome, &regOp, kX86RegTypeGpz, 0)[0] = 0;
		}
		else
		{
			Mem memOp;
			memOp._mem.base = kX86RegIndexEsp;
			memOp._mem.displacement = a.getStackOffset();
			X86Assembler_dumpOperand(memHome, &memOp, kX86RegTypeGpz, 0)[0] = 0;
		}

		logger->logFormat("; %-3u| %-9s| %-3u| %-15s|\n",
			// Argument index.
			i,
			// Argument type.
			cv->getType() < kX86VarTypeCount ? x86VarInfo[cv->getType()].getName() : "invalid",
			// Argument size.
			cv->getSize(),
			// Argument memory home.
			memHome
		);

		first = false;
	}
	logger->logString(";\n");

	// Log variables.
	uint32_t variablesCount = static_cast<uint32_t>(x86Compiler->_vars.getLength());
	first = true;

	logger->logString("; Variables:\n");
	logger->logString(";\n");

	for (i = 0; i < variablesCount; ++i)
	{
		X86CompilerVar *cv = static_cast<X86CompilerVar *>(x86Compiler->_vars[i]);

		// If this variable is not related to this function then skip it.
		if (cv->funcScope != this)
			continue;

		// Get some information about variable type.
		const X86VarInfo &vinfo = x86VarInfo[cv->getType()];

		if (first)
		{
			logger->logString("; ID | Type     | Sz | Home           | Register Access   | Memory Access     |\n");
			logger->logString("; ---+----------+----+----------------+-------------------+-------------------+\n");
		}

		char *memHome = const_cast<char *>("[None]");
		if (cv->homeMemoryData)
		{
			VarMemBlock *memBlock = reinterpret_cast<VarMemBlock *>(cv->homeMemoryData);
			memHome = _buf;

			Mem memOp;
			if (cv->isMemArgument())
			{
				const FuncArg &a = this->_x86Decl.getArgument(i);

				memOp._mem.base = x86Context._argumentsBaseReg;
				memOp._mem.displacement += x86Context._argumentsBaseOffset;
				memOp._mem.displacement += a.getStackOffset();
			}
			else
			{
				memOp._mem.base = x86Context._variablesBaseReg;
				memOp._mem.displacement += x86Context._variablesBaseOffset;
				memOp._mem.displacement += memBlock->offset;
			}
			X86Assembler_dumpOperand(memHome, &memOp, kX86RegTypeGpz, 0)[0] = 0;
		}

		logger->logFormat("; %-3u| %-9s| %-3u| %-15s| r=%-4uw=%-4ux=%-4u| r=%-4uw=%-4ux=%-4u|\n",
			// Variable id.
			static_cast<unsigned>(i & kOperandIdValueMask),
			// Variable type.
			cv->getType() < kX86VarTypeCount ? vinfo.getName() : "invalid",
			// Variable size.
			cv->getSize(),
			// Variable memory home.
			memHome,
			// Register access count.
			static_cast<unsigned>(cv->regReadCount),
			static_cast<unsigned>(cv->regWriteCount),
			static_cast<unsigned>(cv->regRwCount),
			// Memory access count.
			static_cast<unsigned>(cv->memReadCount),
			static_cast<unsigned>(cv->memWriteCount),
			static_cast<unsigned>(cv->memRwCount)
		);
		first = false;
	}
	logger->logString(";\n");

	// Log modified registers.
	p = _buf;

	uint32_t r;
	uint32_t modifiedRegisters = 0;

	for (r = 0; r < 3; ++r)
	{
		bool first = true;
		uint32_t regs = 0;
		uint32_t type = 0;

		switch (r)
		{
			case 0:
				regs = x86Context._modifiedGpRegisters;
				type = kX86RegTypeGpz;
				p = StringUtil::copy(p, "; GP : ");
				break;
			case 1:
				regs = x86Context._modifiedMmRegisters;
				type = kX86RegTypeMm;
				p = StringUtil::copy(p, "; MM : ");
				break;
			case 2:
				regs = x86Context._modifiedXmmRegisters;
				type = kX86RegTypeXmm;
				p = StringUtil::copy(p, "; XMM: ");
				break;
			default:
				ASMJIT_ASSERT(0);
		}

		for (i = 0; i < kX86RegNumBase; ++i)
		{
			if (regs & IntUtil::maskFromIndex(i))
			{
				if (!first)
				{
					*p++ = ',';
					*p++ = ' ';
				}
				p = X86Assembler_dumpRegister(p, type, i);
				first = false;
				++modifiedRegisters;
			}
		}
		*p++ = '\n';
	}
	*p = 0;

	logger->logFormat("; Modified registers (%u):\n", static_cast<unsigned>(modifiedRegisters));
	logger->logString(_buf);

	logger->logString("\n");
}

void X86CompilerFuncDecl::_emitProlog(CompilerContext &cc)
{
	X86Compiler *x86Compiler = this->getCompiler();

	// --------------------------------------------------------------------------
	// [Init]
	// --------------------------------------------------------------------------

	uint32_t i, mask;
	uint32_t preservedGP = this->_gpModifiedAndPreserved;
	uint32_t preservedMM = this->_mmModifiedAndPreserved;
	uint32_t preservedXMM = this->_xmmModifiedAndPreserved;

	int32_t stackOffset = this->_getRequiredStackOffset();
	int32_t stackPos;

	// --------------------------------------------------------------------------
	// [Prolog]
	// --------------------------------------------------------------------------

	if (x86Compiler->getLogger())
		x86Compiler->comment("Prolog");

	// Emit standard prolog entry code (but don't do it if function is set to be
	// naked).
	//
	// Also see the _prologEpilogStackAdjust variable. If function is naked (so
	// prolog and epilog will not contain "push ebp" and "mov ebp, esp", we need
	// to adjust stack by 8 bytes in 64-bit mode (this will give us that stack
	// will remain aligned to 16 bytes).
	if (!this->isNaked())
	{
		x86Compiler->emit(kX86InstPush, zbp);
		x86Compiler->emit(kX86InstMov, zbp, zsp);
	}

	// Align manually stack-pointer to 16-bytes.
	if (this->isPerformed16ByteAlignment())
	{
		ASMJIT_ASSERT(!this->isNaked());
		x86Compiler->emit(kX86InstAnd, zsp, imm(-16));
	}

	// --------------------------------------------------------------------------
	// [Save Gp - Push/Pop]
	// --------------------------------------------------------------------------

	if (preservedGP && this->hasFuncFlag(kX86FuncFlagPushPop))
	{
		for (i = 0, mask = 1; i < kX86RegNumGp; ++i, mask <<= 1)
			if (preservedGP & mask)
				x86Compiler->emit(kX86InstPush, gpz(i));
	}

	// --------------------------------------------------------------------------
	// [Adjust Scack]
	// --------------------------------------------------------------------------

	if (this->isEspAdjusted())
	{
		stackPos = this->_memStackSize16 + this->_funcCallStackSize;
		if (stackOffset)
			x86Compiler->emit(kX86InstSub, zsp, imm(stackOffset));
	}
	else
	{
		stackPos = -(this->_peMovStackSize + this->_peAdjustStackSize);
		//if (this->_pePushPop)
			//stackPos += IntUtil::bitCount(preservedGP) * sizeof(sysint_t);
	}

	// --------------------------------------------------------------------------
	// [Save Xmm - MovDqa/MovDqu]
	// --------------------------------------------------------------------------

	if (preservedXMM)
	{
		for (i = 0, mask = 1; i < kX86RegNumXmm; ++i, mask <<= 1)
		{
			if (preservedXMM & mask)
			{
				x86Compiler->emit(_movDqInstCode, dqword_ptr(zsp, stackPos), xmm(i));
				stackPos += 16;
			}
		}
	}

	// --------------------------------------------------------------------------
	// [Save Mm - MovQ]
	// --------------------------------------------------------------------------

	if (preservedMM)
	{
		for (i = 0, mask = 1; i < 8; ++i, mask <<= 1)
		{
			if (preservedMM & mask)
			{
				x86Compiler->emit(kX86InstMovQ, qword_ptr(zsp, stackPos), mm(i));
				stackPos += 8;
			}
		}
	}

	// --------------------------------------------------------------------------
	// [Save Gp - Mov]
	// --------------------------------------------------------------------------

	if (preservedGP && !this->hasFuncFlag(kX86FuncFlagPushPop))
	{
		for (i = 0, mask = 1; i < kX86RegNumGp; ++i, mask <<= 1)
		{
			if (preservedGP & mask)
			{
				x86Compiler->emit(kX86InstMov, sysint_ptr(zsp, stackPos), gpz(i));
				stackPos += sizeof(sysint_t);
			}
		}
	}

	// --------------------------------------------------------------------------
	// [...]
	// --------------------------------------------------------------------------

	if (x86Compiler->getLogger())
		x86Compiler->comment("Body");
}

void X86CompilerFuncDecl::_emitEpilog(CompilerContext &cc)
{
	X86Compiler *x86Compiler = this->getCompiler();

	const X86CpuInfo *cpuInfo = X86CpuInfo::getGlobal();

	// --------------------------------------------------------------------------
	// [Init]
	// --------------------------------------------------------------------------

	uint32_t i, mask;
	uint32_t preservedGP = this->_gpModifiedAndPreserved;
	uint32_t preservedMM = this->_mmModifiedAndPreserved;
	uint32_t preservedXMM = this->_xmmModifiedAndPreserved;

	int32_t stackOffset = this->_getRequiredStackOffset();
	int32_t stackPos;

	if (this->isEspAdjusted()) 
		stackPos = this->_memStackSize16 + this->_funcCallStackSize;
	else
		stackPos = -(this->_peMovStackSize + this->_peAdjustStackSize);

	// --------------------------------------------------------------------------
	// [Epilog]
	// --------------------------------------------------------------------------

	if (x86Compiler->getLogger())
		x86Compiler->comment("Epilog");

	// --------------------------------------------------------------------------
	// [Restore Xmm - MovDqa/ModDqu]
	// --------------------------------------------------------------------------

	if (preservedXMM)
	{
		for (i = 0, mask = 1; i < kX86RegNumXmm; ++i, mask <<= 1)
		{
			if (preservedXMM & mask)
			{
				x86Compiler->emit(_movDqInstCode, xmm(i), dqword_ptr(zsp, stackPos));
				stackPos += 16;
			}
		}
	}

	// --------------------------------------------------------------------------
	// [Restore Mm - MovQ]
	// --------------------------------------------------------------------------

	if (preservedMM)
	{
		for (i = 0, mask = 1; i < 8; ++i, mask <<= 1)
		{
			if (preservedMM & mask)
			{
				x86Compiler->emit(kX86InstMovQ, mm(i), qword_ptr(zsp, stackPos));
				stackPos += 8;
			}
		}
	}

	// --------------------------------------------------------------------------
	// [Restore Gp - Mov]
	// --------------------------------------------------------------------------

	if (preservedGP && !this->hasFuncFlag(kX86FuncFlagPushPop))
	{
		for (i = 0, mask = 1; i < kX86RegNumGp; ++i, mask <<= 1)
		{
			if (preservedGP & mask)
			{
				x86Compiler->emit(kX86InstMov, gpz(i), sysint_ptr(zsp, stackPos));
				stackPos += sizeof(sysint_t);
			}
		}
	}

	// --------------------------------------------------------------------------
	// [Adjust Stack]
	// --------------------------------------------------------------------------

	if (this->isEspAdjusted() && stackOffset)
		x86Compiler->emit(kX86InstAdd, zsp, imm(stackOffset));

	// --------------------------------------------------------------------------
	// [Restore Gp - Push/Pop]
	// --------------------------------------------------------------------------

	if (preservedGP && this->hasFuncFlag(kX86FuncFlagPushPop))
	{
		for (i = kX86RegNumGp - 1, mask = 1 << i; static_cast<int32_t>(i) >= 0; --i, mask >>= 1)
			if (preservedGP & mask)
				x86Compiler->emit(kX86InstPop, gpz(i));
	}

	// --------------------------------------------------------------------------
	// [Emms]
	// --------------------------------------------------------------------------

	if (this->hasFuncFlag(kX86FuncFlagEmitEmms)) 
		x86Compiler->emit(kX86InstEmms);

	// --------------------------------------------------------------------------
	// [MFence/SFence/LFence]
	// --------------------------------------------------------------------------

	if (this->hasFuncFlag(kX86FuncFlagEmitSFence) && this->hasFuncFlag(kX86FuncFlagEmitLFence))
		x86Compiler->emit(kX86InstMFence);
	else if (this->hasFuncFlag(kX86FuncFlagEmitSFence))
		x86Compiler->emit(kX86InstSFence);
	else if (this->hasFuncFlag(kX86FuncFlagEmitLFence))
		x86Compiler->emit(kX86InstLFence);

	// --------------------------------------------------------------------------
	// [Epilog]
	// --------------------------------------------------------------------------

	// Emit standard epilog leave code (if needed).
	if (!this->isNaked())
	{
		// AMD seems to prefer LEAVE instead of MOV/POP sequence.
		if (cpuInfo->getVendorId() == kCpuAmd)
			x86Compiler->emit(kX86InstLeave);
		else
		{
			x86Compiler->emit(kX86InstMov, zsp, zbp);
			x86Compiler->emit(kX86InstPop, zbp);
		}
	}

	// Emit return.
	if (this->_x86Decl.getCalleePopsStack())
		x86Compiler->emit(kX86InstRet, imm(static_cast<int16_t>(this->_x86Decl.getArgumentsStackSize())));
	else
		x86Compiler->emit(kX86InstRet);
}

// ============================================================================
// [AsmJit::X86CompilerFuncDecl - Function-Call]
// ============================================================================

void X86CompilerFuncDecl::reserveStackForFunctionCall(int32_t size)
{
	size = IntUtil::align(size, 16);
	if (size > this->_funcCallStackSize)
		this->_funcCallStackSize = size;
	this->setFuncFlag(kFuncFlagIsCaller);
}

// ============================================================================
// [AsmJit::X86CompilerFuncEnd - Construction / Destruction]
// ============================================================================

X86CompilerFuncEnd::X86CompilerFuncEnd(X86Compiler *x86Compiler, X86CompilerFuncDecl *func) : CompilerFuncEnd(x86Compiler, func)
{
}

X86CompilerFuncEnd::~X86CompilerFuncEnd()
{
}

// ============================================================================
// [AsmJit::X86CompilerFuncEnd - Interface]
// ============================================================================

void X86CompilerFuncEnd::prepare(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	this->_offset = x86Context._currentOffset++;
}

CompilerItem *X86CompilerFuncEnd::translate(CompilerContext &cc)
{
	this->_isTranslated = true;
	return nullptr;
}

// ============================================================================
// [AsmJit::X86CompilerFuncRet - Construction / Destruction]
// ============================================================================

X86CompilerFuncRet::X86CompilerFuncRet(X86Compiler *x86Compiler, X86CompilerFuncDecl *func, const Operand *first, const Operand *second) : CompilerFuncRet(x86Compiler, func, first, second)
{
/*
	// TODO:?

	// Check whether the return value is compatible.
	uint32_t retValType = function->_x86Decl.getReturnType();
	bool valid = false;

	switch (retValType)
	{
		case kX86VarTypeGpd:
		case kX86VarTypeGpq:
			if ((this->_ret[0].isVar() && reinterpret_cast<const Var &>(this->_ret[0]).isGpVar()) || this->_ret[0].isImm())
				valid = true;
			break;

		case kX86VarTypeX87:
		case kX86VarTypeX87SS:
		case kX86VarTypeX87SD:
			if (this->_ret[0].isVar() && (reinterpret_cast<const Var &>(this->_ret[0]).isX87Var() || reinterpret_cast<const Var &>(this->_ret[0]).isXmmVar()))
				valid = true;
			break;

		case kX86VarTypeMm:
			break;

		case kVarTypeInvalid:
			if (this->_ret[0].isNone() && this->_ret[1].isNone())
				valid = true;
	}

	// Incompatible return value.
	if (!valid)
		c->setError(kErrorIncompatibleReturnType);
*/
}

X86CompilerFuncRet::~X86CompilerFuncRet()
{
}

// ============================================================================
// [AsmJit::X86CompilerFuncRet - Interface]
// ============================================================================

void X86CompilerFuncRet::prepare(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = x86Context.getCompiler();

	uint32_t retValType = this->getFunc()->_x86Decl.getReturnType();
	this->_offset = x86Context._currentOffset;

	if (retValType != kVarTypeInvalid)
	{
		uint32_t i;
		for (i = 0; i < 2; ++i)
		{
			Operand &o = this->_ret[i];

			if (o.isVar())
			{
				ASMJIT_ASSERT(o.getId() != kInvalidValue);
				X86CompilerVar *cv = x86Compiler->_getVar(o.getId());
				ASMJIT_ASSERT(cv);

				// First item (begin of variable scope).
				if (!cv->firstItem)
					cv->firstItem = this;

				// Last item (end of variable scope).
				cv->lastItem = this;

				if (cv->workOffset == _offset)
					continue;
				if (!x86Context._isActive(cv))
					x86Context._addActive(cv);

				cv->workOffset = this->_offset;
				++cv->regReadCount;

				if (X86Util::isVarTypeInt(cv->getType()) && X86Util::isVarTypeInt(retValType))
					x86Context._newRegisterHomeIndex(cv, !i ? kX86RegIndexEax : kX86RegIndexEdx);
			}
		}
	}

	++x86Context._currentOffset;
}

CompilerItem *X86CompilerFuncRet::translate(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = x86Context.getCompiler();

	// Check whether the return value is compatible.
	uint32_t retValType = this->getFunc()->getDecl()->getReturnType();
	uint32_t i;

	switch (retValType)
	{
		case kX86VarTypeGpd:
		case kX86VarTypeGpq:
			for (i = 0; i < 2; ++i)
			{
				uint32_t dstIndex = !i ? kX86RegIndexEax : kX86RegIndexEdx;
				uint32_t srcIndex;

				if (this->_ret[i].isVar())
				{
					if (reinterpret_cast<const Var &>(this->_ret[i]).isGpVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srcIndex = cv->regIndex;
						if (srcIndex == kRegIndexInvalid)
							x86Compiler->emit(kX86InstMov, gpz(dstIndex), x86Context._getVarMem(cv));
						else if (dstIndex != srcIndex)
							x86Compiler->emit(kX86InstMov, gpz(dstIndex), gpz(srcIndex));
					}
				}
				else if (this->_ret[i].isImm())
					x86Compiler->emit(kX86InstMov, gpz(dstIndex), this->_ret[i]);
			}
			break;

		case kX86VarTypeX87:
		case kX86VarTypeX87SS:
		case kX86VarTypeX87SD:
			// There is case that we need to return two values (Unix-ABI specific):
			// - FLD #2
			//-  FLD #1
			i = 2;
			do
			{
				--i;
				uint32_t srci;

				if (this->_ret[i].isVar())
				{
					if (reinterpret_cast<const Var &>(this->_ret[i]).isX87Var())
					{
						// TODO: X87 Support.
					}
					else if (reinterpret_cast<const Var &>(this->_ret[i]).isXmmVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srci = cv->regIndex;
						if (srci != kRegIndexInvalid)
							x86Context.saveXmmVar(cv);

						switch (cv->getType())
						{
							case kX86VarTypeXmmSS:
							case kX86VarTypeXmmPS:
								x86Compiler->emit(kX86InstFLd, _BaseVarMem(reinterpret_cast<Var &>(this->_ret[i]), 4));
								break;
							case kX86VarTypeXmmSD:
							case kX86VarTypeXmmPD:
								x86Compiler->emit(kX86InstFLd, _BaseVarMem(reinterpret_cast<Var &>(this->_ret[i]), 8));
						}
					}
				}
			} while (i);
			break;

		case kX86VarTypeMm:
			for (i = 0; i < 2; ++i)
			{
				uint32_t dsti = i;
				uint32_t srci;

				if (this->_ret[i].isVar())
				{
					if (reinterpret_cast<const Var &>(this->_ret[i]).isGpVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srci = cv->regIndex;
						uint32_t inst = this->_ret[i].isRegType(kX86RegTypeGpq) ? kX86InstMovQ : kX86InstMovD;

						if (srci == kRegIndexInvalid)
							x86Compiler->emit(inst, mm(dsti), x86Context._getVarMem(cv));
						else
#ifdef ASMJIT_X86
							x86Compiler->emit(inst, mm(dsti), gpd(srci));
#else
							x86Compiler->emit(inst, mm(dsti), this->_ret[i].isRegType(kX86RegTypeGpq) ? gpq(srci) : gpd(srci));
#endif
					}
					else if (reinterpret_cast<const Var &>(this->_ret[i]).isMmVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srci = cv->regIndex;
						uint32_t inst = kX86InstMovQ;

						if (srci == kRegIndexInvalid)
							x86Compiler->emit(inst, mm(dsti), x86Context._getVarMem(cv));
						else if (dsti != srci)
							x86Compiler->emit(inst, mm(dsti), mm(srci));
					}
					else if (reinterpret_cast<const Var &>(this->_ret[i]).isXmmVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srci = cv->regIndex;
						uint32_t inst = kX86InstMovQ;
						if (reinterpret_cast<const Var &>(this->_ret[i]).getVarType() == kX86VarTypeXmmSS)
							inst = kX86InstMovD;

						if (srci == kRegIndexInvalid)
							x86Compiler->emit(inst, mm(dsti), x86Context._getVarMem(cv));
						else
							x86Compiler->emit(inst, mm(dsti), xmm(srci));
					}
				}
			}
			break;

		case kX86VarTypeXmm:
		case kX86VarTypeXmmPS:
		case kX86VarTypeXmmPD:
			for (i = 0; i < 2; ++i)
			{
				uint32_t dsti = i;
				uint32_t srci;

				if (this->_ret[i].isVar())
				{
					if (reinterpret_cast<const Var &>(this->_ret[i]).isGpVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srci = cv->regIndex;
						uint32_t inst = this->_ret[i].isRegType(kX86RegTypeGpq) ? kX86InstMovQ : kX86InstMovD;

						if (srci == kRegIndexInvalid)
							x86Compiler->emit(inst, xmm(dsti), x86Context._getVarMem(cv));
						else
#ifdef ASMJIT_X86
							x86Compiler->emit(inst, xmm(dsti), gpd(srci));
#else
							x86Compiler->emit(inst, xmm(dsti), this->_ret[i].isRegType(kX86RegTypeGpq) ? gpq(srci) : gpd(srci));
#endif
					}
					else if (reinterpret_cast<const Var &>(this->_ret[i]).isX87Var())
					{
						// TODO: X87 Support.
					}
					else if (reinterpret_cast<const Var &>(this->_ret[i]).isMmVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srci = cv->regIndex;
						if (srci == kRegIndexInvalid)
							x86Compiler->emit(kX86InstMovQ, xmm(dsti), x86Context._getVarMem(cv));
						else
							x86Compiler->emit(kX86InstMovQ, xmm(dsti), mm(srci));
					}
					else if (reinterpret_cast<const Var &>(this->_ret[i]).isXmmVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srci = cv->regIndex;
						if (srci == kRegIndexInvalid)
							x86Compiler->emit(kX86InstMovDQA, xmm(dsti), x86Context._getVarMem(cv));
						else if (dsti != srci)
							x86Compiler->emit(kX86InstMovDQA, xmm(dsti), xmm(srci));
					}
				}
			}
			break;

		case kX86VarTypeXmmSS:
			for (i = 0; i < 2; ++i)
			{
				uint32_t dsti = i;
				uint32_t srci;

				if (this->_ret[i].isVar())
				{
					if (reinterpret_cast<const Var &>(this->_ret[i]).isX87Var())
					{
						// TODO: X87 Support.
					}
					else if (reinterpret_cast<const Var &>(this->_ret[i]).isXmmVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srci = cv->regIndex;
						switch (cv->getType())
						{
							case kX86VarTypeXmm:
								if (srci == kRegIndexInvalid)
									x86Compiler->emit(kX86InstMovDQA, xmm(dsti), x86Context._getVarMem(cv));
								else if (dsti != srci)
									x86Compiler->emit(kX86InstMovDQA, xmm(dsti), xmm(srci));
								break;
							case kX86VarTypeXmmSS:
							case kX86VarTypeXmmPS:
								if (srci == kRegIndexInvalid)
									x86Compiler->emit(kX86InstMovSS, xmm(dsti), x86Context._getVarMem(cv));
								else
									x86Compiler->emit(kX86InstMovSS, xmm(dsti), xmm(srci));
								break;
							case kX86VarTypeXmmSD:
							case kX86VarTypeXmmPD:
								if (srci == kRegIndexInvalid)
									x86Compiler->emit(kX86InstCvtSD2SS, xmm(dsti), x86Context._getVarMem(cv));
								else if (dsti != srci)
									x86Compiler->emit(kX86InstCvtSD2SS, xmm(dsti), xmm(srci));
						}
					}
				}
			}
			break;

		case kX86VarTypeXmmSD:
			for (i = 0; i < 2; ++i)
			{
				uint32_t dsti = i;
				uint32_t srci;

				if (this->_ret[i].isVar())
				{
					if (reinterpret_cast<const Var &>(this->_ret[i]).isX87Var())
					{
						// TODO: X87 Support.
					}
					else if (reinterpret_cast<const Var &>(this->_ret[i]).isXmmVar())
					{
						X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
						ASMJIT_ASSERT(cv);

						srci = cv->regIndex;
						switch (cv->getType())
						{
							case kX86VarTypeXmm:
								if (srci == kRegIndexInvalid)
									x86Compiler->emit(kX86InstMovDQA, xmm(dsti), x86Context._getVarMem(cv));
								else if (dsti != srci)
									x86Compiler->emit(kX86InstMovDQA, xmm(dsti), xmm(srci));
								break;
							case kX86VarTypeXmmSS:
							case kX86VarTypeXmmPS:
								if (srci == kRegIndexInvalid)
									x86Compiler->emit(kX86InstCvtSS2SD, xmm(dsti), x86Context._getVarMem(cv));
								else
									x86Compiler->emit(kX86InstCvtSS2SD, xmm(dsti), xmm(srci));
								break;
							case kX86VarTypeXmmSD:
							case kX86VarTypeXmmPD:
								if (srci == kRegIndexInvalid)
									x86Compiler->emit(kX86InstMovSD, xmm(dsti), x86Context._getVarMem(cv));
								else
									x86Compiler->emit(kX86InstMovSD, xmm(dsti), xmm(srci));
						}
					}
				}
			}
	}

	if (this->mustEmitJump())
		x86Context._isUnreachable = 1;

	for (i = 0; i < 2; ++i)
	{
		if (this->_ret[i].isVar())
		{
			X86CompilerVar *cv = x86Compiler->_getVar(this->_ret[i].getId());
			x86Context._unuseVarOnEndOfScope(this, cv);
		}
	}

	return this->translated();
}

void X86CompilerFuncRet::emit(Assembler &a)
{
	X86Assembler &x86Asm = static_cast<X86Assembler &>(a);

	if (this->mustEmitJump())
		x86Asm.jmp(this->getFunc()->getExitLabel());
}

// ============================================================================
// [AsmJit::X86CompilerFuncRet - Misc]
// ============================================================================

int X86CompilerFuncRet::getMaxSize() const
{
	return this->mustEmitJump() ? 15 : 0;
}

// ============================================================================
// [AsmJit::X86CompilerFuncCall - Construction / Destruction]
// ============================================================================

X86CompilerFuncCall::X86CompilerFuncCall(X86Compiler *x86Compiler, X86CompilerFuncDecl *caller, const Operand *target) : CompilerFuncCall(x86Compiler, caller, target), _gpParams(0), _mmParams(0), _xmmParams(0),
	_variablesCount(0), _variables(nullptr)
{
}

X86CompilerFuncCall::~X86CompilerFuncCall()
{
	memset(this->_argumentToVarRecord, 0, sizeof(VarCallRecord *) * kFuncArgsMax);
}

// ============================================================================
// [AsmJit::X86CompilerFuncCall - Interface]
// ============================================================================

void X86CompilerFuncCall::prepare(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = this->getCompiler();

	// Prepare is similar to X86CompilerInst::prepare(). We collect unique variables
	// and update statistics, but we don't use standard alloc/free register calls.
	//
	// The calling function is also unique in variable allocator point of view,
	// because we need to alloc some variables that may be destroyed be the 
	// callee (okay, may not, but this is not guaranteed).
	this->_offset = x86Context._currentOffset;

	// Tell EFunction that another function will be called inside. It needs this
	// information to reserve stack for the call and to mark esp adjustable.
	this->getCaller()->reserveStackForFunctionCall(static_cast<int32_t>(this->_x86Decl.getArgumentsStackSize()));

	uint32_t i;
	uint32_t argumentsCount = this->_x86Decl.getArgumentsCount();
	uint32_t operandsCount = argumentsCount;
	uint32_t variablesCount = 0;

	// Create registers used as arguments mask.
	for (i = 0; i < argumentsCount; ++i)
	{
		const FuncArg &fArg = this->_x86Decl.getArguments()[i];

		if (fArg.hasRegIndex())
		{
			switch (fArg.getVarType())
			{
				case kX86VarTypeGpd:
				case kX86VarTypeGpq:
					this->_gpParams |= IntUtil::maskFromIndex(fArg.getRegIndex());
					break;
				case kX86VarTypeMm:
					this->_mmParams |= IntUtil::maskFromIndex(fArg.getRegIndex());
					break;
				case kX86VarTypeXmm:
				case kX86VarTypeXmmSS:
				case kX86VarTypeXmmPS:
				case kX86VarTypeXmmSD:
				case kX86VarTypeXmmPD:
					this->_xmmParams |= IntUtil::maskFromIndex(fArg.getRegIndex());
					break;
				default:
					ASMJIT_ASSERT(0);
			}
		}
		else
			x86Context.getFunc()->setFuncFlag(kX86FuncFlagIsEspAdjusted);
	}

	// Call address.
	++operandsCount;

	// The first and the second return value.
	if (!this->_ret[0].isNone())
		++operandsCount;
	if (!this->_ret[1].isNone())
		++operandsCount;

#define __GET_VARIABLE(__vardata__) \
{ \
	X86CompilerVar *_candidate = __vardata__; \
\
	for (var = cur; ; ) \
	{ \
		if (var == _variables) \
		{ \
			var = cur++; \
			var->vdata = _candidate; \
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

	for (i = 0; i < operandsCount; ++i)
	{
		Operand& o = (i < argumentsCount)  ? this->_args[i] : (i == argumentsCount ? this->_target : this->_ret[i - argumentsCount - 1]);

		if (o.isVar())
		{
			ASMJIT_ASSERT(o.getId() != kInvalidValue);
			X86CompilerVar *cv = x86Compiler->_getVar(o.getId());
			ASMJIT_ASSERT(cv);

			if (cv->workOffset == this->_offset)
				continue;
			if (!x86Context._isActive(cv))
				x86Context._addActive(cv);

			cv->workOffset = this->_offset;
			++variablesCount;
		}
		else if (o.isMem())
		{
			if ((o.getId() & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *cv = x86Compiler->_getVar(o.getId());
				ASMJIT_ASSERT(cv);

				x86Context._markMemoryUsed(cv);
				if (!x86Context._isActive(cv))
					x86Context._addActive(cv);

				continue;
			}
			else if ((o._mem.base & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *cv = x86Compiler->_getVar(o._mem.base);
				ASMJIT_ASSERT(cv);

				if (cv->workOffset == this->_offset)
					continue;
				if (!x86Context._isActive(cv))
					x86Context._addActive(cv);

				cv->workOffset = this->_offset;
				++variablesCount;
			}

			if ((o._mem.index & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *cv = x86Compiler->_getVar(o._mem.index);
				ASMJIT_ASSERT(cv);

				if (cv->workOffset == this->_offset)
					continue;
				if (!x86Context._isActive(cv))
					x86Context._addActive(cv);

				cv->workOffset = this->_offset;
				++variablesCount;
			}
		}
	}

	// Traverse all active variables and set their funcCall pointer to this
	// call. This information can be used to choose between the preserved-first
	// and preserved-last register allocation.
	if (x86Context._active)
	{
		X86CompilerVar *first = static_cast<X86CompilerVar *>(x86Context._active);
		X86CompilerVar *active = first;
		do
		{
			if (!active->funcCall)
				active->funcCall = this;
			active = active->nextActive;
		} while (active != first);
	}

	if (!variablesCount)
	{
		++x86Context._currentOffset;
		return;
	}

	this->_variables = reinterpret_cast<VarCallRecord *>(x86Compiler->getZoneMemory().alloc(sizeof(VarCallRecord) * variablesCount));
	if (!this->_variables)
	{
		x86Compiler->setError(kErrorNoHeapMemory);
		++x86Context._currentOffset;
		return;
	}

	this->_variablesCount = variablesCount;
	memset(this->_variables, 0, sizeof(VarCallRecord) * variablesCount);

	VarCallRecord *cur = this->_variables;
	VarCallRecord *var = nullptr;

	for (i = 0; i < operandsCount; ++i)
	{
		Operand &o = (i < argumentsCount) ? this->_args[i] : (i == argumentsCount ? this->_target : this->_ret[i - argumentsCount - 1]);

		if (o.isVar())
		{
			X86CompilerVar *cv = x86Compiler->_getVar(o.getId());
			ASMJIT_ASSERT(cv);

			__GET_VARIABLE(cv);
			this->_argumentToVarRecord[i] = var;

			if (i < argumentsCount)
			{
				const FuncArg &fArg = this->_x86Decl.getArgument(i);

				if (fArg.hasRegIndex())
				{
					x86Context._newRegisterHomeIndex(cv, fArg.getRegIndex());

					switch (fArg.getVarType())
					{
						case kX86VarTypeGpd:
						case kX86VarTypeGpq:
							var->flags |= VarCallRecord::kFlagInGp;
							++var->inCount;
							break;

						case kX86VarTypeMm:
							var->flags |= VarCallRecord::kFlagInMm;
							++var->inCount;
							break;

						case kX86VarTypeXmm:
						case kX86VarTypeXmmSS:
						case kX86VarTypeXmmPS:
						case kX86VarTypeXmmSD:
						case kX86VarTypeXmmPD:
							var->flags |= VarCallRecord::kFlagInXmm;
							++var->inCount;
							break;

						default:
							ASMJIT_ASSERT(0);
					}
				}
				else
					++var->inCount;

				++cv->regReadCount;
			}
			else if (i == argumentsCount)
			{
				uint32_t mask = ~this->_x86Decl.getGpPreservedMask() & ~this->_x86Decl.getGpArgumentsMask() & IntUtil::maskUpToIndex(kX86RegNumGp);

				x86Context._newRegisterHomeIndex(cv, IntUtil::findFirstBit(mask));
				x86Context._newRegisterHomeMask(cv, mask);

				var->flags |= VarCallRecord::kFlagCallReg;
				++cv->regReadCount;
			}
			else
			{
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
					case kX86VarTypeGpq:
						if (i == argumentsCount+1)
							var->flags |= VarCallRecord::kFlagOutEax;
						else
							var->flags |= VarCallRecord::kFlagOutEdx;
						break;

					case kX86VarTypeX87:
					case kX86VarTypeX87SS:
					case kX86VarTypeX87SD:
#ifdef ASMJIT_X86
						if (i == argumentsCount + 1)
							var->flags |= VarCallRecord::kFlagOutSt0;
						else
							var->flags |= VarCallRecord::kFlagOutSt1;
#else
						if (i == argumentsCount + 1)
							var->flags |= VarCallRecord::kFlagOutXmm0;
						else
							var->flags |= VarCallRecord::kFlagOutXmm1;
#endif
						break;

					case kX86VarTypeMm:
						var->flags |= VarCallRecord::kFlagOutMm0;
						break;

					case kX86VarTypeXmm:
					case kX86VarTypeXmmPS:
					case kX86VarTypeXmmPD:
						if (i == argumentsCount+1)
							var->flags |= VarCallRecord::kFlagOutXmm0;
						else
							var->flags |= VarCallRecord::kFlagOutXmm1;
						break;

					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmSD:
#ifdef ASMJIT_X86
						if (i == argumentsCount + 1)
							var->flags |= VarCallRecord::kFlagOutSt0;
						else
							var->flags |= VarCallRecord::kFlagOutSt1;
#else
						if (i == argumentsCount + 1)
							var->flags |= VarCallRecord::kFlagOutXmm0;
						else
							var->flags |= VarCallRecord::kFlagOutXmm1;
#endif
						break;

					default:
						ASMJIT_ASSERT(0);
				}

				++cv->regWriteCount;
			}
		}
		else if (o.isMem())
		{
			ASMJIT_ASSERT(i == argumentsCount);

			if ((o.getId() & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *cv = x86Compiler->_getVar(o.getId());
				ASMJIT_ASSERT(cv);

				++cv->memReadCount;
			}
			else if ((o._mem.base & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *cv = x86Compiler->_getVar(reinterpret_cast<Mem &>(o).getBase());
				ASMJIT_ASSERT(cv);

				++cv->regReadCount;

				__GET_VARIABLE(cv);
				var->flags |= VarCallRecord::kFlagCallReg | VarCallRecord::kFlagCallMem;
			}

			if ((o._mem.index & kOperandIdTypeMask) == kOperandIdTypeVar)
			{
				X86CompilerVar *cv = x86Compiler->_getVar(reinterpret_cast<Mem &>(o).getIndex());
				ASMJIT_ASSERT(cv);

				++cv->regReadCount;

				__GET_VARIABLE(cv);
				var->flags |= VarCallRecord::kFlagCallReg | VarCallRecord::kFlagCallMem;
			}
		}
	}

	// Traverse all variables and update firstItem / lastItem. This
	// function is called from iterator that scans items using forward
	// direction so we can use this knowledge to optimize the process.
	//
	// Same code is in X86CompilerInst::prepare().
	for (i = 0; i < this->_variablesCount; ++i)
	{
		X86CompilerVar *v = this->_variables[i].vdata;

		// First item (begin of variable scope).
		if (!v->firstItem)
			v->firstItem = this;

		// Last item (end of variable scope).
		v->lastItem = this;
	}

	++x86Context._currentOffset;

#undef __GET_VARIABLE
}

CompilerItem *X86CompilerFuncCall::translate(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = x86Context.getCompiler();

	uint32_t i;
	uint32_t preserved, mask;

	uint32_t temporaryGpReg;
	uint32_t temporaryXmmReg;

	uint32_t offset = x86Context._currentOffset;

	// Constants.
	const FuncArg *targs = this->_x86Decl.getArguments();

	uint32_t argumentsCount = this->_x86Decl.getArgumentsCount();
	uint32_t variablesCount = this->_variablesCount;

	// Processed arguments kFuncArgsMax.
	uint8_t processed[kFuncArgsMax] = { 0 };

	x86Compiler->comment("Call");

	// These variables are used by the instruction so we set current offset
	// to their work offsets -> The getSpillCandidate() method never returns 
	// the variable used by this instruction.
	for (i = 0; i < variablesCount; ++i)
	{
		this->_variables[i].vdata->workOffset = offset;

		// Init back-reference to VarCallRecord.
		this->_variables[i].vdata->tPtr = &this->_variables[i];
	}

	// --------------------------------------------------------------------------
	// STEP 1:
	//
	// Spill variables which are not used by the function call and have to
	// be destroyed. These registers may be used by callee.
	// --------------------------------------------------------------------------

	preserved = this->_x86Decl.getGpPreservedMask();
	for (i = 0, mask = 1; i < kX86RegNumGp; ++i, mask <<= 1)
	{
		X86CompilerVar *cv = x86Context._x86State.gp[i];
		if (cv && cv->workOffset != offset && !(preserved & mask))
			x86Context.spillGpVar(cv);
	}

	preserved = this->_x86Decl.getMmPreservedMask();
	for (i = 0, mask = 1; i < kX86RegNumMm; ++i, mask <<= 1)
	{
		X86CompilerVar *cv = x86Context._x86State.mm[i];
		if (cv && cv->workOffset != offset && !(preserved & mask))
			x86Context.spillMmVar(cv);
	}

	preserved = this->_x86Decl.getXmmPreservedMask();
	for (i = 0, mask = 1; i < kX86RegNumXmm; ++i, mask <<= 1)
	{
		X86CompilerVar *cv = x86Context._x86State.xmm[i];
		if (cv && cv->workOffset != offset && !(preserved & mask))
			x86Context.spillXmmVar(cv);
	}

	// --------------------------------------------------------------------------
	// STEP 2:
	//
	// Move all arguments to the stack which all already in registers.
	// --------------------------------------------------------------------------

	for (i = 0; i < argumentsCount; ++i)
	{
		if (processed[i])
			continue;

		const FuncArg &argType = targs[i];
		if (argType.hasRegIndex())
			continue;

		Operand &operand = this->_args[i];

		if (operand.isVar())
		{
			VarCallRecord *rec = this->_argumentToVarRecord[i];
			X86CompilerVar *cv = x86Compiler->_getVar(operand.getId());

			if (cv->regIndex != kRegIndexInvalid)
			{
				this->_moveAllocatedVariableToStack(cc, cv, argType);

				++rec->inDone;
				processed[i] = true;
			}
		}
	}

	// --------------------------------------------------------------------------
	// STEP 3:
	//
	// Spill all non-preserved variables we moved to stack in STEP #2.
	// --------------------------------------------------------------------------

	for (i = 0; i < argumentsCount; ++i)
	{
		VarCallRecord *rec = this->_argumentToVarRecord[i];
		if (!rec || processed[i])
			continue;

		if (rec->inDone >= rec->inCount)
		{
			X86CompilerVar *cv = rec->vdata;
			if (cv->regIndex == kRegIndexInvalid)
				continue;

			if (rec->outCount)
				// Variable will be rewritten by function return value, it's not needed
				// to spill it. It will be allocated again by X86CompilerFuncCall.
				x86Context.unuseVar(rec->vdata, kVarStateUnused);
			else
			{
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
					case kX86VarTypeGpq:
						if (!(this->_x86Decl.getGpPreservedMask() & IntUtil::maskFromIndex(cv->regIndex)))
							x86Context.spillGpVar(cv);
						break;
					case kX86VarTypeMm:
						if (!(this->_x86Decl.getMmPreservedMask() & IntUtil::maskFromIndex(cv->regIndex)))
							x86Context.spillMmVar(cv);
						break;
					case kX86VarTypeXmm:
					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmSD:
					case kX86VarTypeXmmPS:
					case kX86VarTypeXmmPD:
						if (!(this->_x86Decl.getXmmPreservedMask() & IntUtil::maskFromIndex(cv->regIndex)))
							x86Context.spillXmmVar(cv);
				}
			}
		}
	}

	// --------------------------------------------------------------------------
	// STEP 4:
	//
	// Get temporary register that we can use to pass input function arguments.
	// Now it's safe to do, because the non-needed variables should be spilled.
	// --------------------------------------------------------------------------

	temporaryGpReg = this->_findTemporaryGpRegister(cc);
	temporaryXmmReg = this->_findTemporaryXmmRegister(cc);

	// If failed to get temporary register then we need just to pick one.
	if (temporaryGpReg == kRegIndexInvalid)
	{
		// TODO.
	}
	if (temporaryXmmReg == kRegIndexInvalid)
	{
		// TODO.
	}

	// --------------------------------------------------------------------------
	// STEP 5:
	//
	// Move all remaining arguments to the stack (we can use temporary register).
	// or allocate it to the primary register. Also move immediates.
	// --------------------------------------------------------------------------

	for (i = 0; i < argumentsCount; ++i)
	{
		if (processed[i])
			continue;

		const FuncArg &argType = targs[i];

		if (argType.hasRegIndex())
			continue;

		Operand &operand = this->_args[i];

		if (operand.isVar())
		{
			VarCallRecord *rec = this->_argumentToVarRecord[i];
			X86CompilerVar *cv = x86Compiler->_getVar(operand.getId());

			this->_moveSpilledVariableToStack(cc, cv, argType, temporaryGpReg, temporaryXmmReg);

			++rec->inDone;
			processed[i] = true;
		}
		else if (operand.isImm())
		{
			// TODO.
		}
	}

	// --------------------------------------------------------------------------
	// STEP 6:
	//
	// Allocate arguments to registers.
	// --------------------------------------------------------------------------

	bool didWork;

	do
	{
		didWork = false;

		for (i = 0; i < argumentsCount; ++i)
		{
			if (processed[i])
				continue;

			VarCallRecord *rsrc = this->_argumentToVarRecord[i];

			Operand &osrc = this->_args[i];
			ASMJIT_ASSERT(osrc.isVar());
			X86CompilerVar *vsrc = x86Compiler->_getVar(osrc.getId());

			const FuncArg &srcArgType = targs[i];
			X86CompilerVar *vdst = this->_getOverlappingVariable(cc, srcArgType);

			if (vsrc == vdst)
			{
				++rsrc->inDone;
				processed[i] = true;

				didWork = true;
				continue;
			}
			else if (vdst)
			{
				VarCallRecord *rdst = reinterpret_cast<VarCallRecord *>(vdst->tPtr);

				if (!rdst)
				{
					x86Context.spillVar(vdst);
					vdst = nullptr;
				}
				else if (rdst->inDone >= rdst->inCount && !(rdst->flags & VarCallRecord::kFlagCallReg))
				{
					// Safe to spill.
					if (rdst->outCount || vdst->lastItem == this)
						x86Context.unuseVar(vdst, kVarStateUnused);
					else
						x86Context.spillVar(vdst);
					vdst = nullptr;
				}
				else
				{
					uint32_t x = this->_x86Decl.findArgumentByRegCode(X86Util::getRegCodeFromVarType(vsrc->getType(), vsrc->regIndex));
					bool doSpill = true;

					if (vdst->getClass() & kX86VarClassGp)
					{
						// Try to emit mov to register which is possible for call() operand.
						if (x == kInvalidValue && (rdst->flags & VarCallRecord::kFlagCallReg))
						{
							uint32_t rIndex;
							uint32_t rBit;

							// The mask which contains registers which are not-preserved
							// (these that might be clobbered by the callee) and which are
							// not used to pass function arguments. Each register contained
							// in this mask is ideal to be used by call() instruction.
							uint32_t possibleMask = ~this->_x86Decl.getGpPreservedMask() & ~this->_x86Decl.getGpArgumentsMask() & IntUtil::maskUpToIndex(kX86RegNumGp);

							if (possibleMask)
							{
								for (rIndex = 0, rBit = 1; rIndex < kX86RegNumGp; ++rIndex, rBit <<= 1)
								{
									if (possibleMask & rBit)
									{
										if (!x86Context._x86State.gp[rIndex])
											// This is the best possible solution, the register is
											// free. We do not need to continue with this loop, the
											// rIndex will be used by the call().
											break;
										else
										{
											// Wait until the register is freed or try to find another.
											doSpill = false;
											didWork = true;
										}
									}
								}
							}
							else
							{
								// Try to find a register which is free and which is not used
								// to pass a function argument.
								possibleMask = this->_x86Decl.getGpPreservedMask();

								for (rIndex = 0, rBit = 1; rIndex < kX86RegNumGp; ++rIndex, rBit <<= 1)
								{
									if (possibleMask & rBit)
									{
										// Found one.
										if (!x86Context._x86State.gp[rIndex])
											break;
									}
								}
							}

							if (rIndex < kX86RegNumGp)
							{
								if (temporaryGpReg == vsrc->regIndex)
									temporaryGpReg = rIndex;
								x86Compiler->emit(kX86InstMov, gpz(rIndex), gpz(vsrc->regIndex));

								x86Context._x86State.gp[vsrc->regIndex] = nullptr;
								x86Context._x86State.gp[rIndex] = vsrc;

								vsrc->regIndex = rIndex;
								x86Context._allocatedGpRegister(rIndex);

								doSpill = false;
								didWork = true;
							}
						}
						// Emit xchg instead of spill/alloc if possible.
						else if (x != kInvalidValue)
						{
							const FuncArg &dstArgType = targs[x];
							if (X86Util::getVarClassFromVarType(dstArgType.getVarType()) == X86Util::getVarClassFromVarType(srcArgType.getVarType()))
							{
								uint32_t dstIndex = vdst->regIndex;
								uint32_t srcIndex = vsrc->regIndex;

								if (srcIndex == dstArgType.getRegIndex())
								{
#ifdef ASMJIT_X64
									if (vdst->getType() != kX86VarTypeGpd || vsrc->getType() != kX86VarTypeGpd)
										x86Compiler->emit(kX86InstXchg, gpq(dstIndex), gpq(srcIndex));
									else
#endif
										x86Compiler->emit(kX86InstXchg, gpd(dstIndex), gpd(srcIndex));

									x86Context._x86State.gp[srcIndex] = vdst;
									x86Context._x86State.gp[dstIndex] = vsrc;

									vdst->regIndex = srcIndex;
									vsrc->regIndex = dstIndex;

									++rdst->inDone;
									++rsrc->inDone;

									processed[i] = true;
									processed[x] = true;

									doSpill = false;
								}
							}
						}
					}

					if (doSpill)
					{
						x86Context.spillVar(vdst);
						vdst = nullptr;
					}
				}
			}

			if (!vdst)
			{
				VarCallRecord *rec = reinterpret_cast<VarCallRecord *>(vsrc->tPtr);

				this->_moveSrcVariableToRegister(cc, vsrc, srcArgType);

				switch (srcArgType.getVarType())
				{
					case kX86VarTypeGpd:
					case kX86VarTypeGpq:
						x86Context._markGpRegisterModified(srcArgType.getRegIndex());
						break;
					case kX86VarTypeMm:
						x86Context._markMmRegisterModified(srcArgType.getRegIndex());
						break;
					case kX86VarTypeXmm:
					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmSD:
					case kX86VarTypeXmmPS:
					case kX86VarTypeXmmPD:
						x86Context._markMmRegisterModified(srcArgType.getRegIndex());
				}

				++rec->inDone;
				processed[i] = true;
			}
		}
	} while (didWork);

	// --------------------------------------------------------------------------
	// STEP 7:
	//
	// Allocate operand used by CALL instruction.
	// --------------------------------------------------------------------------

	for (i = 0; i < variablesCount; ++i)
	{
		VarCallRecord &r = this->_variables[i];
		if ((r.flags & VarCallRecord::kFlagCallReg) && r.vdata->regIndex == kRegIndexInvalid)
		{
			// If the register is not allocated and the call form is 'call reg' then
			// it's possible to keep it in memory.
			if (!(r.flags & VarCallRecord::kFlagCallMem))
			{
				this->_target = r.vdata->asGpVar().m();
				break;
			}

			if (temporaryGpReg == kRegIndexInvalid)
				temporaryGpReg = this->_findTemporaryGpRegister(cc);

			x86Context.allocGpVar(r.vdata, IntUtil::maskFromIndex(temporaryGpReg), kVarAllocRegister | kVarAllocRead);
		}
	}

	x86Context.translateOperands(&this->_target, 1);

	// --------------------------------------------------------------------------
	// STEP 8:
	//
	// Spill all preserved variables.
	// --------------------------------------------------------------------------

	preserved = this->_x86Decl.getGpPreservedMask();
	for (i = 0, mask = 1; i < kX86RegNumGp; ++i, mask <<= 1)
	{
		X86CompilerVar *vdata = x86Context._x86State.gp[i];
		if (vdata && !(preserved & mask))
		{
			VarCallRecord *rec = reinterpret_cast<VarCallRecord *>(vdata->tPtr);
			if (rec && (rec->outCount || rec->flags & VarCallRecord::kFlagUnuseAfterUse || vdata->lastItem == this))
				x86Context.unuseVar(vdata, kVarStateUnused);
			else
				x86Context.spillGpVar(vdata);
		}
	}

	preserved = this->_x86Decl.getMmPreservedMask();
	for (i = 0, mask = 1; i < kX86RegNumMm; ++i, mask <<= 1)
	{
		X86CompilerVar *vdata = x86Context._x86State.mm[i];
		if (vdata && !(preserved & mask))
		{
			VarCallRecord *rec = reinterpret_cast<VarCallRecord *>(vdata->tPtr);
			if (rec && (rec->outCount || vdata->lastItem == this))
				x86Context.unuseVar(vdata, kVarStateUnused);
			else
				x86Context.spillMmVar(vdata);
		}
	}

	preserved = this->_x86Decl.getXmmPreservedMask();
	for (i = 0, mask = 1; i < kX86RegNumXmm; ++i, mask <<= 1)
	{
		X86CompilerVar *vdata = x86Context._x86State.xmm[i];
		if (vdata && !(preserved & mask))
		{
			VarCallRecord *rec = reinterpret_cast<VarCallRecord *>(vdata->tPtr);
			if (rec && (rec->outCount || vdata->lastItem == this))
				x86Context.unuseVar(vdata, kVarStateUnused);
			else
				x86Context.spillXmmVar(vdata);
		}
	}

	// --------------------------------------------------------------------------
	// STEP 9:
	//
	// Emit CALL instruction.
	// --------------------------------------------------------------------------

	x86Compiler->emit(kX86InstCall, this->_target);

	// Restore the stack offset.
	if (this->_x86Decl.getCalleePopsStack())
	{
		int32_t s = static_cast<int32_t>(this->_x86Decl.getArgumentsStackSize());

		if (s)
			x86Compiler->emit(kX86InstSub, zsp, imm(s));
	}

	// --------------------------------------------------------------------------
	// STEP 10:
	//
	// Prepare others for return value(s) and cleanup.
	// --------------------------------------------------------------------------

	// Clear temp data, see AsmJit::X86CompilerVar::temp why it's needed.
	for (i = 0; i < variablesCount; ++i)
	{
		VarCallRecord *rec = &this->_variables[i];
		X86CompilerVar *vdata = rec->vdata;

		if (rec->flags & (VarCallRecord::kFlagOutEax | VarCallRecord::kFlagOutEdx))
		{
			if (vdata->getClass() & kX86VarClassGp)
			{
				x86Context.allocGpVar(vdata, IntUtil::maskFromIndex((rec->flags & VarCallRecord::kFlagOutEax) ? kX86RegIndexEax : kX86RegIndexEdx), kVarAllocRegister | kVarAllocWrite);
				vdata->changed = true;
			}
		}

		if (rec->flags & VarCallRecord::kFlagOutMm0)
		{
			if (vdata->getClass() & kX86VarClassMm)
			{
				x86Context.allocMmVar(vdata, IntUtil::maskFromIndex(kX86RegIndexMm0), kVarAllocRegister | kVarAllocWrite);
				vdata->changed = true;
			}
		}

		if (rec->flags & (VarCallRecord::kFlagOutXmm0 | VarCallRecord::kFlagOutXmm1))
		{
			if (vdata->getClass() & kX86VarClassXmm)
			{
				x86Context.allocXmmVar(vdata, IntUtil::maskFromIndex((rec->flags & VarCallRecord::kFlagOutXmm0) ? kX86RegIndexXmm0 : kX86RegIndexXmm1), kVarAllocRegister | kVarAllocWrite);
				vdata->changed = true;
			}
		}

		if (rec->flags & (VarCallRecord::kFlagOutSt0 | VarCallRecord::kFlagOutSt1))
		{
			if (vdata->getClass() & kX86VarClassXmm)
			{
				Mem mem(x86Context._getVarMem(vdata));
				x86Context.unuseVar(vdata, kVarStateMem);

				switch (vdata->getType())
				{
					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmPS:
						mem.setSize(4);
						x86Compiler->emit(kX86InstFStP, mem);
						break;
					case kX86VarTypeXmmSD:
					case kX86VarTypeXmmPD:
						mem.setSize(8);
						x86Compiler->emit(kX86InstFStP, mem);
						break;
					default:
						x86Compiler->comment("*** WARNING: Can't convert float return value to untyped XMM\n");
				}
			}
		}

		// Cleanup.
		vdata->tPtr = nullptr;
	}

	for (i = 0; i < variablesCount; ++i)
		x86Context._unuseVarOnEndOfScope(this, &this->_variables[i]);

	return this->translated();
}

// ============================================================================
// [AsmJit::X86CompilerFuncCall - Misc]
// ============================================================================

int X86CompilerFuncCall::getMaxSize() const
{
	// TODO: Instruction max size.
	return 15;
}

bool X86CompilerFuncCall::_tryUnuseVar(CompilerVar *_v)
{
	X86CompilerVar *cv = static_cast<X86CompilerVar *>(_v);

	for (uint32_t i = 0; i < this->_variablesCount; ++i)
		if (this->_variables[i].vdata == cv)
		{
			this->_variables[i].flags |= VarCallRecord::kFlagUnuseAfterUse;
			return true;
		}

	return false;
}

// ============================================================================
// [AsmJit::X86CompilerFuncCall - Helpers]
// ============================================================================

uint32_t X86CompilerFuncCall::_findTemporaryGpRegister(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);

	uint32_t i;
	uint32_t mask;

	uint32_t passedGP = this->_x86Decl.getGpArgumentsMask();
	uint32_t candidate = kRegIndexInvalid;

	// Find all registers used to pass function arguments. We shouldn't use these
	// if possible.
	for (i = 0, mask = 1; i < kX86RegNumGp; ++i, mask <<= 1)
	{
		if (!x86Context._x86State.gp[i])
		{
			// If this register is used to pass arguments to function, we will mark
			// it and use it only if there is no other one.
			if (passedGP & mask)
				candidate = i;
			else
				return i;
		}
	}

	return candidate;
}

uint32_t X86CompilerFuncCall::_findTemporaryXmmRegister(CompilerContext &cc)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);

	uint32_t i;
	uint32_t mask;

	uint32_t passedXMM = this->_x86Decl.getXmmArgumentsMask();
	uint32_t candidate = kRegIndexInvalid;

	// Find all registers used to pass function arguments. We shouldn't use these
	// if possible.
	for (i = 0, mask = 1; i < kX86RegNumXmm; ++i, mask <<= 1)
	{
		if (!x86Context._x86State.xmm[i])
		{
			// If this register is used to pass arguments to function, we will mark
			// it and use it only if there is no other one.
			if (passedXMM & mask)
				candidate = i;
			else
				return i;
		}
	}

	return candidate;
}

X86CompilerVar *X86CompilerFuncCall::_getOverlappingVariable(CompilerContext &cc, const FuncArg &argType) const
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	ASMJIT_ASSERT(argType.getVarType() != kVarTypeInvalid);

	switch (argType.getVarType())
	{
		case kX86VarTypeGpd:
		case kX86VarTypeGpq:
			return x86Context._x86State.gp[argType.getRegIndex()];
		case kX86VarTypeMm:
			return x86Context._x86State.mm[argType.getRegIndex()];
		case kX86VarTypeXmm:
		case kX86VarTypeXmmSS:
		case kX86VarTypeXmmSD:
		case kX86VarTypeXmmPS:
		case kX86VarTypeXmmPD:
			return x86Context._x86State.xmm[argType.getRegIndex()];
	}

	return nullptr;
}

void X86CompilerFuncCall::_moveAllocatedVariableToStack(CompilerContext &cc, X86CompilerVar *vdata, const FuncArg &argType)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = x86Context.getCompiler();

	ASMJIT_ASSERT(!argType.hasRegIndex());
	ASMJIT_ASSERT(vdata->regIndex != kRegIndexInvalid);

	uint32_t src = vdata->regIndex;
	Mem dst = ptr(zsp, -static_cast<int>(sizeof(uintptr_t)) + argType.getStackOffset());

	switch (vdata->getType())
	{
		case kX86VarTypeGpd:
			switch (argType.getVarType())
			{
				case kX86VarTypeGpd:
					x86Compiler->emit(kX86InstMov, dst, gpd(src));
					return;
#ifdef ASMJIT_X64
				case kX86VarTypeGpq:
				case kX86VarTypeMm:
					x86Compiler->emit(kX86InstMov, dst, gpq(src));
					return;
#endif // ASMJIT_X64
			}
			break;

#ifdef ASMJIT_X64
		case kX86VarTypeGpq:
			switch (argType.getVarType())
			{
				case kX86VarTypeGpd:
					x86Compiler->emit(kX86InstMov, dst, gpd(src));
					return;
				case kX86VarTypeGpq:
					x86Compiler->emit(kX86InstMov, dst, gpq(src));
					return;
				case kX86VarTypeMm:
					x86Compiler->emit(kX86InstMovQ, dst, gpq(src));
					return;
			}
			break;
#endif // ASMJIT_X64

		case kX86VarTypeMm:
			switch (argType.getVarType())
			{
				case kX86VarTypeGpd:
				case kX86VarTypeX87SS:
				case kX86VarTypeXmmSS:
					x86Compiler->emit(kX86InstMovD, dst, mm(src));
					return;
				case kX86VarTypeGpq:
				case kX86VarTypeMm:
				case kX86VarTypeX87SD:
				case kX86VarTypeXmmSD:
					x86Compiler->emit(kX86InstMovQ, dst, mm(src));
					return;
			}
			break;

		// We allow incompatible types here, because the called can convert them
		// to correct format before function is called.

		case kX86VarTypeXmm:
		case kX86VarTypeXmmPS:
		case kX86VarTypeXmmPD:
			switch (argType.getVarType())
			{
				case kX86VarTypeXmm:
					x86Compiler->emit(kX86InstMovDQU, dst, xmm(src));
					return;
				case kX86VarTypeXmmSS:
				case kX86VarTypeXmmPS:
					x86Compiler->emit(kX86InstMovUPS, dst, xmm(src));
					return;
				case kX86VarTypeXmmSD:
				case kX86VarTypeXmmPD:
					x86Compiler->emit(kX86InstMovUPD, dst, xmm(src));
					return;
			}
			break;

		case kX86VarTypeXmmSS:
			switch (argType.getVarType())
			{
				case kX86VarTypeX87SS:
				case kX86VarTypeXmm:
				case kX86VarTypeXmmSS:
				case kX86VarTypeXmmPS:
				case kX86VarTypeXmmSD:
				case kX86VarTypeXmmPD:
					x86Compiler->emit(kX86InstMovSS, dst, xmm(src));
					return;
			}
			break;

		case kX86VarTypeXmmSD:
			switch (argType.getVarType())
			{
				case kX86VarTypeX87SD:
				case kX86VarTypeXmm:
				case kX86VarTypeXmmSS:
				case kX86VarTypeXmmPS:
				case kX86VarTypeXmmSD:
				case kX86VarTypeXmmPD:
					x86Compiler->emit(kX86InstMovSD, dst, xmm(src));
					return;
			}
			break;
	}

	x86Compiler->setError(kErrorIncompatibleArgumentType);
}

void X86CompilerFuncCall::_moveSpilledVariableToStack(CompilerContext &cc, X86CompilerVar *cv, const FuncArg &argType, uint32_t temporaryGpReg, uint32_t temporaryXmmReg)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = x86Context.getCompiler();

	ASMJIT_ASSERT(!argType.hasRegIndex());
	ASMJIT_ASSERT(cv->regIndex == kRegIndexInvalid);

	Mem src = x86Context._getVarMem(cv);
	Mem dst = ptr(zsp, -static_cast<int>(sizeof(sysint_t)) + argType.getStackOffset());

	switch (cv->getType())
	{
		case kX86VarTypeGpd:
			switch (argType.getVarType())
			{
				case kX86VarTypeGpd:
					x86Compiler->emit(kX86InstMov, gpd(temporaryGpReg), src);
					x86Compiler->emit(kX86InstMov, dst, gpd(temporaryGpReg));
					return;
#ifdef ASMJIT_X64
				case kX86VarTypeGpq:
				case kX86VarTypeMm:
					x86Compiler->emit(kX86InstMov, gpd(temporaryGpReg), src);
					x86Compiler->emit(kX86InstMov, dst, gpq(temporaryGpReg));
					return;
#endif // ASMJIT_X64
			}
			break;

#ifdef ASMJIT_X64
		case kX86VarTypeGpq:
			switch (argType.getVarType())
			{
				case kX86VarTypeGpd:
					x86Compiler->emit(kX86InstMov, gpd(temporaryGpReg), src);
					x86Compiler->emit(kX86InstMov, dst, gpd(temporaryGpReg));
					return;
				case kX86VarTypeGpq:
				case kX86VarTypeMm:
					x86Compiler->emit(kX86InstMov, gpq(temporaryGpReg), src);
					x86Compiler->emit(kX86InstMov, dst, gpq(temporaryGpReg));
					return;
			}
			break;
#endif // ASMJIT_X64

		case kX86VarTypeMm:
			switch (argType.getVarType())
			{
				case kX86VarTypeGpd:
				case kX86VarTypeX87SS:
				case kX86VarTypeXmmSS:
					x86Compiler->emit(kX86InstMov, gpd(temporaryGpReg), src);
					x86Compiler->emit(kX86InstMov, dst, gpd(temporaryGpReg));
					return;
				case kX86VarTypeGpq:
				case kX86VarTypeMm:
				case kX86VarTypeX87SD:
				case kX86VarTypeXmmSD:
					// TODO
					return;
			}
			break;

		// We allow incompatible types here, because the caller can convert them
		// to correct format before function is called.

		case kX86VarTypeXmm:
		case kX86VarTypeXmmPS:
		case kX86VarTypeXmmPD:
			switch (argType.getVarType())
			{
				case kX86VarTypeXmm:
					x86Compiler->emit(kX86InstMovDQU, xmm(temporaryXmmReg), src);
					x86Compiler->emit(kX86InstMovDQU, dst, xmm(temporaryXmmReg));
					return;
				case kX86VarTypeXmmSS:
				case kX86VarTypeXmmPS:
					x86Compiler->emit(kX86InstMovUPS, xmm(temporaryXmmReg), src);
					x86Compiler->emit(kX86InstMovUPS, dst, xmm(temporaryXmmReg));
					return;
				case kX86VarTypeXmmSD:
				case kX86VarTypeXmmPD:
					x86Compiler->emit(kX86InstMovUPD, xmm(temporaryXmmReg), src);
					x86Compiler->emit(kX86InstMovUPD, dst, xmm(temporaryXmmReg));
					return;
			}
			break;

		case kX86VarTypeXmmSS:
			switch (argType.getVarType())
			{
				case kX86VarTypeX87SS:
				case kX86VarTypeXmm:
				case kX86VarTypeXmmSS:
				case kX86VarTypeXmmPS:
				case kX86VarTypeXmmSD:
				case kX86VarTypeXmmPD:
					x86Compiler->emit(kX86InstMovSS, xmm(temporaryXmmReg), src);
					x86Compiler->emit(kX86InstMovSS, dst, xmm(temporaryXmmReg));
					return;
			}
			break;

		case kX86VarTypeXmmSD:
			switch (argType.getVarType())
			{
				case kX86VarTypeX87SD:
				case kX86VarTypeXmm:
				case kX86VarTypeXmmSS:
				case kX86VarTypeXmmPS:
				case kX86VarTypeXmmSD:
				case kX86VarTypeXmmPD:
					x86Compiler->emit(kX86InstMovSD, xmm(temporaryXmmReg), src);
					x86Compiler->emit(kX86InstMovSD, dst, xmm(temporaryXmmReg));
					return;
			}
			break;
	}

	x86Compiler->setError(kErrorIncompatibleArgumentType);
}

void X86CompilerFuncCall::_moveSrcVariableToRegister(CompilerContext &cc, X86CompilerVar *cv, const FuncArg &argType)
{
	X86CompilerContext &x86Context = static_cast<X86CompilerContext &>(cc);
	X86Compiler *x86Compiler = x86Context.getCompiler();

	uint32_t dst = argType.getRegIndex();
	uint32_t src = cv->regIndex;

	if (src != kRegIndexInvalid)
	{
		switch (argType.getVarType())
		{
			case kX86VarTypeGpd:
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
#ifdef ASMJIT_X64
					case kX86VarTypeGpq:
#endif // ASMJIT_X64
						x86Compiler->emit(kX86InstMov, gpd(dst), gpd(src));
						return;
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovD, gpd(dst), mm(src));
						return;
				}
				break;

#ifdef ASMJIT_X64
			case kX86VarTypeGpq:
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
						x86Compiler->emit(kX86InstMov, gpd(dst), gpd(src));
						return;
					case kX86VarTypeGpq:
						x86Compiler->emit(kX86InstMov, gpq(dst), gpq(src));
						return;
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, gpq(dst), mm(src));
						return;
				}
				break;
#endif // ASMJIT_X64

			case kX86VarTypeMm:
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
						x86Compiler->emit(kX86InstMovD, gpd(dst), gpd(src));
						return;
#ifdef ASMJIT_X64
					case kX86VarTypeGpq:
						x86Compiler->emit(kX86InstMovQ, gpq(dst), gpq(src));
						return;
#endif // ASMJIT_X64
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, mm(dst), mm(src));
						return;
				}
				break;

			case kX86VarTypeXmm:
			case kX86VarTypeXmmPS:
			case kX86VarTypeXmmPD:
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
						x86Compiler->emit(kX86InstMovD, xmm(dst), gpd(src));
						return;
#ifdef ASMJIT_X64
					case kX86VarTypeGpq:
						x86Compiler->emit(kX86InstMovQ, xmm(dst), gpq(src));
						return;
#endif // ASMJIT_X64
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, xmm(dst), mm(src));
						return;
					case kX86VarTypeXmm:
					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmPS:
					case kX86VarTypeXmmSD:
					case kX86VarTypeXmmPD:
						x86Compiler->emit(kX86InstMovDQA, xmm(dst), xmm(src));
						return;
				}
				break;

			case kX86VarTypeXmmSS:
				switch (cv->getType())
				{
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, xmm(dst), mm(src));
						return;
					case kX86VarTypeXmm:
						x86Compiler->emit(kX86InstMovDQA, xmm(dst), xmm(src));
						return;
					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmPS:
						x86Compiler->emit(kX86InstMovSS, xmm(dst), xmm(src));
						return;
					case kX86VarTypeXmmSD:
					case kX86VarTypeXmmPD:
						x86Compiler->emit(kX86InstCvtSD2SS, xmm(dst), xmm(src));
						return;
				}
				break;

			case kX86VarTypeXmmSD:
				switch (cv->getType())
				{
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, xmm(dst), mm(src));
						return;
					case kX86VarTypeXmm:
						x86Compiler->emit(kX86InstMovDQA, xmm(dst), xmm(src));
						return;
					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmPS:
						x86Compiler->emit(kX86InstCvtSS2SD, xmm(dst), xmm(src));
						return;
					case kX86VarTypeXmmSD:
					case kX86VarTypeXmmPD:
						x86Compiler->emit(kX86InstMovSD, xmm(dst), xmm(src));
						return;
				}
				break;
		}
	}
	else
	{
		Mem mem = x86Context._getVarMem(cv);

		switch (argType.getVarType())
		{
			case kX86VarTypeGpd:
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
#ifdef ASMJIT_X64
					case kX86VarTypeGpq:
#endif // ASMJIT_X64
						x86Compiler->emit(kX86InstMov, gpd(dst), mem);
						return;
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovD, gpd(dst), mem);
						return;
				}
				break;

#ifdef ASMJIT_X64
			case kX86VarTypeGpq:
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
						x86Compiler->emit(kX86InstMov, gpd(dst), mem);
						return;
					case kX86VarTypeGpq:
						x86Compiler->emit(kX86InstMov, gpq(dst), mem);
						return;
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, gpq(dst), mem);
						return;
				}
				break;
#endif // ASMJIT_X64

			case kX86VarTypeMm:
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
						x86Compiler->emit(kX86InstMovD, gpd(dst), mem);
						return;
#ifdef ASMJIT_X64
					case kX86VarTypeGpq:
						x86Compiler->emit(kX86InstMovQ, gpq(dst), mem);
						return;
#endif // ASMJIT_X64
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, mm(dst), mem);
						return;
				}
				break;

			case kX86VarTypeXmm:
			case kX86VarTypeXmmPS:
			case kX86VarTypeXmmPD:
				switch (cv->getType())
				{
					case kX86VarTypeGpd:
						x86Compiler->emit(kX86InstMovD, xmm(dst), mem);
						return;
#ifdef ASMJIT_X64
					case kX86VarTypeGpq:
						x86Compiler->emit(kX86InstMovQ, xmm(dst), mem);
						return;
#endif // ASMJIT_X64
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, xmm(dst), mem);
						return;
					case kX86VarTypeXmm:
					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmPS:
					case kX86VarTypeXmmSD:
					case kX86VarTypeXmmPD:
						x86Compiler->emit(kX86InstMovDQA, xmm(dst), mem);
						return;
				}
				break;

			case kX86VarTypeXmmSS:
				switch (cv->getType())
				{
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, xmm(dst), mem);
						return;
					case kX86VarTypeXmm:
						x86Compiler->emit(kX86InstMovDQA, xmm(dst), mem);
						return;
					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmPS:
						x86Compiler->emit(kX86InstMovSS, xmm(dst), mem);
						return;
					case kX86VarTypeXmmSD:
					case kX86VarTypeXmmPD:
						x86Compiler->emit(kX86InstCvtSD2SS, xmm(dst), mem);
						return;
				}
				break;

			case kX86VarTypeXmmSD:
				switch (cv->getType())
				{
					case kX86VarTypeMm:
						x86Compiler->emit(kX86InstMovQ, xmm(dst), mem);
						return;
					case kX86VarTypeXmm:
						x86Compiler->emit(kX86InstMovDQA, xmm(dst), mem);
						return;
					case kX86VarTypeXmmSS:
					case kX86VarTypeXmmPS:
						x86Compiler->emit(kX86InstCvtSS2SD, xmm(dst), mem);
						return;
					case kX86VarTypeXmmSD:
					case kX86VarTypeXmmPD:
						x86Compiler->emit(kX86InstMovSD, xmm(dst), mem);
						return;
				}
				break;
		}
	}

	x86Compiler->setError(kErrorIncompatibleArgumentType);
}

// Prototype & Arguments Management.
void X86CompilerFuncCall::setPrototype(uint32_t callingConvention, uint32_t returnType, const uint32_t *arguments, uint32_t argumentsCount)
{
	this->_x86Decl.setPrototype(callingConvention, returnType, arguments, argumentsCount);
	this->_args = reinterpret_cast<Operand *>(this->getCompiler()->getZoneMemory().alloc(sizeof(Operand) * argumentsCount));
	memset(this->_args, 0, sizeof(Operand) * argumentsCount);
}

bool X86CompilerFuncCall::setArgument(uint32_t i, const Var &var)
{
	ASMJIT_ASSERT(i < this->_x86Decl.getArgumentsCount());

	if (i >= this->_x86Decl.getArgumentsCount())
		return false;

	this->_args[i] = var;
	return true;
}

bool X86CompilerFuncCall::setArgument(uint32_t i, const Imm &imm)
{
	ASMJIT_ASSERT(i < this->_x86Decl.getArgumentsCount());

	if (i >= _x86Decl.getArgumentsCount())
		return false;

	this->_args[i] = imm;
	return true;
}

bool X86CompilerFuncCall::setReturn(const Operand &first, const Operand &second)
{
	this->_ret[0] = first;
	this->_ret[1] = second;

	return true;
}

} // AsmJit namespace

// [Api-End]
#include "../core/apiend.h"
