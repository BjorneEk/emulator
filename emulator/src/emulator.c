#include "emulator.h"
#include "IO/video_card.h"
#include "cpu.h"
#include "memory.h"
#include "IO/io_emulator.h"
#include "../../arch/interface.h"
#include "../../common/util/types.h"
#include "../../common/util/error.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_long(emulator_t *em, u32_t address, u32_t data)
{
	memory_write_long(em->mem, address, data);
}

static void write_byte(emulator_t *em, u32_t address, u8_t data)
{
	switch (address) {
		case 0:
			BUG("%c", (char)data);
			break;
		case ADDRESS_PORTA:
			io_write_porta(em->io, ((u16_t)data) << 8, IO_INTERNAL_ACCESS);
			break;
		case ADDRESS_PORTA + 1:
			io_write_porta(em->io, data, IO_INTERNAL_ACCESS);
			break;
		case ADDRESS_DDRA:
			io_write_ddra(em->io, ((u16_t)data) << 8);
			break;
		case ADDRESS_DDRA + 1:
			io_write_ddra(em->io, data);
			break;
		case ADDRESS_PORTB:
			io_write_portb(em->io, ((u16_t)data) << 8, IO_INTERNAL_ACCESS);
			break;
		case ADDRESS_PORTB + 1:
			io_write_portb(em->io, data, IO_INTERNAL_ACCESS);
			break;
		case ADDRESS_DDRB:
			io_write_ddrb(em->io, ((u16_t)data) << 8);
			break;
		case ADDRESS_DDRB + 1:
			io_write_ddrb(em->io, data);
			break;
		case ADDRESS_GRAPHICS_CARD_COMM:
			vc_send(em->vc, data);
			break;
		default:
			memory_write_byte(em->mem, address, data);
	}
}

static void write_word(emulator_t *em, u32_t address, u16_t data)
{
	switch (address) {
		case 0:
			BUG("%i, %c\n", data, (char)data);
			break;
		case ADDRESS_PORTA:
			io_write_porta(em->io, data, IO_INTERNAL_ACCESS);
			break;
		case ADDRESS_PORTA + 1: exit_error("unaligned write to memory mapped io: 'port a'");
		case ADDRESS_DDRA:
			io_write_ddra(em->io, data);
			break;
		case ADDRESS_DDRA + 1: exit_error("unaligned write to memory mapped io: 'ddr a'");
		case ADDRESS_PORTB:
			io_write_portb(em->io, data, IO_INTERNAL_ACCESS);
			break;
		case ADDRESS_PORTB + 1: exit_error("unaligned write to memory mapped io: 'port b'");
		case ADDRESS_DDRB:
			io_write_ddrb(em->io, data);
			break;
		case ADDRESS_DDRB + 1: exit_error("unaligned write to memory mapped io: 'ddr b'");
		case ADDRESS_GRAPHICS_CARD_COMM:
			vc_send(em->vc, data); // might be bad, maybe exit_error
			break;
		default:
			memory_write_word(em->mem, address, data);
	}
}

static u32_t read_long(emulator_t *em, u32_t address)
{
	return memory_read_long(em->mem, address);
}

static u16_t read_word(emulator_t *em, u32_t address)
{
	switch (address) {
		case 0:
			error("read NULL\n");
			return 0;
		case ADDRESS_PORTA:
			return io_read_porta(em->io, IO_INTERNAL_ACCESS);
			break;
		case ADDRESS_PORTA + 1: exit_error("unaligned read from memory mapped io: 'port a'");
		case ADDRESS_DDRA:
			return io_read_ddra(em->io);
		case ADDRESS_DDRA + 1: exit_error("unaligned read from memory mapped io: 'ddr a'");
		case ADDRESS_PORTB:
			return io_read_portb(em->io, IO_INTERNAL_ACCESS);
		case ADDRESS_PORTB + 1: exit_error("unaligned read from memory mapped io: 'port b'");
		case ADDRESS_DDRB:
			return io_read_ddrb(em->io);
		case ADDRESS_DDRB + 1: exit_error("unaligned read from memory mapped io: 'ddr b'");
		default:
			return memory_read_word(em->mem, address);
	}
}

