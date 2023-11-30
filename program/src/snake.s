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

VC_COMM		= #0x9
VC_DATA		= #0xA
VC_ADDRESS	= #0xD

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

delay:
	sub	sp, sp, #4
	str	r1, [sp + #2]
	str	r0, [sp]

	ldr	r0, #0x4F
	ldr	r1, #0xFF00
delay_loop1:
	decw	r0, r1
	bz	delay_return
	bra	delay_loop1

delay_return:
	ldr	r0, [sp]
	ldr	r1, [sp + #2]
	add	sp, sp, #4
	ret

game_loop:
	ldr	r0, [snake_head_high]
	; str	r0, [OUT]
	ldr	r1, [snake_head_low]
	; str	r1, [OUT]
	
	call	[set_brush_address]
	call	[draw_block]

	ldrb	r2, [snake_dir]

	sub	r3, r2, SNAKE_DIR_UP
	bz	snake_going_up	

	sub	r3, r2, SNAKE_DIR_DOWN
	bz	snake_going_down

	sub	r3, r2, SNAKE_DIR_LEFT
	bz	snake_going_left

	sub	r3, r2, SNAKE_DIR_RIGHT
	bz	snake_going_right

	bra	game_loop_end
	
snake_going_up:
	addw	r0, r1, SCREEN_WIDTH * (BLOCK_SIZE + #1)
	bra	game_loop_end
snake_going_down:
	subw	r0, r1, SCREEN_WIDTH * (BLOCK_SIZE + #1)
	bra	game_loop_end
snake_going_left:
	subw	r0, r1, BLOCK_SIZE
	bra	game_loop_end
snake_going_right:
	addw	r0, r1, BLOCK_SIZE
	bra	game_loop_end

 game_loop_end:
	str	r0, [snake_head_high]
	str	r1, [snake_head_low]

	call	[delay]
	bra	game_loop

start:
	ldr	sp, INIT_STACK
	call	[setup]

	ldr	r0, #255
	ldr	r1, #0
	ldr	r2, #0
	call	[set_brush_color]

	call	[game_loop]

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
	u16	snake_head_high = #1
	u16	snake_head_low = #0