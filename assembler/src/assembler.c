/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * assembler
 *==========================================================*/
#include "assembler.h"
#include "structures/dynamic_array.h"
#include "structures/hashmap.h"
#include "tokenizer.h"
#include "util/error.h"
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

	res->data = DLA_new(sizeof(asm_t*), 10);
	res->labels = HMAP_new(30, HASH_fnv_1a);
	res->size = 0;
	res->tk = tk;

	return res;
}

__attribute__((unused))
static asm_t *init_instr(tk_t tk, int instype, int *addr, int *regs, constexpr_t *value)
{
	asm_t *res;
	res = malloc(sizeof(asm_t));
	res->token = tk;
	res->type = instype;
	res->value = value;
	res->size = subinstrution_size[instype];
	res->rel_addr = *addr;
	*addr += res->size;
	memmove(res->regs, regs, ASM_REG_COUNT * sizeof(int));
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
		return res;
	}
	else if(tk.type == TK_IDENT) {
		res = new_constexpr(tk);
		res->type = EXPR_REF;
		add_label_ref(as, tk, res);
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
	return toplevel(as);

}

typedef struct precedence_level {
	const int *op;
	const int len;
} prec_t;
#define LOWEST_PRECEDENCY 5

static constexpr_t *binop(assembler_t *as, int precedency_level)
{
	static const int mulops[]	= {'/', '%', '*'};
	static const int addops[]	= {'+', '-'};
	static const int shiftops[]	= {TK_LSR_OP, TK_LSL_OP};
	static const int andop[]	= {'&'};
	static const int xorop[]	= {'^'};
	static const int orop[]		= {'|'};

	static const prec_t ops[] = {
		(prec_t){mulops,	sizeof mulops	/ sizeof(int)},
		(prec_t){addops,	sizeof addops	/ sizeof(int)},
		(prec_t){shiftops,	sizeof shiftops	/ sizeof(int)},
		(prec_t){andop,		sizeof andop	/ sizeof(int)},
		(prec_t){xorop,		sizeof xorop	/ sizeof(int)},
		(prec_t){orop, 		sizeof orop	/ sizeof(int)},
	};

	constexpr_t *res;
	constexpr_t *lhs, *rhs;
	tk_t tk;
	int i;

	lhs = precedency_level > 0 ? binop(as, precedency_level - 1) : unary(as);
	tk = next(as);
	for (i = 0; i < ops[precedency_level].len; i++) {
		if (tk.type == ops[precedency_level].op[i])
			goto found_first;
	}
	pb(as, tk);
	return lhs;
found_first:
	res = new_op_constexpr(tk);
	DLA_append(res->ops, &lhs);

loop_start:
		rhs = precedency_level > 0 ? binop(as, precedency_level - 1) : unary(as);
		DLA_append(res->ops, &rhs);


		if (consume(as, tk.type))
			goto loop_start;

		tk = next(as);
		for (i = 0; i < ops[precedency_level].len; i++) {
			if (tk.type == ops[precedency_level].op[i]) {
				lhs = res;
				res = new_op_constexpr(tk);
				DLA_append(res->ops, &lhs);
				goto loop_start;
			}
		}

		pb(as, tk);
		return res;
}

constexpr_t *expr(assembler_t *as)
{
	return binop(as, LOWEST_PRECEDENCY);
}

void parse_start(assembler_t *as, tk_t t)
{
	switch (t.type) {
		case TK_INSTRUCTION:	printf("instruction\n");break;
		case TK_GLOBAL:
			expect(as, t, ':');
			add_globals(as, t);
			break;

		case TK_IDENT:
			if (consume(as, ':')) {
				add_label_def(as, t, DEF_DEFINED, NULL);
				break;
			}
			if (consume(as, '=')) {
				add_label_def(as, t, DEF_ABSOLUTE, expr(as));
				break;
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
			tk_print(t);
			PARSE_ERROR(t, "expected top level token (instruction, global defs, identifyer, section or typename)");
	}
}

program_t *parse(tokenizer_t *t)
{
	assembler_t as;
	tk_t tk;

	init_assembler(&as, t);

	while (tk = next(&as), tk.type != TK_NULL)
		parse_start(&as, tk);
	return NULL;
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