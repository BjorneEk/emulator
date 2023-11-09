/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsso
 *
 * assembler
 *==========================================================*/

#include "../../common/util/types.h"
#include "assembler.h"
#include <stdio.h>

int main(int argc, const char *argv[])
{
	tokenizer_t *t;

	//tk_t tok;
	t = new_tokenizer(argv[1]);
	/*do {
		tok = tk_next(t);
		tk_print(tok);
	} while (tok.type != TK_NULL);*/
	parse(t);
	return 0;
}