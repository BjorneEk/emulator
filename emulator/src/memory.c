#include "memory.h"
#include "util/error.h"

u8_t mem_read(memory_t *mem, u32_t addr)
{
    ASSERT(0 <= addr && addr < MEMORY_SIZE);
    return mem->data[addr];
}