static u8_t read_byte(emulator_t *em, u32_t address)
{
	switch (address) {
		case 0:
			error("read NULL\n");
			return 0;
		case ADDRESS_PORTA:
			return io_read_porta(em->io, IO_INTERNAL_ACCESS) >> 8;
		case ADDRESS_PORTA + 1:
			return io_read_porta(em->io, IO_INTERNAL_ACCESS);
		case ADDRESS_DDRA:
			return io_read_ddra(em->io) >> 8;
		case ADDRESS_DDRA + 1:
			return io_read_ddra(em->io);
		case ADDRESS_PORTB:
			return io_read_portb(em->io, IO_INTERNAL_ACCESS) >> 8;
		case ADDRESS_PORTB + 1:
			return io_read_portb(em->io, IO_INTERNAL_ACCESS);
		case ADDRESS_DDRB:
			return io_read_ddrb(em->io) >> 8;
		case ADDRESS_DDRB + 1:
			return io_read_ddrb(em->io);
		default:
			return memory_read_byte(em->mem, address);
	}
}

static u8_t fetch_byte(emulator_t *em)
{
	u8_t byte_res;

	byte_res = memory_read_byte(em->mem, em->cpu->pc);
	em->cpu->pc += 1;

	return byte_res;
}

static u16_t fetch_word(emulator_t *em)
{
	u16_t word_res;

	word_res = memory_read_word(em->mem, em->cpu->pc);
	em->cpu->pc += 2;

	return word_res;
}

static u32_t fetch_long(emulator_t *em)
{
	u32_t long_res;

	long_res = memory_read_long(em->mem, em->cpu->pc);
	em->cpu->pc += 4;

	return long_res;
}

static void fetch_reg_pair(emulator_t *em, u16_t **reg_h, u16_t **reg_l)
{
	u8_t reg_byte;

	reg_byte = fetch_byte(em);

	if (reg_h != NULL)
		*reg_h = em->cpu->regs + (reg_byte >> 4);
	if (reg_l != NULL)
		*reg_l = em->cpu->regs + (reg_byte & 0x0F);
}

emulator_t *new_emulator(cpu_t *cpu, memory_t *mem, io_t *io, video_card_t *vc)
{
	emulator_t *res;

	res = malloc(sizeof(emulator_t));
	res->cpu	= cpu;
	res->mem	= mem;
	res->io		= io;
	res->vc		= vc;
	return res;
}

static u32_t get_absolute_address(emulator_t *em , addressing_mode_t addr_mode)
{
	u16_t *reg_h, *reg_l, *reg_i;
	u16_t offset;
	u32_t absolute_value;

	switch (addr_mode) {
		case ADDR_MODE_RELATIVE:
		case ADDR_MODE_IMMIDIATE:
		case ADDR_MODE_REG:
			ERR();
			return 0;
		case ADDR_MODE_ABS:
			return fetch_long(em);
		case ADDR_MODE_ABS_PTR:
			fetch_reg_pair(em, &reg_h, &reg_l);
			return (*reg_h << 16) | *reg_l;
		case ADDR_MODE_ABS_IDX:
		 	absolute_value = fetch_long(em);
			fetch_reg_pair(em, &reg_i, NULL);
			return absolute_value + *reg_i;
		case ADDR_MODE_ABS_PTR_IDX:
			fetch_reg_pair(em, &reg_h, &reg_l);
			fetch_reg_pair(em, &reg_i, NULL);
			return ((*reg_h << 16) | *reg_l) + *reg_i;
		case ADDR_MODE_ABS_PTR_OFF:
			fetch_reg_pair(em, &reg_h, &reg_l);
			offset = fetch_word(em);
			return ((*reg_h << 16) | *reg_l) + offset;
		case ADDR_MODE_ZP_PTR:
			fetch_reg_pair(em, &reg_h, NULL);
			return (u32_t)*reg_h;
		case ADDR_MODE_ZP_OFF:
			fetch_reg_pair(em, &reg_h, NULL);
			offset = fetch_word(em);
			return *reg_h + offset;
		case ADDR_MODE_ZP_IDX:
			fetch_reg_pair(em, &reg_h, &reg_l);
			return *reg_h + *reg_l;

		default:
			ERR();
			return 0;
	}
}

