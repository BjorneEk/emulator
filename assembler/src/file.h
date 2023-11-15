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


void	open_file(file_t *res, const char *filename, int mode);

int	fnext(file_t *f);

void	fpb(file_t *f, int c);

void	close_file(file_t *f);

#endif /* _FILE_H_ */