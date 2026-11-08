// The actual production gate, entered with the rewriter's saved x29/x30.
.text
.macro test_gate name
    .globl _test_gate_\name
    .p2align 2
_test_gate_\name:
    stp x29, x30, [sp, #-32]!
    mov x29, sp
    stp x19, x20, [sp, #16]
    mov x19, sp
    stp x29, x30, [sp, #-16]!
    mov x3, #0x3333
    mov x9, #0x9999
    mov x10, #0xaaaa
    mov x11, #0xbbbb
    mov x12, #0xcccc
    mov x13, #0xdddd
    mov x14, #0xeeee
    mov x15, #0xffff
    mov x16, #0x1616
    mov x17, #0x1717
    movi v0.16b, #0x5a
    movi v31.16b, #0xa5
    cmp x0, x0
    bl _runtime_gate
    .globl _test_gate_\name\()_ret
_test_gate_\name\()_ret:
    mrs x20, nzcv
    lsr x20, x20, #28
    cmp x20, #6
    b.ne 9f
    mov x20, #0x9999
    cmp x9, x20
    b.ne 9f
    mov x20, #0x1616
    cmp x16, x20
    b.ne 9f
    mov x20, #0x1234
    cmp x17, x20
    b.ne 9f
    fmov x10, d0
    mov x20, #0x5a5a
    movk x20, #0x5a5a, lsl #16
    movk x20, #0x5a5a, lsl #32
    movk x20, #0x5a5a, lsl #48
    cmp x10, x20
    b.ne 9f
    umov x10, v31.d[1]
    mov x20, #0xa5a5
    movk x20, #0xa5a5, lsl #16
    movk x20, #0xa5a5, lsl #32
    movk x20, #0xa5a5, lsl #48
    cmp x10, x20
    b.ne 9f
    add x20, sp, #16
    cmp x20, x19
    b.ne 9f
    b 8f
9:  mov x0, #-1
8:  ldp x29, x30, [sp], #16
    ldp x19, x20, [sp, #16]
    ldp x29, x30, [sp], #32
    ret
.endm
test_gate observe
test_gate svc
