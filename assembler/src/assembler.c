/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * assembler
 *==========================================================*/
#include "assembler.h"
#include "../../common/structures/dynamic_array.h"
#include "../../common/structures/hashmap.h"
#include "tokenizer.h"
#include "../../common/util/error.h"
#include <stdbool.h>
#include <string.h>

typedef struct assembler_ctx {
	dla_t		*globals;	/* def_t*		*/
	hashmap_t	*sections;	/* char*->section_t*	*/
	hashmap_t	*labels;	/* char*->def_t*	*/
	section_t	*current_section;
	int		slen;
	tokenizer_t	*tknz;
} assembler_t;

int subinstrution_size[SINSTR_NULL + 1] = {1};

#define PARSE_ERROR(...) do {		\
	tk_print_error(__VA_ARGS__);	\
	exit(-1);			\
	} while(0);

#define PARSE_WARNING(...) do {		\
	tk_warning(__VA_ARGS__);	\
	} while(0);

static tk_t next(assembler_t *as)
{
	return tk_next(as->tknz);
}

static void pb(assembler_t *as, tk_t t)
{
	tk_rev(as->tknz, t);
}

static tk_t expect(assembler_t *as, tk_t prev, int type)
{
	tk_t tk;
	char s1[40], s2[40], s3[40];
	tk = next(as);
	if (tk.type != type) {
		tk_type_tostring(type, s1);
		tk_type_tostring(prev.type, s2);
		tk_type_tostring(tk.type, s3);
		PARSE_ERROR(tk, "Expected %s after %s, found %s", s1, s2, s3);
	}
	return tk;
}

static bool consume(assembler_t *as, int type)
{
	tk_t tk;
	tk = next(as);
	if (tk.type == type)
		return true;
	pb(as, tk);
	return false;
}

static section_t *new_section(tk_t tk)
{
	section_t *res;

	res = malloc(sizeof(section_t));

	res->data	= DLA_new(sizeof(asm_t*), 10);
	res->labels	= HMAP_new(30, HASH_fnv_1a);
	res->size	= 0;
	res->tk		= tk;
	res->has_raw_data	= false;

	return res;
}

static asm_t *new_instr(assembler_t *as, tk_t tk)
{
	asm_t *res;

	if (as->current_section == NULL)
		PARSE_ERROR(tk, "instructions need to be placed in sections, define a section before instruction with '.section_name:'");

	if (as->current_section->has_raw_data)
		PARSE_WARNING(tk, "mixing data and instructions in same section (%s)", as->current_section->tk.str_val);

	res = malloc(sizeof(asm_t));
	DLA_append(as->current_section->data, &res);
	res->type		= TK_INSTRUCTION;
	res->instruction	= tk.instype;
	res->instruction_size	= instruction_size[res->instruction];
	res->token		= tk;
	res->rel_addr		= as->current_section->size;
	res->nregs		= 0;
	return res;
}



static void init_assembler(assembler_t *as, tokenizer_t *t)
{
	as->slen		= 0;
	as->current_section	= NULL;
	as->tknz		= t;
	as->globals		= DLA_new(sizeof(def_t*), 10);
	as->sections		= HMAP_new(10, HASH_fnv_1a);
	as->labels		= HMAP_new(100, HASH_fnv_1a);
}

static def_t *new_def(int type, bool is_global, char *ident, tk_t tk)
{
	def_t *res;

	res = malloc(sizeof(def_t));
	res->type	= type;
	res->is_global	= is_global;
	res->lbl	= ident;
	res->ready	= false;
	res->refs	= DLA_new(sizeof(constexpr_t*), 10);
	res->tk		= tk;
	res->val	= NULL;
	return res;
}

static constexpr_t *new_constexpr(tk_t tk)
{
	constexpr_t *res;

	res = malloc(sizeof(constexpr_t));
	res->token = tk;
	res->type = tk.type;
	res->val = 0;
	return res;
}

static constexpr_t *new_op_constexpr(tk_t tk)
{
	constexpr_t *res;

	res = malloc(sizeof(constexpr_t));
	res->token = tk;
	res->type = tk.type;
	res->val = 0;
	res->ops = DLA_new(sizeof(constexpr_t*), 2);
	return res;
}

