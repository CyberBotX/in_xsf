/*
	Copyright (C) 2006 yopyop
	Copyright (C) 2006-2012 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ARM_CPU
#define ARM_CPU

#include "types.h"
#include "bits.h"
#include "MMU.h"
#include "common.h"

static inline uint32_t CODE(uint32_t i) { return (i >> 25) & 0x7; }

static const uint32_t EXCEPTION_RESET = 0x00;
static const uint32_t EXCEPTION_UNDEFINED_INSTRUCTION = 0x04;
static const uint32_t EXCEPTION_SWI = 0x08;
static const uint32_t EXCEPTION_PREFETCH_ABORT = 0x0C;
static const uint32_t EXCEPTION_DATA_ABORT = 0x10;
static const uint32_t EXCEPTION_RESERVED_0x14 = 0x14;
static const uint32_t EXCEPTION_IRQ = 0x18;
static const uint32_t EXCEPTION_FAST_IRQ = 0x1C;

static inline uint32_t INSTRUCTION_INDEX(uint32_t i) { return ((i >> 16) & 0xFF0) | ((i >> 4) & 0xF); }

static inline uint32_t ROR(uint32_t i, uint32_t j) { return (i >> j) | (i << (32 - j)); }

template<typename T> static inline T UNSIGNED_OVERFLOW(T a, T b, T c) { return BIT31((a & b) | ((a | b) & ~c)); }

template<typename T> static inline T UNSIGNED_UNDERFLOW(T a, T b, T c) { return BIT31((~a & b) | ((~a | b) & c)); }

template<typename T> static inline T SIGNED_OVERFLOW(T a, T b, T c) { return BIT31((a & b & ~c) | (~a & ~b & c)); }

template<typename T> static inline T SIGNED_UNDERFLOW(T a, T b, T c) { return BIT31((a & ~b & ~c) | (~a & b & c)); }

// ============================= CPRS flags funcs
static inline bool CarryFrom(int32_t left, int32_t right)
{
	uint32_t res = 0xFFFFFFFFU - static_cast<uint32_t>(left);

	return static_cast<uint32_t>(right) > res;
}

static inline bool BorrowFrom(int32_t left, int32_t right)
{
	return static_cast<uint32_t>(right) > static_cast<uint32_t>(left);
}

static inline bool OverflowFromADD(int32_t alu_out, int32_t left, int32_t right)
{
	return ((left >= 0 && right >= 0) || (left < 0 && right < 0)) && ((left < 0 && alu_out >= 0) || (left >= 0 && alu_out < 0));
}

inline bool OverflowFromSUB(int32_t alu_out, int32_t left, int32_t right)
{
	return ((left < 0 && right >= 0) || (left >= 0 && right < 0)) && ((left < 0 && alu_out >= 0) || (left >= 0 && alu_out < 0));
}

//zero 15-feb-2009 - these werent getting used and they were getting in my way
//#define EQ	0x0
//#define NE	0x1
//#define CS	0x2
//#define CC	0x3
//#define MI	0x4
//#define PL	0x5
//#define VS	0x6
//#define VC	0x7
//#define HI	0x8
//#define LS	0x9
//#define GE	0xA
//#define LT	0xB
//#define GT	0xC
//#define LE	0xD
//#define AL	0xE

extern const uint8_t arm_cond_table[16 * 16];

enum Mode
{
	USR = 0x10,
	FIQ = 0x11,
	IRQ = 0x12,
	SVC = 0x13,
	ABT = 0x17,
	UND = 0x1B,
	SYS = 0x1F
};

#ifdef WORDS_BIGENDIAN
typedef union
{
	struct
	{
		uint32_t N : 1,
		Z : 1,
		C : 1,
		V : 1,
		Q : 1,
		RAZ : 19,
		I : 1,
		F : 1,
		T : 1,
		mode : 5;
	} bits;
	uint32_t val;
} Status_Reg;
#else
typedef union
{
	struct
	{
		uint32_t mode : 5,
		T : 1,
		F : 1,
		I : 1,
		RAZ : 19,
		Q : 1,
		V : 1,
		C : 1,
		Z : 1,
		N : 1;
	} bits;
	uint32_t val;
} Status_Reg;
#endif

static inline uint8_t TEST_COND(uint32_t cond, uint32_t inst, Status_Reg CPSR) { return arm_cond_table[((CPSR.val >> 24) & 0xf0) | cond] & (1 << inst); }

/**
 * The control interface to a CPU
 */
