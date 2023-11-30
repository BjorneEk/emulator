
0: putchar debug NULL
1: port a
2: port a
3: ddra 0=in, 1=out
4: ddra 0=in, 1=out
5: port b
6: port b
7: ddrb
8: ddrb

9: video-card-comm
10: video-data
11: video-data
12: video-data
13: vram-addr
14: vram-addr
15: vram-addr
16: vram-addr

0000 0000 0000 0000
ps: carry, zero, interrupt, overflow, negative, break

direct:
#val                - im,           16-bit val
reg                 - reg,          -

absolute:
[#val]              - abs,          32-bit val
[reg, reg]          - abs-ptr,      -

absolute-idx:
[#val] , reg         - abs-idx,     32-bit val
[reg, reg] , reg     - abs-ptr-idx, -
[reg, reg] , #val    - abs-ptr-off, 16-bit val

zero-page:
[reg]               - ze-ptr,       -
[reg + #val]        - zp-offset,    16-bit val
[reg + reg]         - zp-idx,       -

special:
(nothing)           - implied (no standard addressing mode used)
relative            - 8-bit val, (usualy a label)

12-addressing modes

instructions:
(suported adressing modes), [unsuported adressing modes + special], 'all' all except implied and relative

load and store:
ldr     Rd                  - load register         all 0-10
ldrb    Rd                  - load register byte    all 0-10
ldrw    Rdh, Rdl            - load register wide    [im, reg] 10-20
str     Rd                  - store register        [im, reg]
strb    Rd                  - store register byte   [im, reg]
cprp    Rdh, Rdl, Rsh, Rsl  - copy register pair    (implied)

branch and jump:
bz                  - branch on zero            (relative)
bnz                 - branch not zero           (relative)
bcc                 - branch on carry clear     (relative)
bcs                 - branch on carry set       (relative)
brn                 - branch negative           (relative)
brp                 - branch posetive           (relative)
bra                 - unconditional branch      (relative)

bbs 4bv, Rs         - branch register bit set   (relative) --- NEW
bbc 4bv, Rs         - branch register bit clear (relative) --- NEW

lbra                - unconditional long branch (abs, abs-ptr)
call                - long call to subrutine    (abs, abs-ptr, zp)
ret                 - return from subrutine     (implied)
rti                 - return from interrupt     (implied)

arithmetic and logic:
adc     Rd, Rs      - add with carry                all
add     Rd, Rs      - add without carry             all
adcw    Rdh, Rdl    - add with carry wide           all
addw    Rdh, Rdl    - add without carry wide        all
sbc     Rd, Rs      - subtract with carry           all
sub     Rd, Rs      - subtract without carry        all
sbcw    Rdh, Rdl    - subtract with carry wide      all
subw    Rdh, Rdl    - subtract without carry wide   all
eor     Rd, Rs      - exclusive or                  all
orr     Rd, Rs      - or                            all
and     Rd, Rs      - and                           all
cmp     Rd          - compare                       all
asr     Rd          - arithmetic shift right        (implied)
lsr     Rd          - logic shift right             (implied)
lsl     Rd          - logic shift left              (implied)
not     Rd          - negate                        (implied)
dec     Rd          - decrement                     (implied)
decw    Rdh, Rdl    - decrement wide                (implied)
inc     Rd          - increment                     (implied)
incw    Rdh, Rdl    - increment wide                (implied)
crb     Rd,         - clear bit in register         all
srb     Rd,         - set bit in register           all

general:
brk     - halt execution    (implied)
nop     - no op             (implied)

addressing modes:

relative:
|ADDDDDDR|
|rel-addr|
width: 1

immediate:
|VVVVVVVV|VVVVVVVV|
| immediate value |
width: 2

register:
|RRRR0000|
|R  ||   |
width: 1

absolute:
|VVVVVVVV|VVVVVVVV|VVVVVVVV|VVVVVVVV|
|           absolute value          |
width: 4

absolute-pointer:
|RRRhRRRl|
|Rh || Rl|
width: 1

absolute-indexed:
|VVVVVVVV|VVVVVVVV|VVVVVVVV|VVVVVVVV|RRRR0000|
|           absolute value          |Ri ||   |
width: 5

absolute-pointer-indexed:
|RRRhRRRl|RRRR0000|
|Rh || Rl|Ri ||   |
width: 2

absolute-pointer-offset:
|RRRhRRRl|VVVVVVVV|VVVVVVVV|
|Rh || Rl|     offset      |
width: 3

zero-page-pointer:
|RRRR0000|
|R  ||   |
width: 1

zero-page-offset:
|RRRR0000|VVVVVVVV|VVVVVVVV|
|R  ||   |     offset      |
width: 3

zero-page-indexed:
|RRRRRRRi|
|R  || Ri|
width: 1

instruction:

ldr:
ldrb:
str:
strb:
cmp:
asr:
lsr:
lsl:
not:
dec:
inc:
crb:
srb:
|XXXXXXXX|RRRR0000|
|opcode  |Rd      |
width: 2

ldrw:
|XXXXXXXX|RRRhRRRl|
|opcode  |Rh || Rl|
width: 2

cprp:
|XXXXXXXX|RRRhRRRl|RRRhRRRl|
|opcode  |Rdh||Rdl|Rsh||Rsl|
width: 3

bz:
bnz:
bcc:
bcs:
brn:
brp:
bra:
lbra:
call:
ret:
rti:
brk:
nop:
|XXXXXXXX|
|opcode  |
width: 1

bbs:
bbc:
|XXXXXXXX|VVVVRRRR|
|opcode  |4bv |  R|
width: 2

adc:
add:
sbc:
sub:
eor:
orr:
and:
|XXXXXXXX|RRRRRRRR|
|opcode  |Rd || Rs|
width: 2

adcw:
addw:
sbcw:
subw:
decw:
incw:
|XXXXXXXX|RRRRRRRR|
|opcode  |Rdh||Rdl|
width: 2