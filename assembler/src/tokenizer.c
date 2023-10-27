/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * tokenizer
 *==========================================================*/

#include "tokenizer.h"
#include "../../instructions/interface.h"
#include "util/error.h"
#include <_ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

typedef struct tokenizer {
	FILE		*fp;
	const char 	*filename;
	int		line;
	int		column;
	fpos_t		prevline_pos;
	fpos_t		line_pos;

	tk_t pb;
	bool has_pb;
} tokenizer_t;

struct tokenizer_pos {
	int		line;
	int		column;
	fpos_t		prevline_pos;
	fpos_t		line_pos;
};

static inline struct tokenizer_pos get_pos(tokenizer_t *t)
{
	return (struct tokenizer_pos){t->line, t->column, t->prevline_pos, t->line_pos};
}
static inline void set_pos(tokenizer_t *t, struct tokenizer_pos p)
{
	t->line = p.line;
	t->column = p.column;
	t->prevline_pos = p.prevline_pos;
	t->line_pos = p.line_pos;
}
static inline tpos_t tpos(tokenizer_t *t)
{
	return (tpos_t) {t->line, t->column, t->prevline_pos};
}

static inline int next(tokenizer_t *t)
{
	return getc(t->fp);
}

static inline void pb(tokenizer_t *t, int c)
{
	ungetc(c, t->fp);
}
static inline int peak(tokenizer_t *t)
{
	int c;
	c = next(t);
	pb(t, c);
	return c;
}
static inline tk_t	tk(tokenizer_t *t, tpos_t p, int type)
{
	tk_t res;
	res.type = type;
	res.debug.p		= p;
	res.debug.filename	= t->filename;
	res.debug.fp		= t->fp;

	return res;
}

static inline tk_t	treg(tokenizer_t *t, tpos_t p, int reg)
{
	tk_t res;

	res = tk(t, p, TK_REGISTER);
	res.reg = reg;
	return res;
}

static inline tk_t	tstr(tokenizer_t *t, tpos_t p, int type, char *nm)
{
	tk_t res;

	res = tk(t, p, type);
	res.str_val = nm;
	return res;
}

static inline tk_t	tref(tokenizer_t *t, tpos_t p, char *nm)
{
	return tstr(t, p, TK_LABEL_REFERENCE, nm);
}

static inline tk_t	tdef(tokenizer_t *t, tpos_t p, char *nm)
{
	return tstr(t, p, TK_LABEL_DEFINITION, nm);
}

static inline tk_t	tstrlit(tokenizer_t *t, tpos_t p, char *nm)
{
	tk_t res;

	res = tk(t, p, TK_STRING_LITERAL);
	res.str_val = nm;
	return res;
}

static inline tk_t	tintlit(tokenizer_t *t, tpos_t p, int val)
{
	tk_t res;

	res = tk(t, p, TK_INTEGER_LITERAL);
	res.int_val = val;
	return res;
}

static void incr_pos(tokenizer_t *t, int c)
{
	++t->column;
	if (c == '\n') {
		++t->line;
		t->column = 0;
		t->prevline_pos = t->line_pos;
		fgetpos(t->fp, &t->line_pos);
	}
}

static void skip_chars(tokenizer_t *t, char *chrs)
{
	int c;

	while(strchr(chrs, c = next(t)))
		incr_pos(t, c);

	pb(t, c);
}

static void skip_char(tokenizer_t *t, char ch)
{
	int c;

	while((c = next(t)) == ch)
		incr_pos(t, c);
	pb(t, c);
}

static bool skip_until(tokenizer_t *t, char ch)
{
	int c;

	while((c = next(t)) != ch && c != EOF)
		incr_pos(t, c);
	if (c == EOF) {
		pb(t, c);
		return false;
	}
	pb(t, c);
	return true;
}

static bool rec_skip_until_chars(tokenizer_t *t, char *s)
{
	int c, i;

	i = 0;
	while(s[i] != '\0' && (c = next(t)) == s[i++])
		incr_pos(t, c);
	if (c == EOF || c != s[i]) {
		pb(t, c);
		return false;
	}
	return true;
}

