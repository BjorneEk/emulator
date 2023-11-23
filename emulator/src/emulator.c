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
#define SP (em->cpu->regs[REG_STACK_POINTER])

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

static void fetch_regs(emulator_t *em, u16_t **reg_h, u16_t **reg_l)
{
	u8_t reg_byte;

	reg_byte = fetch_byte(em);

	if (reg_h != NULL)
		*reg_h = em->cpu->regs + (reg_byte >> 4);
	if (reg_l != NULL)
		*reg_l = em->cpu->regs + (reg_byte & 0x0F);
}

emulator_t *new_emulator(cpu_t *cpu, memory_t *mem)
{
	emulator_t *res;

	res = malloc(sizeof(emulator_t));
	res->cpu = cpu;
	res->mem = mem;

	return res;
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

static void print_instruction(int ins)
{
	print_tolower(name_map[ins]);
}

static u32_t addr_from_addr_mode(emulator_t *em , addressing_mode_t addr_mode)
{
	u8_t 	b0, b1;
	u16_t 	w0;
	u32_t 	addr;

	switch (addr_mode) {
		case ADDR_MODE_RELATIVE:
			b0 = memory_read_byte(MEM, PC);
			addr = PC + b0;
			break;
		case ADDR_MODE_IMMIDIATE:
			break;
		case ADDR_MODE_REG:
			break;
		case ADDR_MODE_ABS:
			addr = memory_read_long(MEM, PC);
			break;
		case ADDR_MODE_ABS_PTR:
			b0 = memory_read_byte(MEM, PC);
			addr = (REGS[b0 >> 4] << 16) | REGS[b0 & 0x0F];
			break;
		case ADDR_MODE_ABS_IDX:
		 	addr = memory_read_long(MEM, PC);
			b0 = memory_read_byte(MEM, PC + 4);
			addr += REGS[b0 >> 4];
			break;
		case ADDR_MODE_ABS_PTR_IDX:
			b0 = memory_read_byte(MEM, PC);
			b1 = memory_read_byte(MEM, PC + 1);
			addr = (REGS[b0 >> 4] << 16) | REGS[b0 & 0x0F];
			addr += REGS[b1 >> 4];
			break;
		case ADDR_MODE_ABS_PTR_OFF:
			b0 = memory_read_byte(MEM, PC);
			w0 = memory_read_word(MEM, PC + 1);
			addr = (REGS[b0 >> 4] << 16) | REGS[b0 & 0x0F];
			addr += w0;
			break;
		case ADDR_MODE_ZP_PTR:
			b0 = memory_read_byte(MEM, PC);
			addr = REGS[b0 >> 4];
			break;
		case ADDR_MODE_ZP_OFF:
			b0 = memory_read_byte(MEM, PC);
			w0 = memory_read_word(MEM, PC + 1);
			addr = REGS[b0 >> 4] + w0;
			break;
		case ADDR_MODE_ZP_IDX:
			b0 = memory_read_byte(MEM, PC);
			addr = REGS[b0 >> 4] + REGS[b0 & 0x0F];
			break;
		case ADDR_MODE_NULL:
			addr = 0;
			break;
	}
	return addr;
}

static u16_t word_from_addr_mode(emulator_t *em, addressing_mode_t addr_mode)
{
	u32_t addr;

	if (addr_mode == ADDR_MODE_IMMIDIATE) {
		return memory_read_word(MEM, PC);
	} else if (addr_mode == ADDR_MODE_REG) {
		return REGS[memory_read_byte(MEM, PC) >> 4];
	} else {
		addr = addr_from_addr_mode(em, addr_mode);
		return memory_read_word(MEM, addr);
	}
}

static u32_t long_from_addr_mode(emulator_t *em, addressing_mode_t addr_mode)
{
	u32_t addr;

	addr = addr_from_addr_mode(em, addr_mode);
	return memory_read_long(em->mem, addr);
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

u32_t adc_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{	
	u32_t alu_res;

	alu_res = reg_data + ext_data;

	if (cpu_get_flag(em->cpu, FLAG_CARRY))
		alu_res += 1;

	set_carry(em, alu_res);

	return alu_res;
}

u32_t add_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{	
	return reg_data + ext_data;
}

u32_t sbc_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{	
	return adc_binop_func(em, reg_data, (u16_t)-ext_data);
}

u32_t sub_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{	
	return add_binop_func(em, reg_data, (u16_t)-ext_data);
}

u32_t eor_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	return reg_data ^ ext_data;
}

u32_t orr_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	return reg_data | ext_data;
}

u32_t and_binop_func(emulator_t *em, u16_t reg_data, u16_t ext_data)
{
	return reg_data & ext_data;
}

