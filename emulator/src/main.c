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

int main(int argc, const char *argv[])
{
	cpu_t		*cpu;
	memory_t	*mem;
	emulator_t	*em;
	int res;

	cpu = new_cpu(0xFFFFFFFF - 8);
	mem = new_memory();
	em = new_emulator(cpu, mem);

	memory_from_file(mem, argv[1]);
	do {
		res = emulator_execute(em);
		//fgetc(stdin);
	} while(res != 1);
}

void test(void)
{
	cpu_t		*cpu;
	memory_t	*mem;
	emulator_t	*em;
	cpu = new_cpu(0);
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