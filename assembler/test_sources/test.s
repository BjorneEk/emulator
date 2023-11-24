

.text:

ABSOLUTE_LABEL = #10 * #0xFFFFFFFF + #5

global_func1:
my_label = #1 - #0b0011 + ABSOLUTE_LABEL * (ABSOLUTE_LABEL + #-10) ; 999973

global_func3: /*
	here is a multiline comment
 */

	ldr r0, [#0xFF_FF]	; comment
	cprp r1, r0, r3, r2	// comment
	bbc #4, r5, #-2
.data:
	string a = """Hello, World
 line 2"""