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
	file_t file;
	//tk_t tok;
	open_file(&file, argv[1], MODE_READ);
	t = new_tokenizer(&file);
	/*do {
		tok = tk_next(t);
		tk_print(tok);
	} while (tok.type != TK_NULL);*/
	parse(t);
	return 0;
}