
.text:
ldrr:
	ldr r0, [#0xFFFF] ; comment
	cpr sp, r0 // comment
	add sp, 'Q'
.data:
	a = "Hello, World\n"