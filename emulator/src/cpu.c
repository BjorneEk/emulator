#include "cpu.h"

#include <stdlib.h>
#include <stdio.h>

cpu_t *new_cpu()
{
	cpu_t *res;

	res = calloc(1, sizeof(cpu_t));

	return res;
}

void cpu_print(cpu_t *cpu)
{
	int i;

	for (i = 0; i < 13; i++)
	{
		printf("REG %d: %d\n", i, cpu->regs[i]);
	}

	printf("PS: %d\n", cpu->ps);
	printf("PC: %d\n", cpu->pc);
}