static u16_t get_word_indirect(emulator_t *em, addressing_mode_t addr_mode, bool is_byte)
{
	u16_t *reg;

	switch (addr_mode) {
		case ADDR_MODE_RELATIVE:
			ERR();
			return 0;
		case ADDR_MODE_IMMIDIATE:
			return fetch_word(em);
		case ADDR_MODE_REG:
			fetch_reg_pair(em, &reg, NULL);
			return *reg;
		#define CASE_OF(am) case ADDR_MODE_##am :
		XMACRO_DIRECT_ADDRESSING_MODES(CASE_OF)
			if (is_byte)
				return read_byte(em, get_absolute_address(em, addr_mode));
			return read_word(em, get_absolute_address(em, addr_mode));
		#undef CASE_OF

		default:
			ERR();
			return 0;
	}
}

static u32_t get_long_indirect(emulator_t *em, addressing_mode_t addr_mode)
{
	switch (addr_mode) {
		case ADDR_MODE_RELATIVE:
		case ADDR_MODE_IMMIDIATE:
		case ADDR_MODE_REG:
			ERR();
			return 0;
		#define CASE_OF(am) case ADDR_MODE_##am :
		XMACRO_DIRECT_ADDRESSING_MODES(CASE_OF)
			return read_long(em, get_absolute_address(em, addr_mode));
		#undef CASE_OF

		default:
			ERR();
			return 0;
	}
}

static void set_zn_flags(emulator_t *em, u16_t alu_res)
{
	if (alu_res == 0)
		cpu_set_flag(em->cpu, FLAG_ZERO);
	else
		cpu_clear_flag(em->cpu, FLAG_ZERO);

	if ((i16_t)alu_res < 0)
		cpu_set_flag(em->cpu, FLAG_NEGATIVE);
	else
		cpu_clear_flag(em->cpu, FLAG_NEGATIVE);
}

static void set_zn_flags_wide(emulator_t *em, u32_t alu_res)
{
	if (alu_res == 0)
		cpu_set_flag(em->cpu, FLAG_ZERO);
	else
		cpu_clear_flag(em->cpu, FLAG_ZERO);

	if ((i32_t)alu_res < 0)
		cpu_set_flag(em->cpu, FLAG_NEGATIVE);
	else
		cpu_clear_flag(em->cpu, FLAG_NEGATIVE);
}

static void set_carry(emulator_t *em, u32_t alu_res)
{
	if (alu_res > 0xFFFF)
		cpu_set_flag(em->cpu, FLAG_CARRY);
	else
		cpu_clear_flag(em->cpu, FLAG_CARRY);
}

static void set_carry_wide(emulator_t *em, u64_t alu_res)
{
	if (alu_res > 0xFFFFFFFF)
		cpu_set_flag(em->cpu, FLAG_CARRY);
	else
		cpu_clear_flag(em->cpu, FLAG_CARRY);
}

typedef u32_t (*perform_binop_t)(emulator_t*, u16_t, u16_t);

static u32_t adc_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	u32_t alu_res;

	alu_res = reg_data + ext_data;

	if (cpu_get_flag(em->cpu, FLAG_CARRY))
		alu_res += 1;

	set_carry(em, alu_res);

	return alu_res;
}

static u32_t add_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	return reg_data + ext_data;
}

static u32_t sbc_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	return adc_binop_func(em, reg_data, (u16_t)-ext_data);
}

