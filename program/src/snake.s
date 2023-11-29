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

KEY_RIGHT	= (#262)
KEY_LEFT	= (#263)
KEY_DOWN	= (#264)
KEY_UP		= (#265)

SNAKE_DIR_UP	= #0
SNAKE_DIR_DOWN	= #1
SNAKE_DIR_LEFT	= #2
SNAKE_DIR_RIGHT	= #3

BLOCK_SIZE	= #16

; r0 = scancode
debug_char:
	sub	sp, sp, #2
	str	r1, [sp]

	strb	r0, [OUT]	
	ldrb	r1, #10
	strb	r1, [OUT]

	ldr	r1, [sp]
	add	sp, sp, #2
	ret

; r0 = interrupt type
; r1 = scancode
keypress_handler:
	sub	sp, sp, #4
	str	r0, [sp + #2]
	str	r1, [sp]

	sub	r0, r0, KEY_PRESSED
	bz	kh_not_return
	ldr	r1, [sp]
	ldr	r0, [sp + #2]
	add	sp, sp, #4
	ret
	
kh_not_return:

	ldr	r0, r1
	call	[debug_char]

	sub	r0, r0, 'W'
	bz	kh_snake_up
	ldr	r0, [sp]
	sub	r0, r0, KEY_UP
	bz	kh_snake_up

	ldr	r0, [sp]
	sub	r0, r0, 'S'
	bz	kh_snake_down
	ldr	r0, [sp]
	sub	r0, r0, KEY_DOWN
	bz	kh_snake_down

	ldr	r0, [sp]
	sub	r0, r0, 'A'
	bz	kh_snake_left
	ldr	r0, [sp]
	sub	r0, r0, KEY_LEFT
	bz	kh_snake_left

	ldr	r0, [sp]
	sub	r0, r0, 'D'
	bz	kh_snake_right
	ldr	r0, [sp]
	sub	r0, r0, KEY_RIGHT
	bz	kh_snake_right

	bra	kh_return

kh_snake_up:
	ldrb	r0, SNAKE_DIR_UP
	strb	r0, [snake_dir]
	bra	kh_return

kh_snake_down:
	ldrb	r0, SNAKE_DIR_DOWN
	strb	r0, [snake_dir]
	bra	kh_return

kh_snake_left:
	ldrb	r0, SNAKE_DIR_LEFT
	strb	r0, [snake_dir]
	bra	kh_return

kh_snake_right:
	ldrb	r0, SNAKE_DIR_RIGHT
	strb	r0, [snake_dir]
	bra	kh_return

kh_return:
	ldr	r1, [sp]
	ldr	r0, [sp + #2]
	add	sp, sp, #4
	ret

; in
; r0 = x
; r1 = y
; out
; r0 = address high
; r1 = address low
calculate_address:
	sub	sp, sp, #6
	str	r2, [sp + #4]
	str	r3, [sp + #2]
	str	r4, [sp]

	ldr	r2, #0
	ldr	r3, #0

	sub r1, r1, #0	
	bz ca_return
ca_loop1:
	dec	r1	
	bz	ca_return
	adcw	r2, r3, SCREEN_WIDTH
	bra ca_loop1
ca_return:
	adcw	r2, r3, r0

	ldr	r0, r2
	ldr	r1, r3

	ldr	r4, [sp]	
	ldr	r3, [sp + #2]	
	ldr	r2, [sp + #4]	
	add	sp, sp, #6
	ret

; r0 = x
; r1 = y
draw_block:
	sub	sp, sp, #16
	str	r0, [sp + #14]
	str	r1, [sp + #12]
	str	r2, [sp + #10]
	str	r3, [sp + #8]
	str	r4, [sp + #6]
	str	r5, [sp + #4]
	str	r6, [sp + #2]
	str	r7, [sp]

	ldrb	r0, #255
	ldrb	r1, #255
	ldrb	r2, #255

	ldr	r3, VC_COMM

	strb	r0, [r3 + #1]	;r
	strb	r1, [r3 + #2]	;g
	strb	r2, [r3 + #3]	;b

	ldr	r0, [sp + #14]
	ldr	r1, [sp + #12]

	str	r0, [OUT]
	str	r1, [OUT]

	call	[calculate_address]

	str	r0, [OUT]
	str	r1, [OUT]

	cprp	r4, r5, r0, r1
	ldr	r6, BLOCK_SIZE
	ldr	r7, BLOCK_SIZE

	ldr	r2, VC_WRITE_RGB
db_loop1:
	dec	r6
	bz	db_loop2

	str	r4, [r3 + #4]
	str	r5, [r3 + #6]
	strb	r2, [r3]

	incw	r4, r5
	bra	db_loop1
db_loop2:
	dec	r7
	bz	db_return

	str	r4, [OUT]
	str	r5, [OUT]

	subw	r4, r5, BLOCK_SIZE
	adcw	r4, r5, SCREEN_WIDTH

	ldr	r6, BLOCK_SIZE
	bra	db_loop1

db_return:
	ldr	r2, VC_SWAP_BUFFERS
	str	r2, [r3]

	ldr	r7, [sp]
	ldr	r6, [sp + #2]
	ldr	r5, [sp + #4]
	ldr	r4, [sp + #6]
	ldr	r3, [sp + #8]
	ldr	r2, [sp + #10]
	ldr	r1, [sp + #12]
	ldr	r0, [sp + #14]
	add	sp, sp, #16
	ret

setup:
	ldr	r0, VC_COMM
	ldr	r1, VC_SET_RESOLUTION
	strb	r1, [r0]
	ldr	r1, [resolution]
	str	r1, [r0]

	srb	ps, #13
	ldr	r0, #0
	str	r0, [DDRB]
	str	r0, [DDRA]

	ldr	r0, ENABLE_INTERRUPT
	str	r0, [PORTA]

	ldr	r0, #0
	str	r0, [DDRA]
	ret
	
start:
	ldr	sp, INIT_STACK
	call	[setup]

	ldr	r0, #0
	ldr	r1, #0
	call	[draw_block]

loop:
	bra	loop

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
	call	[keypress_handler]

interrupt_return:
	ldr	r0, CLEAR_INTERRUPT << #8
	str	r0, [PORTA]

	ldr	r2, [sp]
	ldr	r1, [sp + #2]
	ldr	r0, [sp + #4]
	add	sp, sp, #6
	rti

.data:
	u16	resolution = VC_RESOLUTION_640X480
	u8	snake_dir = SNAKE_DIR_UP