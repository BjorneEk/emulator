#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "../../instructions/interface.h"
#include "util/types.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define REGS (em->cpu->regs)
#define PC (em->cpu->pc)

void init_emulator(emulator_t *em)
{
	em->cpu = calloc(1, sizeof(cpu_t));
	em->mem = calloc(1, sizeof(memory_t));
}

u8_t fetch_byte(emulator_t *em)
{
	u8_t byte;

	byte = mem_read(em->mem, PC);
	PC++;
	return byte;
}

u16_t read_word(emulator_t *em, int addr)
{
	u8_t b1, b2;

	b1 = mem_read(em->mem, addr);
	b2 = mem_read(em->mem, addr + 1);

	if (endian == LITTLE_ENDIAN)
		return b2 + (b1 << 8);
	return b1 + (b2 << 8);
}

u16_t absolute_ptr(emulator_t *em)
{
	u8_t regs;
	int rl, rh;

	regs = fetch_byte(em);
	rl = REGS[regs & 0x0F];
	rh = REGS[(regs >> 4)];

	return read_word(em, rl + (rh << 16));
}

static void print_tolower(char *str)
{
	int i;

	for (i = 0; str[i] != '\0'; ++i)
		putchar(tolower(str[i]));
	putchar('\n');
}
char *name_map[] = {
#define MAP_ENTRY(name) [INSTR_##name] = #name,
	XMACRO_INSTRUCTIONS(MAP_ENTRY)
#undef MAP_ENTRY
};
void print_instruction_(int ins)
{
	print_tolower(name_map[ins]);
}

void print_instruction(int ins)
{
	switch(ins) {
		#define PRINT_INSTRUCTION(name)				\
			case INSTR_##name: print_tolower(#name); break;
			XMACRO_INSTRUCTIONS(PRINT_INSTRUCTION)
		#undef PRINT_INSTRUCTION
	}
}

i32_t execute(emulator_t *em)
{
	u8_t opcode;
	int addr_mode;

	opcode = fetch_byte(em);

	switch (opcode) {
		#define SINSTR_ADC(addr_mode) case SINSTR_ADC_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADC)
			addr_mode = opcode - INSTR_ADC;
			break;
		#undef SINSTR_ADC
	}

	return 0;
}