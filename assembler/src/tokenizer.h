/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * tokenizer
 *==========================================================*/

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "../../arch/interface.h"
#include "../../common/util/types.h"
#include "file.h"
#include <stdio.h>

#define XMACRO_OTHER_TOKENS(X)				\
	X(REGISTER)		X(CHARACTER_LITERAL)	\
	X(STRING_LITERAL)	X(INTEGER_LITERAL)	\
	X(LSR_OP)		X(LSL_OP)		\
	X(IDENT)		X(GLOBAL)		\
	X(SECTION)		X(INSTRUCTION)		\
	X(STRING)		X(CHAR)			\
	X(I8)			X(U8)			\
	X(I16)			X(U16)			\
	X(I32)			X(U32)			\

#define XMACRO_LINKER_TOKENS(X)			\
	X(RO_MEMORY)	X(RA_MEMORY)		\
	X(PROGRAM)	X(FORMAT)		\
	X(BINARY)	X(HEX)			\
	X(ENTRY)	X(ENTRY_STORE)		\
	X(ENDIAN)	X(LITTLE_ENDIAN)	\
	X(BIG_ENDIAN)	X(PAD)			\
	X(PUT)		X(ORG)			\
	X(LENGTH)

enum token_type {
	TK_NULL = 0xFF,
#define TOKEN_NAME(name) TK_##name,
	XMACRO_OTHER_TOKENS(TOKEN_NAME)
#undef TOKEN_NAME
	TK_LAST
};

#define MAX_IDENT (500)
typedef struct tpos {
	int		line;
	int		column;
	fpos_t		prevline_pos;
	file_t		*file;
} tpos_t;

typedef struct token {
	int type;
	int tklen;
	union {
		int instype;
		int int_val;
		char *str_val;
		int reg;
	};
	tpos_t debug;
} tk_t;

typedef struct tokenizer tokenizer_t;

tokenizer_t	*new_tokenizer(file_t *file);

tk_t		tk_next(tokenizer_t *tokenizer);
tk_t		tk_prev(tokenizer_t *tokenizer);
void		tk_rev(tokenizer_t *tokenizer, tk_t t);
void		tk_close(tokenizer_t **tokenizer);

void		tk_warning(tk_t tk, const char *fmt, ...);
void		tk_error(tpos_t p, int tklen, const char *fmt, ...);
void		tk_print_error(tk_t tk, const char *fmt, ...);

void		tk_tostring(tk_t t, char *res);
void		tk_type_tostring(int t, char *res);
void		tk_print(tk_t t);
void		tk_debug(tk_t t);

void		tk_close(tokenizer_t **tokenizer);
#endif /* _TOKENIZER_H_ */