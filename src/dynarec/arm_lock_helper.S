//arm lock helper
//there is 2 part: read and write
// write return 0 on success, 1 on fail (value has been changed)

.text
.align 4

.global arm_lock_read_b
.global arm_lock_write_b
.global arm_lock_read_d
.global arm_lock_write_d
.global arm_lock_read_dd
.global arm_lock_write_dd

arm_lock_read_b:
    // address is r0, return is r0
    ldrexb  r0, [r0]
    bx      lr

arm_lock_write_b:
    // address is r0, value is r1, return is r0
    mov     r2, r0
    strexb  r0, r1, [r2]
    bx      lr

arm_lock_read_d:
    // address is r0, return is r0
    // r0 needs to be aligned
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