void binop(emulator_t *em, int addr_mode, perform_binop_t perform_op)
{
	u16_t *reg_d, *reg_s;
	u16_t rhs;
	u32_t alu_res;

	fetch_regs(em, &reg_d, &reg_s);

	rhs = word_from_addr_mode(em, addr_mode);
	alu_res = perform_op(em, *reg_s, rhs);

	set_zn_flags(em, alu_res);

	*reg_d = alu_res;
}

typedef u64_t (*perform_binop_wide_t)(emulator_t*, u32_t, u16_t);

u64_t adcw_binop_func(emulator_t *em, u32_t reg_data, u16_t ext_data)
{
	u64_t alu_res;

	alu_res = reg_data + ext_data;

	if (cpu_get_flag(em->cpu, FLAG_CARRY))
		alu_res += 1;

	set_carry_wide(em, alu_res);

	return alu_res;
}

u64_t addw_binop_func(emulator_t *em, u32_t reg_data, u16_t ext_data)
{
	return reg_data + ext_data;
}

u64_t sbcw_binop_func(emulator_t *em, u32_t reg_data, u16_t ext_data)
{
	return adcw_binop_func(em, reg_data, (u16_t)-ext_data);
}

u64_t subw_binop_func(emulator_t *em, u32_t reg_data, u16_t ext_data)
{
	return addw_binop_func(em, reg_data, (u16_t)-ext_data);
}

void binop_wide(emulator_t *em, int addr_mode, perform_binop_wide_t perform_op_wide)
{
	u16_t *reg_dh, *reg_dl;
	u32_t lhs;
	u16_t rhs;
	u64_t alu_res;

	fetch_regs(em, &reg_dh, &reg_dl);

	lhs = (u32_t)(*reg_dh) << 16 | (u32_t)(*reg_dl);

	rhs = word_from_addr_mode(em, addr_mode);
	alu_res = perform_op_wide(em, lhs, rhs);

	set_zn_flags_wide(em, alu_res);

	*reg_dh = alu_res >> 16;
	*reg_dl = alu_res & 0xFFFF;
}

void unop(emulator_t *em, int ins, int addr_mode)
{
	u16_t *reg;
	u32_t alu_res;

	fetch_regs(em, &reg, NULL);

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
			alu_res = *reg & ~(1 << word_from_addr_mode(em, addr_mode));
			set_zn_flags(em, alu_res);
			break;	
		case INSTR_SRB:
			alu_res = *reg | (1 << word_from_addr_mode(em, addr_mode));
			set_zn_flags(em, alu_res);
			break;	
	
		default:
			printf("not a unop: %d\n", ins);
			break;
	}

	*reg = (u16_t)alu_res;
}

void unop_wide(emulator_t *em, int ins)
{
	u16_t *reg_dh, *reg_dl;
	u32_t lhs;
	u64_t alu_res;

	fetch_regs(em, &reg_dh, &reg_dl);

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
			printf("not a wide unop: %d\n", ins);
			break;
	}

	*reg_dh = alu_res >> 16;
	*reg_dl = alu_res & 0xFFFF;
}

void cond_branch(emulator_t *em, bool should_branch)
{
	if (should_branch) {
		em->cpu->pc += fetch_byte(em);
	}
}

void branch_on_bit_in_register(emulator_t *em, bool bit_should_be_set)
{
	u8_t value_reg_byte;
	u8_t value;
	u16_t reg;

	value_reg_byte = fetch_byte(em);

	value = value_reg_byte >> 4;
	reg = em->cpu->regs[value_reg_byte & 0x0F];

	cond_branch(em, (reg & 1 << value) == bit_should_be_set);
}

void load_register(emulator_t *em, int addr_mode, bool is_byte)
{
	u16_t *reg;
	u16_t data;

	fetch_regs(em, &reg, NULL);

	data = word_from_addr_mode(em, addr_mode);

	*reg = is_byte ? data >> 8 : data;
}

void load_register_wide(emulator_t *em, int addr_mode)
{
	u16_t *reg_h, *reg_l;
	u32_t long_data;

	fetch_regs(em, &reg_h, &reg_l);

	long_data = long_from_addr_mode(em, addr_mode);

	*reg_h = long_data >> 16;
	*reg_l = long_data & 0x00FF;
}

void store_register(emulator_t *em, int addr_mode, bool is_byte)
{
	u16_t *reg;
	u32_t addr;

	fetch_regs(em, &reg, NULL);

	addr = addr_from_addr_mode(em, addr_mode);

	if (is_byte) {
		memory_write_byte(em->mem, addr, *reg & 0x0F);
	} else {
		memory_write_word(em->mem, addr, *reg);
	}
}

