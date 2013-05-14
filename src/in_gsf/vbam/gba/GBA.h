#ifndef GBA_H
#define GBA_H

#include <cstdint>

struct memoryMap
{
	uint8_t *address;
	uint32_t mask;
};

union reg_pair
{
	struct
	{
#ifdef WORDS_BIGENDIAN
		uint8_t B3;
		uint8_t B2;
		uint8_t B1;
		uint8_t B0;
#else
		uint8_t B0;
		uint8_t B1;
		uint8_t B2;
		uint8_t B3;
#endif
	} B;
	struct
	{
#ifdef WORDS_BIGENDIAN
		uint16_t W1;
		uint16_t W0;
#else
		uint16_t W0;
		uint16_t W1;
#endif
	} W;
#ifdef WORDS_BIGENDIAN
	volatile uint32_t I;
#else
	uint32_t I;
#endif
};

#ifndef NO_GBA_MAP
extern memoryMap map[256];
#endif

extern reg_pair reg[45];
extern uint8_t biosProtected[4];

extern bool N_FLAG;
extern bool Z_FLAG;
extern bool C_FLAG;
extern bool V_FLAG;
extern bool armIrqEnable;
extern bool armState;
extern int armMode;

extern int CPULoadRom();
extern void CPUUpdateRegister(uint32_t, uint16_t);
extern void CPUInit();
extern void CPUReset();
extern void CPULoop(int);
extern void CPUCheckDMA(int, int);

const uint32_t R13_IRQ = 18;
const uint32_t R14_IRQ = 19;
const uint32_t SPSR_IRQ = 20;
const uint32_t R13_USR = 26;
const uint32_t R14_USR = 27;
const uint32_t R13_SVC = 28;
const uint32_t R14_SVC = 29;
const uint32_t SPSR_SVC = 30;
const uint32_t R13_ABT = 31;
const uint32_t R14_ABT = 32;
const uint32_t SPSR_ABT = 33;
const uint32_t R13_UND = 34;
const uint32_t R14_UND = 35;
const uint32_t SPSR_UND = 36;
const uint32_t R8_FIQ = 37;
const uint32_t R9_FIQ = 38;
const uint32_t R10_FIQ = 39;
const uint32_t R11_FIQ = 40;
const uint32_t R12_FIQ = 41;
const uint32_t R13_FIQ = 42;
const uint32_t R14_FIQ = 43;
const uint32_t SPSR_FIQ = 44;

#include "Globals.h"

#endif // GBA_H
