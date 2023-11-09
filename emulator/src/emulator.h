/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * emulator
 *==========================================================*/

#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "cpu.h"
#include "memory.h"
#include "../../common/util/types.h"

typedef struct emulator {
	cpu_t *cpu;
	memory_t *mem;
} emulator_t;

void init_emulator(emulator_t *em);

u8_t fetch_byte(emulator_t *em);

i32_t execute(emulator_t *em);

#endif /* _EMULATOR_H_ */