static void add_section(assembler_t *as, tk_t tk)
{
	section_t *s;

	s = HMAP_get(as->sections, tk.str_val, tk.tklen);
	if (s != NULL) {
		PARSE_ERROR(tk, "Redefenition of section %s, previously defined here: file: %s, line: %i, col: %i",
		tk.str_val, s->tk.debug.file, s->tk.debug.line + 1, s->tk.debug.column + 1);
	}
	s = new_section(tk);
	HMAP_add(as->sections, tk.str_val, tk.tklen, s);
	as->current_section = s;
}

static void add_label_def(assembler_t *as, tk_t tk, int type, constexpr_t *val)
{
	def_t *l;
	l = HMAP_get(as->labels, tk.str_val, tk.tklen);
	if (l != NULL && (!l->is_global || l->type == DEF_DEFINED)) {
		PARSE_ERROR(tk, "Redefenition of label %s, previously defined here: file: %s, line: %i, col: %i",
		tk.str_val, l->tk.debug.file, l->tk.debug.line + 1, l->tk.debug.column + 1);
	} else if (l != NULL) {
		l->type	= type;
		l->tk	= tk;
		l->val	= val;

		HMAP_add(as->current_section->labels, l->tk.str_val, l->tk.tklen, l);
		return;
	}
	l = new_def(type, false, tk.str_val, tk);
	l->val = val;
	HMAP_add(as->current_section->labels, l->tk.str_val, l->tk.tklen, l);
	HMAP_add(as->labels, l->tk.str_val, l->tk.tklen, l);
}

static void add_label_ref(assembler_t *as, tk_t tk, constexpr_t *ref)
{
	def_t *l;
	l = HMAP_get(as->labels, tk.str_val, tk.tklen);
	if (l != NULL) {
		DLA_append(l->refs, &ref);
		ref->ref = l;
		return;
	}
	l = new_def(DEF_UNDEFINED_REF, false, tk.str_val, tk);
	DLA_append(l->refs, &ref);
	ref->ref = l;
	HMAP_add(as->labels, l->tk.str_val, l->tk.tklen, l);
}


static void add_globals(assembler_t *as, tk_t globt)
{
	tk_t t;
	def_t *d;

	do {
		t = expect(as, globt, TK_IDENT);

		d = HMAP_get(as->labels, t.str_val, t.tklen);
		if (d == NULL) {
			d = new_def(DEF_UNDEFINED, true, t.str_val, t);
			DLA_append(as->globals, &d);
			HMAP_add(as->labels, t.str_val, t.tklen, d);
		} else if (!d->is_global) {
			DLA_append(as->globals, &d);
		}
	} while (consume(as, ','));
}
static constexpr_t *expr(assembler_t *as);

static constexpr_t *toplevel(assembler_t *as)
{
	tk_t tk;
	constexpr_t *res;
	char s1[40];
	tk = next(as);

	if(tk.type == '(') {
		res = expr(as);
		expect(as, tk, ')');
		return res;
	}
	else if(tk.type == TK_INTEGER_LITERAL || tk.type == TK_CHARACTER_LITERAL) {
		res = new_constexpr(tk);
		res->val = tk.int_val;
		BUG("\nFOUND LITERAL\n\n");
		return res;
	}
	else if(tk.type == TK_IDENT) {
		res = new_constexpr(tk);
		add_label_ref(as, tk, res);
		BUG("\nFOUND LABEL-REF\n\n");
		return res;
	}
	tk_type_tostring(tk.type, s1);
	PARSE_ERROR(tk, "constant expression expected, found %s", s1);
}

static constexpr_t *unary(assembler_t *as)
{
	constexpr_t *res;
	tk_t tk;
	tk = next(as);
	if(tk.type == '~' || tk.type == '-') {
		res = new_constexpr(tk);
		res->expr = unary(as);
		return res;
	}
	pb(as, tk);
	return toplevel(as);

}

typedef struct const_int_arr {
	const int *arr;
	const int len;
} const_int_arr_t;

static inline const tpos_t not_a_pos()
{
	return (tpos_t){-1, -1, -1, NULL};
}

static constexpr_t *minus(constexpr_t *e)
{
	constexpr_t *res;

	res = new_constexpr((tk_t){'-', 1, .str_val = "generated_negation_token", not_a_pos()});
	res->expr = e;
	return res;
}

