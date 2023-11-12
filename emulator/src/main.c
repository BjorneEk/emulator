/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * emulator
 *==========================================================*/

#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "util/types.h"
#include "../../arch/interface.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
	cpu_t *cpu;
	memory_t *mem;
	emulator_t *em;

	cpu = new_cpu();
	mem = new_memory();
	em = new_emulator(cpu, mem);

	memory_load_test(mem);

	cpu_print(cpu);
	emulator_execute(em);
	cpu_print(cpu);
	emulator_execute(em);
	cpu_print(cpu);
	emulator_execute(em);
	cpu_print(cpu);
	emulator_execute(em);
	cpu_print(cpu);
}