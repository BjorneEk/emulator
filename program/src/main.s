


.text:

OUT = #0
INIT_STACK = #0xFFFF

PORTA	= #0x00000001
DDRA	= #0x00000003
PORTB	= #0x00000005
DDRB	= #0x00000007

ENABLE_INTERRUPT = #0
KEYPRESS_INTERRUPT = #3

VC_COMM		= 0x00000009
VC_DATA		= 0x00000010
VC_ADDRESS	= 0x00000013

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
	sub	r1, r1, 'q'
	bz	exit_program
	call	[debug_char]
	ret

exit_program:
	brk


; r0: red, r1: green, r2: blue, (r3, r4): addr
vc_set_pixel:
	sub	sp, sp, #4
	str	r5, [sp + #2]
	str	r6, [sp]

	ldr	r5, VC_COMM
	strb	r0, [r5 + #1] ; VC_DATA = VC_COMM + 1
	strb	r1, [r5 + #2]
	strb	r2, [r5 + #3]
	str	r3, [r5 + #4] ; VC_DATA + 3 = VC_ADDRESS
	str	r4, [r5 + #6]

	ldr	r6, VC_WRITE_RGB ; send signal
	str	r6, [r5]

	ldr	r6, [sp]
	ldr	r5, [sp + #2]
	add	sp, sp, #4
	ret
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

; r0: is_black ; (r3, r4): addr, r5: idx
draw_bw:
	sub	sp, sp, #10
	str	r6, [sp + #6]
	str	r4, [sp + #6]
	str	r3, [sp + #4]
	str	r2, [sp + #2]
	str	r1, [sp]

	addw	r3, r4, r5	; indexed address

	sub	r6, r0, #0
	bz	draw_white
	ldr	r0, #0
	ldr	r1, #0
	ldr	r2, #0
	ldr	r0, #0
	bra	draw_bw_ret
draw_white:
	ldr	r0, #0xFFFF
	ldr	r1, #0xFFFF
	ldr	r2, #0xFFFF
	ldr	r0, #1
draw_bw_ret:
	call	[vc_set_pixel]

	ldr	r1, [sp]
	ldr	r2, [sp + #2]
	str	r3, [sp + #4]
	str	r4, [sp + #6]
	str	r6, [sp + #8]
	add	sp, sp, #10
	ret

; r0, first (r3, r4): addr start
draw_line:
	sub	sp, sp, #4
	str	r0, [sp + #2]
	str	r5, [sp]

	ldr	r5, 0
draw_line_loop:
	sub	r5, SCREEN_WIDTH
	bz	draw_line_ret
	call	[draw_bw]
	incw	r5
	bra	draw_line_loop
draw_line_ret:

	ldr	r5, [sp]
	ldr	r0, [sp + #2]
	add	sp, sp, #4
	ret

draw_chess_pattern:
	sub	sp, sp, #10
	str	r4, [sp + #8]
	str	r3, [sp + #6]
	str	r2, [sp + #4]
	str	r1, [sp + #2]
	str	r0, [sp]

	ldr	r0, #0
	ldr	r1, #0
	ldr	r3, #0
	ldr	r4, #0

draw_chess_pattern_loop:
	sub	r2, r1, SCREEN_HEIGHT
	bz	draw_chess_pattern_ret
	sub	r2, r0, #0
	bz	wfirst
	ldr	r0, #0
	call	[draw_line]
	inc	r1
	bra	draw_chess_pattern_loop
wfirst:
	ldr	r0, #1
	call	[draw_line]
	inc	r1
	bra	draw_chess_pattern_loop

draw_chess_pattern_ret:
	ldr	r0, [sp]
	ldr	r1, [sp + #2]
	ldr	r2, [sp + #4]
	ldr	r3, [sp + #6]
	ldr	r4, [sp + #8]
	add	sp, sp, #10
	ret

; program entry point
start:
	ldr	sp, INIT_STACK ; initialize stack-pointer

	ldr	r0, [resolution]
	call	[vc_set_def_resolution]

	call	[draw_chess_pattern]

	srb	ps, #13
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
	u16	resolution = VC_RESOLUTION_640X480
