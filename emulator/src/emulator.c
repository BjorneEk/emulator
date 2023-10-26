#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "../../instructions/interface.h"

#include "util/types.h"

#define PC (em->cpu.pc)

u8_t fetch_byte(emulator_t *em)
{
	u8_t byte;

	byte = mem_read(&em->mem, PC);
	PC++;
	return byte;
}

u16_t read_word(emulator_t *em, int addr)
{
	u8_t b1, b2;

	b1 = mem_read(&em->mem, addr);
	b2 = mem_read(&em->mem, addr + 1);

	if (endian == LITTLE_ENDIAN)
		return b2 + (b1 << 8);
	return b1 + (b2 << 8);
}

u16_t absolute_ptr(emulator_t *em)
{
	u8_t regs;
	int rl, rh;

	regs = fetch_byte(em);
	rl = em->cpu.regs[regs & 0x0F];
	rh = em->cpu.regs[(regs >> 4)];

	return read_word(em, rl + (rh << 16));
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
}