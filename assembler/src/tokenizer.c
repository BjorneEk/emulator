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

static char esc_chars[256] = {
	['a'] = '\a', ['b'] = '\b',   ['f'] = '\f',
	['n'] = '\n', ['r'] = '\r',   ['t'] = '\t',
	['v'] = '\v', ['e'] = '\033', ['E'] = '\033',
};
 __attribute__((unused))
static inline struct tokenizer_pos get_pos(tokenizer_t *t)
{
	return (struct tokenizer_pos){t->line, t->column, t->prevline_pos, t->line_pos};
}
 __attribute__((unused))
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

static inline tk_t	tident(tokenizer_t *t, tpos_t p, char *nm)
{
	return tstr(t, p, TK_IDENT, nm);
}

static inline tk_t	tstrlit(tokenizer_t *t, tpos_t p, char *nm)
{
	tk_t res;

	res = tk(t, p, TK_STRING_LITERAL);
	res.str_val = nm;
	return res;
}
static inline tk_t	tint(tokenizer_t *t, tpos_t p, int type, int val)
{
	tk_t res;

	res = tk(t, p, type);
	res.int_val = val;
	return res;
}
static inline tk_t	tintlit(tokenizer_t *t, tpos_t p, int val)
{
	return tint(t, p, TK_INTEGER_LITERAL, val);
}
static inline tk_t	tcharlit(tokenizer_t *t, tpos_t p, int val)
{
	return tint(t, p, TK_CHARACTER_LITERAL, val);
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
__attribute__((unused))
static void skip_chars(tokenizer_t *t, char *chrs)
{
	int c;

	while(strchr(chrs, c = next(t)))
		incr_pos(t, c);

	pb(t, c);
}
__attribute__((unused))
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
	int lc;

	org = fgetpos(fp, &org);

	fsetpos(fp, &tpos.prevline_pos);
	sprintf(ln1, "\033[32m\033[1m%5i\033[0m \033[1m|\033[0m ", tpos.line - 1);
	sprintf(ln2, "\033[32m\033[1m%5i\033[0m \033[1m|\033[0m ", tpos.line);
	sprintf(ln3, "\033[32m\033[1m%5i\033[0m \033[1m|\033[0m ", tpos.line + 1);
	fgets(l1, sizeof(l1), fp);
	lc = (fgets(l2, sizeof(l2), fp) && fgets(l3, sizeof(l3), fp)) ? 3 : 2;



	res = malloc(strlen(l1) + strlen(l2) + strlen(l3) + 400);
	strcat(res, ln1);
	strcat(res, l1);
	strcat(res, ln2);
	strncat(res, l2, tpos.column);
	strcat(res, "\033[31;1;4m");
	strncat(res, l2 + tpos.column, tklen);
	strcat(res, "\033[0m");
	strcat(res, l2 + tpos.column + tklen);
	if (lc == 3) {
		strcat(res, ln3);
		strcat(res, l3);
	}
	fsetpos(fp, &org);
	return res;
}

static void tk_error(tokenizer_t *t, tpos_t p, int tklen, const char *restrict fmt, ...)
{
	va_list args;
	char *debug_lines;

	va_start(args, fmt);
	verror_custom("Error", fmt, args);
	fprintf(stderr, "In file: \033[1m%s\033[0m, line: \033[1m%i\033[0m, col: \033[1m%i\033[0m\n", t->filename, p.line, p.column);
	debug_lines = get_debug_lines(t->fp, p, tklen);
	puts(debug_lines);
	free(debug_lines);
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
			--t->column;
			pb(t, c);
			return;
	}
}

static bool is_ident(int c)
{
	return isalnum(c) || c == '_';
}

