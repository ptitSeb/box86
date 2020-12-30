//arm prologue for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global arm_prolog
arm_prolog:
    //save all used register
    push     {r4-r11, lr}
    //vpush     {d8-d15}
    //setup emu -> register
    //push     {r0}
    ldm      r0, {r4-r12}    // all 8 register in direct access, plus flags, no EIP (so r14 can be used as scratch)
    //TODO
    //jump to function
    bx       r1
