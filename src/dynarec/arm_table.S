//arm try to update linker table for dynarec
//and address of table to as 1st parameter
//jump addres as second and ip as third
// don't retry if failed to write...

.text
.align 4

.global arm_tableupdate
arm_tableupdate:
    // jump address is r0 and IP address is r1
    // table offset is r2
    push    {r4, r5}
    ldrexd  r4, r5, [r2]
    strexd  r3, r0, r1, [r2]
    pop     {r4, r5}
    mov     r0, r3
    bx      lr

