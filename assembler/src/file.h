/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * file
 *==========================================================*/

#ifndef _FILE_H_
#define _FILE_H_

#include "../../common/util/types.h"
#include "../../common/structures/codegen_node_structures.h"
#include <stdio.h>

enum file_extensions {
	EXT_ASM_FILE,
	EXT_LINKER_SCRIPT,
	EXT_BINARY
};

enum file_mode {
	MODE_READ,
	MODE_WRITE,
	MODE_READWRITE
};

typedef struct file {
	FILE		*fp;
	const char	*filename;
	int		extension;
	int		mode;
} file_t;


CODEGEN_STATIC_STACK(file_t *, fstack_t, _file)

void	open_file(file_t *res, const char *filename, int mode);

__attribute__((format(printf, 3, 4)))
void	ftemp_with(file_t *res, int ext, const char *fmt, ...);

int	fnext(file_t *f);

void	fpb(file_t *f, int c);

void	close_file(file_t *f);

void	fw_u8(file_t *f, u8_t v);
void	fw_u16(file_t *f, u16_t v, bool little_endian);
void	fw_u32(file_t *f, u32_t v, bool little_endian);
void	fw_string(file_t *f, char *str);
void	fw_string_len(file_t *f, char *s, int len);

#endif /* _FILE_H_ */