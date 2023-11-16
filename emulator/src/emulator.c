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
		case ADDR_MODE_IMMIDIATE:
		case ADDR_MODE_REG:
		case ADDR_MODE_NULL:
			addr = 0;
			break;
	}
	PC += addressing_mode_size[addr_mode];
	return addr;
}

static u16_t data_from_addr_mode(emulator_t *em, addressing_mode_t addr_mode)
{
	u16_t res;
	u32_t addr;

	if (addr_mode == ADDR_MODE_IMMIDIATE) {
		res = memory_read_word(MEM, PC);
	} else if (addr_mode == ADDR_MODE_REG) {
		res = REGS[memory_read_byte(MEM, PC) >> 4];
	} else {
		addr = addr_from_addr_mode(em, addr_mode);
		res = memory_read_word(MEM, addr);
	}

	return res;
}

static void set_zn_flags(emulator_t *em, u16_t val)
{
	if (val == 0)
		cpu_set_flag(em->cpu, FLAG_ZERO);
	else
		cpu_clear_flag(em->cpu, FLAG_ZERO);

	if ((i16_t)val < 0)
		cpu_set_flag(em->cpu, FLAG_NEGATIVE);
	else
		cpu_clear_flag(em->cpu, FLAG_NEGATIVE);
}

static void set_zn_flags_wide(emulator_t *em, u32_t val)
{
	if (val == 0)
		cpu_set_flag(em->cpu, FLAG_ZERO);
	else
		cpu_clear_flag(em->cpu, FLAG_ZERO);

	if ((i32_t)val < 0)
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

void binop(emulator_t *em, int addr_mode, int instr_size, perform_binop_t perform_op)
{
	u8_t reg_byte;
	u16_t *reg_d, *reg_s;
	u16_t rhs;
	u32_t alu_res;

	reg_byte = memory_read_byte(MEM, PC + 1);
	reg_d = REGS + (reg_byte >> 4);
	reg_s = REGS + (reg_byte & 0x0F);

	PC += instr_size;

	rhs = data_from_addr_mode(em, addr_mode);
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

void binop_wide(emulator_t *em, int addr_mode, int instr_size, perform_binop_wide_t perform_op_wide)
{
	u8_t reg_byte;
	u16_t *reg_dh, *reg_dl;
	u32_t lhs;
	u16_t rhs;
	u64_t alu_res;

	reg_byte = memory_read_byte(MEM, PC + 1);
	reg_dh = REGS + (reg_byte >> 4);
	reg_dl = REGS + (reg_byte & 0x0F);

	lhs = (u32_t)(*reg_dh) << 16 | (u32_t)(*reg_dl);
	PC += instr_size;

	rhs = data_from_addr_mode(em, addr_mode);
	alu_res = perform_op_wide(em, lhs, rhs);

	set_zn_flags_wide(em, alu_res);

	*reg_dh = alu_res >> 16;
	*reg_dl = alu_res & 0xFFFF;
}

typedef u32_t (*perform_unop_t)(emulator_t*, u16_t);

u32_t asr_unop_func(emulator_t *em, u16_t reg_data)
{
	u32_t alu_res;

	alu_res = reg_data >> 1;
	set_zn_flags(em, alu_res);
	return alu_res;
}

u32_t lsr_unop_func(emulator_t *em, u16_t reg_data)
{
	return reg_data >> 1;
}

u32_t lsl_unop_func(emulator_t *em, u16_t reg_data)
{
	return reg_data << 1;
}

u32_t not_unop_func(emulator_t *em, u16_t reg_data)
{
	u32_t alu_res;

	alu_res = ~reg_data;
	set_zn_flags(em, alu_res);
	return alu_res;
}

u32_t dec_unop_func(emulator_t *em, u16_t reg_data)
{
	u32_t alu_res;

	alu_res = reg_data -= 1;
	// maybe something about carry here

	set_zn_flags(em, alu_res);
	return alu_res;
}

u32_t inc_unop_func(emulator_t *em, u16_t reg_data)
{
	u32_t alu_res;

	alu_res = reg_data += 1;
	// maybe something about carry here

	set_zn_flags(em, alu_res);
	return alu_res;
}

void unop(emulator_t *em, int instr_size, perform_unop_t perform_op)
{
	u8_t reg_byte;
	u16_t *reg;
	u32_t alu_res;

	reg_byte = memory_read_byte(MEM, PC + 1);
	reg = REGS + (reg_byte >> 4);

	PC += instr_size;

	alu_res = perform_op(em, *reg);

	*reg = alu_res;
}

i32_t emulator_execute(emulator_t *em)
{
	u8_t opcode;
	u8_t reg_byte;
	u16_t *reg_d; //,*reg_s;
	u16_t data;
	u32_t addr;
	int addr_mode;

	opcode = memory_read_byte(MEM, PC);
	printf("EXECUTING INSTRUCTION WITH OPCODE: %d\n", opcode);
	printf("PC = %d\n", PC);

	switch (opcode) {
		#define SINSTR_LDR(addr_mode) case SINSTR_LDR_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_LDR)
			addr_mode = opcode - INSTR_LDR + ADDR_MODE_IMMIDIATE;
			printf("addr_mode = %d\n", addr_mode);

			reg_byte = memory_read_byte(MEM, PC + 1);
			reg_d = REGS + (reg_byte >> 4);

			PC += instruction_size[INSTR_LDR];

			data = data_from_addr_mode(em, addr_mode);
			*reg_d = data;

			break;
		#undef SINSTR_LDR

		#define SINSTR_LDRB(addr_mode) case SINSTR_LDRB_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_LDRB)
			addr_mode = opcode - INSTR_LDRB;

			reg_byte = memory_read_byte(MEM, PC + 1);
			reg_d = REGS + (reg_byte >> 4);

			PC += instruction_size[INSTR_LDRB];

			data = data_from_addr_mode(em, addr_mode);
			*reg_d = data >> 8;

			break;
		#undef SINSTR_LDRB

		case SINSTR_LBRA_ABS:
		case SINSTR_LBRA_ABS_PTR:
			addr_mode = opcode - INSTR_LBRA + ADDR_MODE_ABS;
			printf("addr_mode = %d\n", addr_mode);

			PC += instruction_size[INSTR_LBRA];

			addr = addr_from_addr_mode(em, addr_mode);
			printf("addr = %d\n", addr);

			PC = addr;
			break;

		#define SINSTR_ADC(addr_mode) case SINSTR_ADC_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADC)
			addr_mode = opcode - INSTR_ADC + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, instruction_size[INSTR_ADC], adc_binop_func);
			break;
		#undef SINSTR_ADC

		#define SINSTR_ADD(addr_mode) case SINSTR_ADD_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADD)
			addr_mode = opcode - INSTR_ADD + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, instruction_size[INSTR_ADD], add_binop_func);
			break;
		#undef SINSTR_ADD

		#define SINSTR_ADCW(addr_mode) case SINSTR_ADCW_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADCW)
			addr_mode = opcode - INSTR_ADCW + ADDR_MODE_IMMIDIATE;
			binop_wide(em, addr_mode, instruction_size[INSTR_ADCW], adcw_binop_func);
			break;
		#undef SINSTR_ADCW

		#define SINSTR_ADDW(addr_mode) case SINSTR_ADDW_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ADDW)
			addr_mode = opcode - INSTR_ADDW + ADDR_MODE_IMMIDIATE;
			binop_wide(em, addr_mode, instruction_size[INSTR_ADDW], addw_binop_func);
			break;
		#undef SINSTR_ADDW

		#define SINSTR_SBC(addr_mode) case SINSTR_SBC_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_SBC)
			addr_mode = opcode - INSTR_SBC + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, instruction_size[INSTR_SBC], sbc_binop_func);
			break;
		#undef SINSTR_SBC

		#define SINSTR_SUB(addr_mode) case SINSTR_SUB_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_SUB)
			addr_mode = opcode - INSTR_SUB + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, instruction_size[INSTR_SUB], sub_binop_func);
			break;
		#undef SINSTR_SUB

		#define SINSTR_SBCW(addr_mode) case SINSTR_SBCW_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_SBCW)
			addr_mode = opcode - INSTR_SBCW + ADDR_MODE_IMMIDIATE;
			binop_wide(em, addr_mode, instruction_size[INSTR_SBCW], sbcw_binop_func);
			break;
		#undef SINSTR_SBCW

		#define SINSTR_SUBW(addr_mode) case SINSTR_SUBW_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_SUBW)
			addr_mode = opcode - INSTR_SUBW + ADDR_MODE_IMMIDIATE;
			binop_wide(em, addr_mode, instruction_size[INSTR_SUBW], subw_binop_func);
			break;
		#undef SINSTR_SBCW

		#define SINSTR_EOR(addr_mode) case SINSTR_EOR_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_EOR)
			addr_mode = opcode - INSTR_EOR + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, instruction_size[INSTR_EOR], eor_binop_func);
			break;
		#undef SINSTR_EOR

		#define SINSTR_ORR(addr_mode) case SINSTR_ORR_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_ORR)
			addr_mode = opcode - INSTR_ORR + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, instruction_size[INSTR_ORR], orr_binop_func);
			break;
		#undef SINSTR_ORR

		#define SINSTR_AND(addr_mode) case SINSTR_AND_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_AND)
			addr_mode = opcode - INSTR_AND + ADDR_MODE_IMMIDIATE;
			binop(em, addr_mode, instruction_size[INSTR_ADD], and_binop_func);
			break;
		#undef SINSTR_AND

		#define SINSTR_CMP(addr_mode) case SINSTR_CMP_##addr_mode :
		XMACRO_ADDRESSING_MODES(SINSTR_CMP)
			break;
		#undef SINSTR_CMP

		case SINSTR_ASR:
			unop(em, instruction_size[INSTR_ASR], asr_unop_func);
			break;
		case SINSTR_LSR:
			unop(em, instruction_size[INSTR_LSR], lsr_unop_func);
			break;
		case SINSTR_LSL:
			unop(em, instruction_size[INSTR_LSL], lsl_unop_func);
			break;
		case SINSTR_NOT:
			unop(em, instruction_size[INSTR_NOT], not_unop_func);
			break;
		case SINSTR_DEC:
			unop(em, instruction_size[INSTR_DEC], dec_unop_func);
			break;
		case SINSTR_DECW:
			break;
		case SINSTR_INC:
			unop(em, instruction_size[INSTR_INC], inc_unop_func);
			break;
		case SINSTR_INCW:
			break;

		case SINSTR_NOP:
		 	PC += instruction_size[INSTR_NOP];
			break;

		default:
			printf("UNHANDLED OPCODE: %d\n", opcode);
	}

	return 0;
}