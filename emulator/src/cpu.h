/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * cpu
 *==========================================================*/

#ifndef _CPU_H_
#define _CPU_H_

#include "../../common/util/types.h"

typedef struct cpu {
	u16_t regs[13];
	u16_t ps;
	u32_t pc;

	bool	is_reset;
	u32_t	boot_location;
	u32_t	interrupt_handler_location;
	bool	nmi;
	bool	irq;
} cpu_t;

static const int FLAG_CARRY = 15;
static const int FLAG_ZERO = 14;
static const int FLAG_INTERRUPT = 13;
static const int FLAG_OVERFLOW = 12;
static const int FLAG_NEGATIVE = 11;
static const int FLAG_BREAK = 10;

static inline int cpu_get_flag(cpu_t *cpu, int flag)
{
	return cpu->ps & 1 << flag;
}

static inline void cpu_set_flag(cpu_t *cpu, int flag)
{
	cpu->ps |= 1 << flag;
}

static inline void cpu_clear_flag(cpu_t *cpu, int flag)
{
	cpu->ps &= ~(1 << flag);
}

cpu_t *new_cpu(u32_t boot_location);

void cpu_print(cpu_t *cpu);

#endif /* _CPU_H_ */