/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 *==========================================================*/

#include "../arch/interface.h"


const int endian = LITTLE_ENDIAN;

const int addressing_mode_size[] = {
	[ADDR_MODE_RELATIVE]	= 1,
	[ADDR_MODE_IMMIDIATE]	= 2,
	[ADDR_MODE_REG]		= 1,
	[ADDR_MODE_ABS]		= 4,
	[ADDR_MODE_ABS_PTR]	= 1,
	[ADDR_MODE_ABS_IDX]	= 5,
	[ADDR_MODE_ABS_PTR_IDX]	= 2,
	[ADDR_MODE_ABS_PTR_OFF]	= 3,
	[ADDR_MODE_ZP_PTR]	= 1,
	[ADDR_MODE_ZP_OFF]	= 3,
	[ADDR_MODE_ZP_IDX]	= 1
};

const int instruction_size[] = {
	[INSTR_NOP]	=	1,
	[INSTR_BRK]	=	1,
	[INSTR_LDR]	=	2,
	[INSTR_LDRB]	=	2,
	[INSTR_LDRW]	=	2,
	[INSTR_STR]	=	2,
	[INSTR_STRB]	=	2,
	[INSTR_CPRP]	=	3,
	[INSTR_BZ]	=	1,
	[INSTR_BNZ]	=	1,
	[INSTR_BCC]	=	1,
	[INSTR_BCS]	=	1,
	[INSTR_BRN]	=	1,
	[INSTR_BRP]	=	1,
	[INSTR_BBS]	=	2,
	[INSTR_BBC]	=	2,
	[INSTR_BRA]	=	1,
	[INSTR_LBRA]	=	1,
	[INSTR_CALL]	=	1,
	[INSTR_RET]	=	1,
	[INSTR_RTI]	=	1,
	[INSTR_ADC]	=	2,
	[INSTR_ADD]	=	2,
	[INSTR_ADCW]	=	2,
	[INSTR_ADDW]	=	2,
	[INSTR_SBC]	=	2,
	[INSTR_SUB]	=	2,
	[INSTR_SBCW]	=	2,
	[INSTR_SUBW]	=	2,
	[INSTR_EOR]	=	2,
	[INSTR_ORR]	=	2,
	[INSTR_AND]	=	2,
	[INSTR_CMP]	=	2,
	[INSTR_ASR]	=	2,
	[INSTR_LSL]	=	2,
	[INSTR_LSR]	=	2,
	[INSTR_NOT]	=	2,
	[INSTR_DEC]	=	2,
	[INSTR_DECW]	=	2,
	[INSTR_INC]	=	2,
	[INSTR_INCW]	=	2,
	[INSTR_CRB]	=	2,
	[INSTR_SRB]	=	2,
};

const int supported_addressing_modes[INSTR_NULL][ADDR_MODE_NULL] = {
	[INSTR_NOP] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BRK] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_LDR] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_LDRB] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_LDRW] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_NULL,		ADDR_MODE_NULL
	},
	[INSTR_STR] = {
		ADDR_MODE_ZP_IDX,	ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_NULL,		ADDR_MODE_NULL
	},
	[INSTR_STRB] = {
		ADDR_MODE_ZP_IDX,	ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_NULL,		ADDR_MODE_NULL
	},
	[INSTR_CPRP] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BZ] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BNZ] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BCC] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BCS] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BRN] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BRP] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BBS] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BBC] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_BRA] = {
		ADDR_MODE_RELATIVE, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_LBRA] = {
		ADDR_MODE_ABS, ADDR_MODE_ABS_PTR,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_CALL] = {
		ADDR_MODE_ABS, ADDR_MODE_ABS_PTR,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_RET] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_RTI] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_ADC] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_ADD] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_ADCW] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_ADDW] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_SBC] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_SUB] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_SBCW] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_SUBW] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_EOR] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_ORR] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_AND] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_CMP] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_ASR] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_LSL] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_LSR] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_NOT] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_DEC] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_DECW] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_INC] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_INCW] = {
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL,
		ADDR_MODE_NULL, ADDR_MODE_NULL
	},
	[INSTR_CRB] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
	[INSTR_SRB] = {
		ADDR_MODE_IMMIDIATE,
		ADDR_MODE_REG,		ADDR_MODE_ABS,
		ADDR_MODE_ABS_PTR,	ADDR_MODE_ABS_IDX,
		ADDR_MODE_ABS_PTR_IDX,	ADDR_MODE_ABS_PTR_OFF,
		ADDR_MODE_ZP_PTR,	ADDR_MODE_ZP_OFF,
		ADDR_MODE_ZP_IDX,	ADDR_MODE_NULL
	},
};