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
	[MODE_WRITE] = "wb",
	[MODE_READWRITE] = "rw"
};

static int get_fileext(const char *str, int mode)
{
	if (*str != '.')
		return EXT_BINARY;

	if (str[1] == 'S' || str[1] == 's')
		return EXT_ASM_FILE;

	if ((str[1] == 'b' || str[1] == 'B') && (str[2] == 'i' || str[2] == 'I') && (str[3] == 'n' || str[3] == 'N'))
		return EXT_BINARY;

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


	res->extension = get_fileext(filename + i, mode);
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
__attribute__((format(printf, 3, 4)))
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

void fw_u8(file_t *f, u8_t v)
{
	if (fputc(v, f->fp) == EOF)
		error("Error writing u8 to the file: %s\n", f->filename);
}
void fw_u16(file_t *f, u16_t v, bool little_endian)
{
	if (!little_endian) {
		fw_u8(f, (v & 0xFF00) >> 8);
		fw_u8(f, (v & 0x00FF));
	} else {
		fw_u8(f, (v & 0x00FF));
		fw_u8(f, (v & 0xFF00) >> 8);
	}
}
void fw_u32(file_t *f, u32_t v, bool little_endian)
{
	if (!little_endian) {
		fw_u16(f, (v & 0xFFFF0000) >> 16, little_endian);
		fw_u16(f, (v & 0x0000FFFF), little_endian);
	} else {
		fw_u16(f, (v & 0x0000FFFF), little_endian);
		fw_u16(f, (v & 0xFFFF0000) >> 16, little_endian);
	}
}

void fw_string(file_t *f, char *s)
{
	while (*(s++) != '\0')
		fw_u8(f, s[-1]);
}
void fw_string_len(file_t *f, char *s, int len)
{
	int i;

	for (i = 0; i < len; i++)
		fw_u8(f, s[i]);
	fw_u8(f, '\0');
}

void	close_file(file_t *f)
{
	fclose(f->fp);
}