static bool is_ident_begin(int c)
{
	return isalpha(c) || c == '_';
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

static int tk_instr_rec(tokenizer_t *t, char **rs, char *strb, struct insmap_kvpair *map, int len, int d)
{
	int c;
	int i;
	int len_;
	struct insmap_kvpair map_[len];
	char strb_[d + 1];

	c = next(t);
	++t->column;
	strb_[0] = '\0';
	if (d != 0 && strb[0] != '\0') {
		strcat(strb_, strb);
	}
	len_ = 0;
	for (i = 0; i < len; i++) {
		if (d <= map[i].slen && map[i].s[d] == toupper(c) && map[i].slen - 2 == d) {
			if (is_ws(peak(t))) {
				t->column += d + 1;
				return map[i].ins;
			}
		} else if(d <= map[i].slen && map[i].s[d] == toupper(c) && map[i].slen >= d) {
			map_[len_++] = map[i];
		}
	}
	if (is_ident(c)) {
		strb_[d] = c;
		strb_[d + 1] = '\0';
	} else {
		pb(t, c);
	}

	if (len_ <= 0) {
		*rs = strdup(strb_);
		return TK_NULL;
	}

	return tk_instr_rec(t, rs, strb_, map_, len_, d + 1);
}
static bool tk_instr(tokenizer_t *t, char **ifnot, tk_t *tok)
{
	int type;
	int d;
	tpos_t pos;

	d = 0;
	pos = tpos(t);

	type = tk_instr_rec(t, ifnot, NULL, insmap, sizeof(insmap) / sizeof (struct insmap_kvpair), d);

	if (type == TK_NULL) {
		return false;
	}
	*tok = tk(t, pos, type);
	return true;
}


static bool tk_reg(tokenizer_t *t, tk_t *tok)
{
	int c, c1, rn;
	tpos_t pos;

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
		++t->column;
		if((chs[i] = next(t)) != s[i]) {
			goto not_true;
		}
	}
	if (!check_if_ident || !is_ident(peak(t))) {
		return true;
	}
not_true:
	for(; i >= 0; i--) {
		--t->column;
		pb(t, chs[i]);
	}
	return false;
}


void *paste_buffer(void *dst, u32_t *sz, void *buff, u32_t *bz, size_t width)
{
	void *res;

	res = realloc(dst, (*sz + *bz) * width);
	memcpy(res + (*sz * width), buff, *bz * width);
	*sz += *bz;
	*bz = 0;
	return res;
}

static bool isoctal(char c)
{
	return '0' <= c && c <= '7';
}
static int hex(tokenizer_t *t, tpos_t p, char c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	tk_error(t, p, 1, "error reading hexadecimal in literal");
	return 0;
}

int next_charlit(tokenizer_t *t)
{
	int c;
	int res;

	c = next(t);
	incr_pos(t, c);
	if (c != '\\')
		return c;

	res = esc_chars[(u8_t)(c = next(t))];
	incr_pos(t, c);
	if (res)
		return res;

	if (c == 'x') {
		res = 0;
		c = next(t);
		++t->column;
		while (isxdigit(c = next(t))) {
			res = res * 16 + hex(t, tpos(t), c);
			++t->column;
		}
		--t->column;
		pb(t, c);
		return res;
	}

	if (isoctal(c)) {
		res = c - '0';
		if (isoctal(c = next(t))) {
			++t->column;
			res = res * 8 + c - '0';
		} else {
			pb(t, c);
			return res;
		}
		if (isoctal(c = next(t))) {
			++t->column;
			res = res * 8 + c - '0';
		} else {
			pb(t, c);
		}
		return res;
	}
	return c;
}

