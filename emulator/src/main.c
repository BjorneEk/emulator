/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * emulator
 *==========================================================*/

#include "emulator.h"
#include "../../instructions/interface.h"
#include "../../common/util/types.h"
#include <stdio.h>

extern void print_instruction_(int);
extern void print_instruction(int);
#define _(name) print_instruction(INSTR_##name);
#define __(name) print_instruction_(INSTR_##name);
int main(int argc, char *argv[])
{
	XMACRO_INSTRUCTIONS(_)
}