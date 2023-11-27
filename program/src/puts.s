.text:

OUT = #0
INIT_STACK = #0xFFFF

PORTA	= #0x00000001
DDRA	= #0x00000003
PORTB	= #0x00000005
DDRB	= #0x00000007

ENABLE_INTERRUPT = #0
KEYPRESS_INTERRUPT = #3

STRING_START = #0xFFFF6969

enable_interrupt_controller:
	sub	sp, sp, #2
	str	r0, [sp]

	ldr	r0, #0xFFFF
	str	r0, [DDRA]		; set port a as output (not neccesary)

	ldr	r0, ENABLE_INTERRUPT
	str	r0, [PORTA]		; enable interrupts

	ldr	r0, #0
	str	r0, [DDRA]		; set port a as input

	ldr	r0, [sp]
	add	sp, sp, #2
	ret

print_from_string_start:
	sub	sp, sp, #6
	str	r9, [sp + #4]
	str	r1, [sp + #2]
	str	r0, [sp]

	ldr	r1, #0

print_from_string_loop:
	ldrb	r0, [STRING_START], r1
	sub	r0, r0, #0
	bz	print_from_string_start_return
	strb	r0, [#0]
	inc	r1
	bra	print_from_string_loop

print_from_string_start_return:
	strb	r9, [OUT]	
	
	strb	r9, [STRING_START], r1
	inc	r1
	ldr	r9, #0
	strb	r9, [STRING_START], r1

	ldrb	r9, #10
	strb	r9, [OUT]	

	ldr	r0, [sp]
	ldr	r1, [sp + #2]
	ldr	r9, [sp + #4]
	add	sp, sp, #6
	ret

start:
	ldr	sp, INIT_STACK			; initialize stack-pointer

	srb	ps, #13				; enable cpu hw interrupts
	ldr	r0, #0
	str	r0, [DDRB]			; set ddrb and ddra as input
	str	r0, [DDRA]			; set ddrb as input

	str	r0, [STRING_START]

	call	[enable_interrupt_controller]	; enable io-board interrupt controller
end:
	bra end

interrupt:
	sub	sp, sp, #6
	str	r9, [sp + #4]
	str	r1, [sp + #2]
	str	r0, [sp]

	ldrb	r0, [PORTB + #1]
	sub	r1, r0, #10
	bz	interrupt_return

	; strb	r0, [OUT]
	ldr	r9, r0
	call	[print_from_string_start]

interrupt_return:
	ldr	r0, [sp]
	ldr	r1, [sp + #2]
	ldr	r9, [sp + #4]
	add	sp, sp, #6
	rti