static bool skip_until_chars(tokenizer_t *t, char *s)
{
	int c;

	if (skip_until(t, *s) && rec_skip_until_chars(t, s + 1)) {
		return true;
	}

	if ((c = next(t)) == EOF) {
		pb(t, c);
		return false;
	}
	pb(t, c);
	return skip_until_chars(t, s);
}
static char *get_debug_lines(FILE *fp, tpos_t tpos, int tklen)
{
	fpos_t org;
	char l1[1024];
	char l2[1024];
	char l3[1024];
	char ln1[32];
	char ln2[32];
	char ln3[32];
	char *res;

	org = fgetpos(fp, &org);

	fsetpos(fp, &tpos.prevline_pos);
	sprintf(ln1, "\033[32m\033[1m%5i\033[0m \033[1m|\033[0m ", tpos.line - 1);
	sprintf(ln2, "\033[32m\033[1m%5i\033[0m \033[1m|\033[0m ", tpos.line);
	sprintf(ln3, "\033[32m\033[1m%5i\033[0m \033[1m|\033[0m ", tpos.line + 1);
	fgets(l1, sizeof(l1), fp);
	fgets(l2, sizeof(l2), fp);
	fgets(l3, sizeof(l3), fp);

	res = malloc(strlen(l1) + strlen(l2) + strlen(l3) + 400);
	strcat(res, ln1);
	strcat(res, l1);
	strcat(res, "\n");
	strcat(res, ln2);
	strncat(res, l2, tpos.column);
	strcat(res, "\033[31;1;4m");
	strncat(res, l2 + tpos.column, tklen);
	strcat(res, "\033[0m");
	strcat(res, l2 + tpos.column + tklen);
	strcat(res, "\n");
	strcat(res, ln3);
	strcat(res, l3);
	strcat(res, "\n");
	fsetpos(fp, &org);
	return res;
}

static void tk_error(tokenizer_t *t, tpos_t p, int tklen, const char *restrict fmt, ...)
{
	va_list args;
	char *debug_lines;

	va_start(args, fmt);
	verror_custom("error", fmt, args);
	fprintf(stderr, "\033[1mIn file: %s, line: %i, col: %i\033[0m\n", t->filename, p.line, p.column);
	debug_lines = get_debug_lines(t->fp, p, tklen);
	exit(-1);
}

bool is_ws(i32_t c)
{
	return c == ' '
		|| c == '\t'
		|| c == '\r'
		|| c == '\f';
}

static void skip_ws(tokenizer_t *t)
{
	int c, c1;
	tpos_t pos;

	c = next(t);
	++t->column;

	pos = tpos(t);

	switch(c) {
		case '\f':
		case '\r':
		case '\t':
		case ' ':
			skip_ws(t);
			break;
		case '/': // '//' comment
			switch (c1 = next(t)) {
				case '*': goto multiline;
				case '/': goto fullline;
				default:
					pb(t, c1);
					pb(t, c);
					return;
			}
		fullline:
		case ';': // ';' comment
			skip_until(t, '\n');
			break;
		multiline: // /* */ comment
			if (!skip_until_chars(t, "*/"))
				tk_error(t, pos, 2, "unclosed multiline comment");
			break;
		default:
			pb(t, c);
			return;
	}
}


struct insmap_kvpair {
	const char *s;
	int slen;
	int ins;
};

static struct insmap_kvpair insmap[] = {
#define INSMAP_KVPAIR(instr) {#instr, sizeof(#instr), TK_##instr},
	XMACRO_INSTRUCTIONS(INSMAP_KVPAIR)
#undef INSMAP_KVPAIR
};

static int tk_instr_rec(tokenizer_t *t, char *rs, char *strb, struct insmap_kvpair *map, int len, int *d)
{
	int c, c1;
	int i;
	int len_;
	struct insmap_kvpair map_[len];
	char strb_[*d + 2];

	c = next(t);
	++t->column;

	if (*d != 0) {
		strcat(strb_, strb);
	}

	for (len_ = 0;i < len; i++) {
		if (map[i].s[*d] == toupper(c) && map[i].slen == *d - 1 && is_ws(peak(t))) {

			return map[i].ins;
		} else if(map[i].s[*d] == toupper(c) && map[i].slen >= *d) {
			map_[len_++] = map[i];
		}
	}
	strb_[*d] = c;
	strb_[*d + 1] = '\0';
	if (len_ <= 0) {
		rs = strdup(strb_);
		return TK_NULL;
	}
	++*d;
	return tk_instr_rec(t, rs, strb_, map_, len_, d);
}
static bool tk_instr(tokenizer_t *t, char *ifnot, tk_t *tok)
{
	int type;
	int d;
	tpos_t pos;

	d = 0;
	pos = tpos(t);
	type = tk_instr_rec(t, ifnot, NULL, insmap, sizeof(insmap), &d);

	if (type == TK_NULL) {
		return false;
	}
	*tok = tk(t, pos, type);
	return true;
}
static bool is_ident(int c)
{
	return isalnum(c) || c == '_';
}

