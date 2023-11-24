

.text:
OUT = #0
INIT_STACK = #0xFFFF
; r0: ptr-msw
; r1: ptr-lsw
puts:
	sub	sp, sp, #2
	str	r2, [sp]
puts_loop:
	ldrb	r2, [r0, r1]
	sub	r2, r2, '\0'
	bz	puts_ret
	strb	r2, [OUT]
	incw	r0, r1
	bra	puts_loop
puts_ret:
	ldrb	r2, '\n'
	strb	r2, [OUT]

	ldr	r2, [sp]
	add	sp, sp, #2
	ret
main:
	ldr	sp, INIT_STACK ; initialize stack-pointer

	; call puts to print greeting
	ldr	r0, (greeting & #0xFFFF0000) >> #16
	ldr	r1, greeting & #0xFFFF
	call	[puts]
loop:
	bra	loop

interrupt:
	rti

.data:
	string greeting = "Hello, World!"