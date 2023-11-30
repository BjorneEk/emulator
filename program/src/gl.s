.another_text:

; r0 = r (0-255)
; r1 = g (0-255)
; r2 = b (0-255)
set_brush_color:
	sub	sp, sp, #2
	str	r3, [sp]

	ldr	r3, VC_DATA
	strb	r0, [r3]
	strb	r1, [r3 + #1]
	strb	r2, [r3 + #2]

	ldr	r3, [sp]
	add	sp, sp, #2
	ret

; r0 = address high
; r1 = address low
set_brush_address:
	sub	sp, sp, #2
	str	r2, [sp]

	ldr	r2, VC_ADDRESS
	str	r0, [r2]
	str	r1, [r2 + #2]

	ldr	r2, [sp]
	add	sp, sp, #2
	ret

draw_block:
	sub	sp, sp, #12
	str	r5, [sp + #10]
	str	r4, [sp + #8]
	str	r3, [sp + #6]
	str	r2, [sp + #4]
	str	r1, [sp + #2]
	str	r0, [sp]

	ldr	r2, VC_COMM
	ldr	r3, VC_WRITE_RGB
	ldr	r4, BLOCK_SIZE - #1
	ldr	r5, BLOCK_SIZE

	ldr	r0, [r2 + #4]
	ldr	r1, [r2 + #6]

db_loop1:
	strb	r3, [r2]

	dec	r4
	bz	db_loop2

	incw	r0, r1
	call 	[set_brush_address]

	bra	db_loop1
db_loop2:
	dec	r5
	bz	db_return

	addw	r0, r1, SCREEN_WIDTH - BLOCK_SIZE + #1

	ldr	r4, BLOCK_SIZE
	bra	db_loop1

 db_return:
	ldr	r3, VC_SWAP_BUFFERS
	strb	r3, [r2]

	ldr	r0, [sp]
	ldr	r1, [sp + #2]
	ldr	r2, [sp + #4]
	ldr	r3, [sp + #6]
	ldr	r4, [sp + #8]
	ldr	r5, [sp + #10]
	add	sp, sp, #12
	ret
