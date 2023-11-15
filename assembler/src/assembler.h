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
#include "../../common/structures/dynamic_array.h"
#include "../../common/structures/hashmap.h"

#define ASM_REG_COUNT (5)

enum parser_types {
	DATA_BYTE = SINSTR_NULL + 1,
	DATA_WORD,
	DATA_LONG,
	DATA_ARRAY,
	DATA_STRING,

	DEF_UNDEFINED = TK_LAST,
	DEF_UNDEFINED_REF,
	DEF_SECTION,
	DEF_ABSOLUTE,
	DEF_DEFINED,

	EXPRESSION
};
typedef struct constexpr constexpr_t;

typedef struct def {
	int		type;
	bool		is_global;
	char		*lbl;
	constexpr_t	*val;
	bool		ready;
	dla_t		*refs; /* constexpr_t* */
	tk_t		tk;
} def_t;



typedef struct constexpr {
	int type;
	tk_t token;

	union {
		int		val;
		def_t		*ref;
		constexpr_t	*expr;
		dla_t		*ops; /* constexpr_t* */
	};
} constexpr_t;

typedef struct section {
	dla_t		*data;	/* asm_t* */
	hashmap_t	*labels;
	int		size;
	tk_t		tk;
	bool		has_raw_data;
} section_t;



typedef struct asm_entry {

	int	type;
	int	rel_addr;
	tk_t	token;

	union {
		struct {
			int		instruction;
			int		addr_mode;
			constexpr_t	*value;
			constexpr_t	*bit;			/* used by bbc and bbs */
			int		regs[ASM_REG_COUNT];
			int		nregs;
			int		instruction_size;
			int		addr_mode_size;

		};

		union {
			int		array_type;
			int		data_size;
			constexpr_t	*data_value;
			dla_t		*array_values;	/* constexpr_t* */
			char		*string;
		};
	};

} asm_t;

typedef struct program {
	dla_t		*global_export_defs;	/* def_t*		*/
	hashmap_t	*sections;		/* char*->section_t*	*/
	hashmap_t	*labels;		/* char*->def_t*	*/
} program_t;


void		print_constexpr(constexpr_t *ex);
program_t	*parse(tokenizer_t *t);

#endif /* _ASSEMBLER_H_ */