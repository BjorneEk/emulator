/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * cpu
 *==========================================================*/

#ifndef _CPU_H_
#define _CPU_H_

#include "util/types.h"

typedef struct cpu {
	u16_t regs[14];
	u32_t pc;
} cpu_t;

#endif /* _CPU_H_ */