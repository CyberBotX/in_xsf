#ifndef BITS_H
#define BITS_H

static inline uint32_t BIT(uint32_t n) { return 1 << n; }

static inline uint32_t BIT_N(uint32_t i, uint32_t n) { return (i >> n) & 1; }
static inline uint32_t BIT0(uint32_t i) { return i & 1; }
static inline uint32_t BIT1(uint32_t i) { return BIT_N(i, 1); }
static inline uint32_t BIT2(uint32_t i) { return BIT_N(i, 2); }
static inline uint32_t BIT3(uint32_t i) { return BIT_N(i, 3); }
static inline uint32_t BIT4(uint32_t i) { return BIT_N(i, 4); }
static inline uint32_t BIT5(uint32_t i) { return BIT_N(i, 5); }
static inline uint32_t BIT6(uint32_t i) { return BIT_N(i, 6); }
static inline uint32_t BIT7(uint32_t i) { return BIT_N(i, 7); }
static inline uint32_t BIT8(uint32_t i) { return BIT_N(i, 8); }
static inline uint32_t BIT9(uint32_t i) { return BIT_N(i, 9); }
static inline uint32_t BIT10(uint32_t i) { return BIT_N(i, 10); }
static inline uint32_t BIT11(uint32_t i) { return BIT_N(i, 11); }
static inline uint32_t BIT12(uint32_t i) { return BIT_N(i, 12); }
static inline uint32_t BIT13(uint32_t i) { return BIT_N(i, 13); }
static inline uint32_t BIT14(uint32_t i) { return BIT_N(i, 14); }
static inline uint32_t BIT15(uint32_t i) { return BIT_N(i, 15); }
static inline uint32_t BIT16(uint32_t i) { return BIT_N(i, 16); }
static inline uint32_t BIT17(uint32_t i) { return BIT_N(i, 17); }
static inline uint32_t BIT18(uint32_t i) { return BIT_N(i, 18); }
static inline uint32_t BIT19(uint32_t i) { return BIT_N(i, 19); }
static inline uint32_t BIT20(uint32_t i) { return BIT_N(i, 20); }
static inline uint32_t BIT21(uint32_t i) { return BIT_N(i, 21); }
static inline uint32_t BIT22(uint32_t i) { return BIT_N(i, 22); }
static inline uint32_t BIT23(uint32_t i) { return BIT_N(i, 23); }
static inline uint32_t BIT24(uint32_t i) { return BIT_N(i, 24); }
static inline uint32_t BIT25(uint32_t i) { return BIT_N(i, 25); }
static inline uint32_t BIT26(uint32_t i) { return BIT_N(i, 26); }
static inline uint32_t BIT27(uint32_t i) { return BIT_N(i, 27); }
static inline uint32_t BIT28(uint32_t i) { return BIT_N(i, 28); }
static inline uint32_t BIT29(uint32_t i) { return BIT_N(i, 29); }
static inline uint32_t BIT30(uint32_t i) { return BIT_N(i, 30); }
static inline uint32_t BIT31(uint32_t i) { return i >> 31; }

static inline uint32_t CONDITION(uint32_t i) { return i >> 28; }

static inline uint32_t REG_POS(uint32_t i, uint32_t n) { return (i >> n) & 0xF; }

#endif
