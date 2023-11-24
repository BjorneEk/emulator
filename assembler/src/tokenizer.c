/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * tokenizer
 *==========================================================*/

#include "tokenizer.h"
#include "../../arch/interface.h"
#include "file.h"
#include "../../common/util/error.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>



typedef struct tokenizer {

	fstack_t	*files;
	file_t		*file;

	int		line;
	int		column;
	fpos_t		prevline_pos;
	fpos_t		line_pos;

	tk_t pb;

	bool has_pb;
} tokenizer_t;


void		(*tokenizer_free)(tokenizer_t *) = (void (*)(tokenizer_t *))free;

static char esc_chars[256] = {
	['a'] = '\a', ['b'] = '\b',   ['f'] = '\f',
	['n'] = '\n', ['r'] = '\r',   ['t'] = '\t',
	['v'] = '\v', ['e'] = '\033', ['E'] = '\033',
};

static inline tpos_t tpos(tokenizer_t *t)
{
	return (tpos_t) {t->line, t->column, t->prevline_pos, t->file};
}

static inline int next(tokenizer_t *t)
{
	int res;

	res = fnext(t->file);
	if (res == EOF && t->files != NULL) {
		t->file = pop_file(&t->files);
		t->column = 0;
		t->line = 0;
		fgetpos(t->file->fp, &t->line_pos);
		t->prevline_pos = t->line_pos;
		t->has_pb = false;
		return next(t);
	}
	return res;
}

static inline void pb(tokenizer_t *t, int c)
{
	fpb(t->file, c);
}

static inline int peak(tokenizer_t *t)
{
	int c;
	c = next(t);
	pb(t, c);
	return c;
}

static tk_t	tk(tokenizer_t *t, tpos_t p, int type)
{
	tk_t res;
	res.type = type;
	res.debug = p;
	res.tklen = 1;
	return res;
}
static tk_t	tglob(tokenizer_t *t, tpos_t p)
{
	tk_t res;
	res = tk(t, p, TK_GLOBAL);
	res.tklen = 6;
	return res;
}
static tk_t	tins(tokenizer_t *t, tpos_t p, int instype)
{
	tk_t res;
	res = tk(t, p, TK_INSTRUCTION);
	res.instype = instype;
	res.tklen = 3;
	return res;
}

static tk_t	treg(tokenizer_t *t, tpos_t p, int reg)
{
	tk_t res;

	res = tk(t, p, TK_REGISTER);
	res.reg = reg;
	res.tklen = 2;
	return res;
}

static tk_t	tstr(tokenizer_t *t, tpos_t p, int type, char *nm)
{
	tk_t res;

	res = tk(t, p, type);
	res.str_val = nm;
	res.tklen = strlen(nm);
	return res;
}

static tk_t	tident(tokenizer_t *t, tpos_t p, char *nm)
{
	return tstr(t, p, TK_IDENT, nm);
}

static tk_t	tsection(tokenizer_t *t, tpos_t p, char *nm)
{
	return tstr(t, p, TK_SECTION, nm);
}

static tk_t	tstrlit(tokenizer_t *t, tpos_t p, char *nm)
{
	tk_t res;
	res = tstr(t, p, TK_STRING_LITERAL, nm);
	return res;
}
static tk_t	tint(tokenizer_t *t, tpos_t p, int type, int val)
{
	tk_t res;

	res = tk(t, p, type);
	res.int_val = val;
	res.tklen = 1;
	return res;
}
static tk_t	tintlit(tokenizer_t *t, tpos_t p, int val, int tklen)
{
	tk_t res;
	res = tint(t, p, TK_INTEGER_LITERAL, val);
	res.tklen = tklen;
	return res;
}
static tk_t	tcharlit(tokenizer_t *t, tpos_t p, int val)
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
		fgetpos(t->file->fp, &t->line_pos);
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

