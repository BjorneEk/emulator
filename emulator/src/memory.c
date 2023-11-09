#include "memory.h"
#include "../../common/util/error.h"
#include "../../arch/interface.h"

#include <stdlib.h>

memory_t *new_memory()
{
	memory_t *res;

	res = calloc(1, sizeof(memory_t));

	return res;
}

u8_t memory_read_byte(memory_t *mem, u32_t addr)
{
	ASSERT(0 <= addr && addr < MEMORY_SIZE);

	return mem->data[addr];
}

u16_t memory_read_word(memory_t *mem, u32_t addr)
{
	u8_t b0, b1;
	u16_t word;

	b0 = memory_read_byte(mem, addr);
	b1 = memory_read_byte(mem, addr + 1);
	word = b0 | (b1 << 8);

	if (endian == LITTLE_ENDIAN)
		return word;
	return swap_u16(word);
}