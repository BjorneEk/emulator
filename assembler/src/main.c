/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsso
 *
 * assembler
 *==========================================================*/

#include "util/types.h"
#include "tokenizer.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	tokenizer_t *t;
	tk_t tok;
	t = new_tokenizer(argv[1]);
	do {
		tok = tk_next(t);
		tk_print(tok);
	} while (tok.type != TK_NULL);
	return 0;
}