static constexpr_t *binop(assembler_t *as, int precedency_level)
{
	static const int mulops[]	= {'/', '%', '*'};
	static const int addops[]	= {'+', '-'};
	static const int shiftops[]	= {TK_LSR_OP, TK_LSL_OP};
	static const int andop[]	= {'&'};
	static const int xorop[]	= {'^'};
	static const int orop[]		= {'|'};

	static const const_int_arr_t ops[] = {
		(const_int_arr_t){orop,		sizeof orop	/ sizeof(int)},
		(const_int_arr_t){xorop,	sizeof xorop	/ sizeof(int)},
		(const_int_arr_t){andop,	sizeof andop	/ sizeof(int)},
		(const_int_arr_t){shiftops,	sizeof shiftops	/ sizeof(int)},
		(const_int_arr_t){addops,	sizeof addops	/ sizeof(int)},
		(const_int_arr_t){mulops,	sizeof mulops	/ sizeof(int)},
	};
	#define OPS_LEN (sizeof ops / sizeof(const_int_arr_t))
	constexpr_t *res;
	constexpr_t *lhs, *rhs;
	bool is_subtraction;
	tk_t tk;
	int i;

	BUG("plev: %i\n", precedency_level);
	lhs = precedency_level < OPS_LEN - 1 ? binop(as, precedency_level + 1) : unary(as);
	tk = next(as);
	for (i = 0; i < ops[precedency_level].len; i++) {
		char s1[100];
		char s2[100];
		tk_type_tostring(tk.type, s1);
		tk_type_tostring(ops[precedency_level].arr[i], s2);
		BUG("level: %i, ttype: %s (%i), op: %s (%i)\n", precedency_level, s1, tk.type, s2, ops[precedency_level].arr[i]);
		if (tk.type == ops[precedency_level].arr[i]) {
			BUG("match\n");
			goto found_first;
		}
	}
	BUG("done\n");
	pb(as, tk);
	return lhs;
found_first:
	res = new_op_constexpr(tk);
	if (tk.type == '-') {
		res->type = '+';
		is_subtraction = true;
	}
	DLA_append(res->ops, &lhs);

loop_start:
		rhs = precedency_level < OPS_LEN - 1 ? binop(as, precedency_level + 1) : unary(as);
		rhs = is_subtraction ? minus(rhs) : rhs;
		DLA_append(res->ops, &rhs);


		if (consume(as, tk.type))
			goto loop_start;

		tk = next(as);
		is_subtraction = false;
		for (i = 0; i < ops[precedency_level].len; i++) {
			char s1[100];
				char s2[100];
				tk_type_tostring(tk.type, s1);
				tk_type_tostring(ops[precedency_level].arr[i], s2);
				BUG("level: %i, ttype: %s (%i), op: %s (%i)\n", precedency_level, s1, tk.type, s2, ops[precedency_level].arr[i]);
			if (tk.type == ops[precedency_level].arr[i]) {
				BUG("match\n");
				lhs = res;
				res = new_op_constexpr(tk);
				if (tk.type == '-') {
					res->type = '+';
					is_subtraction = true;
				}
				DLA_append(res->ops, &lhs);
				goto loop_start;
			}
		}
		BUG("done\n");
		pb(as, tk);
		return res;
}

static constexpr_t *expr(assembler_t *as)
{
	constexpr_t *res;

	res = binop(as, 0);
	print_constexpr(res);
	return res;
}
static const int PATTERN_IMMIDIATE[]	= {EXPRESSION};
static const int PATTERN_REG[]		= {TK_REGISTER};
static const int PATTERN_ABS[]		= {'[', EXPRESSION, ']'};
static const int PATTERN_ABS_PTR[]	= {'[', TK_REGISTER, ',', TK_REGISTER, ']'};
static const int PATTERN_ABS_IDX[]	= {'[', EXPRESSION, ']', ',', TK_REGISTER};
static const int PATTERN_ABS_PTR_IDX[]	= {'[', TK_REGISTER, ',', TK_REGISTER, ']', ',', TK_REGISTER};
static const int PATTERN_ABS_PTR_OFF[]	= {'[', TK_REGISTER, ',', TK_REGISTER, ']', ',', EXPRESSION};
static const int PATTERN_ZP_PTR[]	= {'[', TK_REGISTER, ']'};
static const int PATTERN_ZP_OFF[]	= {'[', TK_REGISTER, '+', EXPRESSION, ']'};
static const int PATTERN_ZP_IDX[]	= {'[', TK_REGISTER, '+', TK_REGISTER, ']'};

