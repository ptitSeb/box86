//arm update linker table for dynarec
//called with pointer to emu as 1st parameter
//and address of table to as 2nd parameter
//ip is at r12

.text
.align 4

.extern LinkNext

.global arm_next
    .4byte  0   // NULL pointer before arm64_next, for getDB
arm_next:
    // emu is r0
    //stm     r0, {r4-r12,r14}   // don't put put back reg value in emu, faster but more tricky to debug
    // IP address is r1
    push    {r0, r1, r12, r14}
    // call the function
    bl      LinkNext
    // return offset is jump address
    mov     r3, r0
    pop     {r0, r1, r12, r14}
    bx      r3

