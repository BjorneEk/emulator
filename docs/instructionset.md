
ps: carry, zero, interrupt, overflow, negative, break

direct:
#val                - im,           !, -
reg                 - reg,          !, -

absolute:
[#val]              - abs,          !, -
[(reg, reg)]        - abs-ptr,      !, -

absolute-idx:
[#val + reg]         - abs-idx,     !, -
[(reg, reg) + reg]   - abs-ptr-idx, !, -
[(reg, reg) + #val]  - abs-ptr-off, !, -

zero-page:
[reg]               - zp-ptr,       !, -
[reg + #val]        - zp-offset,    !, -
[reg + reg]         - zp-idx,       !, -

special:
implied, (relative)

12-addressing modes

instructions:
(suported adressing modes), [unsuported adressing modes + special], 'all' all except implied and relative

load and store:
ldr     Rd                  - load register         [im, reg] 0-10
ldrw    Rdh, Rdl            - load register wide    [im, reg] 10-20
str     Rd                  - store register        [im, reg]
cpr     Rd                  - copy register         (im, reg)
cprp    Rdh, Rdl, Rsh, Rsl  - copy register pair    (implied)

branch and jump:
bz      - branch on zero            (relative)
bnz     - branch not zero           (relative)
bcc     - branch on cary clear      (relative)
bcs     - branch on cary set        (relative)
brn     - branch negative           (relative)
brp     - branch posetive           (relative)
bra     - unconditional branch      (relative)
lbra    - unconditional long branch (abs, abs-ptr)
call    - long call to subrutine    (abs, abs-ptr, zp)
rts     - return from subrutine     (implied)
rti     - return from interrupt     (implied)

arithmetic and logic:
adc     Rd, Rs      - add with cary                 all
add     Rd, Rs      - add without cary              all
adcw    Rdh, Rdl    - add with cary wide            all
addw    Rdh, Rdl    - add without cary wide         all
sbc     Rd, Rs      - subtract with cary            all
sub     Rd, Rs      - subtract without cary         all
sbcw    Rdh, Rdl    - subtract with cary wide       all
subw    Rdh, Rdl    - subtract without cary wide    all
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
incw    Rdh, Rdl    - increment wide                (implied)
crb     Rd,         - clear bit in register         all
srb     Rd,         - set bit in register           all

general:
brk     - halt execution    (implied)
nop     - no op             (implied)
