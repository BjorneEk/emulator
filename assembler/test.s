.global: a, b, x
.text:
my_label = 1 - 1 + ldrr
ldrr:
	ldr r0, [#0xFF_FF] ; comment
	cpr sp, r0 // comment
	bbc sp, 'Q'
.data:
	a = """Hello, World
	line 2"""