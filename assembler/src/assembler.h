/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * assembler
 *==========================================================*/
#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#include "tokenizer.h"
#include "structures/dynamic_array.h"
#include "structures/hashmap.h"

enum def_types {
	DEF_UNDEFINED,
	DEF_TEXT,
	DEF_DATA,
	DEF_RELATIVE_TEXT,
	DEF_RELATIVE_DATA,
	DEF_RELATIVE,
	DEF_ABSOLUTE
};

typedef struct def {
	int	type;
	bool	is_global;
	char	*lbl;
	u32_t	val;
	bool	defined;
	dla_t	*refs; // constexpr_t
	tk_t	tk;
} def_t;

typedef struct constexpr {
	int type;
	int expected_type;
	union {
		int	val;
		def_t	*ref;
		dla_t	*ops; // constexpr_t
	};
	struct constexpr *parent;
} constexpr_t;

typedef struct asm_instr {
	int type;
	union {
		constexpr_t *val32;
		constexpr_t *val16;
	};
	int regs[5];
} asm_t;

typedef struct data {
	int type;
	union {
		char	*str;
		u8_t	*bytes;
		u16_t	*words;
		u32_t	*lwords;
	};
} data_t;

typedef struct program {
	def_t *text_section;
	def_t *data_section;
	hashmap_t *undef_lbls;
	hashmap_t *lbls;
	dla_t	*global_export_defs;
	dla_t	*instructions;
} program_t;

#endif /* _ASSEMBLER_H_ */