void call_to_subroutine(emulator_t *em, u32_t addr)
{
	em->cpu->regs[REG_STACK_POINTER] -= 4;
	memory_write_long(em->mem, em->cpu->regs[REG_STACK_POINTER], em->cpu->pc);

	em->cpu->pc = addr;
}

void return_from_subroutine(emulator_t *em)
{
	em->cpu->pc = memory_read_long(em->mem, em->cpu->regs[REG_STACK_POINTER]);
	em->cpu->regs[REG_STACK_POINTER] += 4;
}

void copy_register_pair(emulator_t *em)
{
	u16_t *reg_dh, *reg_dl, *reg_sh, *reg_sl;

	fetch_regs(em, &reg_dh, &reg_dl);
	fetch_regs(em, &reg_sh, &reg_sl);

	*reg_dh = *reg_sh;
	*reg_dl = *reg_sl;
}

int emulator_execute(emulator_t *em)
{
	u8_t opcode;
	int addr_mode;

	opcode = fetch_byte(em);

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

		case SINSTR_LDRW_ABS:
		case SINSTR_LDRW_ABS_PTR:
		case SINSTR_LDRW_ABS_IDX:
		case SINSTR_LDRW_ABS_PTR_IDX:
		case SINSTR_LDRW_ABS_PTR_OFF:
		case SINSTR_LDRW_ZP_PTR:
		case SINSTR_LDRW_ZP_OFF:
		case SINSTR_LDRW_ZP_IDX:
			addr_mode = opcode - INSTR_LDRW + ADDR_MODE_ABS;
			load_register_wide(em, addr_mode);
			break;

		case SINSTR_STR_ABS:
		case SINSTR_STR_ABS_PTR:
		case SINSTR_STR_ABS_IDX:
		case SINSTR_STR_ABS_PTR_IDX:
		case SINSTR_STR_ABS_PTR_OFF:
		case SINSTR_STR_ZP_PTR:
		case SINSTR_STR_ZP_OFF:
		case SINSTR_STR_ZP_IDX:
			addr_mode = opcode - INSTR_STR + ADDR_MODE_ABS;
			store_register(em, addr_mode, false);
			break;

		case SINSTR_STRB_ABS:
		case SINSTR_STRB_ABS_PTR:
		case SINSTR_STRB_ABS_IDX:
		case SINSTR_STRB_ABS_PTR_IDX:
		case SINSTR_STRB_ABS_PTR_OFF:
		case SINSTR_STRB_ZP_PTR:
		case SINSTR_STRB_ZP_OFF:
		case SINSTR_STRB_ZP_IDX:
			addr_mode = opcode - INSTR_STRB + ADDR_MODE_ABS;
			store_register(em, addr_mode, true);
			break;

		case SINSTR_CPRP:
			copy_register_pair(em);
			break;

		case SINSTR_BZ:
			cond_branch(em, cpu_get_flag(em->cpu, FLAG_ZERO));
			break;
		case SINSTR_BNZ:
			cond_branch(em, !cpu_get_flag(em->cpu, FLAG_ZERO));
			break;
		case SINSTR_BCC:
			cond_branch(em, !cpu_get_flag(em->cpu, FLAG_CARRY));
			break;
		case SINSTR_BCS:
			cond_branch(em,  cpu_get_flag(em->cpu, FLAG_CARRY));
			break;
		case SINSTR_BRN:
			cond_branch(em, cpu_get_flag(em->cpu, FLAG_NEGATIVE));
			break;
		case SINSTR_BRP:
			cond_branch(em, !cpu_get_flag(em->cpu, FLAG_NEGATIVE));
			break;
		case SINSTR_BRA:
			cond_branch(em, true);
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
			em->cpu->pc = addr_from_addr_mode(em, addr_mode);
			break;

		case SINSTR_CALL_ABS:
		case SINSTR_CALL_ABS_PTR:
		case SINSTR_CALL_ZP_PTR:
			addr_mode = opcode - INSTR_LBRA + ADDR_MODE_ABS;
			call_to_subroutine(em, addr_mode);
			break;

		case SINSTR_RET:
			return_from_subroutine(em);
			break;
		
		case SINSTR_RTI:
			printf("unsure how to handle RTI\n");
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

		/*
		#define SINSTR_CMP(addr_mode) case SINSTR_CMP_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_CMP)
			break;
		#undef SINSTR_CMP
		*/

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
		 	printf("unsure how to handle BRK\n");
			break;
		case SINSTR_NOP:
		 	PC += instruction_size[INSTR_NOP];
			break;

		default:
			printf("UNHANDLED OPCODE: %d\n", opcode);
	}

	return 0;
}