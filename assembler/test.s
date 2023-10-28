
.text:
ldrr:
	ldr r0, [#0xFF_FF] ; comment
	cpr sp, r0 // comment
	add sp, 'Q'
.data:
	a = """Hello, World
	line 2"""