char *parse_strlit(tokenizer_t *t, tpos_t p)
{
	char buff[4096];
	char *res = NULL;
	u32_t res_len;
	u32_t buff_len;
	bool multiline;
	char c;

	c = next(t);
	incr_pos(t, c);
	multiline = (c == '"');
	buff_len = res_len = 0;

	if(multiline && (c = next(t)) != '"') {	/*empty single line string*/
		pb(t, c);
		res = malloc(sizeof(char));
		*res = '\0';
		return res;
	} else if (!multiline) {
		pb(t, c);
	}
	for(;;) {
		buff[buff_len++] = c = next(t);
		if (c == '\\') {
			pb(t, c);
			buff[buff_len-1] = next_charlit(t);
		} else if (multiline && c == '"') {
			++t->column;
			if((c = next(t)) == '"' && (c = next(t)) == '"') {
				t->column += 2;
				buff_len--;
				break;
			} else {
				++t->column;
				buff[buff_len++] = c;
			}
		} else if(c == '"') {
			++t->column;
			--buff_len;
			break;
		}

		if (!multiline && c == '\n') {

			incr_pos(t, c);
			res = paste_buffer(res, &res_len, buff, (--buff_len, &buff_len), sizeof(char));
			tk_error(t, p, buff_len, "unclosed string literal after \033[32;1;4m'%s'\033[0m \nUse \"\"\" 'string' \"\"\" for multiline strings", res);
			return NULL;
		}
		if(buff_len + 2 == 4096) {
			res = paste_buffer(res, &res_len, buff, &buff_len, sizeof(char));
		}
	}
	buff[buff_len++] = '\0';
	res = paste_buffer(res, &res_len, buff, &buff_len, sizeof(char));
	return res;
}

u64_t read_hex(tokenizer_t *t, tpos_t p)
{
	u64_t res;
	int c;
	res = 0;
	while(isxdigit(c = next(t)) || c == '_') {
		--t->column;
		if (c != '_')
			res = res * 16 + hex(t, p, c);
	}
	--t->column;
	pb(t, c);
	return res;
}


u64_t read_bin(tokenizer_t *t)
{
	u64_t res;
	int c;

	res = 0;
	while(((c = next(t)) == '1' || c == '0') || c == '_') {
		--t->column;
		if (c != '_')
			res = res * 2 + c - '0';
	}
	--t->column;
	pb(t, c);
	return res;
}

u64_t read_int(tokenizer_t *t)
{
	u64_t res;
	int c;

	res = 0;
	while(isdigit(c = next(t)) || c == '_') {
		--t->column;
		if (c != '_')
			res = res * 10 + c - '0';
	}
	--t->column;
	pb(t, c);
	return res;
}

static int parse_intlit(tokenizer_t *t, tpos_t p)
{
	char c, pref;
	c = next(t);
	++t->column;
	if(c == '0') {
		pref = next(t);
		if(pref == 'X' || pref == 'x') {
			++t->column;
			return read_hex(t, p);
		} else if(pref == 'b' || pref == 'B') {
			++t->column;
			return read_bin(t);
		}
		pb(t, pref);
	}
	--t->column;
	pb(t, c);
	return read_int(t);
}

static char *parse_ident(tokenizer_t *t, tpos_t p, char *head)
{
	char buffer[MAX_IDENT];
	char *res;
	int i, j;
	bool undo;
	size_t len, len_;

	len_ = len = strlen(head) + 1;
	strcpy(buffer, head);

	if (!is_ident_begin(buffer[0]))
		tk_error(t, p, 1, "Unexpected token: '%c'", buffer[0]);
	undo = false;
	for (i = 1, j = 0; i < len - 1; i++) {
		if (!undo && !is_ident(buffer[i])) {
			undo = true;
			len = i + 2;
		}
		if (undo) {
			pb(t, buffer[(len_ - 2) - j++]);
		}
	}
	if (undo)
		goto end;
	while(is_ident(buffer[len++] = next(t)) && len < MAX_IDENT)
		++t->column;
	pb(t, buffer[len-1]);
	--t->column;
end:
	res = malloc(len * sizeof(char));
	memcpy(res, buffer, len-1);

	res[len-1] = '\0';

	return res;
}
const char *SIMPLE_TOK = "*%&()[]=+|~-,:";