static u32_t sub_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	return add_binop_func(em, reg_data, (u16_t)-ext_data);
}

static u32_t eor_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	return reg_data ^ ext_data;
}

static u32_t orr_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	return reg_data | ext_data;
}

static u32_t and_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	return reg_data & ext_data;
}

static void binop(emulator_t *em, int addr_mode, perform_binop_t perform_op)
{
	u16_t *reg_d, *reg_s;
	u16_t rhs;
	u32_t alu_res;

	fetch_reg_pair(em, &reg_d, &reg_s);

	rhs = get_word_indirect(em, addr_mode, false);
	alu_res = perform_op(em, *reg_s, rhs);

	set_zn_flags(em, alu_res);

	*reg_d = alu_res;
}

typedef u64_t (*perform_binop_wide_t)(emulator_t*, u32_t, u16_t);

static u64_t adcw_binop_func(emulator_t *em, u32_t reg_data, u16_t ext_data)
{
	u64_t alu_res;

	alu_res = reg_data + ext_data;

	if (cpu_get_flag(em->cpu, FLAG_CARRY))
		alu_res += 1;

	set_carry_wide(em, alu_res);

	return alu_res;
}

static u64_t addw_binop_func(emulator_t *em, u32_t reg_data, u16_t ext_data)
{
	return reg_data + ext_data;
}

static u64_t sbcw_binop_func(emulator_t *em, u32_t reg_data, u16_t ext_data)
{
	return adcw_binop_func(em, reg_data, (u16_t)-ext_data);
}

static u64_t subw_binop_func(emulator_t *em, u32_t reg_data, u16_t ext_data)
{
	return addw_binop_func(em, reg_data, (u16_t)-ext_data);
}

static void binop_wide(emulator_t *em, int addr_mode, perform_binop_wide_t perform_op_wide)
{
	u16_t *reg_dh, *reg_dl;
	u32_t lhs;
	u16_t rhs;
	u64_t alu_res;

	fetch_reg_pair(em, &reg_dh, &reg_dl);

	lhs = (u32_t)(*reg_dh) << 16 | (u32_t)(*reg_dl);

	rhs = get_word_indirect(em, addr_mode, false);
	alu_res = perform_op_wide(em, lhs, rhs);

	set_zn_flags_wide(em, alu_res);

	*reg_dh = (u32_t)alu_res >> 16;
	*reg_dl = (u32_t)alu_res & 0xFFFF;
}

static void unop(emulator_t *em, int ins, int addr_mode)
{
	u16_t *reg;
	u32_t alu_res;

	fetch_reg_pair(em, &reg, NULL);

	switch (ins) {
		case INSTR_ASR:
			alu_res = *reg >> 1;
			set_zn_flags(em, alu_res);
			break;
		case INSTR_LSR:
			alu_res = *reg >> 1;
			break;
		case INSTR_LSL:
			alu_res = *reg << 1;
			break;
		case INSTR_NOT:
			alu_res = ~*reg;
			set_zn_flags(em, alu_res);
			break;
		case INSTR_DEC:
			alu_res = *reg - 1;
			set_zn_flags(em, alu_res);
			break;
		case INSTR_INC:
			alu_res = *reg + 1;
			set_zn_flags(em, alu_res);
			break;
		case INSTR_CRB:
			alu_res = *reg & ~(1 << get_word_indirect(em, addr_mode, false));
			set_zn_flags(em, alu_res);
			break;
		case INSTR_SRB:
			alu_res = *reg | (1 << get_word_indirect(em, addr_mode, false));
			set_zn_flags(em, alu_res);
			break;
		default:
			ERR();
			break;
	}

	*reg = (u16_t)alu_res;
}

