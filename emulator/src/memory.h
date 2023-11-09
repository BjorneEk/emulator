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

u8_t mem_read(memory_t *mem, u32_t addr);

inline u16_t swap_u16(u16_t u)
{
	return (u << 8) | (u >> 8);
}

inline i16_t swap_i16(i16_t u)
{
	return (u << 8) | (u >> 8);
}

#endif /* _MEMORY_H_ */