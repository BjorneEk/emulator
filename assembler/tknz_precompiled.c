	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 12, 0	sdk_version 12, 3
	.globl	_is_ws                          ; -- Begin function is_ws
	.p2align	2
_is_ws:                                 ; @is_ws
	.cfi_startproc
; %bb.0:
	sub	w8, w0, #9
	cmp	w8, #24
	cset	w9, lo
	mov	w10, #25
	movk	w10, #128, lsl #16
	lsr	w8, w10, w8
	and	w0, w8, w9
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_tk_next                        ; -- Begin function tk_next
	.p2align	2
_tk_next:                               ; @tk_next
	.cfi_startproc
; %bb.0:
	stp	x26, x25, [sp, #-80]!           ; 16-byte Folded Spill
	stp	x24, x23, [sp, #16]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #32]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #48]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #64]             ; 16-byte Folded Spill
	add	x29, sp, #64
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	mov	x19, x0
	ldrb	w9, [x0, #88]
	cbz	w9, LBB1_2
; %bb.1:
	strb	wzr, [x19, #88]
	ldur	q0, [x19, #40]
	ldur	q1, [x19, #56]
	stp	q0, q1, [x8]
	ldur	q0, [x19, #72]
	str	q0, [x8, #32]
	b	LBB1_31
LBB1_2:
	add	x20, x19, #32
	mov	w24, #1
	mov	x25, #12800
	movk	x25, #1, lsl #32
LBB1_3:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB1_11 Depth 2
                                        ;     Child Loop BB1_15 Depth 2
                                        ;     Child Loop BB1_23 Depth 2
	ldr	x0, [x19]
	bl	_getc
	ldr	w8, [x19, #20]
	add	w26, w8, #1
	str	w26, [x19, #20]
	cmp	w0, #59
	b.hi	LBB1_25
; %bb.4:                                ;   in Loop: Header=BB1_3 Depth=1
	mov	w8, w0
	lsl	x9, x24, x8
	tst	x9, x25
	b.ne	LBB1_3
; %bb.5:                                ;   in Loop: Header=BB1_3 Depth=1
	cmp	x8, #47
	b.eq	LBB1_7
; %bb.6:                                ;   in Loop: Header=BB1_3 Depth=1
	cmp	x8, #59
	b.eq	LBB1_23
	b	LBB1_25
LBB1_7:                                 ;   in Loop: Header=BB1_3 Depth=1
	ldr	w22, [x19, #16]
	ldr	x21, [x19, #24]
	ldr	x0, [x19]
	bl	_getc
	cmp	w0, #47
	b.eq	LBB1_23
; %bb.8:                                ;   in Loop: Header=BB1_3 Depth=1
	cmp	w0, #42
	b.ne	LBB1_24
; %bb.9:                                ;   in Loop: Header=BB1_3 Depth=1
	ldr	x0, [x19]
	bl	_getc
	mov	x23, x0
	cmn	w0, #1
	b.eq	LBB1_11
LBB1_10:                                ;   in Loop: Header=BB1_3 Depth=1
	cmp	w23, #42
	b.ne	LBB1_15
LBB1_11:                                ;   Parent Loop BB1_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	x1, [x19]
	mov	x0, x23
	bl	_ungetc
	cmn	w23, #1
	b.eq	LBB1_19
; %bb.12:                               ;   in Loop: Header=BB1_11 Depth=2
	ldr	x0, [x19]
	bl	_getc
	cmp	w0, #47
	b.eq	LBB1_17
; %bb.13:                               ;   in Loop: Header=BB1_11 Depth=2
	cbnz	w0, LBB1_18
	b	LBB1_26
LBB1_14:                                ;   in Loop: Header=BB1_15 Depth=2
	ldr	x0, [x19]
	bl	_getc
	mov	x23, x0
	cmp	w0, #42
	ccmn	w0, #1, #4, ne
	b.eq	LBB1_11
LBB1_15:                                ;   Parent Loop BB1_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	w8, [x19, #20]
	add	w8, w8, #1
	str	w8, [x19, #20]
	cmp	w23, #10
	b.ne	LBB1_14
; %bb.16:                               ;   in Loop: Header=BB1_15 Depth=2
	ldr	w8, [x19, #16]
	add	w8, w8, #1
	stp	w8, wzr, [x19, #16]
	ldr	x8, [x19, #32]
	str	x8, [x19, #24]
	ldr	x0, [x19]
	mov	x1, x20
	bl	_fgetpos
	b	LBB1_14
LBB1_17:                                ;   in Loop: Header=BB1_11 Depth=2
	ldr	w8, [x19, #20]
	add	w8, w8, #1
	str	w8, [x19, #20]
LBB1_18:                                ;   in Loop: Header=BB1_11 Depth=2
	ldr	x1, [x19]
	bl	_ungetc
LBB1_19:                                ;   in Loop: Header=BB1_11 Depth=2
	ldr	x0, [x19]
	bl	_getc
	ldr	x1, [x19]
	cmn	w0, #1
	b.eq	LBB1_32
; %bb.20:                               ;   in Loop: Header=BB1_11 Depth=2
	bl	_ungetc
	ldr	x0, [x19]
	bl	_getc
	mov	x23, x0
	cmn	w0, #1
	b.ne	LBB1_10
	b	LBB1_11
LBB1_21:                                ;   in Loop: Header=BB1_23 Depth=2
	ldr	w8, [x19, #20]
	add	w8, w8, #1
	str	w8, [x19, #20]
	cmp	w0, #10
	b.ne	LBB1_23
; %bb.22:                               ;   in Loop: Header=BB1_23 Depth=2
	ldr	w8, [x19, #16]
	add	w8, w8, #1
	stp	w8, wzr, [x19, #16]
	ldr	x8, [x19, #32]
	str	x8, [x19, #24]
	ldr	x0, [x19]
	mov	x1, x20
	bl	_fgetpos
LBB1_23:                                ;   Parent Loop BB1_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	x0, [x19]
	bl	_getc
	cmn	w0, #1
	ccmp	w0, #10, #4, ne
	b.ne	LBB1_21
	b	LBB1_25
LBB1_24:                                ;   in Loop: Header=BB1_3 Depth=1
	ldr	x1, [x19]
	bl	_ungetc
	mov	w0, #47
LBB1_25:                                ;   in Loop: Header=BB1_3 Depth=1
	ldr	x1, [x19]
	bl	_ungetc
LBB1_26:                                ;   in Loop: Header=BB1_3 Depth=1
	ldr	x0, [x19]
	bl	_getc
	cmp	w0, #10
	b.ne	LBB1_28
; %bb.27:                               ;   in Loop: Header=BB1_3 Depth=1
	ldr	x8, [x19, #32]
	str	x8, [x19, #24]
	ldr	x0, [x19]
	mov	x1, x20
	bl	_fgetpos
	ldr	w8, [x19, #16]
	add	w8, w8, #1
	stp	w8, wzr, [x19, #16]
	b	LBB1_3
LBB1_28:
	ldr	x1, [x19]
	bl	_ungetc
	ldr	x0, [x19]
	bl	_getc
	ldr	w8, [x19, #20]
	add	w8, w8, #1
	str	w8, [x19, #20]
	cmp	w0, #60
	b.ne	LBB1_31
; %bb.29:
	ldr	x0, [x19]
	bl	_getc
	mov	x20, x0
	ldr	x1, [x19]
	bl	_ungetc
	cmp	w20, #60
	b.ne	LBB1_31
; %bb.30:
	ldr	x0, [x19]
	ldp	x29, x30, [sp, #64]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp], #80             ; 16-byte Folded Reload
	b	_getc
LBB1_31:
	ldp	x29, x30, [sp, #64]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp], #80             ; 16-byte Folded Reload
	ret
LBB1_32:
	bfi	x22, x26, #32, #32
	bl	_ungetc
	mov	x0, x19
	mov	x1, x22
	mov	x2, x21
	bl	_tk_error
	.cfi_endproc
                                        ; -- End function
	.globl	_new_tokenizer                  ; -- Begin function new_tokenizer
	.p2align	2
_new_tokenizer:                         ; @new_tokenizer
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x0
	mov	w0, #96
	bl	_malloc
	mov	x20, x0
Lloh0:
	adrp	x1, l_.str.43@PAGE
Lloh1:
	add	x1, x1, l_.str.43@PAGEOFF
	mov	x0, x19
	bl	_fopen
	str	x0, [x20]
	cbz	x0, LBB2_2
; %bb.1:
	stp	x19, xzr, [x20, #8]
	add	x1, x20, #24
	bl	_fgetpos
	ldr	x8, [x20, #24]
	str	x8, [x20, #32]
	mov	x0, x20
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
LBB2_2:
	str	x19, [sp]
Lloh2:
	adrp	x0, l_.str.44@PAGE
Lloh3:
	add	x0, x0, l_.str.44@PAGEOFF
Lloh4:
	adrp	x1, l_.str.45@PAGE
Lloh5:
	add	x1, x1, l_.str.45@PAGEOFF
	bl	_exit_error_custom
	.loh AdrpAdd	Lloh0, Lloh1
	.loh AdrpAdd	Lloh4, Lloh5
	.loh AdrpAdd	Lloh2, Lloh3
	.cfi_endproc
                                        ; -- End function
	.globl	_tk_prev                        ; -- Begin function tk_prev
	.p2align	2
_tk_prev:                               ; @tk_prev
	.cfi_startproc
; %bb.0:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	ldrb	w9, [x0, #88]
	cbz	w9, LBB3_2
; %bb.1:
	strb	wzr, [x0, #88]
	ldur	q0, [x0, #40]
	ldur	q1, [x0, #56]
	stp	q0, q1, [x8]
	ldur	q0, [x0, #72]
	str	q0, [x8, #32]
	ldp	x29, x30, [sp], #16             ; 16-byte Folded Reload
	ret
LBB3_2:
Lloh6:
	adrp	x0, l_.str.46@PAGE
Lloh7:
	add	x0, x0, l_.str.46@PAGEOFF
Lloh8:
	adrp	x1, l_.str.47@PAGE
Lloh9:
	add	x1, x1, l_.str.47@PAGEOFF
	bl	_exit_error_custom
	.loh AdrpAdd	Lloh8, Lloh9
	.loh AdrpAdd	Lloh6, Lloh7
	.cfi_endproc
                                        ; -- End function
	.globl	_tk_rev                         ; -- Begin function tk_rev
	.p2align	2
_tk_rev:                                ; @tk_rev
	.cfi_startproc
; %bb.0:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	ldrb	w8, [x0, #88]
	cbnz	w8, LBB4_2
; %bb.1:
	mov	w8, #1
	strb	w8, [x0, #88]
	ldp	q0, q1, [x1]
	ldr	q2, [x1, #32]
	stur	q2, [x0, #72]
	stur	q1, [x0, #56]
	stur	q0, [x0, #40]
	ldp	x29, x30, [sp], #16             ; 16-byte Folded Reload
	ret
LBB4_2:
Lloh10:
	adrp	x0, l_.str.46@PAGE
Lloh11:
	add	x0, x0, l_.str.46@PAGEOFF
Lloh12:
	adrp	x1, l_.str.48@PAGE
Lloh13:
	add	x1, x1, l_.str.48@PAGEOFF
	bl	_exit_error_custom
	.loh AdrpAdd	Lloh12, Lloh13
	.loh AdrpAdd	Lloh10, Lloh11
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function tk_error
_tk_error:                              ; @tk_error
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #80
	stp	x22, x21, [sp, #32]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #48]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #64]             ; 16-byte Folded Spill
	add	x29, sp, #64
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x19, x2
	mov	x20, x1
	mov	x21, x0
	lsr	x22, x1, #32
	add	x8, x29, #16
	str	x8, [sp, #24]
Lloh14:
	adrp	x0, l_.str.51@PAGE
Lloh15:
	add	x0, x0, l_.str.51@PAGEOFF
Lloh16:
	adrp	x1, l_.str.50@PAGE
Lloh17:
	add	x1, x1, l_.str.50@PAGEOFF
	add	x2, x29, #16
	bl	_verror_custom
Lloh18:
	adrp	x8, ___stderrp@GOTPAGE
Lloh19:
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
Lloh20:
	ldr	x0, [x8]
	ldr	x8, [x21, #8]
	stp	x20, x22, [sp, #8]
	str	x8, [sp]
Lloh21:
	adrp	x1, l_.str.52@PAGE
Lloh22:
	add	x1, x1, l_.str.52@PAGEOFF
	bl	_fprintf
	ldr	x0, [x21]
	mov	x1, x20
	mov	x2, x19
	bl	_get_debug_lines
	mov	w0, #-1
	bl	_exit
	.loh AdrpAdd	Lloh21, Lloh22
	.loh AdrpLdrGotLdr	Lloh18, Lloh19, Lloh20
	.loh AdrpAdd	Lloh16, Lloh17
	.loh AdrpAdd	Lloh14, Lloh15
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function get_debug_lines
_get_debug_lines:                       ; @get_debug_lines
	.cfi_startproc
; %bb.0:
	stp	x24, x23, [sp, #-64]!           ; 16-byte Folded Spill
	stp	x22, x21, [sp, #16]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	sub	sp, sp, #3216
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x19, x0
Lloh23:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh24:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh25:
	ldr	x8, [x8]
	stur	x8, [x29, #-56]
	add	x8, sp, #24
	add	x20, x8, #8
	stp	x1, x2, [sp, #24]
	add	x1, sp, #16
	bl	_fgetpos
                                        ; kill: def $w0 killed $w0 def $x0
	sxtw	x8, w0
	str	x8, [sp, #16]
	mov	x0, x19
	mov	x1, x20
	bl	_fsetpos
	ldr	w8, [sp, #24]
	sub	w8, w8, #1
	str	x8, [sp]
Lloh26:
	adrp	x20, l_.str.53@PAGE
Lloh27:
	add	x20, x20, l_.str.53@PAGEOFF
	add	x0, sp, #104
	mov	w1, #0
	mov	w2, #32
	mov	x3, x20
	bl	___sprintf_chk
	ldr	w8, [sp, #24]
	str	x8, [sp]
	add	x0, sp, #72
	mov	w1, #0
	mov	w2, #32
	mov	x3, x20
	bl	___sprintf_chk
	ldr	w8, [sp, #24]
	add	w8, w8, #1
	str	x8, [sp]
	add	x0, sp, #40
	mov	w1, #0
	mov	w2, #32
	mov	x3, x20
	bl	___sprintf_chk
	add	x0, sp, #2184
	mov	w1, #1024
	mov	x2, x19
	bl	_fgets
	add	x22, sp, #1160
	add	x0, sp, #1160
	mov	w1, #1024
	mov	x2, x19
	bl	_fgets
	add	x0, sp, #136
	mov	w1, #1024
	mov	x2, x19
	bl	_fgets
	add	x0, sp, #2184
	bl	_strlen
	mov	x20, x0
	add	x0, sp, #1160
	bl	_strlen
	mov	x21, x0
	add	x0, sp, #136
	bl	_strlen
	add	x8, x20, x21
	add	x8, x8, x0
	add	x0, x8, #400
	bl	_malloc
	mov	x20, x0
	add	x1, sp, #104
	bl	_strcat
	add	x1, sp, #2184
	bl	_strcat
	bl	_strlen
	mov	w23, #10
	strh	w23, [x20, x0]
	add	x1, sp, #72
	mov	x0, x20
	bl	_strcat
	ldrsw	x21, [sp, #28]
	add	x1, sp, #1160
	mov	x2, x21
	bl	_strncat
	bl	_strlen
	add	x8, x20, x0
	mov	w9, #109
	strh	w9, [x8, #8]
Lloh28:
	adrp	x9, l_.str.55@PAGE
Lloh29:
	add	x9, x9, l_.str.55@PAGEOFF
Lloh30:
	ldr	x9, [x9]
	str	x9, [x8]
	add	x1, x22, x21
	mov	x0, x20
	mov	w2, #2
	bl	_strncat
	bl	_strlen
	add	x8, x20, x0
	mov	w9, #23323
	movk	w9, #27952, lsl #16
	str	w9, [x8]
	strb	wzr, [x8, #4]
	ldrsw	x8, [sp, #28]
	add	x8, x22, x8
	add	x1, x8, #2
	mov	x0, x20
	bl	_strcat
	bl	_strlen
	strh	w23, [x20, x0]
	add	x1, sp, #40
	mov	x0, x20
	bl	_strcat
	add	x1, sp, #136
	bl	_strcat
	bl	_strlen
	strh	w23, [x20, x0]
	add	x1, sp, #16
	mov	x0, x19
	bl	_fsetpos
	ldur	x8, [x29, #-56]
Lloh31:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh32:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh33:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB6_2
; %bb.1:
	add	sp, sp, #3216
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp], #64             ; 16-byte Folded Reload
	ret
LBB6_2:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh31, Lloh32, Lloh33
	.loh AdrpAddLdr	Lloh28, Lloh29, Lloh30
	.loh AdrpAdd	Lloh26, Lloh27
	.loh AdrpLdrGotLdr	Lloh23, Lloh24, Lloh25
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__const
	.globl	_endian                         ; @endian
	.p2align	2
_endian:
	.long	1                               ; 0x1

	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"nop"

l_.str.1:                               ; @.str.1
	.asciz	"brk"

l_.str.2:                               ; @.str.2
	.asciz	"ldr"

l_.str.3:                               ; @.str.3
	.asciz	"ldrb"

l_.str.4:                               ; @.str.4
	.asciz	"ldrw"

l_.str.5:                               ; @.str.5
	.asciz	"str"

l_.str.6:                               ; @.str.6
	.asciz	"strb"

l_.str.7:                               ; @.str.7
	.asciz	"cpr"

l_.str.8:                               ; @.str.8
	.asciz	"cprp"

l_.str.9:                               ; @.str.9
	.asciz	"bz"

l_.str.10:                              ; @.str.10
	.asciz	"bnz"

l_.str.11:                              ; @.str.11
	.asciz	"bcc"

l_.str.12:                              ; @.str.12
	.asciz	"bcs"

l_.str.13:                              ; @.str.13
	.asciz	"brn"

l_.str.14:                              ; @.str.14
	.asciz	"brp"

l_.str.15:                              ; @.str.15
	.asciz	"bra"

l_.str.16:                              ; @.str.16
	.asciz	"lbra"

l_.str.17:                              ; @.str.17
	.asciz	"call"

l_.str.18:                              ; @.str.18
	.asciz	"ret"

l_.str.19:                              ; @.str.19
	.asciz	"rti"

l_.str.20:                              ; @.str.20
	.asciz	"adc"

l_.str.21:                              ; @.str.21
	.asciz	"add"

l_.str.22:                              ; @.str.22
	.asciz	"adcw"

l_.str.23:                              ; @.str.23
	.asciz	"addw"

l_.str.24:                              ; @.str.24
	.asciz	"sbc"

l_.str.25:                              ; @.str.25
	.asciz	"sub"

l_.str.26:                              ; @.str.26
	.asciz	"sbcw"

l_.str.27:                              ; @.str.27
	.asciz	"subw"

l_.str.28:                              ; @.str.28
	.asciz	"eor"

l_.str.29:                              ; @.str.29
	.asciz	"orr"

l_.str.30:                              ; @.str.30
	.asciz	"and"

l_.str.31:                              ; @.str.31
	.asciz	"cmp"

l_.str.32:                              ; @.str.32
	.asciz	"asr"

l_.str.33:                              ; @.str.33
	.asciz	"lsl"

l_.str.34:                              ; @.str.34
	.asciz	"lsr"

l_.str.35:                              ; @.str.35
	.asciz	"not"

l_.str.36:                              ; @.str.36
	.asciz	"dec"

l_.str.37:                              ; @.str.37
	.asciz	"decw"

l_.str.38:                              ; @.str.38
	.asciz	"inc"

l_.str.39:                              ; @.str.39
	.asciz	"incw"

l_.str.40:                              ; @.str.40
	.asciz	"crb"

l_.str.41:                              ; @.str.41
	.asciz	"srb"

	.section	__DATA,__data
	.globl	_insmap                         ; @insmap
	.p2align	3
_insmap:
	.quad	l_.str
	.long	4                               ; 0x4
	.long	0                               ; 0x0
	.quad	l_.str.1
	.long	4                               ; 0x4
	.long	1                               ; 0x1
	.quad	l_.str.2
	.long	4                               ; 0x4
	.long	2                               ; 0x2
	.quad	l_.str.3
	.long	5                               ; 0x5
	.long	10                              ; 0xa
	.quad	l_.str.4
	.long	5                               ; 0x5
	.long	18                              ; 0x12
	.quad	l_.str.5
	.long	4                               ; 0x4
	.long	26                              ; 0x1a
	.quad	l_.str.6
	.long	5                               ; 0x5
	.long	34                              ; 0x22
	.quad	l_.str.7
	.long	4                               ; 0x4
	.long	42                              ; 0x2a
	.quad	l_.str.8
	.long	5                               ; 0x5
	.long	44                              ; 0x2c
	.quad	l_.str.9
	.long	3                               ; 0x3
	.long	45                              ; 0x2d
	.quad	l_.str.10
	.long	4                               ; 0x4
	.long	46                              ; 0x2e
	.quad	l_.str.11
	.long	4                               ; 0x4
	.long	47                              ; 0x2f
	.quad	l_.str.12
	.long	4                               ; 0x4
	.long	48                              ; 0x30
	.quad	l_.str.13
	.long	4                               ; 0x4
	.long	49                              ; 0x31
	.quad	l_.str.14
	.long	4                               ; 0x4
	.long	50                              ; 0x32
	.quad	l_.str.15
	.long	4                               ; 0x4
	.long	51                              ; 0x33
	.quad	l_.str.16
	.long	5                               ; 0x5
	.long	52                              ; 0x34
	.quad	l_.str.17
	.long	5                               ; 0x5
	.long	54                              ; 0x36
	.quad	l_.str.18
	.long	4                               ; 0x4
	.long	57                              ; 0x39
	.quad	l_.str.19
	.long	4                               ; 0x4
	.long	58                              ; 0x3a
	.quad	l_.str.20
	.long	4                               ; 0x4
	.long	59                              ; 0x3b
	.quad	l_.str.21
	.long	4                               ; 0x4
	.long	69                              ; 0x45
	.quad	l_.str.22
	.long	5                               ; 0x5
	.long	79                              ; 0x4f
	.quad	l_.str.23
	.long	5                               ; 0x5
	.long	89                              ; 0x59
	.quad	l_.str.24
	.long	4                               ; 0x4
	.long	99                              ; 0x63
	.quad	l_.str.25
	.long	4                               ; 0x4
	.long	109                             ; 0x6d
	.quad	l_.str.26
	.long	5                               ; 0x5
	.long	119                             ; 0x77
	.quad	l_.str.27
	.long	5                               ; 0x5
	.long	129                             ; 0x81
	.quad	l_.str.28
	.long	4                               ; 0x4
	.long	139                             ; 0x8b
	.quad	l_.str.29
	.long	4                               ; 0x4
	.long	149                             ; 0x95
	.quad	l_.str.30
	.long	4                               ; 0x4
	.long	159                             ; 0x9f
	.quad	l_.str.31
	.long	4                               ; 0x4
	.long	169                             ; 0xa9
	.quad	l_.str.32
	.long	4                               ; 0x4
	.long	179                             ; 0xb3
	.quad	l_.str.33
	.long	4                               ; 0x4
	.long	181                             ; 0xb5
	.quad	l_.str.34
	.long	4                               ; 0x4
	.long	180                             ; 0xb4
	.quad	l_.str.35
	.long	4                               ; 0x4
	.long	182                             ; 0xb6
	.quad	l_.str.36
	.long	4                               ; 0x4
	.long	183                             ; 0xb7
	.quad	l_.str.37
	.long	5                               ; 0x5
	.long	184                             ; 0xb8
	.quad	l_.str.38
	.long	4                               ; 0x4
	.long	185                             ; 0xb9
	.quad	l_.str.39
	.long	5                               ; 0x5
	.long	186                             ; 0xba
	.quad	l_.str.40
	.long	4                               ; 0x4
	.long	187                             ; 0xbb
	.quad	l_.str.41
	.long	4                               ; 0x4
	.long	197                             ; 0xc5

	.section	__TEXT,__cstring,cstring_literals
l_.str.42:                              ; @.str.42
	.asciz	"#%&()[]=+|*~-,"

	.section	__DATA,__data
	.globl	_SIMPLE_TOK                     ; @SIMPLE_TOK
	.p2align	3
_SIMPLE_TOK:
	.quad	l_.str.42

	.section	__TEXT,__cstring,cstring_literals
l_.str.43:                              ; @.str.43
	.asciz	"r"

l_.str.44:                              ; @.str.44
	.asciz	"ASSEMBLER"

l_.str.45:                              ; @.str.45
	.asciz	"could not open file: %s"

l_.str.46:                              ; @.str.46
	.asciz	"Assertion Error"

l_.str.47:                              ; @.str.47
	.asciz	"no previous available"

l_.str.48:                              ; @.str.48
	.asciz	"cannot reverse tokenizer twice"

l_.str.50:                              ; @.str.50
	.asciz	"unclosed multiline comment"

l_.str.51:                              ; @.str.51
	.asciz	"error"

l_.str.52:                              ; @.str.52
	.asciz	"\033[1mIn file: %s, line: %i, col: %i\033[0m\n"

l_.str.53:                              ; @.str.53
	.asciz	"\033[32m\033[1m%5i\033[0m \033[1m|\033[0m "

l_.str.55:                              ; @.str.55
	.asciz	"\033[31;1;4m"

l_.str.56:                              ; @.str.56
	.asciz	"\033[0m"

.subsections_via_symbols
