// arm lock helper
// there is 2 part: read and write
// write return 0 on success, 1 on fail (value has been changed)

.text
.align 4

.global arm_lock_read_b
.global arm_lock_write_b
.global arm_lock_read_h
.global arm_lock_write_h
.global arm_lock_read_d
.global arm_lock_write_d
.global arm_lock_read_dd
.global arm_lock_write_dd
.global arm_lock_xchg
.global arm_lock_storeifnull
.global arm_lock_storeifref
.global arm_lock_storeifref2
.global arm_lock_decifnot0b
.global arm_lock_incb
.global arm_lock_incif0b
.global arm_lock_stored
.global arm_lock_storeb
.global arm_lock_storeif0b
.global arm_lock_readb

arm_lock_read_b:
    // address is r0, return is r0
    dmb     ish
    ldrexb  r0, [r0]
    bx      lr

arm_lock_write_b:
    // address is r0, value is r1, return is r0
    mov     r2, r0
    strexb  r0, r1, [r2]
    bx      lr

arm_lock_read_h:
    // address is r0, return is r0
    dmb     ish
    ldrexh  r0, [r0]
    bx      lr

arm_lock_write_h:
    // address is r0, value is r1, return is r0
    mov     r2, r0
    strexh  r0, r1, [r2]
    bx      lr

arm_lock_read_d:
    // address is r0, return is r0
    // r0 needs to be aligned
    dmb     ish
    ldrex   r0, [r0]
    bx      lr

arm_lock_write_d:
    // address is r0, value is r1, return is r0
    // r0 needs to be aligned
    mov     r2, r0
    strex   r0, r1, [r2]
    bx      lr

arm_lock_read_dd:
    // address is r2, return is r0, r1
    dmb     ish
    ldrexd  r2, r3, [r2]
    str     r2, [r0]
    str     r3, [r1]
    bx      lr

arm_lock_write_dd:
    // address is r2, value is r0, r1, return is r0
    // r0 needs to be aligned
    strexd  r3, r0, r1, [r2]
    mov     r0, r3
    bx      lr

arm_lock_xchg:
    // address is r0, value is r1, return old value in r0
    // r0 needs to be aligned
    dmb     ish
    ldrex   r2, [r0]
    strex   r3, r1, [r0]
    cmp     r3, #1
    beq     arm_lock_xchg
    mov     r0, r2
    bx      lr

arm_lock_storeifnull:
    // address is r0, value is r1, r1 store to r0 only if [r0] is 0. return old [r0] value
    // r0 needs to be aligned
    dmb     ish
    ldrex   r2, [r0]
    cmp     r2, #0
    bne     arm_lock_storeifnull_exit
    strex   r3, r1, [r0]
    cmp     r3, #1
    beq     arm_lock_storeifnull
arm_lock_storeifnull_exit:
    mov     r0, r2
    dmb     ish
    bx      lr

arm_lock_storeifref2:
    dmb     ish
arm_lock_storeifref2_loop:
    // address is r0, value is r1, r1 store to r0 only if [r0] is r2. return old [r0] value (so r2 or old value)
    ldrex   r3, [r0]
    cmp     r3, r2
    movne   r0, r3
    bne     arm_lock_storeifref2_exit
    strex   r3, r1, [r0]
    cmp     r3, #1
    beq     arm_lock_storeifref_loop
    mov     r0, r2
arm_lock_storeifref2_exit:
    dmb     ish
    bx      lr

arm_lock_storeifref:
    dmb     ish
arm_lock_storeifref_loop:
    // address is r0, value is r1, r1 store to r0 only if [r0] is r2. return new [r0] value (so r1 or old value)
    ldrex   r3, [r0]
    cmp     r3, r2
    movne   r0, r3
    bne     arm_lock_storeifref_exit
    strex   r3, r1, [r0]
    cmp     r3, #1
    beq     arm_lock_storeifref_loop
    mov     r0, r1
arm_lock_storeifref_exit:
    dmb     ish
    bx      lr

arm_lock_decifnot0b:
    dmb     ish
arm_lock_decifnot0b_0:
    ldrexb  r1, [r0]
    cmp     r1, #0
    subne   r3, r1, #1
    moveq   r3, r1
    strexb  r2, r3, [r0]
    cmp     r2, #1
    beq     arm_lock_decifnot0b_0
    dmb     ish
    mov     r0, r1
    bx      lr

arm_lock_incb:
    dmb     ish
arm_lock_incb_0:
    ldrexb  r1, [r0]
    add     r1, r1, #1
    strexb  r2, r1, [r0]
    cmp     r2, #1
    beq     arm_lock_incb_0
arm_lock_incb_exit:
    dmb     ish
    bx      lr

arm_lock_incif0b:
    dmb     ish
arm_lock_incif0b_0:
    ldrexb  r1, [r0]
    cmp     r1, #0
    addeq   r3, r1, #1
    movne   r3, r1
    strexb  r2, r3, [r0]
    cmp     r2, #1
    beq     arm_lock_incif0b_0
    dmb     ish
    mov     r0, r1
    bx      lr

arm_lock_stored:
    dmb     ish
    str     r1, [r0]
    dmb     ish
    bx      lr

arm_lock_storeb:
    dmb     ish
    strb    r1, [r0]
    dmb     ish
    bx      lr

arm_lock_storeif0b:
    dmb     ish
arm_lock_storeif0b_0:
    ldrexb  r2, [r0]
    cmp     r2, #0
    bne     arm_lock_storeif0b_exit
    strexb  r3, r1, [r0]
    cmp     r3, #1
    beq     arm_lock_storeif0b_0
    mov     r2, #0
    dmb     ish
arm_lock_storeif0b_exit:
    mov     r0, r2
    bx      lr

arm_lock_readb:
    dmb     ish
    ldrb    r0, [r0]
    bx      lr