static void print_debug_lines(FILE *f, tpos_t tpos, int tklen)
{
	fpos_t org;
	char l1[1024];
	char l2[1024];
	char l3[1024];

	char l_[1024];
	char tok[1024];
	int lc;

	if (tpos.line == -1) {
		puts("generated token");
		return;
	}

	fgetpos(tpos.file->fp, &org);

	fsetpos(tpos.file->fp, &tpos.prevline_pos);


	fgets(l1, 1024, tpos.file->fp);
	fgets(l2, 1024, tpos.file->fp);
	lc = (fgets(l3, 1024, tpos.file->fp)) ? 3 : 2;

	if (tpos.line == 0) {
		strncpy(l_, l1, tpos.column);
		l_[tpos.column] = '\0';
		strncpy(tok, l1 + tpos.column, tklen);
		tok[tklen] = '\0';
		fprintf(f, "\033[32m\033[1m%5i\033[0m \033[1m│\033[0m %s\033[31;1;4m%s\033[0m%s\033[32m", tpos.line + 1, l_, tok, l1 + tpos.column + tklen);
		fprintf(f, "\033[32m\033[1m%5i\033[0m \033[1m│\033[0m %s", tpos.line + 2, l2);
	} else {
		strncpy(l_, l2, tpos.column);
		l_[tpos.column] = '\0';
		strncpy(tok, l2 + tpos.column, tklen);
		tok[tklen] = '\0';
		fprintf(f, "\033[32m\033[1m%5i\033[0m \033[1m│\033[0m %s", tpos.line, l1);
		fprintf(f, "\033[32m\033[1m%5i\033[0m \033[1m│\033[0m %s\033[31;1;4m%s\033[0m%s\033[32m", tpos.line + 1, l_, tok, l2 + tpos.column + tklen);
	}

	if (lc == 3)
		printf("\033[32m\033[1m%5i\033[0m \033[1m│\033[0m %s\n", tpos.line + ((tpos.line == 0) ? 3 : 2), l3);
	fsetpos(tpos.file->fp, &org);
	return;
}

void tk_error(tpos_t p, int tklen, const char *restrict fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	verror_custom("Error", fmt, args);
	fprintf(stderr, "In file: \033[1m%s\033[0m, line: \033[1m%i\033[0m, col: \033[1m%i\033[0m\n", p.file->filename, p.line + 1, p.column + 1);
	print_debug_lines(stderr, p, tklen);
	exit(-1);
}

void tk_print_error(tk_t tk, const char *restrict fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	verror_custom("Error", fmt, args);
	fprintf(stderr, "In file: \033[1m%s\033[0m, line: \033[1m%i\033[0m, col: \033[1m%i\033[0m\n", tk.debug.file->filename, tk.debug.line + 1, tk.debug.column + 1);
	print_debug_lines(stderr, tk.debug, tk.tklen);
}

void tk_warning(tk_t tk, const char *restrict fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vwarn_custom("Warning", fmt, args);
	fprintf(stderr, "In file: \033[1m%s\033[0m, line: \033[1m%i\033[0m, col: \033[1m%i\033[0m\n", tk.debug.file->filename, tk.debug.line + 1, tk.debug.column + 1);
	print_debug_lines(stderr, tk.debug, tk.tklen);
}

bool is_ws(i32_t c)
{
	return c == ' '
		|| c == '\t'
		|| c == '\r'
		|| c == '\f' || c == '	';
}
static void line_comment(tokenizer_t *t)
{
	int c;

	c = next(t);

	if (c == '\n' || c == EOF) {
		pb(t, c);
		return;
	}

	incr_pos(t, c);
	line_comment(t);
}
static void skip_ws(tokenizer_t *t);

