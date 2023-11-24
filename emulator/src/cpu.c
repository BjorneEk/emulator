#include "cpu.h"

#include <stdlib.h>
#include <stdio.h>

cpu_t *new_cpu(u32_t boot_location)
{
	cpu_t *res;

	res = calloc(1, sizeof(cpu_t));
	res->is_reset = true;
	res->boot_location = boot_location;
	res->interrupt_handler_location = 0;
	res->nmi = false;
	res->irq = false;
	return res;
}

void cpu_print(cpu_t *cpu)
{
	int i;

	for (i = 0; i < 13; i++)
	{
		printf("REG %d: %d\n", i, cpu->regs[i]);
	}

	printf("PS: 0x%04x\n", cpu->ps);
	printf("PC: 0x%08x\n", cpu->pc);
}