#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x87emu_private.h"
#include "x86primop.h"
#include "x86trace.h"

#define PI		3.14159265358979323846
#define L2E		1.4426950408889634
#define L2T		3.3219280948873623
#define LN2		0.69314718055994531
#define LG2		0.3010299956639812

void RunD9(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch((nextop>>3)&7) {
        case 0:     /* FLD ST0, Ed float */
            GetEd(emu, &op2, &ea2, nextop);
            *(uint32_t*)&f = op2->dword[0];
            fpu_do_push(emu);
            ST0.d = f;
            break;
        case 3:     /* FSTP Ed, ST0 float with partial alias on mod=3=>ST1 */
            GetEd(emu, &op1, &ea1, nextop);
            if((nextop>>6)==3)
                f = ST1.d;
            else
                f = ST0.d;
            fpu_do_pop(emu);
            op1->dword[0] = *(uint32_t*)&f;
            break;
        case 5:
            if(nextop==0xEE) {  /* FLDZ */
                fpu_do_push(emu);
                ST0.d = 0.0;
            } else if(nextop==0xE8) {  /* FLD1 */
                fpu_do_push(emu);
                ST0.d = 1.0;
            } else if(nextop==0xE9) {  /* FLDL2T */
                fpu_do_push(emu);
                ST0.d = L2T;
            } else if(nextop==0xEA) {  /* FLDL2E */
                fpu_do_push(emu);
                ST0.d = L2E;
            } else if(nextop==0xEB) {  /* FLDPI */
                fpu_do_push(emu);
                ST0.d = PI;
            } else if(nextop==0xEC) {  /* FLDLG2 */
                fpu_do_push(emu);
                ST0.d = LG2;
            } else if(nextop==0xED) {  /* FLDLN2 */
                fpu_do_push(emu);
                ST0.d = LN2;
            } else {    /* FLDCW */
                GetEw(emu, &op1, &ea1, nextop);
                emu->cw = op1->word[0];
                // do something with cw?
                emu->round = (fpu_round_t)((emu->cw >> 10) & 3);

            }
            break;
        case 7: /* FNSTCW Ew */
            GetEw(emu, &op1, &ea1, nextop);
            op1->word[0] = emu->cw;
            break;
        default:
            printf_log(LOG_NONE, "Unimplemented Opcode D9 %02X %02X \n", nextop, Peek(emu, 0));
            emu->quit=1;
            emu->error |= ERR_UNIMPL;
    }
}

void RunDB(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    int32_t tmp32s;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch((nextop>>3)&7) {
        case 0: /* FILD ST0, Gd */
            GetEd(emu, &op2, &ea2, nextop);
            *(uint32_t*)&tmp32s = op2->dword[0];
            fpu_do_push(emu);
            ST0.d = tmp32s;
            break;
        case 4: /* group */
            if(nextop==0xE2) {    /* FNCLEX */
            //Clears the floating-point exception flags (PE, UE, OE, ZE, DE, and IE), 
            // the exception summary status flag (ES), the stack fault flag (SF), and the busy flag (B) in the FPU status word
                emu->sw = 0;
            } else if(nextop==0xE3) {    /* FNINIT */
                reset_fpu(emu);
            } else {
                printf_log(LOG_NONE, "Unimplemented Opcode DB %02X %02X \n", nextop, Peek(emu, 0));
                emu->quit=1;
                emu->error |= ERR_UNIMPL;
            }
            break;
        case 7: /* FSTP float */
            GetEd(emu, &op1, &ea1, nextop);
            f = ST0.d;
            fpu_do_pop(emu);
            op1->dword[0] = *(uint32_t*)&f;
            break;
        default:
            printf_log(LOG_NONE, "Unimplemented Opcode DB %02X %02X \n", nextop, Peek(emu, 0));
            emu->quit=1;
            emu->error |= ERR_UNIMPL;
    }
}

void RunDD(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch((nextop>>3)&7) {
        case 3: /* FSTP double */
            GetEd(emu, &op1, &ea1, nextop);
            *(uint64_t*)&op1->dword[0] = ST0.ll;
            fpu_do_pop(emu);
            break;
        default:
            printf_log(LOG_NONE, "Unimplemented Opcode DD %02X %02X \n", nextop, Peek(emu, 0));
            emu->quit=1;
            emu->error |= ERR_UNIMPL;
    }
}

void RunDE(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch((nextop>>3)&7) {
        case 1: 
            if(nextop==0xC9) {  /* FMULP ST1, ST0 */
                d = ST0.d;
                fpu_do_pop(emu);
                ST0.d *= d;
            } else {
                printf_log(LOG_NONE, "Unimplemented Opcode DE %02X %02X \n", nextop, Peek(emu, 0));
                emu->quit=1;
                emu->error |= ERR_UNIMPL;
            }
            break;
        default:
            printf_log(LOG_NONE, "Unimplemented Opcode DE %02X %02X \n", nextop, Peek(emu, 0));
            emu->quit=1;
            emu->error |= ERR_UNIMPL;
    }
}