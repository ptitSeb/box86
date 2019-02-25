#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86run_private.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "x86run.h"

static uint32_t x86emu_parity_tab[8] =
{
	0x96696996,
	0x69969669,
	0x69969669,
	0x96696996,
	0x69969669,
	0x96696996,
	0x96696996,
	0x69969669,
};

#define PARITY(x)   (((x86emu_parity_tab[(x) / 32] >> ((x) % 32)) & 1) == 0)
#define XOR2(x) 	(((x) ^ ((x)>>1)) & 0x1)

int32_t EXPORT my___libc_start_main(x86emu_t* emu, int *(main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end))
{
    //TODO: register rtld_fini
    //TODO: register fini
    if(init) {
        PushExit(emu);
        R_EIP=(uint32_t)*init;
        printf_log(LOG_DEBUG, "Calling init(%p) from __libc_start_main\n", *init);
        Run(emu);
        if(emu->error)  // any error, don't bother with more
            return 0;
        emu->quit = 0;
    }
    // let's cheat and set all args...
    // call main and finish
    Push(emu, (uint32_t)emu->context->envv);
    Push(emu, (uint32_t)emu->context->argv);
    Push(emu, (uint32_t)emu->context->argc);
    PushExit(emu);
    R_EIP=(uint32_t)main;
    printf_log(LOG_DEBUG, "Transfert to main(%d, %p, %p)=>%p from __libc_start_main\n", emu->context->argc, emu->context->argv, emu->context->envv, main);
    return 0;
}

const char* GetNativeName(void* p)
{
    static char buff[500] = {0};
    Dl_info info;
    if(dladdr(p, &info)==0)
        strcpy(buff, "???");
    else {
        if(info.dli_sname) {
            strcpy(buff, info.dli_sname);
            if(info.dli_fname) {
                strcat(buff, " ("); strcat(buff, info.dli_fname); strcat(buff, ")");
            }
        } else
            strcpy(buff, "unknown");
    }
    return buff;
}

uintptr_t pltResolver = ~0;
void PltResolver(x86emu_t* emu, uint32_t id, uintptr_t ofs)
{
    printf_log(LOG_INFO, "PltResolver: Ofs=%p, Id=%d (IP=%p)", (void*)ofs, id, *(void**)(R_ESP));
    emu->quit=1;
}

