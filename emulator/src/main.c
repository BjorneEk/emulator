/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * emulator
 *==========================================================*/

#include "cpu.h"
#include "emulator.h"
#include "../../arch/interface.h"
#include "memory.h"
#include "util/types.h"
#include <stdio.h>

extern void print_instruction_(int);
extern void print_instruction(int);
#define _(name) print_instruction(INSTR_##name);
#define __(name) print_instruction_(INSTR_##name);
int main(int argc, char *argv[])
{
	cpu_t *cpu;
	memory_t *mem;
	emulator_t *em;

	cpu = new_cpu();
	mem = new_memory();
	em = new_emulator(cpu, mem);
	XMACRO_INSTRUCTIONS(_)
}