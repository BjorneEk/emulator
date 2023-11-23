/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsso
 *
 * assembler
 *==========================================================*/

#include "../../common/util/types.h"
#include "../../common/util/error.h"
#include "assembler.h"
#include "file.h"
#include <stdio.h>
#include <string.h>

file_t *alloc_file_copy(file_t *f)
{
	file_t *res;

	res = malloc(sizeof(file_t));
	memmove(res, f, sizeof(file_t));
	return res;
}
int main(int argc, const char *argv[])
{
	int i;
	int filecount;
	tokenizer_t *t;
	file_t file;
	bool has_linker_script;
	file_t linker_script;
	file_t *fptr;
	fstack_t *files = NULL;
	fstack_t *files_ = NULL;


	filecount = 0;

	for(i = 1; i < argc; i++) {
		open_file(&file, argv[i], MODE_READ);
		if (file.extension == EXT_LINKER_SCRIPT && !has_linker_script) {
			has_linker_script = true;
			memmove(&linker_script, &file, sizeof(file_t));
		} else if (file.extension == EXT_LINKER_SCRIPT) {
			error("multiple liker-scripts suplied: %s, but had: %s already", file.filename, linker_script.filename);
		} else if (file.extension == EXT_ASM_FILE) {
			fptr = alloc_file_copy(&file);
			push_file(&files, fptr);
			push_file(&files_, fptr);
			++filecount;
		} else {
			error("unsuported input file format: %s", file.filename);
		}
	}

	//ASSERT_(has_linker_script, "no linker-script found, supplied")


	t = new_tokenizer(&files);

	for(i = 1; i < filecount; i++) {
		fptr = pop_file(&files_);
		close_file(fptr);
		free(fptr);
	}
	parse(t);
	return 0;
}