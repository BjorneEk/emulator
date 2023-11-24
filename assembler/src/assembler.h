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
	DEF_SECTION,
	DEF_ABSOLUTE,
	DEF_DEFINED,

	EXPRESSION
};
typedef struct constexpr constexpr_t;

typedef struct def {
	int		type;
	char		*lbl;
	int		val;
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
	hashmap_t	*labels; /* def_t* */
	int		size;
	tk_t		tk;
	bool		has_raw_data;
	u32_t		begin_addr;
} section_t;



typedef struct asm_entry {

	int	type;
	u32_t	rel_addr;
	tk_t	token;
	union {
		struct {
			int		instruction;
			int		addr_mode;
			union {
				constexpr_t	*value;
				int		evaluated_value;
			};
			union {
				constexpr_t	*bit;		/* used by bbc and bbs */
				int		evaluated_bit;
			};
			int		regs[ASM_REG_COUNT];
			int		nregs;
			int		instruction_size;
			int		addr_mode_size;

		};

		struct {
			int		array_type;
			int		data_size;
			union {
				constexpr_t	*data_value;
				int		evaluated_data_value;
				dla_t		*array_values;	/* constexpr_t* */
				struct {
					int	*evaluated_array;
					int	evaluated_array_len;
				};
				char		*string;
			};
		};
	};

} asm_t;

typedef struct program {
	dla_t		*sections;	/*	section_t*		*/
	hashmap_t	*labels;	/*	(char* -> def_t*)	*/
} program_t;


void		print_constexpr(constexpr_t *ex);
void	assemble(
	fstack_t	*in_files,
	u64_t		final_addr,
	const char	*entry_point,
	const char	*interrupt_handler,
	const char	*out_name);
program_t	*parse(tokenizer_t *t);

void		program_free(program_t **p);

#endif /* _ASSEMBLER_H_ */