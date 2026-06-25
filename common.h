#ifndef COMMON_H
#define COMMON_H

#include <p33FJ256MC710.h>

// Doğru Bitwise Makrolar (Register'ın diğer bitlerine zarar vermez)
#define SETBIT(reg, bit)   ((reg) |= (1 << (bit)))
#define CLEARBIT(reg, bit) ((reg) &= ~(1 << (bit)))

#endif
