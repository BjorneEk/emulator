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

#define MEMORY_SIZE (0xFFFFFFFF)

typedef struct memory {
	u8_t data[MEMORY_SIZE];
} memory_t;

memory_t *new_memory();

u8_t memory_read_byte(memory_t *mem, u32_t addr);

u16_t memory_read_word(memory_t *mem, u32_t addr);

u32_t memory_read_long(memory_t *mem, u32_t addr);

inline u16_t swap_u16(u16_t u)
{
	return (u << 8) | (u >> 8);
}

inline i16_t swap_i16(i16_t i)
{
	return (i << 8) | (i >> 8);
}

#endif /* _MEMORY_H_ */