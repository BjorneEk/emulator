


.text:

OUT = #0
INIT_STACK = #0xFFFF

PORTA	= #0x00000001
DDRA	= #0x00000003
PORTB	= #0x00000005
DDRB	= #0x00000007

ENABLE_INTERRUPT = #0
KEYPRESS_INTERRUPT = #3


; r0: char
debug_char:
	sub	sp, sp, #2
	str	r1, [sp]

	ldr	r1, #10 ;'\n'
	str	r1, [OUT]
	str	r0, [OUT]
	str	r1, [OUT]
	ldr	r1, [sp]
	add	sp, sp, #2
	ret

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

; r0 = interrupt type
; r1 = scancode
debug_keypress_handler:
	ldr	r0, r1
	call	[debug_char]
	ret


; program entry point
start:
	ldr	sp, INIT_STACK ; initialize stack-pointer

	ldr	r0, #0
	str	r0, [DDRB]			; set ddrb and ddra as input
	str	r0, [DDRA]			; set ddrb as input

	call	[enable_interrupt_controller]	; enable io-board interrupt controller

	ldr	r0, (debug_keypress_handler & #0xFFFF0000) >> #16
	ldr	r1, debug_keypress_handler & #0xFFFF

	str	r0, [keypress_handler_hb]
	str	r1, [keypress_handler_lb]	; set keypress handler
	; setup interrupts
loop:
	bra	loop

; r0 = interrupt type
; r1 = scancode
keyboard_event:
	sub	sp, sp, #4
	str	r2, [sp + #2]
	str	r3, [sp]

	ldr	r2, [keypress_handler_hb]
	ldr	r3, [keypress_handler_lb]
	call	[r2, r3]

	ldr	r3, [sp]
	ldr	r2, [sp + #2]
	add	sp, sp, #4
	ret

; program hardware interrupt handler
interrupt:
	sub	sp, sp, #4
	str	r0, [sp + #2]
	str	r1, [sp]

	ldr	r0, [PORTA]
	sub	r0, r0, KEYPRESS_INTERRUPT
	bz	keypress
	bra	interrupt_return
keypress:
	ldr	r1, [PORTB]
	call	[keyboard_event]
interrupt_return:
	ldr	r1, [sp]
	ldr	r0, [sp + #2]
	add	sp, sp, #4
	rti


.data:
	u16	keypress_handler_hb = #0
	u16	keypress_handler_lb = #0