/*struct armcpu_ctrl_iface
{*/
	/** stall the processor */
	//void (*stall)(void *instance);

	/** unstall the processor */
	//void (*unstall)(void *instance);

	/** read a register value */
	//uint32_t (*read_reg)(void *instance, uint32_t reg_num);

	/** set a register value */
	//void (*set_reg)(void *instance, uint32_t reg_num, uint32_t value);

	/** install the post execute function */
	//void (*install_post_ex_fn)( void *instance, void (*fn)(void *, uint32_t, int), void *fn_data);

	/** remove the post execute function */
	//void (*remove_post_ex_fn)(void *instance);

	/** the private data passed to all interface functions */
	/*void *data;
};*/

typedef void *armcp_t;

struct armcpu_t
{
	uint32_t proc_ID;
	uint32_t instruction; //4
	uint32_t instruct_adr; //8
	uint32_t next_instruction; //12

	uint32_t R[16]; //16
	Status_Reg CPSR;  //80
	Status_Reg SPSR;

	void changeCPSR();

	uint32_t R13_usr, R14_usr;
	uint32_t R13_svc, R14_svc;
	uint32_t R13_abt, R14_abt;
	uint32_t R13_und, R14_und;
	uint32_t R13_irq, R14_irq;
	uint32_t R8_fiq, R9_fiq, R10_fiq, R11_fiq, R12_fiq, R13_fiq, R14_fiq;
	Status_Reg SPSR_svc, SPSR_abt, SPSR_und, SPSR_irq, SPSR_fiq;

	armcp_t *coproc[16];

	uint32_t intVector;
	uint8_t LDTBit; // 1 : ARMv5 style 0 : non ARMv5
	bool waitIRQ;
	bool halt_IE_and_IF; //the cpu is halted, waiting for IE&IF to signal something
	uint8_t intrWaitARM_state;

	bool BIOS_loaded;

	uint32_t (**swi_tab)();

	// flag indicating if the processor is stalled (for debugging)
	int stalled;

#ifdef GDB_STUB
	/** there is a pending irq for the cpu */
	int irq_flag;

	/** the post executed function (if installed) */
	void (*post_ex_fn)(void *, uint32_t adr, int thumb);

	/** data for the post executed function */
	void *post_ex_fn_data;

	/** the memory interface */
	armcpu_memory_iface *mem_if;

	/** the ctrl interface */
	armcpu_ctrl_iface ctrl_iface;
#endif
};

#ifdef GDB_STUB
int armcpu_new(armcpu_t *armcpu, uint32_t id, armcpu_memory_iface *mem_if, armcpu_ctrl_iface **ctrl_iface_ret);
#else
int armcpu_new(armcpu_t *armcpu, uint32_t id);
#endif
void armcpu_init(armcpu_t *armcpu, uint32_t adr);
uint32_t armcpu_switchMode(armcpu_t *armcpu, uint8_t mode);

template<int PROCNUM> uint32_t armcpu_exec();

bool armcpu_irqException(armcpu_t *armcpu);
//bool armcpu_flagIrq( armcpu_t *armcpu);
void armcpu_exception(armcpu_t *cpu, uint32_t number);
uint32_t TRAPUNDEF(armcpu_t* cpu);
uint32_t armcpu_Wait4IRQ(armcpu_t *cpu);

extern armcpu_t NDS_ARM7, NDS_ARM9;

static inline void setIF(int PROCNUM, uint32_t flag)
{
	// don't set generated bits!!!
	assert(!(flag&0x00200000));

	MMU.reg_IF_bits[PROCNUM] |= flag;

	extern void NDS_Reschedule();
	NDS_Reschedule();
}

static inline void NDS_makeIrq(int PROCNUM, uint32_t num)
{
	setIF(PROCNUM, 1<<num);
}

/*static inline char *decodeIntruction(bool thumb_mode, uint32_t instr)
{
	char txt[20] = "";
	uint32_t tmp = 0;
	if (thumb_mode)
	{
		tmp = instr >> 6;
		strcpy(txt, intToBin(static_cast<uint16_t>(tmp)) + 6);
	}
	else
	{
		tmp = ((instr >> 16) & 0x0FF0) | ((instr >> 4) & 0x0F);
		strcpy(txt, intToBin(tmp) + 20);
	}
	return _strdup(txt);
}*/

#endif
