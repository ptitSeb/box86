//arm update linker table for dynarec
//called with pointer to emu as 1st parameter
//and address of table to as 2nd parameter
//ip is at r12

.text
.align 4

.extern UpdateLinkTable

.global arm_linker
arm_linker:
    // emu is r0
    // table offset is r1
    // IP address is r12
    push    {r0, r1, r12}
    mov     r2, r12
    // call the function
    bl      UpdateLinkTable
    // return offset is jump address
    mov     r3, r0
    pop     {r0, r1, r12}
    bx      r3