static void unop_wide(emulator_t *em, int ins)
{
	u16_t *reg_dh, *reg_dl;
	u32_t lhs;
	u64_t alu_res;

	fetch_reg_pair(em, &reg_dh, &reg_dl);

	lhs = (u32_t)(*reg_dh) << 16 | (u32_t)(*reg_dl);

	switch (ins) {
		case INSTR_DECW:
			alu_res = lhs - 1;
			set_zn_flags_wide(em, alu_res);
			break;
		case INSTR_INCW:
			alu_res = lhs + 1;
			set_zn_flags_wide(em, alu_res);
			break;

		default:
			ERR();
			break;
	}

	*reg_dh = (u32_t)alu_res >> 16;
	*reg_dl = (u32_t)alu_res & 0xFFFF;
}

static void relative_conditional_branch(emulator_t *em, bool should_branch)
{
	char rel;
	rel = fetch_byte(em);
	if (should_branch) {
		//BUG("Relative branch: %i result [0x%08X]\n", rel, (int)em->cpu->pc + rel);
		em->cpu->pc = ((int)em->cpu->pc + rel);
	}
}

static void branch_on_bit_in_register(emulator_t *em, bool bit_should_be_set)
{
	u8_t value_reg_byte;
	u8_t value;
	u16_t reg;

	value_reg_byte = fetch_byte(em);

	value = value_reg_byte >> 4;
	reg = em->cpu->regs[value_reg_byte & 0x0F];

	relative_conditional_branch(em, (reg & 1 << value) == bit_should_be_set);
}

static void load_register(emulator_t *em, int addr_mode, bool is_byte)
{
	u16_t *reg;

	fetch_reg_pair(em, &reg, NULL);

	*reg = get_word_indirect(em, addr_mode, is_byte);
}

static void load_register_wide(emulator_t *em, int addr_mode)
{
	u16_t *reg_h, *reg_l;
	u32_t long_data;

	fetch_reg_pair(em, &reg_h, &reg_l);

	long_data = get_long_indirect(em, addr_mode);

	*reg_h = long_data >> 16;
	*reg_l = long_data & 0x00FF;
}

static void store_register(emulator_t *em, int addr_mode, bool is_byte)
{
	u16_t *reg;
	u32_t addr;

	fetch_reg_pair(em, &reg, NULL);

	addr = get_absolute_address(em, addr_mode);

	if (is_byte)
		write_byte(em, addr, *reg & 0xFF);
	else
		write_word(em, addr, *reg);
}

static void	push_long(emulator_t *em, u32_t val)
{
	em->cpu->regs[REG_STACK_POINTER] -= 4;
	memory_write_long(em->mem, em->cpu->regs[REG_STACK_POINTER], val);
}

static u32_t	pop_long(emulator_t *em)
{
	u32_t res;
	res = memory_read_long(em->mem, em->cpu->regs[REG_STACK_POINTER]);
	em->cpu->regs[REG_STACK_POINTER] += 4;
	return res;
}

static void copy_register_pair(emulator_t *em)
{
	u16_t *reg_dh, *reg_dl, *reg_sh, *reg_sl;

	fetch_reg_pair(em, &reg_dh, &reg_dl);
	fetch_reg_pair(em, &reg_sh, &reg_sl);

	*reg_dh = *reg_sh;
	*reg_dl = *reg_sl;
}

static void interrupt(emulator_t *em)
{
	push_long(em, em->cpu->pc);
	em->cpu->pc = em->cpu->interrupt_handler_location;
}

