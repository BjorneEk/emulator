/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * emulator
 *==========================================================*/

#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "IO/video_card.h"
#include "cpu.h"
#include "memory.h"
#include "IO/io_emulator.h"
#include "../../common/util/types.h"

typedef struct emulator {
	cpu_t		*cpu;
	memory_t	*mem;
	io_t		*io;
	video_card_t	*vc;
} emulator_t;

emulator_t *new_emulator(cpu_t *cpu, memory_t *mem, io_t *io, video_card_t *vc);

i32_t emulator_execute(emulator_t *em);

void emulator_debug(emulator_t *em);

#endif /* _EMULATOR_H_ */