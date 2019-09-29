//arm prologue for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 2

.global arm_prolog
arm_prolog:
    //save all used register
    push     {r4-r12}
    //setup emu -> register
    push     {r0}
    stmdb    r0, {r4-r12}   // all 8 register in direct access
    //TODO
    //jump to function
    bx       r1
