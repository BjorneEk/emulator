#include "memory.h"
#include "../../common/util/error.h"
#include "../../arch/interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

memory_t *new_memory()
{
	memory_t *res;

	res = calloc(1, sizeof(memory_t));

	return res;
}
static u64_t file_len(const char *filename)
{
	struct stat st;

	if (stat(filename, &st) == -1)
		exit_error("failed to open file: %s\n", filename);
	return st.st_size;
}

void memory_from_file(memory_t *mem, const char *filename)
{
	u64_t i;
	FILE *f;

	i = MEMORY_SIZE - file_len(filename);

	f = fopen(filename, "rb");
	while(i < MEMORY_SIZE)
		mem->data[i++] = getc(f);
}

u8_t memory_read_byte(memory_t *mem, u32_t addr)
{
	return mem->data[addr];
}

void memory_write_byte(memory_t *mem, u32_t addr, u8_t data)
{
	mem->data[addr] = data;
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

void memory_write_word(memory_t *mem, u32_t addr, u16_t data)
{
	if (endian == BIG_ENDIAN)
		data = swap_u16(data);

	memory_write_byte(mem, addr, data >> 8);
	memory_write_byte(mem, addr + 1, data & 0xFF);
}

u32_t memory_read_long(memory_t *mem, u32_t addr)
{
	u16_t w0, w1;
	u32_t res;

	w0 = memory_read_word(mem, addr);
	w1 = memory_read_word(mem, addr + 2);
	res = w0 | (w1 << 16);

	if (endian == LITTLE_ENDIAN)
		return res;
	return swap_u32(res);
}

void memory_write_long(memory_t *mem, u32_t addr, u32_t data)
{
	if (endian == BIG_ENDIAN)
		data = swap_u32(data);

	memory_write_word(mem, addr, data >> 16);
	memory_write_word(mem, addr + 2, data & 0xFFFF);
}

void memory_load_test(memory_t *mem)
{
	memory_write_byte(mem, 0, SINSTR_LBRA_ABS);
	memory_write_long(mem, 1, 0x0000FFFF);

	memory_write_byte(mem, 0x0000FFFF, SINSTR_LDR_IMMIDIATE);
	memory_write_byte(mem, 0x0000FFFF + 1, (u8_t)(REG_R5 << 4));
	memory_write_word(mem, 0x0000FFFF + 2, 0x1000);

	memory_write_byte(mem, 0x0000FFFF + 4, SINSTR_ADC_IMMIDIATE);
	memory_write_byte(mem, 0x0000FFFF + 5, (u8_t)(REG_R6 << 4) | REG_R5);
	memory_write_word(mem, 0x0000FFFF + 6, 0xF000);

	memory_write_byte(mem, 0x0000FFFF + 8, SINSTR_ADC_IMMIDIATE);
	memory_write_byte(mem, 0x0000FFFF + 9, (u8_t)(REG_R7 << 4) | REG_R5);
	memory_write_word(mem, 0x0000FFFF + 10, 0x2000);
}