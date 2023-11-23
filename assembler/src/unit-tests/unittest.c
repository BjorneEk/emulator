/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * unit tests
 *==========================================================*/

#include "../../../common/util/types.h"
#include "../../../common/util/error.h"
#include "../assembler.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define VAL8_INT	0xFC
#define VAL16_INT	0xFEAD
#define VAL32_INT	0xABABABAB
#define VAL8_STR	"#0xFC"
#define VAL16_STR	"#0xFEAD"
#define VAL32_STR	"#0xABABABAB"

static const struct addr_mode_test {
	const char	*inp;
	int		reg_count;
	bool		has_val;
	int		val;
} ADDR_MODE_TEST_DATA[] = {
	[ADDR_MODE_RELATIVE]	= {.inp = VAL8_STR,			.reg_count = 0, .has_val = true,	.val = VAL8_INT},
	[ADDR_MODE_IMMIDIATE]	= {.inp = VAL16_STR,			.reg_count = 0, .has_val = true,	.val = VAL16_INT},
	[ADDR_MODE_REG]		= {.inp = "%s",				.reg_count = 1, .has_val = false,	.val = 0},
	[ADDR_MODE_ABS]		= {.inp = "[" VAL32_STR "]",		.reg_count = 0, .has_val = true,	.val = VAL32_INT},
	[ADDR_MODE_ABS_PTR]	= {.inp = "[%s, %s]",			.reg_count = 2, .has_val = false,	.val = 0},
	[ADDR_MODE_ABS_IDX]	= {.inp = "[" VAL32_STR "], %s",	.reg_count = 1, .has_val = true,	.val = VAL32_INT},
	[ADDR_MODE_ABS_PTR_IDX]	= {.inp = "[%s, %s], %s",		.reg_count = 3, .has_val = false,	.val = 0},
	[ADDR_MODE_ABS_PTR_OFF]	= {.inp = "[%s, %s], " VAL16_STR,	.reg_count = 2, .has_val = true,	.val = VAL16_INT},
	[ADDR_MODE_ZP_PTR]	= {.inp = "[%s]",			.reg_count = 1, .has_val = false,	.val = 0},
	[ADDR_MODE_ZP_OFF]	= {.inp = "[%s + " VAL16_STR "]",	.reg_count = 1, .has_val = true,	.val = VAL16_INT},
	[ADDR_MODE_ZP_IDX]	= {.inp = "[%s + %s]",			.reg_count = 2, .has_val = false,	.val = 0},
};

