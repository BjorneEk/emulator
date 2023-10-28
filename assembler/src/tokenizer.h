/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * tokenizer
 *==========================================================*/

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "../../instructions/interface.h"
#include "util/types.h"
#include <stdio.h>

#define XMACRO_OTHER_TOKENS(X)				\
	X(REGISTER)		X(CHARACTER_LITERAL)	\
	X(STRING_LITERAL)	X(INTEGER_LITERAL)	\
	X(LSR_OP)		X(LSL_OP)		\
	X(IDENT)		X(SECTION_GLOBAL)	\
	X(SECTION_TEXT)		X(SECTION_DATA)		\
	X(STRING)		X(CHAR)			\
	X(I8)			X(U8)			\
	X(I16)			X(U16)			\
	X(INSTRUCTION)

enum token_type {
	TK_NULL = 0xFF,
#define TOKEN_NAME(name) TK_##name,
	XMACRO_OTHER_TOKENS(TOKEN_NAME)
#undef TOKEN_NAME
	TK_CONSTEXPR16,
	TK_CONSTEXPR32,
	TK_ADDR_MODE
};

#define MAX_IDENT (500)
typedef struct tpos {
	int		line;
	int		column;
	fpos_t		prevline_pos;
} tpos_t;

typedef struct debug_info {
	tpos_t p;
	FILE *fp;
	const char *filename;
} debug_info_t;

typedef struct token {
	int type;
	union {
		int instype;
		int int_val;
		char *str_val;
		int reg;
	};
	debug_info_t debug;
} tk_t;

typedef struct tokenizer tokenizer_t;

tokenizer_t	*new_tokenizer(const char *const filename);

tk_t		tk_next(tokenizer_t *tokenizer);
tk_t		tk_prev(tokenizer_t *tokenizer);
void		tk_rev(tokenizer_t *tokenizer, tk_t t);
void		tk_close(tokenizer_t **tokenizer);

void		tk_print(tk_t t);

#endif /* _TOKENIZER_H_ */