int emulator_execute(emulator_t *em)
{
	u8_t	opcode;
	int	addr_mode;
	u32_t	next_pc;

	if (em->cpu->is_reset) {
		//memory_debug(em->mem, em->cpu->boot_location, 16);
		em->cpu->pc = memory_read_long(em->mem, em->cpu->boot_location);
		//memory_debug(em->mem, em->cpu->pc, 16);
		em->cpu->interrupt_handler_location = memory_read_long(em->mem, em->cpu->boot_location + 4);
		em->cpu->is_reset = false;
		return 0;
	}

	if (cpu_get_flag(em->cpu, FLAG_INTERRUPT) && io_irq(em->io)) {
		cpu_clear_flag(em->cpu, FLAG_INTERRUPT);
		interrupt(em);
		//BUG("INTERRUPT\n");
	} else if (em->cpu->nmi) {
		em->cpu->nmi = false;
		interrupt(em);
	}
	//BUG("PC: [0x%08X]\n", em->cpu->pc);
	opcode = fetch_byte(em);
	//BUG("Instruction: ");
	//print_sinstr(opcode);

	switch (opcode) {
		#define SINSTR_LDR(addr_mode) case SINSTR_LDR_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_LDR)
			addr_mode = opcode - INSTR_LDR + ADDR_MODE_IMMIDIATE;
			load_register(em, addr_mode, false);
			break;
		#undef SINSTR_LDR

		#define SINSTR_LDRB(addr_mode) case SINSTR_LDRB_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_LDRB)
			addr_mode = opcode - INSTR_LDRB + ADDR_MODE_IMMIDIATE;
			load_register(em, addr_mode, true);
			break;
		#undef SINSTR_LDRB

		#define SINSTR_LDRW(addr_mode) case SINSTR_LDRW_##addr_mode :
		XMACRO_DIRECT_ADDRESSING_MODES(SINSTR_LDRW)
			addr_mode = opcode - INSTR_LDRW + ADDR_MODE_ABS;
			load_register_wide(em, addr_mode);
			break;
		#undef SINSTR_LDRW

		#define SINSTR_STR(addr_mode) case SINSTR_STR_##addr_mode :
		XMACRO_DIRECT_ADDRESSING_MODES(SINSTR_STR)
			addr_mode = opcode - INSTR_STR + ADDR_MODE_ABS;
			store_register(em, addr_mode, false);
			break;
		#undef SINSTR_STR

		#define SINSTR_STRB(addr_mode) case SINSTR_STRB_##addr_mode :
		XMACRO_DIRECT_ADDRESSING_MODES(SINSTR_STRB)
			addr_mode = opcode - INSTR_STRB + ADDR_MODE_ABS;
			store_register(em, addr_mode, true);
			break;
		#undef SINSTR_STRB

		case SINSTR_CPRP:
			copy_register_pair(em);
			break;

		case SINSTR_BZ:
			relative_conditional_branch(em, cpu_get_flag(em->cpu, FLAG_ZERO));
			break;
		case SINSTR_BNZ:
			relative_conditional_branch(em, !cpu_get_flag(em->cpu, FLAG_ZERO));
			break;
		case SINSTR_BCC:
			relative_conditional_branch(em, !cpu_get_flag(em->cpu, FLAG_CARRY));
			break;
		case SINSTR_BCS:
			relative_conditional_branch(em, cpu_get_flag(em->cpu, FLAG_CARRY));
			break;
		case SINSTR_BRN:
			relative_conditional_branch(em, cpu_get_flag(em->cpu, FLAG_NEGATIVE));
			break;
		case SINSTR_BRP:
			relative_conditional_branch(em, !cpu_get_flag(em->cpu, FLAG_NEGATIVE));
			break;
		case SINSTR_BRA:
			relative_conditional_branch(em, true);
			break;
		case SINSTR_BBS:
			branch_on_bit_in_register(em, true);
			break;
		case SINSTR_BBC:
			branch_on_bit_in_register(em, false);
			break;

		case SINSTR_LBRA_ABS:
		case SINSTR_LBRA_ABS_PTR:
			addr_mode = opcode - INSTR_LBRA + ADDR_MODE_ABS;
			em->cpu->pc = get_absolute_address(em, addr_mode);
			break;

		case SINSTR_CALL_ABS:
		case SINSTR_CALL_ABS_PTR:
		case SINSTR_CALL_ZP_PTR:
			addr_mode = opcode - INSTR_CALL + ADDR_MODE_ABS;
			next_pc = get_absolute_address(em, addr_mode);
			//BUG("Call: [0x%08X]\n", next_pc);
			push_long(em, em->cpu->pc);
			em->cpu->pc = next_pc;
			break;

		case SINSTR_RTI:
			cpu_set_flag(em->cpu, FLAG_INTERRUPT);
		case SINSTR_RET:
			em->cpu->pc = pop_long(em);
			//BUG("Return to: [0x%08X]\n", em->cpu->pc);
			break;

		#define SINSTR_ADC(addr_mode) case SINSTR_ADC_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADC)
			addr_mode = opcode - INSTR_ADC + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, adc_binop_func);
			break;
		#undef SINSTR_ADC

		#define SINSTR_ADD(addr_mode) case SINSTR_ADD_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADD)
			addr_mode = opcode - INSTR_ADD + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, add_binop_func);
			break;
		#undef SINSTR_ADD

		#define SINSTR_ADCW(addr_mode) case SINSTR_ADCW_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADCW)
			addr_mode = opcode - INSTR_ADCW + ADDR_MODE_IMMIDIATE;
			binop_wide(em, addr_mode, adcw_binop_func);
			break;
		#undef SINSTR_ADCW

		#define SINSTR_ADDW(addr_mode) case SINSTR_ADDW_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADDW)
			addr_mode = opcode - INSTR_ADDW + ADDR_MODE_IMMIDIATE;
			binop_wide(em, addr_mode, addw_binop_func);
			break;
		#undef SINSTR_ADDW

		#define SINSTR_SBC(addr_mode) case SINSTR_SBC_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_SBC)
			addr_mode = opcode - INSTR_SBC + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, sbc_binop_func);
			break;
		#undef SINSTR_SBC

		#define SINSTR_SUB(addr_mode) case SINSTR_SUB_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_SUB)
			addr_mode = opcode - INSTR_SUB + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, sub_binop_func);
			break;
		#undef SINSTR_SUB

		#define SINSTR_SBCW(addr_mode) case SINSTR_SBCW_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_SBCW)
			addr_mode = opcode - INSTR_SBCW + ADDR_MODE_IMMIDIATE;
			binop_wide(em, addr_mode, sbcw_binop_func);
			break;
		#undef SINSTR_SBCW

		#define SINSTR_SUBW(addr_mode) case SINSTR_SUBW_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_SUBW)
			addr_mode = opcode - INSTR_SUBW + ADDR_MODE_IMMIDIATE;
			binop_wide(em, addr_mode, subw_binop_func);
			break;
		#undef SINSTR_SBCW

		#define SINSTR_EOR(addr_mode) case SINSTR_EOR_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_EOR)
			addr_mode = opcode - INSTR_EOR + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, eor_binop_func);
			break;
		#undef SINSTR_EOR

		#define SINSTR_ORR(addr_mode) case SINSTR_ORR_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ORR)
			addr_mode = opcode - INSTR_ORR + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, orr_binop_func);
			break;
		#undef SINSTR_ORR

		#define SINSTR_AND(addr_mode) case SINSTR_AND_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_AND)
			addr_mode = opcode - INSTR_AND + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, and_binop_func);
			break;
		#undef SINSTR_AND

		case SINSTR_ASR:
			unop(em, INSTR_ASR, ADDR_MODE_NULL);
			break;
		case SINSTR_LSR:
			unop(em, INSTR_LSR, ADDR_MODE_NULL);
			break;
		case SINSTR_LSL:
			unop(em, INSTR_LSL, ADDR_MODE_NULL);
			break;
		case SINSTR_NOT:
			unop(em, INSTR_NOT, ADDR_MODE_NULL);
			break;
		case SINSTR_DEC:
			unop(em, INSTR_DEC, ADDR_MODE_NULL);
			break;
		case SINSTR_DECW:
			unop_wide(em, INSTR_DECW);
			break;
		case SINSTR_INC:
			unop(em, INSTR_INC, ADDR_MODE_NULL);
			break;
		case SINSTR_INCW:
			unop_wide(em, INSTR_INCW);
			break;

		#define SINSTR_CRB(addr_mode) case SINSTR_CRB_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_CRB)
			addr_mode = opcode - INSTR_CRB + ADDR_MODE_IMMIDIATE;
			unop(em, INSTR_CRB, addr_mode);
			break;
		#undef SINSTR_CRB

		#define SINSTR_SRB(addr_mode) case SINSTR_SRB_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_SRB)
			addr_mode = opcode - INSTR_SRB + ADDR_MODE_IMMIDIATE;
			unop(em, INSTR_SRB, addr_mode);
			break;
		#undef SINSTR_SRB

		case SINSTR_BRK:
			return 1;
		case SINSTR_NOP:
			break;

		default:
			exit_error("Unhandled opcode: %02X\n", opcode);
	}

	return 0;
}