static const const_int_arr_t ops[] = {
	[ADDR_MODE_IMMIDIATE]	= PATTERN_IMMIDIATE,
	[ADDR_MODE_REG]		= PATTERN_REG,
	[ADDR_MODE_ABS]		= PATTERN_ABS,
	[ADDR_MODE_ABS_PTR]	= PATTERN_ABS_PTR,
	[ADDR_MODE_ABS_IDX]	= PATTERN_ABS_IDX,
	[ADDR_MODE_ABS_PTR_IDX]	= PATTERN_ABS_PTR_IDX,
	[ADDR_MODE_ABS_PTR_OFF]	= PATTERN_ABS_PTR_OFF,
	[ADDR_MODE_ZP_PTR]	= PATTERN_ZP_PTR,
	[ADDR_MODE_ZP_OFF]	= PATTERN_ZP_OFF,
	[ADDR_MODE_ZP_IDX]	= PATTERN_ZP_IDX
};

static void sort_expr_last(int *supported, int len, int depth)
{
	int i;
	int tmp;
	for (i = 0; i < len - 1; i++) {

		if (ops[supported[i]].arr[depth] == EXPRESSION) {
			tmp = supported[i];
			memmove(supported + i, supported + i + 1, ((len - i) - 1) * sizeof(int));
			supported[len - 1] = tmp;
		}
	}
}

static void addr_mode(assembler_t *as, asm_t *ins, int depth, int *supported, int len)
{
	int 	_supported[ADDR_MODE_NULL];
	int	_len;
	int	result;
	tk_t	tk;
	int	i;
	char	str[100];
	bool	found_not_expr;
	bool	is_expr;
	bool	reg_set;

	sort_expr_last(supported, len, depth);

	tk		= next(as);

	_len		= 0;
	found_not_expr	= false;
	reg_set		= false;
	is_expr		= false;

	for (i = 0; i < len; i++) if (ops[supported[i]].len > depth) {
		if (!is_expr && (ops[supported[i]].arr[depth] == tk.type && ops[supported[i]].len == depth - 1)) { // perfect match

			if (tk.type == TK_REGISTER && !reg_set) {
				ins->regs[ins->nregs++] = tk.reg;
				reg_set = true;
			}
perfect:
			ins->addr_mode		= supported[i];
			ins->type		= ins->instruction + ins->addr_mode;
			ins->addr_mode_size	= addressing_mode_size[ins->addr_mode];
			addr_mode(as, ins, depth + 1, _supported, _len);
			return;
		} else if (!is_expr && (ops[supported[i]].arr[depth] == tk.type)) { // match
			if (tk.type == TK_REGISTER && !reg_set) {
				ins->regs[ins->nregs++] = tk.reg;
				reg_set = true;
			}
match:
			_supported[_len++] = supported[i];
			found_not_expr = true;
		} else if (found_not_expr && ops[supported[i]].arr[depth] == EXPRESSION) {
			break;
		} else if (ops[supported[i]].arr[depth] == EXPRESSION && ops[supported[i]].len == depth - 1) {
			if (!is_expr) {
				pb(as, tk);
				ins->value = expr(as);
				is_expr = true;
			}
			goto perfect;
		} else if (ops[supported[i]].arr[depth] == EXPRESSION) {
			if (!is_expr) {
				pb(as, tk);
				ins->value = expr(as);
				is_expr = true;
			}
			goto match;
		}
	}
	if (found_not_expr || is_expr) {
		addr_mode(as, ins, depth + 1, _supported, _len);
		return;
	} else {
		tk_type_tostring(tk.type, str);
		PARSE_ERROR(tk, "unexpected instruction argument: ", str);
	}
}

