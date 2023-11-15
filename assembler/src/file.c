/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * file
 *==========================================================*/
#include "file.h"
#include "../../common/util/error.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


const char * const mode_map[3] = {
	[MODE_READ] = "r",
	[MODE_WRITE] = "w",
	[MODE_READWRITE] = "rw"
};

static int get_fileext(const char *str)
{
	if (*str != '.')
		return EXT_BINARY;

	if (str[1] == 'S' || str[1] == 's')
		return EXT_ASM_FILE;

	if ((str[1] == 'l' || str[1] == 'L') && (str[2] == 'd' || str[2] == 'D'))
		return EXT_LINKER_SCRIPT;

	warn_custom("ASSEMBLER", "unsuported extension '%s', defaulting to .S (assembly file)", str);
	return EXT_ASM_FILE;
}

void	open_file(file_t *res, const char *filename, int mode)
{
	int i;
	int len;
	res->mode = mode;
	res->filename = filename;
	res->fp	= fopen(filename, mode_map[mode]);

	if (!res->fp)
		exit_error_custom("ASSEMBLER", "could not open file: %s", filename);
	len = strlen(filename);
	for (i = len; i >= 0 && filename[i] != '.'; i--)
		;


	res->extension = get_fileext(filename + i);
	return;
}
static FILE *tmp(const char *restrict fmt, va_list args)
{
	FILE	*res;

	res = tmpfile();
	vfprintf(res, fmt, args);
	rewind(res);
	return res;
}

void	ftemp_with(file_t *res, int ext, const char *restrict fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	res->mode = MODE_READWRITE;
	res->extension = ext;
	res->filename = "temporary-file";
	res->fp = tmp(fmt, args);
}

int	fnext(file_t *f)
{
	return fgetc(f->fp);
}

void fpb(file_t *f, int c)
{
	ungetc(c, f->fp);
}

void	close_file(file_t *f)
{
	fclose(f->fp);
}