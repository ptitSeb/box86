//arm epilog for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global arm_epilog
arm_epilog:
    //update register -> emu
    //pop     {r0}
    stm     r0, {r4-r12,r14}   // put back reg value in emu, including EIP (so r14 must be EIP now)
    //restore all used register
    //vpop     {d8-d15}
    pop     {r4-r11, pc}
    //end, return...
    //bx      lr


.global arm_epilog_fast
arm_epilog_fast:
    //restore all used register
    //vpop     {d8-d15}
    pop     {r4-r11, pc}
    //end, return...
    //bx      lr
