#include "cpu.h"

#include <stdlib.h>

cpu_t *new_cpu()
{
	cpu_t *res;

	res = calloc(1, sizeof(cpu_t));

	return res;
}