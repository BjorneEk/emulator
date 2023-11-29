.text:

OUT = #0
INIT_STACK = #0xFFFF

PORTA	= #0x00000001
DDRA	= #0x00000003
PORTB	= #0x00000005
DDRB	= #0x00000007

ENABLE_INTERRUPT = #0
KEYBOARD_INTERRUPT = #4
CLEAR_INTERRUPT	= #2

KEY_PRESSED = #1
KEY_RELEASED = #0

VC_COMM		= #0x00000009
VC_DATA		= #0x00000010
VC_ADDRESS	= #0x00000013

VC_WRITE_RGB		= #0
VC_READ_RGB		= #1
VC_SWAP_BUFFERS		= #2
VC_SET_RESOLUTION	= #3
VC_RESOLUTION_800X600	= #4
VC_RESOLUTION_640X480	= #5

SCREEN_WIDTH = #640
SCREEN_HEIGHT = #480


; r0: char
debug_char:
	sub	sp, sp, #2
	str	r1, [sp]

	ldr	r1, #10 ;'\n'
	;str	r1, [OUT]
	strb	r0, [OUT]
	strb	r1, [OUT]
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
	sub	sp, sp, #6
	str	r0, [sp + #4]
	str	r2, [sp + #2]
	str	r1, [sp]

	sub	r0, r0, KEY_PRESSED
	bnz	khret

	ldr	r0, r1
	sub	r1, r1, 'Q'
	bz	exit_program

	call	[debug_char]

	sub	r0, r0, 'R'
	bz	red

	ldr	r0, [sp]
	sub	r0, r0, 'G'
	bz	green

	ldr	r0, [sp]
	sub	r0, r0, 'B'
	bz	blue

	bra	khret
red:
	ldr	r0, #255
	ldr	r1, #0
	ldr	r2, #0
	call	[fill_screen]
	bra	khret
green:
	ldr	r0, #0
	ldr	r1, #255
	ldr	r2, #0
	call	[fill_screen]
	bra	khret
blue:
	ldr	r0, #0
	ldr	r1, #0
	ldr	r2, #255
	call	[fill_screen]
	bra	khret
khret:
	ldr	r1, [sp]
	ldr	r2, [sp + #2]
	ldr	r0, [sp + #4]
	add	sp, sp, #6
	ret

exit_program:
	brk

; r0 resolution
vc_set_def_resolution:
	sub	sp, sp, #4
	str	r0, [sp + #2]
	str	r1, [sp]

	ldr	r1, VC_COMM

	ldr	r0, VC_SET_RESOLUTION
	strb	r0, [r1]
	ldr	r0, [sp + #2]
	str	r0, [r1]

	ldr	r1, [sp]
	add	sp, sp, #4

fill_screen:
	sub	sp, sp, #14
	str	r6, [sp + #12]
	str	r5, [sp + #10]
	str	r4, [sp + #8]
	str	r3, [sp + #6]
	str	r2, [sp + #4]
	str	r1, [sp + #2]
	str	r0, [sp]

	;ldrb	r0, #0
	;ldrb	r1, #255
	ldr	r3, r2
	ldr	r2, VC_COMM
	strb	r0, [r2 + #1] ; VC_DATA = VC_COMM + 1
	strb	r1, [r2 + #2]
	strb	r3, [r2 + #3]

	ldr	r3, #0
	ldr	r4, #0

	ldr	r5, #0
	ldr	r6, #0

	ldr	r1,	VC_WRITE_RGB
loop1:
	sub	r0, r3, SCREEN_WIDTH
	bz	loop2

	str	r5, [r2 + #4]
	str	r6, [r2 + #6]
	strb	r1, [r2]
	incw	r5, r6

	inc	r3
	bra	loop1
loop2:
	sub	r0, r4, SCREEN_HEIGHT
	bz	fill_screen_ret

	inc	r4
	ldr	r3, #0
	bra	loop1

fill_screen_ret:
	ldr	r1, VC_SWAP_BUFFERS
	str	r1, [r2]

	ldr	r6, [sp + #12]
	ldr	r5, [sp + #10]
	ldr	r4, [sp + #8]
	ldr	r3, [sp + #6]
	ldr	r2, [sp + #4]
	ldr	r1, [sp + #2]
	ldr	r0, [sp]
	add	sp, sp, #14
	ret
; program entry point
start:
	ldr	sp, INIT_STACK ; initialize stack-pointer

	ldr	r0, [resolution]
	call	[vc_set_def_resolution]

	ldr	r0, #0
	ldr	r1, #0
	ldr	r2, #255
	call	[fill_screen]

	srb	ps, #13				; enable cpu hw interrupts

	ldr	r0, #0
	str	r0, [DDRB]			; set ddrb and ddra as input
	str	r0, [DDRA]			; set ddrb as input

	call	[enable_interrupt_controller]	; enable io-board interrupt controller

	;ldr	r0, (debug_keypress_handler & #0xFFFF0000) >> #16
	;ldr	r1, debug_keypress_handler & #0xFFFF

	;str	r0, [keypress_handler_hb]
	;str	r1, [keypress_handler_lb]	; set keypress handler
	; setup interrupts
loop:
	bra	loop

; r0 = interrupt type
; r1 = key scancode
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
	sub	sp, sp, #6
	str	r0, [sp + #4]
	str	r1, [sp + #2]
	str	r2, [sp]

	ldrb	r2, [PORTA]		; interrupt type
	ldrb	r0, [PORTA + #1]	; interrupt info

	sub	r2, r2, KEYBOARD_INTERRUPT
	bz	keypress

	bra	interrupt_return
keypress:
	ldr	r1, [PORTB]		; key info
	call	[keyboard_event]

interrupt_return:
	ldr	r0, CLEAR_INTERRUPT << #8
	str	r0, [PORTA]

	ldr	r2, [sp]
	ldr	r1, [sp + #2]
	ldr	r0, [sp + #4]
	add	sp, sp, #6
	rti


.data:
	u16	keypress_handler_hb = (debug_keypress_handler & #0xFFFF0000) >> #16
	u16	keypress_handler_lb = debug_keypress_handler & #0xFFFF
	u16	resolution = VC_RESOLUTION_640X480
