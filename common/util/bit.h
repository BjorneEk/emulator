/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * bit and endianesse related
 *
 *==========================================================*/

#ifndef _BIT_H_
#define _BIT_H_

#include "types.h"
#include "../../arch/interface.h"

static inline int set_bit(int val, int bit, int set)
{
	return (set) ? val | 1 << bit : val & ~(1 << bit);
}

static inline u8_t	msb_16(u16_t val)
{
	return (endian == BIG_ENDIAN) ? val >> 8 : val;
}

static inline u8_t	lsb_16(u16_t val)
{
	return (endian == LITTLE_ENDIAN) ? val >> 8 : val;
}

static inline u16_t	msw_32(u32_t val)
{
	return (endian == BIG_ENDIAN) ? val >> 16 : val;
}

static inline u16_t	lsw_32(u32_t val)
{
	return (endian == LITTLE_ENDIAN) ? val >> 16 : val;
}

static inline u8_t	msb_32(u32_t val)
{
	return msb_16(msw_32(val));
}

static inline u8_t	lsb_32(u32_t val)
{
	return lsb_16(lsw_32(val));
}

static inline u16_t swap_16(u16_t val)
{
	return val << 8 | val >> 8;
}
static inline u32_t swap_32(u32_t val)
{
	return swap_16(lsw_32(val)) << 16 | swap_16(msw_32(val));
}
#endif /* _BIT_H_ */