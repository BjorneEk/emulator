.global: global_func1, global_func2, global_func3

.text:

global_func1:
my_label = #1 - #0b0011 + global_func1 * (global_func3 + #-10)

global_func3: /*
	here is a multiline comment
 */

	ldr r0, [#0xFF_FF]
	cprp r1, r0, r3, r2 // comment
	bbc sp, 'Q'
.data:
	string a = """Hello, World
	line 2"""