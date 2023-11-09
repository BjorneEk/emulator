#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "../../arch/interface.h"
#include "../../common/util/types.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define MEM (em->mem)
#define REGS (em->cpu->regs)
#define PC (em->cpu->pc)

emulator_t *new_emulator(cpu_t *cpu, memory_t *mem)
{
	emulator_t *res;

	res = malloc(sizeof(emulator_t));
	res->cpu = cpu;
	res->mem = mem;

	return res;
}

/*
u16_t absolute_ptr(emulator_t *em)
{
	u8_t regs;
	int rl, rh;

	regs = fetch_byte(em);
	rl = REGS[regs & 0x0F];
	rh = REGS[(regs >> 4)];

	return memory_read_word(MEM, rl + (rh << 16));
}
*/

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

u16_t data_from_addr_mode(emulator_t *em, int addr_mode)
{
	u8_t b0, b1;
	u16_t w0, w1;
	u16_t res;
	u32_t addr;

	switch (addr_mode) {
		case ADDR_MODE_RELATIVE:
			b0 = memory_read_byte(MEM, PC);
			res = memory_read_word(MEM, PC + b0);
			break;
		case ADDR_MODE_IMMIDIATE:
			res = memory_read_word(MEM, PC);
			break;
		case ADDR_MODE_REG:
			res = REGS[memory_read_byte(MEM, PC) >> 4];
			break;
		case ADDR_MODE_ABS:
			addr = memory_read_long(MEM, PC);
			res = memory_read_word(MEM, addr);
			break;
		case ADDR_MODE_ABS_PTR:
			b0 = memory_read_byte(MEM, PC);
			addr = (REGS[b0 >> 4] << 16) | REGS[b0 & 0x0F];
			res = memory_read_word(MEM, addr);
			break;
		case ADDR_MODE_ABS_IDX:
		 	addr = memory_read_long(MEM, PC);
			b0 = memory_read_byte(MEM, PC + 4);
			addr += REGS[b0 >> 4];
			res = memory_read_word(MEM, addr);
			break;
		case ADDR_MODE_ABS_PTR_IDX:
			b0 = memory_read_byte(MEM, PC);
			b1 = memory_read_byte(MEM, PC + 1);
			addr = (REGS[b0 >> 4] << 16) | REGS[b0 & 0x0F];
			addr += REGS[b1 >> 4];
			res = memory_read_word(MEM, addr);
			break;
		case ADDR_MODE_ABS_PTR_OFF:
			b0 = memory_read_byte(MEM, PC);
			w0 = memory_read_word(MEM, PC + 1);
			addr = (REGS[b0 >> 4] << 16) | REGS[b0 & 0x0F];
			addr += w0;
			res = memory_read_word(MEM, addr);
			break;
		case ADDR_MODE_ZP_PTR:
			b0 = memory_read_byte(MEM, PC);
			addr = REGS[b0 >> 4];
			res = memory_read_word(MEM, addr);
			break;
		case ADDR_MODE_ZP_OFF:
			b0 = memory_read_byte(MEM, PC);
			w0 = memory_read_word(MEM, PC + 1);
			addr = REGS[b0 >> 4] + w0;
			res = memory_read_word(MEM, addr);
			break;
		case ADDR_MODE_ZP_IDX:
			b0 = memory_read_byte(MEM, PC);
			addr = REGS[b0 >> 4] + REGS[b0 & 0x0F];
			res = memory_read_word(MEM, addr);
			break;
	}
	PC += addressing_mode_size[addr_mode];
	return res;
}

i32_t execute(emulator_t *em)
{
	u8_t opcode;
	u8_t reg_byte;
	u16_t *reg_d, *reg_s;
	u32_t alu_res;
	int addr_mode;

	opcode = memory_read_byte(MEM, PC);

	switch (opcode) {
		#define SINSTR_ADC(addr_mode) case SINSTR_ADC_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADC)
			addr_mode = opcode - INSTR_ADC;

			reg_byte = memory_read_byte(MEM, PC + 1);
			reg_d = REGS + (reg_byte >> 4);
			reg_s = REGS + (reg_byte & 0x0F);

			PC += instruction_size[INSTR_ADC];

			alu_res = *reg_s + data_from_addr_mode(em, addr_mode);

			break;
		#undef SINSTR_ADC
	}

	return 0;
}