tk_t		tk_next(tokenizer_t *t)
{
	int c;
	int charlit;
	tk_t res;
	char *ifnot = NULL;
	tpos_t pos;

	#define MATCH(str) if (next_is(t, str, sizeof(str) - 1, true))
	#define MATCH_(str) if (next_is(t, str, sizeof(str) - 1, false))
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
			MATCH_("<")	return tk(t, pos, TK_LSL_OP);
			break;
		case '>':
			MATCH_(">")	return tk(t, pos, TK_LSR_OP);
			break;
		case 'i':
			MATCH("8")	return tk(t, pos, TK_I8);
			MATCH("16")	return tk(t, pos, TK_I16);
			goto base;
		case 'u':
			MATCH("8")	return tk(t, pos, TK_U8);
			MATCH("16")	return tk(t, pos, TK_U16);
			goto base;
		case 'c':
			MATCH("har")	return tk(t, pos, TK_CHAR);
			goto base;
		case 's':
			MATCH("string")	return tk(t, pos, TK_CHAR);
			goto base;
		case '.':
			MATCH("global")	return tk(t, pos, TK_SECTION_GLOBAL);
			MATCH("data")	return tk(t, pos, TK_SECTION_DATA);
			MATCH("text")	return tk(t, pos, TK_SECTION_TEXT);
			tk_error(t, pos, 1, "Expected section name: 'global', 'data' or 'text' after '.' character");
		case '"':
			return tstrlit(t, pos, parse_strlit(t, pos));
		case '\'':
			charlit = next_charlit(t);
			if (next(t) != '\'')
				tk_error(t, pos, 3, "Unclosed character literal: \033[32;1;4m\'%c\033[0m", charlit);
			return tcharlit(t, pos, charlit);
		case EOF:
			return tk(t, pos, TK_NULL);
		case '#':
			return tintlit(t, pos, parse_intlit(t, pos));
base:		default:
		--t->column;
		pb(t, c);
		if (tk_reg(t, &res))
			return res;
		if (tk_instr(t, &ifnot, &res))
			return res;
		if(is_ident_begin(ifnot[0]))
			return tident(t, pos, parse_ident(t, pos, ifnot));
		tk_error(t, pos, 1, "Unexpected token '%c'", peak(t));
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
	ASSERT_(t->has_pb, "no previous available");
	if (t->has_pb) {
		t->has_pb = false;
		return t->pb;
	}
	return (tk_t){0};
}

void		tk_close(tokenizer_t **tokenizer);

void tk_rev(tokenizer_t *tk, tk_t t)
{
	ASSERT_(!tk->has_pb, "cannot reverse tokenizer twice");
	tk->has_pb = true;
	tk->pb = t;
}
#define XMACRO_SIMPLE_TOK_(X)				\
	X(LSR_OP)		X(LSL_OP)		\
	X(SECTION_GLOBAL)	X(SECTION_TEXT)		\
	X(SECTION_DATA)		X(STRING)		\
	X(CHAR)			X(I8)			\
	X(U8)			X(I16)			\
	X(U16)
void tk_print(tk_t t)
{
	switch(t.type) {
	#define INSTR_CASE(name) case TK_##name : printf("tok:	%s\n", #name); break;
		XMACRO_INSTRUCTIONS(INSTR_CASE)
		XMACRO_SIMPLE_TOK_(INSTR_CASE)
	#undef INSTR_CASE


		case TK_IDENT:
			printf("ident:	%s\n", t.str_val);
			break;
		case TK_REGISTER:
			switch (t.reg) {
			#define REG_CASE(rname) case REG_##rname : printf("reg:	%s\n", #rname); break;
				XMACRO_REGISTER(REG_CASE)
			#undef REG_CASE
			}
			break;
		case TK_INTEGER_LITERAL:
			printf("intlit:	%llu\n", t.int_val);
			break;
		case TK_CHARACTER_LITERAL:
			printf("chlit:	'%c'\n", (int)t.int_val);
			break;
		case TK_STRING_LITERAL:
			printf("strlit:	\"%s\"\n", t.str_val);
			break;
		case TK_NULL:
			printf("tok:	EOF\n");
			break;
		default:
			printf("tok:	'%c'\n", t.type);
	}
}