static const struct ins_test {
	int		itype;
	const char	*inp;
	int		reg_count;
	bool		is_implied;
	int		val;
} INSTRUCTION_TEST_DATA[] = {
	{.itype = INSTR_NOP,	.inp = "nop",			.reg_count = 0, .is_implied = true,	.val = 0},
	{.itype = INSTR_BRK,	.inp = "brk",			.reg_count = 0, .is_implied = true,	.val = 0},
	{.itype = INSTR_LDR,	.inp = "ldr r0, %s",		.reg_count = 1, .is_implied = false,	.val = 0},
	{.itype = INSTR_LDRB,	.inp = "ldrb r0, %s",		.reg_count = 1, .is_implied = false,	.val = 0},
	{.itype = INSTR_LDRW,	.inp = "ldrw r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_STR,	.inp = "str r0, %s",		.reg_count = 1, .is_implied = false,	.val = 0},
	{.itype = INSTR_STRB,	.inp = "strb r0, %s",		.reg_count = 1, .is_implied = false,	.val = 0},
	{.itype = INSTR_CPRP,	.inp = "cprp r0, r1, r2, r3",	.reg_count = 4, .is_implied = true,	.val = 0},
	{.itype = INSTR_BZ,	.inp = "bz %s",			.reg_count = 0, .is_implied = false,	.val = 0},
	{.itype = INSTR_BNZ,	.inp = "bnz %s",		.reg_count = 0, .is_implied = false,	.val = 0},
	{.itype = INSTR_BCC,	.inp = "bcc %s",		.reg_count = 0, .is_implied = false,	.val = 0},
	{.itype = INSTR_BCS,	.inp = "bcs %s",		.reg_count = 0, .is_implied = false,	.val = 0},
	{.itype = INSTR_BRN,	.inp = "brn %s",		.reg_count = 0, .is_implied = false,	.val = 0},
	{.itype = INSTR_BRP,	.inp = "brp %s",		.reg_count = 0, .is_implied = false,	.val = 0},
	{.itype = INSTR_BBS,	.inp = "bbs #1, r0, %s",	.reg_count = 1, .is_implied = false,	.val = 1},
	{.itype = INSTR_BBC,	.inp = "bbc #1, r0, %s",	.reg_count = 1, .is_implied = false,	.val = 1},
	{.itype = INSTR_BRA,	.inp = "bra %s",		.reg_count = 0, .is_implied = false,	.val = 0},
	{.itype = INSTR_LBRA,	.inp = "lbra %s",		.reg_count = 0, .is_implied = false,	.val = 0},
	{.itype = INSTR_CALL,	.inp = "call %s",		.reg_count = 0, .is_implied = false,	.val = 0},
	{.itype = INSTR_RET,	.inp = "ret",			.reg_count = 0, .is_implied = true,	.val = 0},
	{.itype = INSTR_RTI,	.inp = "rti",			.reg_count = 0, .is_implied = true,	.val = 0},
	{.itype = INSTR_ADC,	.inp = "adc r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_ADD,	.inp = "add r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_ADCW,	.inp = "adcw r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_ADDW,	.inp = "addw r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_SBC,	.inp = "sbc r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_SUB,	.inp = "sub r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_SBCW,	.inp = "sbcw r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_SUBW,	.inp = "subw r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_EOR,	.inp = "eor r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_ORR,	.inp = "orr r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_AND,	.inp = "and r0, r1, %s",	.reg_count = 2, .is_implied = false,	.val = 0},
	{.itype = INSTR_CMP,	.inp = "cmp r0, %s",		.reg_count = 1, .is_implied = false,	.val = 0},
	{.itype = INSTR_ASR,	.inp = "asr r0",		.reg_count = 1, .is_implied = true,	.val = 0},
	{.itype = INSTR_LSL,	.inp = "lsl r0",		.reg_count = 1, .is_implied = true,	.val = 0},
	{.itype = INSTR_LSR,	.inp = "lsr r0",		.reg_count = 1, .is_implied = true,	.val = 0},
	{.itype = INSTR_NOT,	.inp = "not r0",		.reg_count = 1, .is_implied = true,	.val = 0},
	{.itype = INSTR_DEC,	.inp = "dec r0",		.reg_count = 1, .is_implied = true,	.val = 0},
	{.itype = INSTR_DECW,	.inp = "decw r0, r1",		.reg_count = 2, .is_implied = true,	.val = 0},
	{.itype = INSTR_INC,	.inp = "inc r0",		.reg_count = 1, .is_implied = true,	.val = 0},
	{.itype = INSTR_INCW,	.inp = "incw r0, r1",		.reg_count = 2, .is_implied = true,	.val = 0},
	{.itype = INSTR_CRB,	.inp = "crb r0, %s",		.reg_count = 1, .is_implied = false,	.val = 0},
	{.itype = INSTR_SRB,	.inp = "srb r0, %s",		.reg_count = 1, .is_implied = false,	.val = 0},
};
void onfail()
{
	exit(-1);
}
__attribute__((format(printf, 1, 2)))
void fail_test(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "[\033[31;1;4m" "Unit-Test Failed" "\033[0m]" ": ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	onfail();
}
__attribute__((format(printf, 1, 2)))
void pass_test(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	printf("[\033[32;1;4m" "Unit-Test passed" "\033[0m]" ": ");
	vprintf(fmt, args);
	printf("\n");
}

#define TEST(pred, ...) if (pred) { pass_test(__VA_ARGS__); } else {fail_test(__VA_ARGS__);}
#define TEST_QUIET(pred, ...) if (!(pred)) { fail_test(__VA_ARGS__); }

static void get_am_str(char *res, int addr_mode, int first_reg)
{
	const struct addr_mode_test *am;
	char r0[3];
	char r1[3];
	char r2[3];

	am = &ADDR_MODE_TEST_DATA[addr_mode];

	if (am->reg_count == 0) {
		strcpy(res, am->inp);
		return;
	}

	r0[0] = 'r';
	r0[1] = first_reg + '0';
	r0[2] = '\0';

	r1[0] = 'r';
	r1[1] = (first_reg + 1) + '0';
	r1[2] = '\0';

	r2[0] = 'r';
	r2[1] = (first_reg + 2) + '0';
	r2[2] = '\0';

	switch (am->reg_count) {
		case 1:
			sprintf(res, am->inp, r0);
			break;
		case 2:
			sprintf(res, am->inp, r0, r1);
			break;
		case 3:
			sprintf(res, am->inp, r0, r1, r2);
			break;
	}
	return;
}

void append_instr_str(char **res, int *reslen, const struct ins_test *ins)
{
	const int	*ams;
	int		ams_len;
	int		i;
	int		linelen;
	char		ins_str[200];
	char		am_str[200];

	if (ins->is_implied) {
		linelen = strlen(ins->inp);
		strcpy(ins_str, ins->inp);
		ins_str[linelen++] = '\n';
		ins_str[linelen] = '\0';
		*res = realloc(*res, *reslen + linelen + 1);
		strcat(*res, ins_str);
		*reslen += linelen;
		return;
	}

	ams_len = ADDR_MODE_NULL;
	ams = supported_addressing_modes[ins->itype];

	for (i = 0; i < ams_len; i++) {
		if (ams[i] == ADDR_MODE_NULL && *res == NULL)
			exit_error("unit-test, no addressing modes found for instruction, %s", ins->inp);
		if (ams[i] == ADDR_MODE_NULL)
			return;

		ins_str[0] = '\0';

		get_am_str(am_str, ams[i], ins->reg_count);
		sprintf(ins_str, ins->inp, am_str);
		linelen = strlen(ins_str);
		ins_str[linelen++] = '\n';
		ins_str[linelen] = '\0';

		*res = realloc(*res, *reslen + linelen + 1);
		strcat(*res, ins_str);
		*reslen += linelen;
	}
}

static char *instructions_string()
{
	int	i;
	int	len;
	char	*str;

	str = NULL;
	for (i = 0; i < sizeof INSTRUCTION_TEST_DATA / sizeof(struct ins_test); i++) {
		append_instr_str(&str, &len, &INSTRUCTION_TEST_DATA[i]);
	}

	return str;
}

tokenizer_t * tk_from_file(file_t *file)
{
	fstack_t *s = NULL;
	push_file(&s, file);
	return new_tokenizer(&s);
}

static void test_instruction(const struct ins_test *ins)
{
	const int	*ams;
	int		ams_len;
	int		i, j;
	char		ins_str[200];
	char		am_str[200];
	file_t		tmp;
	tokenizer_t	*tk;
	program_t	*p;
	section_t	*section;
	asm_t		*data;

	if (ins->is_implied) {
		strcpy(ins_str, ins->inp);
		ftemp_with(&tmp, EXT_ASM_FILE, ".section:\n	%s\n", ins_str);
		tk = tk_from_file(&tmp);
		p = parse(tk);
		section = HMAP_get(p->sections, "section", 7);
		TEST_QUIET(section != NULL, "section found (%s)", ins->inp);
		TEST_QUIET(section->data->len == 1, "section length == 1 (%s)", ins->inp);
		data = *(asm_t**)DLA_get(section->data, 0);
		TEST_QUIET(data->instruction == ins->itype, "instruction type (%s)", ins->inp);
		for (j = 0; j < ins->reg_count; j++)
			TEST_QUIET(data->regs[j] == j, "matching register: r%i to register r%i", j, data->regs[j]);
		//pass_test("registers (%s)", ins->inp);

		program_free(&p);
		tk_close(&tk);
		close_file(&tmp);
		pass_test("instruction (%s)", ins->inp);
		return;
	}

	ams_len = ADDR_MODE_NULL;
	ams = supported_addressing_modes[ins->itype];

	for (i = 0; i < ams_len; i++) {
		if (ams[i] == ADDR_MODE_NULL) {
			pass_test("instruction (%s)", ins->inp);
			return; /* success */
		}

		ins_str[0] = '\0';

		get_am_str(am_str, ams[i], ins->reg_count);

		sprintf(ins_str, ins->inp, am_str);
		ftemp_with(&tmp, EXT_ASM_FILE, ".section:\n	%s\n", ins_str);
		tk = tk_from_file(&tmp);
		p = parse(tk);
		section = HMAP_get(p->sections, "section", 7);
		TEST_QUIET(section != NULL, "section found (%s)", ins_str);
		TEST_QUIET(section->data->len == 1, "section length == 1 (%s)", ins_str);
		data = *(asm_t**)DLA_get(section->data, 0);
		TEST_QUIET(data->instruction == ins->itype, "instruction type (%s)", ins_str);
		for (j = 0; j < ins->reg_count + ADDR_MODE_TEST_DATA[ams[i]].reg_count; j++)
			TEST_QUIET(data->regs[j] == j, "matching register: r%i to register r%i", j, data->regs[j]);
		//pass_test("registers (%s)", ins->inp);

		program_free(&p);
		tk_close(&tk);
		close_file(&tmp);
		//pass_test("instruction (%s)", ins_str);
	}

	pass_test("instruction (%s)", ins->inp);
}

static void test_all_instructions()
{
	int i;
	for (i = 0; i < sizeof INSTRUCTION_TEST_DATA / sizeof(struct ins_test); i++)
		test_instruction(&INSTRUCTION_TEST_DATA[i]);
}

static void test_big_program()
{
	file_t		tmp;
	tokenizer_t	*tk;
	program_t	*p;
	section_t	*section;
	char		*code_str;

	code_str = instructions_string();
	ftemp_with(&tmp, EXT_ASM_FILE, ".section:\n%s\n",
		code_str);
	free(code_str);

	tk = tk_from_file(&tmp);
	p = parse(tk);
	program_free(&p);
	tk_close(&tk);
	close_file(&tmp);
}

int main(int argc, const char *argv[])
{
	char *instruction_test_code;

	instruction_test_code = instructions_string();
	printf("UNIT TEST!\n");
	printf("%s\n", instruction_test_code);
	free(instruction_test_code);
	test_all_instructions();
	test_big_program();
}