void emulator_debug(emulator_t *em)
{
	long i, j, k, cnt;
	u8_t r;
	u32_t addr;

	printf("emulator status\n");
	printf("Registers:\n");
	for (i = 0; i < 12; i++)
		printf("r%li: %04X %u\n", i, em->cpu->regs[i], em->cpu->regs[i]);

	printf("PC: %08X %u\n", em->cpu->pc, em->cpu->pc);
	printf("SP: %04X %u\n\n", em->cpu->regs[12], em->cpu->regs[12]);

	printf("STATUS REGISTER: \n");
	printf("┌─┬─┬─┬─┬─┬─┬─┬─┐\n");
	printf("│C│Z│I│U│B│U│V│N│\n");
	printf("│%i│%i│%i│%i│%i│%i│%i│%i│\n",
		cpu_get_flag(em->cpu, FLAG_CARRY),
		cpu_get_flag(em->cpu, FLAG_ZERO),
		cpu_get_flag(em->cpu, FLAG_INTERRUPT),
		0,
		cpu_get_flag(em->cpu, FLAG_BREAK),
		0,
		cpu_get_flag(em->cpu, FLAG_OVERFLOW),
		cpu_get_flag(em->cpu, FLAG_NEGATIVE));
	printf("└─┴─┴─┴─┴─┴─┴─┴─┘\n");

	/*
	printf("MEMORY:\n");
	printf("┌─────────┬─────────────────────────┬─────────────────────────┬────────┬────────┐\n");

	for (i = r = cnt = 0; i < MEMORY_SIZE - 1; i += 0x10) {
		if(i != 0 && !memcmp(&em->mem->data[i], &em->mem->data[i - 0x10], 0x10) &&
			(em->cpu->pc < i || em->cpu->pc > (i + 0x0F)) && (em->cpu->regs[12] < i || em->cpu->regs[12] > (i + 0x0F))){
			r = 1; cnt++; continue;
		}

		if (r) {
			printf("│*%7li │                         │                         │        │        │\n",cnt);
			r = cnt = 0;
		}

		printf("│%08lx ", i);

		j = 0;
		while (j < 2) {
			printf("│ ");
			for(k = 0; k < 0x8; k++) {
				addr = (i+k+(j*0x8));
				if (addr == em->cpu->pc)
					printf("\033[43m%02x\033[0m ", em->mem->data[addr]);
				else if (addr == em->cpu->regs[12])
					printf("\033[42m%02x\033[0m ", em->mem->data[addr]);
				else
					printf("%02x ", em->mem->data[addr]);
			}
			j++;
		}

		j = 0;
		while (j < 2) {
			printf("│");
			for(k = 0; k < 0x8; k++) {
				addr = (i+k+(j*0x8));
				if(31 < em->mem->data[addr] && 127 > em->mem->data[addr])
					printf("%c", (char)em->mem->data[addr]);
				else if(em->mem->data[addr] == 0)
					printf("0");
				else
					printf("×");
			}
			j++;
		}
		printf("│");
		printf("\n");
	}
	printf("└─────────┴─────────────────────────┴─────────────────────────┴────────┴────────┘\n");
	*/
}