static void multiline_comment(tokenizer_t *t, tpos_t prev_pos)
{
	int c, c1;
	tpos_t pos;

	skip_ws(t);
	c = next(t);
	++t->column;
	pos = tpos(t);

	if (c == '*') {
		c1 = next(t);
		++t->column;
		if (c1 == '/')
			return;
	}

	if (c == EOF)
		tk_error(prev_pos, 2, "unclosed multiline comment");

	multiline_comment(t, prev_pos);
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
			return;
		case '/': // '//' comment
			++t->column;
			switch (c1 = next(t)) {
				case '*':
					multiline_comment(t, pos);
					return;
				case '/':
					line_comment(t);
					return;
				default:
					t->column -= 2;
					pb(t, c1);
					pb(t, c);
					return;
			}
		case ';': // ';' comment
			line_comment(t);
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
#define INSMAP_KVPAIR(instr) {#instr, sizeof(#instr), INSTR_##instr},
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
			if (is_ws(peak(t)) || peak(t) == '\n') {
				//t->column += d + 1;
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
		--t->column;
		pb(t, c);
	}

	if (len_ <= 0) {
		*rs = strdup(strb_);
		return INSTR_NULL;
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

	if (type == INSTR_NULL) {
		return false;
	}
	*tok = tins(t, pos, type);
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
			switch(c1 = next(t)) {
				case 's':
					rn = REG_PROCESSOR_STATUS;
					++t->column;
					goto found;
				case 'c':
					rn = REG_PROGRAM_COUNTER_L;
					++t->column;
					goto found;
				default:
					pb(t, c1);
					goto not_found;
			}
		case 's':
			if((c1 = next(t)) == 'p') {
				rn = REG_STACK_POINTER;
				++t->column;
				goto found;
			}
			pb(t, c1);
			goto not_found;
		case 'r':
		case 'b':
			if(isdigit(c1 = next(t)) && (rn = digittoint(c1)) >= 0 && rn <= 11) {
				++t->column;
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
		fgetpos(t->file->fp, &t->line_pos);
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
	tk_error(p, 1, "error reading hexadecimal in literal");
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
			tk_error(p, strlen(res), "unclosed string literal after \033[32;1;4m'%s'\033[0m \nUse \"\"\" 'string' \"\"\" for multiline strings", res);
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

u64_t read_hex(tokenizer_t *t, tpos_t p, int *res)
{
	u64_t tklen;
	int c;

	tklen = t->column;
	*res = 0;

	while(isxdigit(c = next(t)) || c == '_') {
		++t->column;
		if (c != '_')
			*res = *res * 16 + hex(t, p, c);
	}
	pb(t, c);
	return t->column - tklen;
}


u64_t read_bin(tokenizer_t *t, int *res)
{
	u64_t tklen;
	int c;

	tklen = t->column;
	*res = 0;

	while(((c = next(t)) == '1' || c == '0') || c == '_') {
		++t->column;
		if (c != '_')
			*res = *res * 2 + c - '0';
	}
	pb(t, c);
	return t->column - tklen;
}

u64_t read_int(tokenizer_t *t, int *res)
{
	u64_t tklen;
	int c;

	tklen = t->column;
	*res = 0;

	while(isdigit(c = next(t)) || c == '_') {
		++t->column;
		if (c != '_')
			*res = *res * 10 + c - '0';
	}
	pb(t, c);
	return t->column - tklen;
}

static int parse_intlit(tokenizer_t *t, tpos_t p, int *res)
{
	char c, pref;
	int len;
	c = next(t);
	++t->column;

	if(c == '0') {
		pref = next(t);
		if(pref == 'X' || pref == 'x') {
			++t->column;
			return read_hex(t, p, res);
		} else if(pref == 'b' || pref == 'B') {
			++t->column;
			return read_bin(t, res);
		}
		pb(t, pref);
	} else if (c == '-') {
		len = read_int(t, res) + 1;
		*res = -(*res);
		return len;
	}
	--t->column;
	pb(t, c);
	return read_int(t, res);
}

static char *parse_ident(tokenizer_t *t, tpos_t p, char *head)
{
	char buffer[MAX_IDENT];
	char *res;
	int i, j;
	bool undo;
	size_t len, len_;


	if (head == NULL) {
		len_ = len = 0;
		goto read;
	}

	len_ = len = strlen(head);
	strcpy(buffer, head);

	if (!is_ident_begin(buffer[0]))
		tk_error(p, 1, "Unexpected token: '%c'", buffer[0]);

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
read:
	do {
		++t->column;
	} while(is_ident(buffer[len++] = next(t)) && len < MAX_IDENT);

	if (len < MAX_IDENT) {
		pb(t, buffer[len-1]);
		--t->column;
	}
end:
	res = malloc(len * sizeof(char));
	strncpy(res, buffer, len - 1);

	res[len - 1] = '\0';
	return res;
}
const char *SIMPLE_TOK = "*%&()[]=+|~-,:^";

tk_t		_tk_next(tokenizer_t *t)
{
	int c;
	int char_literal;
	int int_literal;
	int tklen;
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
			MATCH("tring")	return tk(t, pos, TK_STRING);
			goto base;
		case '.':
			MATCH("global")	return tglob(t, pos);
			return tsection(t, pos, parse_ident(t, pos, NULL));
		case '"':
			return tstrlit(t, pos, parse_strlit(t, pos));
		case '\'':
			char_literal = next_charlit(t);
			if (next(t) != '\'')
				tk_error(pos, 3, "Unclosed character literal: \033[32;1;4m\'%c\033[0m", char_literal);
			return tcharlit(t, pos, char_literal);
		case EOF:
			return tk(t, pos, TK_NULL);
		case '#':
			tklen = parse_intlit(t, pos, &int_literal);
			return tintlit(t, pos, int_literal, tklen + 1);
base:		default:
		--t->column;
		pb(t, c);

		if (tk_reg(t, &res))
			return res;
		if (tk_instr(t, &ifnot, &res))
			return res;
		if(is_ident_begin(ifnot[0]))
			return tident(t, pos, parse_ident(t, pos, ifnot));

		tk_error(pos, 1, "Unexpected token '%c' (%i)", c, c);
	}
	return tk(t, pos, c);
}
tk_t		tk_next(tokenizer_t *t)
{
	tk_t res;

	res = _tk_next(t);
	//tk_debug(res);
	return res;
}

tokenizer_t	*new_tokenizer(fstack_t **files)
{
	tokenizer_t *res;

	res	= malloc(sizeof(tokenizer_t));
	res->files = *files;
	ASSERT_(files != NULL, "no files supplied when creating tokenizer");

	res->file = pop_file(&res->files);

	res->line	= 0;
	res->column	= 0;
	fgetpos(res->file->fp, &res->prevline_pos);
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



void _tk_rev(tokenizer_t *tk, tk_t t)
{
	ASSERT_(!tk->has_pb, "cannot reverse tokenizer twice");
	tk->has_pb = true;
	tk->pb = t;
}
void tk_rev(tokenizer_t *tk, tk_t t)
{
	//BUG("Putback: ");
	//tk_print(t);
	_tk_rev(tk, t);
}
#define XMACRO_SIMPLE_TOK_(X)				\
	X(LSR_OP)		X(LSL_OP)		\
	X(GLOBAL)		X(STRING)		\
	X(CHAR)			X(I8)			\
	X(U8)			X(I16)			\
	X(U16)

void tk_type_tostring(int t, char *res)
{
	switch(t) {
	#define INSTR_CASE(name) case TK_##name : sprintf(res,"%s", #name); break;
		XMACRO_SIMPLE_TOK_(INSTR_CASE)
	#undef INSTR_CASE
		case TK_INSTRUCTION:
			sprintf(res, "instruction");
			break;
		case TK_SECTION:
			sprintf(res, "section");
			break;
		case TK_IDENT:
			sprintf(res, "identifyer");
			break;
		case TK_REGISTER:
			sprintf(res, "register");
			break;
		case TK_INTEGER_LITERAL:
			sprintf(res, "integer-literal");
			break;
		case TK_CHARACTER_LITERAL:
			sprintf(res, "character-literal");
			break;
		case TK_STRING_LITERAL:
			sprintf(res, "string-literal");
			break;
		case TK_NULL:
			sprintf(res, "EOF");
			break;
		default:
			sprintf(res, "'%c'", t);
	}
	for (;*res != '\0'; res++)
		*res = tolower(*res);
}

void tk_tostring(tk_t t, char *res)
{
	char typestr[100];
	char valstr[4096];
	tk_type_tostring(t.type, typestr);
	switch(t.type) {
	#define INSTR_CASE(name) case TK_##name : sprintf(valstr, "%s", #name); break;
		XMACRO_SIMPLE_TOK_(INSTR_CASE)
	#undef INSTR_CASE

		case TK_INSTRUCTION:
			switch(t.instype) {
			#define INSTR_CASE(name) case INSTR_##name : sprintf(valstr, "%s", #name); break;
				XMACRO_INSTRUCTIONS(INSTR_CASE)
			#undef INSTR_CASE
			}
			break;
		case TK_IDENT:
		case TK_SECTION:
			sprintf(valstr, "%s", t.str_val);
			break;
		case TK_REGISTER:
			switch (t.reg) {
			#define REG_CASE(rname) case REG_##rname : sprintf(valstr, "%s", #rname); break;
				XMACRO_REGISTER(REG_CASE)
			#undef REG_CASE
			}
			break;
		case TK_INTEGER_LITERAL:
			sprintf(valstr, "%i", t.int_val);
			break;
		case TK_CHARACTER_LITERAL:
			sprintf(valstr, "'%c'", (int)t.int_val);
			break;
		case TK_STRING_LITERAL:
			sprintf(valstr, "\"%s\"", t.str_val);
			break;
		case TK_NULL:
			sprintf(valstr, "EOF");
			break;
		default:
			sprintf(res, "Token: %s", typestr);
			return;
	}
	sprintf(res, "Token: %s (%s)", typestr, valstr);
}

void tk_debug(tk_t t)
{
	char str[4096];
	tk_tostring(t, str);

	printf("%s, tklen: %i, file: %s, line: %i, col: %i\n", str,
	t.tklen, t.debug.file->filename, t.debug.line + 1, t.debug.column + 1);
}

void tk_print(tk_t t)
{
	char str[4096];
	tk_tostring(t, str);
	puts(str);
}