static int get_addr_modes(int type, int *supported)
{
	int i;
	int len;

	for (len = i = 0; i < ADDR_MODE_NULL; i++) {
		if (supported_addressing_modes[type][i] == ADDR_MODE_NULL)
			return len;
		supported[len++] = supported_addressing_modes[type][i];
	}
	return len;
}

static int expect_reg(assembler_t *as)
{
	tk_t	tk;
	char	str[100];

	tk = next(as);

	if (tk.type != TK_REGISTER) {
		tk_type_tostring(tk.type, str);
		PARSE_ERROR(tk, "Expected register, found: %s", str);
	}
	return tk.reg;
}

static void instr(assembler_t *as, tk_t t)
{
	asm_t *ins;
	int supported[ADDR_MODE_NULL];
	int supported_len;
	int i;

	ins = new_instr(as, t);


	switch (ins->instruction) {
		case INSTR_CPRP:
			ins->regs[ins->nregs++] = expect_reg(as);
			ins->regs[ins->nregs++] = expect_reg(as);
			ins->regs[ins->nregs++] = expect_reg(as);
			ins->regs[ins->nregs++] = expect_reg(as);
		case INSTR_RET:
		case INSTR_RTI:
		case INSTR_BRK:
		case INSTR_NOP:
			goto implied;
		case INSTR_BBS:
		case INSTR_BBC:
			ins->bit = expr(as);
			ins->regs[ins->nregs++] = expect_reg(as);
		case INSTR_BZ:
		case INSTR_BNZ:
		case INSTR_BCC:
		case INSTR_BCS:
		case INSTR_BRN:
		case INSTR_BRP:
		case INSTR_BRA:
			goto relative;
		case INSTR_ADC:
		case INSTR_ADD:
		case INSTR_SBC:
		case INSTR_SUB:
		case INSTR_EOR:
		case INSTR_ORR:
		case INSTR_AND:
		case INSTR_LDRW:
		case INSTR_ADCW:
		case INSTR_ADDW:
		case INSTR_SBCW:
		case INSTR_SUBW:
			ins->regs[ins->nregs++] = expect_reg(as);
		case INSTR_LDR:
		case INSTR_LDRB:
		case INSTR_STR:
		case INSTR_STRB:
		case INSTR_CMP:
		case INSTR_CRB:
		case INSTR_SRB:
			ins->regs[ins->nregs++] = expect_reg(as);
		case INSTR_LBRA:
		case INSTR_CALL:
			goto handle_addressing_mode;
		case INSTR_DECW:
		case INSTR_INCW:
			ins->regs[ins->nregs++] = expect_reg(as);
		case INSTR_ASR:
		case INSTR_LSR:
		case INSTR_LSL:
		case INSTR_NOT:
		case INSTR_DEC:
		case INSTR_INC:
			ins->regs[ins->nregs++] = expect_reg(as);
			goto implied;
	}

implied:
	ins->addr_mode = ADDR_MODE_NULL;
	ins->addr_mode_size = 0;
	ins->type = ins->instruction;
	as->current_section->size += ins->instruction_size;
	ins->value = expr(as);
	return;

relative:
	ins->addr_mode = ADDR_MODE_RELATIVE;
	ins->addr_mode_size = addressing_mode_size[ADDR_MODE_RELATIVE];
	ins->type = ins->instruction;
	as->current_section->size += ins->addr_mode_size + ins->instruction_size;
	ins->value = expr(as);
	return;

handle_addressing_mode:
	supported_len = get_addr_modes(ins->instruction, supported);
	addr_mode(as, ins, 0, supported, supported_len);
	as->current_section->size += ins->addr_mode_size + ins->instruction_size;
	return;
}


static void parse_start(assembler_t *as, tk_t t)
{
	char str[100];
	switch (t.type) {
		case TK_INSTRUCTION:	printf("instruction\n");break;
		case TK_GLOBAL:
			expect(as, t, ':');
			add_globals(as, t);
			break;

		case TK_IDENT:
			if (consume(as, ':')) {
				add_label_def(as, t, DEF_DEFINED, NULL);
				return;
			}
			if (consume(as, '=')) {
				add_label_def(as, t, DEF_ABSOLUTE, expr(as));
				return;
			}
			PARSE_ERROR(t, "Expected ':' or assignment after label");
			break;
		case TK_SECTION:
			expect(as, t, ':');
			add_section(as, t);
			break;

		case TK_STRING:		printf("string\n");break;
		case TK_CHAR:
		case TK_I8:		printf("char\n");break;
		case TK_U8:		printf("byte\n");break;
		case TK_I16:		printf("sword\n");break;
		case TK_U16:		printf("word\n");break;
		default:
			tk_type_tostring(t.type, str);
			PARSE_ERROR(t, "expected top level token (instruction, global defs, identifyer, section or typename), found: %s", str);
	}
}

