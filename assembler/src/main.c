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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char *DEFAULT_OUTPUT_NAME = "out.bin";

#define ENTRY_POINT(_e)		(!strcmp((_e), "-e")	|| !strcmp((_e), "-entry-point"))
#define INTERRUPT_HANDLER(_e)	(!strcmp((_e), "-i")	|| !strcmp((_e), "-interrupt-handler"))
#define LAST_ADDR(_e)		(!strcmp((_e), "-sz")	|| !strcmp((_e), "-size"))
#define OUT_FILE(_e)		(!strcmp((_e), "-o")	|| !strcmp((_e), "-output"))
#define IS_HELP(_e)		(!strcmp((_e), "-h")	|| !strcmp((_e), "-help"))
file_t *alloc_file_copy(file_t *f)
{
	file_t *res;

	res = malloc(sizeof(file_t));
	memmove(res, f, sizeof(file_t));
	return res;
}

void	help()
{
	printf("	'\033[1m-e\033[0m'	<label>		 '\033[1m-entry-point\033[0m'	<label>\n");
	printf("	'\033[1m-i\033[0m'	<label>		 '\033[1m-interrupt-handler\033[0m'	<label>\n");
	printf("	'\033[1m-sz\033[0m'	<size>		 '\033[1m-size\033[0m'	<size>\n");
	printf("	'\033[1m-o\033[0m'	<filename>	 '\033[1m-output\033[0m'	<filename>\n");
	printf("	'\033[1m-h\033[0m'			 '\033[1m-help\033[0m'\n");
	exit(0);
}

int main(int argc, const char *argv[])
{
	int i;
	int filecount;
	file_t file;
	file_t *fptr;
	fstack_t *files = NULL;
	fstack_t *files_ = NULL;

	u64_t		final_addr		= 0;
	const char	*entry_point		= NULL;
	const char	*interrupt_handler	= NULL;
	const char	*out_name		= NULL;


	filecount = 0;

	for(i = 1; i < argc; i++) {
		if (IS_HELP(argv[i]))
			help();
		if (ENTRY_POINT(argv[i])) {
			++i;
			if (argc > i && entry_point != NULL)
				exit_error(
					"multiple entry points specified: %s, and %s\n",
					entry_point,
					argv[i]);
			else if (argc <= i)
				exit_error(
					"no entry point specified after '%s'\n",
					argv[i-1]);
			entry_point = argv[i];
			continue;
		}
		if (INTERRUPT_HANDLER(argv[i])) {
			++i;
			if (argc > i && interrupt_handler != NULL)
				exit_error(
					"multiple interrupt handlers specified: %s, and %s\n",
					interrupt_handler,
					argv[i]);
			else if (argc <= i)
				exit_error(
					"no interrupt handler specified after '%s'\n",
					argv[i-1]);
			interrupt_handler = argv[i];
			continue;
		}
		if (LAST_ADDR(argv[i])) {
			++i;
			if (argc > i && final_addr != 0)
				exit_error(
					"multiple output memory sizes specified: %i, and %i\n",
					final_addr,
					strtoull(argv[i], NULL, 0));
			else if (argc <= i)
				exit_error(
					"no memory size specified after '%s'\n",
					argv[i-1]);
			final_addr = strtoull(argv[i], NULL, 0);
			continue;
		}
		if (OUT_FILE(argv[i])) {
			++i;
			if (argc > i && out_name != NULL)
				exit_error(
					"multiple output files specified: %s, and %s\n",
					out_name,
					argv[i]);
			else if (argc <= i)
				exit_error(
					"no output file specified after '%s'\n",
					argv[i-1]);
			out_name = argv[i];
			continue;
		}
		open_file(&file, argv[i], MODE_READ);
		if (file.extension == EXT_ASM_FILE) {
			fptr = alloc_file_copy(&file);
			push_file(&files, fptr);
			push_file(&files_, fptr);
			++filecount;
		} else {
			error("unsuported input file format: %s", file.filename);
		}
	}
	if (entry_point == NULL)
		exit_error("no entry point specified, use '\033[1m-e\033[0m <label>'\n");
	if (interrupt_handler == NULL)
		exit_error("no interrupt handler specified, use '\033[1m-i\033[0m <label>'\n");
	if (final_addr == 0)
		exit_error("no memory size specified, use '\033[1m-sz\033[0m <size>'\n");
	if (out_name == NULL)
		out_name = DEFAULT_OUTPUT_NAME;

	printf("output file: %s\n", out_name);
	printf("entry point: %s\n", entry_point);
	printf("interrupt handler: %s\n", interrupt_handler);
	printf("memory size: %llu\n", final_addr);

	assemble(files, final_addr, entry_point, interrupt_handler, out_name);

	for(i = 1; i < filecount; i++) {
		fptr = pop_file(&files_);
		close_file(fptr);
		free(fptr);
	}
	return 0;
}