static bool is_ident_begin(int c)
{
	return isalpha(c) || c == '_';
}

static bool tk_reg(tokenizer_t *t, tk_t *tok)
{
	int c, c1, rn;
	tpos_t pos;
	char str[2];

	pos = tpos(t);
	c = next(t);
	++t->column;
	switch(c) {
		case 'p':
			switch(peak(t)) {
				case 's':
					rn = REG_PROCESSOR_STATUS;
					goto found;
				case 'c':
					rn = REG_PROGRAM_COUNTER_L;
					goto found;
				default:
					goto not_found;
			}
		case 's':
			if((c1 = next(t)) == 'p') {
				rn = REG_STACK_POINTER;
				goto found;
			}
			pb(t, c1);
			goto not_found;
		case 'r':
		case 'b':
			if(isdigit(c1 = next(t)) && (rn = digittoint(c1)) >= 0 && rn <= 11) {
				goto found;
			}
			pb(t, c1);
			goto not_found;
		default:
			goto not_found;
	}
found:
	*tok = treg(t, pos, rn);
	if(!is_ident(peak(t)))
		return true;
not_found:
	pb(t, c);
		--t->column;
		return false;
}
static bool skip_newline(tokenizer_t *t)
{
	int c;

	c = next(t);
	if (c == '\n') {
		t->prevline_pos = t->line_pos;
		fgetpos(t->fp, &t->line_pos);
		t->column = 0;
		++t->line;
		return true;
	}
	pb(t, c);
	return false;
}
static bool next_is(tokenizer_t *t, char *s, int len, bool check_if_ident)
{
	int i;

	int chs[len];
	for (i = 0; i < len; i++) {
		if((chs[i] = next(t)) != s[i])
			goto not_true;
	}
	if (!check_if_ident || !is_ident(peak(t)))
		return true;
not_true:
	for(; i > 0; i--) {
		pb(t, chs[i]);
	}
	return false;
}

const char *SIMPLE_TOK = "*%&()[]=+|~-,";

tk_t		tk_next(tokenizer_t *t)
{
	int c, c1, c2;
	tpos_t pos;


	if (t->has_pb) {
		t->has_pb = false;
		return t->pb;
	}

	do {
		skip_ws(t);
	} while(skip_newline(t));

	pos = tpos(t);
	c = next(t);
	++t->column;

	if (strchr(SIMPLE_TOK, c))
		return tk(t, pos, c);

	switch (c) {
		case '<':
			if(next_is(t, "<", 1, false))
				return tk(t, pos, TK_LSL_OP);
			break;
		case '>':
			if(next_is(t, ">", 1, false))
				return tk(t, pos, TK_LSR_OP);
			break;
		case 'i':
			if (next_is(t, "8", 1, true))
				return tk(t, pos, TK_I8);
			if (next_is(t, "16", 1, true))
				return tk(t, pos, TK_I16);
			goto base;
		case 'u':
			if (next_is(t, "8", 1, true))
				return tk(t, pos, TK_U8);
			if (next_is(t, "16", 1, true))
				return tk(t, pos, TK_U16);
			goto base;
		case '.': // section

base:		default:

		break;
	}
	return tk(t, pos, c);
}

tokenizer_t	*new_tokenizer(const char *const filename)
{
	tokenizer_t *res;

	res	= malloc(sizeof(tokenizer_t));

	res->fp	= fopen(filename, "r");

	if (!res->fp)
		exit_error_custom("ASSEMBLER", "could not open file: %s", filename);

	res->line	= 0;
	res->column	= 0;
	res->filename	= filename;
	fgetpos(res->fp, &res->prevline_pos);
	res->line_pos = res->prevline_pos;

	return res;
}


tk_t		tk_prev(tokenizer_t *t)
{
	ASSERT(t->has_pb, "no previous available");
	if (t->has_pb) {
		t->has_pb = false;
		return t->pb;
	}
}

void		tk_close(tokenizer_t **tokenizer);

void tk_rev(tokenizer_t *tk, tk_t t)
{
	ASSERT(!tk->has_pb, "cannot reverse tokenizer twice");
	tk->has_pb = true;
	tk->pb = t;
}