program_t *parse(tokenizer_t *t)
{
	assembler_t as;
	tk_t tk;

	init_assembler(&as, t);

	while (tk = next(&as), tk.type != TK_NULL) {
		BUG("START\n");
		parse_start(&as, tk);
		BUG("created\n");
	}
	return NULL;
}

static void print_deftype(def_t *d)
{
	switch(d->type) {
		case DEF_UNDEFINED:	printf("undefined");		break;
		case DEF_UNDEFINED_REF:	printf("undefined-ref");	break;
		case DEF_SECTION:	printf("section");		break;
		case DEF_ABSOLUTE:	printf("absolute");		break;
		case DEF_DEFINED:	printf("defined");		break;
	}
}
static void _print_constexpr(constexpr_t *exp, char *pre, bool final)
{
	char buff[4096];

	printf("%s",pre);
	strcpy(buff, pre);

	if(final)
		printf("└───");
	else
		printf("├───");

	switch(exp->type) {
		case TK_INTEGER_LITERAL:
			printf("─\e[1minteger literal\033[0m: \033[33;1;4m%i\033[0m\n", exp->token.int_val);
			return;
		case TK_CHARACTER_LITERAL:
			printf("─\e[1mcharacter literal\033[0m: \033[33;1;4m'%c'\033[0m\n", exp->token.int_val);
			return;
		case TK_IDENT:
			printf("─\e[1mlabel-ref\033[0m: \033[33;1;4m'%s'\033[0m (\033[34m", exp->token.str_val);
			print_deftype(exp->ref);
			printf("\033[0m)\n");
			return;
		case '-':
		case '~':
			printf("┬(\e[1m%c\033[0m) \033[34m\033[0m\n", exp->type);
			if(final)
				strcat(buff, "    ");
			else
				strcat(buff, "│   ");
			_print_constexpr(exp->expr, buff, true);
			return;
		case '+':
		case '*':
		case '/':
		case '%':
		case '&':
		case '^':
		case '|':
			printf("┬(\e[1m%c\033[0m) \033[34m\033[0m\n", exp->type);
			if(final)
				strcat(buff, "    ");
			else
				strcat(buff, "│   ");
			FOREACH_DLA(exp->ops, i, constexpr_t*, op, {
				_print_constexpr(op, buff, i == exp->ops->len - 1);
			});
			return;
		case TK_LSL_OP:
			printf("┬(\e[1m<<\033[0m) \033[34m\033[0m\n");
			if(final)
				strcat(buff, "    ");
			else
				strcat(buff, "│   ");
			FOREACH_DLA(exp->ops, i, constexpr_t*, op, {
				_print_constexpr(op, buff, i == exp->ops->len - 1);
			});
			return;
		case TK_LSR_OP:
			printf("┬(\e[1m>>\033[0m) \033[34m\033[0m\n");
			if(final)
				strcat(buff, "    ");
			else
				strcat(buff, "│   ");
			FOREACH_DLA(exp->ops, i, constexpr_t*, op, {
				_print_constexpr(op, buff, i == exp->ops->len - 1);
			});
			return;
		default:
			printf("─\e[1minvalid-expression\033[0m\n");

	}
}

void print_constexpr(constexpr_t *ex)
{
	_print_constexpr(ex, "", true);
}

// absolute:
// [#val]              - abs,          !, -
// [reg, reg]          - abs-ptr,      !, -

// absolute-idx:
// [#val] + reg         - abs-idx,     !, -
// [reg, reg] + reg     - abs-ptr-idx, !, -
// [reg, reg] + #val    - abs-ptr-off, !, -

// zero-page:
// [reg]               - zp-ptr,       !, -
// [reg + #val]        - zp-offset,    !, -
// [reg + reg]         - zp-idx,       !, -