void UpdateFlags(x86emu_t *emu)
{
    uint32_t cc;
    uint32_t lo, hi;
    uint32_t bc;
    uint32_t cnt;

    switch(emu->df) {
        case d_none:
            return;
        case d_add8:
            CONDITIONAL_SET_FLAG(emu->res & 0x100, F_CF);
            CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);

            cc = (emu->op1 & emu->op2) | ((~emu->res) & (emu->op1 | emu->op2));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add16:
            CONDITIONAL_SET_FLAG(emu->res & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            cc = (emu->op1 & emu->op2) | ((~emu->res) & (emu->op1 | emu->op2));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add32:
            lo = (emu->op2 & 0xFFFF) + (emu->op1 & 0xFFFF);
            hi = (lo >> 16) + (emu->op2 >> 16) + (emu->op1 >> 16);
            CONDITIONAL_SET_FLAG(hi & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffffffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            cc = (emu->op1 & emu->op2) | ((~emu->res) & (emu->op1 | emu->op2));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_and8:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res), F_PF);
            break;
        case d_and16:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            break;
        case d_and32:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            break;
        case d_dec8:
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | 1)) | (~emu->op1 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_dec16:
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | 1)) | (~emu->op1 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_dec32:
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffffffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | 1)) | (~emu->op1 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_inc8:
            CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            cc = ((1 & emu->op1) | (~emu->res)) & (1 | emu->op1);
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_inc16:
            CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            cc = (1 & emu->op1) | ((~emu->res) & (1 | emu->op1));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_inc32:
            CONDITIONAL_SET_FLAG((emu->res & 0xffffffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            cc = (1 & emu->op1) | ((~emu->res) & (1 | emu->op1));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_imul8:
            lo = emu->res & 0xff;
            hi = (emu->res>>8)&0xff;
            if (((lo & 0x80) == 0 && hi == 0x00) ||
                ((lo & 0x80) != 0 && hi == 0xFF)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(lo & 0xff), F_PF);
            break;
        case d_imul16:
            lo = (uint16_t)emu->res;
            hi = (uint16_t)(emu->res >> 16);
            if (((lo & 0x8000) == 0 && hi == 0x00) ||
                ((lo & 0x8000) != 0 && hi == 0xFFFF)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(lo & 0xff), F_PF);
            break;
        case d_imul32:
            if (((emu->res & 0x80000000) == 0 && emu->op1 == 0x00) ||
                ((emu->res & 0x80000000) != 0 && emu->op1 == 0xFFFFFFFF)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            break;
        case d_mul8:
            lo = emu->res & 0xff;
            hi = (emu->res>>8)&0xff;
            if (hi == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(lo & 0xff), F_PF);
            break;
        case d_mul16:
            lo = (uint16_t)emu->res;
            hi = (uint16_t)(emu->res >> 16);
            if (hi == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(lo & 0xff), F_PF);
            break;
        case d_mul32:
            if (emu->op1 == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            break;
        case d_or8:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res), F_PF);
            break;
        case d_or16:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            break;
        case d_or32:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            break;
        case d_neg8:
            CONDITIONAL_SET_FLAG(emu->op1 != 0, F_CF);
            CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res), F_PF);
            bc = emu->res | emu->op1;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_neg16:
            CONDITIONAL_SET_FLAG(emu->op1 != 0, F_CF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = emu->res | emu->op1;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_neg32:
            CONDITIONAL_SET_FLAG(emu->op1 != 0, F_CF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffffffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = emu->res | emu->op1;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_shl8:
            if (emu->op2 < 8) {
                cnt = emu->op2 % 8;
                if (cnt > 0) {
                    cc = emu->op1 & (1 << (8 - cnt));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG((((emu->res & 0x80) == 0x80) ^(ACCESS_FLAG(F_CF) != 0)), F_OF);
                } else {
                    CLEAR_FLAG(F_OF);
                }
            } else {
                CONDITIONAL_SET_FLAG((emu->op1 << (emu->op2-1)) & 0x80, F_CF);
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
                SET_FLAG(F_ZF);
            }
            break;
        case d_shl16:
            if (emu->op2 < 16) {
                cnt = emu->op2 % 16;
                if (cnt > 0) {
                    cc = emu->op1 & (1 << (16 - cnt));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG(((!!(emu->res & 0x8000)) ^(ACCESS_FLAG(F_CF) != 0)), F_OF);
                } else {
                    CLEAR_FLAG(F_OF);
                }
            } else {
                CONDITIONAL_SET_FLAG((emu->op1 << (emu->op2-1)) & 0x8000, F_CF);
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
                SET_FLAG(F_ZF);
            }
            break;
        case d_shl32:
            if (emu->op2 > 0) {
                cc = emu->op1 & (1 << (32 - emu->op2));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG((emu->res & 0xffffffff) == 0, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            }
            if (emu->op2 == 1) {
                CONDITIONAL_SET_FLAG(((!!(emu->res & 0x80000000)) ^
                                        (ACCESS_FLAG(F_CF) != 0)), F_OF);
            } else {
                CLEAR_FLAG(F_OF);
            }
            break;
        case d_sar8:
            if (emu->op2 < 8) {
                if(emu->op2) {
                    cc = emu->op1 & (1 << (emu->op2 - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
                    CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
                }
            } else {
                if (emu->op1&0x80) {
                    SET_FLAG(F_CF);
                    CLEAR_FLAG(F_ZF);
                    SET_FLAG(F_SF);
                    SET_FLAG(F_PF);
                } else {
                    CLEAR_FLAG(F_CF);
                    SET_FLAG(F_ZF);
                    CLEAR_FLAG(F_SF);
                    SET_FLAG(F_PF);
                }
            }
            break;
        case d_sar16:
            if (emu->op2 < 16) {
                if(emu->op2) {
                    cc = emu->op1 & (1 << (emu->op2 - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
                }
            } else {
                if (emu->op1&0x8000) {
                    SET_FLAG(F_CF);
                    CLEAR_FLAG(F_ZF);
                    SET_FLAG(F_SF);
                    SET_FLAG(F_PF);
                } else {
                    CLEAR_FLAG(F_CF);
                    SET_FLAG(F_ZF);
                    CLEAR_FLAG(F_SF);
                    SET_FLAG(F_PF);
                }
            }
            break;
        case d_sar32:
            if(emu->op2) {
                cc = emu->op1 & (1 << (emu->op2 - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            }
            break;
        case d_shr8:
            if (emu->op2 < 8) {
                cnt = emu->op2 % 8;
                if (cnt > 0) {
                    cc = emu->op1 & (1 << (cnt - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG(XOR2(emu->res >> 6), F_OF);
                }
            } else {
                CONDITIONAL_SET_FLAG((emu->op1 >> (emu->op2-1)) & 0x1, F_CF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
                SET_FLAG(F_ZF);
            }
            break;
        case d_shr16:
            if (emu->op2 < 16) {
                cnt = emu->op2 % 16;
                if (cnt > 0) {
                    cc = emu->op1 & (1 << (cnt - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG(XOR2(emu->res >> 14), F_OF);
                }
            } else {
                CLEAR_FLAG(F_CF);
                SET_FLAG(F_ZF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
            }
            break;
        case d_shr32:
            cnt = emu->op2;
            if (cnt > 0) {
                cc = emu->op1 & (1 << (cnt - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG((emu->res & 0xffffffff) == 0, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            }
            if (cnt == 1) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res >> 30), F_OF);
            }
            break;
        case d_sub8:
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | emu->op2)) | (~emu->op1 & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub16:
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | emu->op2)) | (~emu->op1 & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub32:
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffffffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | emu->op2)) | (~emu->op1 & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_xor8:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res), F_PF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            break;
        case d_xor16:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            break;
        case d_xor32:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            break;
    }
    RESET_FLAGS(emu);
}


void PackFlags(x86emu_t* emu)
{
    #define GO(A) emu->packed_eflags.f.F__##A = emu->flags[F_##A];
    GO(CF);
    GO(res1);
    GO(PF);
    GO(res2);
    GO(AF);
    GO(res3);
    GO(ZF);
    GO(SF);
    GO(TF);
    GO(IF);
    GO(DF);
    GO(OF);
    GO(IOPL);
    GO(NT);
    GO(dummy);
    GO(RF);
    GO(VM);
    GO(AC);
    GO(VIF);
    GO(VIP);
    GO(ID);
    #undef GO
}
void UnpackFlags(x86emu_t* emu)
{
    #define GO(A) emu->flags[F_##A] = emu->packed_eflags.f.F__##A;
    GO(CF);
    GO(res1);
    GO(PF);
    GO(res2);
    GO(AF);
    GO(res3);
    GO(ZF);
    GO(SF);
    GO(TF);
    GO(IF);
    GO(DF);
    GO(OF);
    GO(IOPL);
    GO(NT);
    GO(dummy);
    GO(RF);
    GO(VM);
    GO(AC);
    GO(VIF);
    GO(VIP);
    GO(ID);
    #undef GO
}
