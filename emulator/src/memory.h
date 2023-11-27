/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * memory
 *==========================================================*/

#ifndef _MEMORY_H_
#define _MEMORY_H_


#include "../../common/util/types.h"
#include <stdio.h>

#define MEMORY_SIZE (0x100000000)
//#define MEMORY_SIZE (0xFFFFFFFF)

typedef struct memory {
	u8_t data[MEMORY_SIZE];
} memory_t;

memory_t *new_memory();

void memory_from_file(memory_t *mem, const char *filename);

u8_t memory_read_byte(memory_t *mem, u32_t addr);

void memory_write_byte(memory_t *mem, u32_t addr, u8_t data);

u16_t memory_read_word(memory_t *mem, u32_t addr);

void memory_write_word(memory_t *mem, u32_t addr, u16_t data);

u32_t memory_read_long(memory_t *mem, u32_t addr);

void memory_write_long(memory_t *mem, u32_t addr, u32_t data);

static inline u16_t swap_u16(u16_t u)
{
	return (u << 8) | (u >> 8);
}

static inline u32_t swap_u32(u32_t u)
{
	return (u << 16) | (u >> 16);
}

void memory_debug(memory_t *mem, u32_t addr, int nbr_bytes);

#endif /* _MEMORY_H_ */