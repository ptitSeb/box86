#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_TRACE
#include <unistd.h>
#include <sys/syscall.h>
#endif

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86run_private.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "x86run.h"
#include "librarian.h"
#include "elfloader.h"
#include "bridge.h"
#ifdef HAVE_TRACE
#include "x86trace.h"
#endif
#include "x86tls.h"
#include "x86emu.h"

#define PARITY(x)   (((emu->x86emu_parity_tab[(x) / 32] >> ((x) % 32)) & 1) == 0)
#define XOR2(x) 	(((x) ^ ((x)>>1)) & 0x1)

#ifdef ANDROID
void EXPORT my___libc_init(x86emu_t* emu, void* raw_args __unused, void (*onexit)(void) __unused, int (*main)(int, char**, char**), void const * const structors __unused)
{
    //TODO: register fini
    // let's cheat and set all args...
    Push(emu, (uint32_t)my_context->envv);
    Push(emu, (uint32_t)my_context->argv);
    Push(emu, (uint32_t)my_context->argc);

    printf_log(LOG_DEBUG, "Transfert to main(%d, %p, %p)=>%p from __libc_init\n", my_context->argc, my_context->argv, my_context->envv, main);
    // should call structors->preinit_array and structors->init_array!
    // call main and finish
    PushExit(emu);
    R_EIP=(uint32_t)main;

    DynaRun(emu);

    emu->quit = 1; // finished!
}
#else
int32_t EXPORT my___libc_start_main(x86emu_t* emu, int (*main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end))
{
    (void)argc; (void)ubp_av; (void)fini; (void)rtld_fini; (void)stack_end;
    // let's cheat and set all args...
    Push(emu, (uint32_t)my_context->envv);
    Push(emu, (uint32_t)my_context->argv);
    Push(emu, (uint32_t)my_context->argc);
    if(init) {
        PushExit(emu);
        R_EIP=(uint32_t)*init;
        printf_dump(LOG_DEBUG, "Calling init(%p) from __libc_start_main\n", *init);
        DynaRun(emu);
        if(emu->error)  // any error, don't bother with more
            return 0;
        emu->quit = 0;
    } else {
        if(my_context->elfs[0]) {
            printf_dump(LOG_DEBUG, "Calling init from main elf\n");
            RunElfInit(my_context->elfs[0], emu);
        }
    }
    MarkElfInitDone(my_context->elfs[0]);
    printf_log(LOG_DEBUG, "Transfert to main(%d, %p, %p)=>%p from __libc_start_main\n", my_context->argc, my_context->argv, my_context->envv, main);
    // call main and finish
    PushExit(emu);
    R_EIP=(uint32_t)main;

    DynaRun(emu);

    emu->quit = 1; // finished!
    return (int)R_EAX;
}
#endif

const char* GetNativeName(void* p)
{
    static char buff[500] = {0};
    #ifdef HAVE_TRACE
    {
        const char* n = getBridgeName(p);
        if(n)
            return n;
    }
    #endif
    Dl_info info;
    if(dladdr(p, &info)==0) {
        const char *ret = GetNameOffset(my_context->maplib, p);
        if(ret)
            return ret;
        sprintf(buff, "%s(%p)", "???", p);
        return buff;
    } else {
        if(info.dli_sname) {
            strcpy(buff, info.dli_sname);
            if(info.dli_fname) {
                strcat(buff, " ("); strcat(buff, info.dli_fname); strcat(buff, ")");
            }
        } else {
            sprintf(buff, "%s(%s/%p)", "???", info.dli_fname, p);
            return buff;
        }
    }
    return buff;
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
            CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
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
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
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
            CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
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
            CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
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
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
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
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
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
            CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = emu->res | emu->op1;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_rcl8:
            cc = (emu->op1 >> (8 - emu->op2)) & 0x1;
            CONDITIONAL_SET_FLAG(cc, F_CF);
            CONDITIONAL_SET_FLAG(emu->op2 == 1 && XOR2(cc + ((emu->res >> 6) & 0x2)), F_OF);
            break;
        case d_rcl16:
            cc = (emu->op1 >> (16 - emu->op2)) & 0x1;
            CONDITIONAL_SET_FLAG(cc, F_CF);
    		CONDITIONAL_SET_FLAG(emu->op2 == 1 && XOR2(cc + ((emu->res >> 14) & 0x2)), F_OF);
            break;
        case d_rcl32:
            cc = (emu->op1 >> (32 - emu->op2)) & 0x1;
            CONDITIONAL_SET_FLAG(cc, F_CF);
		    CONDITIONAL_SET_FLAG(emu->op2 == 1 && XOR2(cc + ((emu->res >> 30) & 0x2)), F_OF);
            break;
        case d_rcr8:
            cnt = emu->op2 % 9;
            if (cnt == 1) {
                CONDITIONAL_SET_FLAG(emu->op1 & 0x1, F_CF);
    			cc = ACCESS_FLAG(F_CF) != 0;
                CONDITIONAL_SET_FLAG(XOR2(cc + ((emu->op1 >> 6) & 0x2)), F_OF);
            } else {
                CONDITIONAL_SET_FLAG((emu->op1 >> (cnt - 1)) & 0x1, F_CF);
            }
            break;
        case d_rcr16:
            cnt = emu->op2 % 17;
            if (cnt == 1) {
                CONDITIONAL_SET_FLAG(emu->op1 & 0x1, F_CF);
    			cc = ACCESS_FLAG(F_CF) != 0;
                CONDITIONAL_SET_FLAG(XOR2(cc + ((emu->op1 >> 14) & 0x2)), F_OF);
            } else {
                CONDITIONAL_SET_FLAG((emu->op1 >> (cnt - 1)) & 0x1, F_CF);
            }
            break;
        case d_rcr32:
            cnt = emu->op2 % 33;
            if (cnt == 1) {
                CONDITIONAL_SET_FLAG(emu->op1 & 0x1, F_CF);
    			cc = ACCESS_FLAG(F_CF) != 0;
                CONDITIONAL_SET_FLAG(XOR2(cc + ((emu->op1 >> 30) & 0x2)), F_OF);
            } else {
                CONDITIONAL_SET_FLAG((emu->op1 >> (cnt - 1)) & 0x1, F_CF);
            }
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
                CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
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
                    if(emu->op2==1)
                        CLEAR_FLAG(F_OF);
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
                    if(emu->op2==1)
                        CLEAR_FLAG(F_OF);
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
                if(emu->op2==1)
                    CLEAR_FLAG(F_OF);
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
                    CONDITIONAL_SET_FLAG(emu->op1 & 0x80, F_OF);
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
                    CONDITIONAL_SET_FLAG(emu->op1 & 0x8000, F_OF);
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
                CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            }
            if (cnt == 1) {
                CONDITIONAL_SET_FLAG(emu->op1 & 0x80000000, F_OF);
            }
            break;
        case d_shrd32:
            cnt = emu->op2;
            if (cnt > 0) {
                cc = emu->op1 & (1 << (cnt - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG((emu->op1 ^ emu->res) & 0x80000000, F_OF);
                }
            }
            break;
        case d_shld32:
            cnt = emu->op2;
            if (cnt > 0) {
                cc = emu->op1 & (1 << (32 - cnt));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG((emu->op1 ^ emu->res) & 0x80000000, F_OF);
                } else {
                    CLEAR_FLAG(F_OF);
                }
            }
            break;
        case d_sub8:
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & ((~emu->op1) | emu->op2)) | ((~emu->op1) & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub16:
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & ((~emu->op1) | emu->op2)) | ((~emu->op1) & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub32:
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & ((~emu->op1) | emu->op2)) | ((~emu->op1) & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_xor8:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
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
        case d_cmp8:
            CLEAR_FLAG(F_CF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | emu->op2)) | (~emu->op1 & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_cmp16:
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | emu->op2)) | (~emu->op1 & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_cmp32:
        	CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
        	CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
        	CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
        	bc = (emu->res & (~emu->op1 | emu->op2)) | (~emu->op1 & emu->op2);
        	CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
        	CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
        	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_tst8:
        	CLEAR_FLAG(F_OF);
        	CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
        	CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
        	CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
        	CLEAR_FLAG(F_CF);
            break;
        case d_tst16:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            CLEAR_FLAG(F_CF);
            break;
        case d_tst32:
        	CLEAR_FLAG(F_OF);
        	CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
        	CONDITIONAL_SET_FLAG(emu->res == 0, F_ZF);
        	CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
        	CLEAR_FLAG(F_CF);
            break;
        case d_adc8:
            CONDITIONAL_SET_FLAG(emu->res & 0x100, F_CF);
            CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            cc = (emu->op1 & emu->op2) | ((~emu->res) & (emu->op1 | emu->op2));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc16:
            CONDITIONAL_SET_FLAG(emu->res & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            cc = (emu->op1 & emu->op2) | ((~emu->res) & (emu->op1 | emu->op2));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc32:
            if(emu->res == (emu->op1+emu->op2)) {
                lo = (emu->op1 & 0xFFFF) + (emu->op2 & 0xFFFF);
            } else {
                lo = 1 + (emu->op1 & 0xFFFF) + (emu->op2 & 0xFFFF);
            }
            hi = (lo >> 16) + (emu->op1 >> 16) + (emu->op2 >> 16);
            CONDITIONAL_SET_FLAG(hi & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            cc = (emu->op2 & emu->op1) | ((~emu->res) & (emu->op2 | emu->op1));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_sbb8:
            CONDITIONAL_SET_FLAG(emu->res & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | emu->op2)) | (~emu->op1 & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sbb16:
            CONDITIONAL_SET_FLAG(emu->res & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | emu->op2)) | (~emu->op1 & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sbb32:
            CONDITIONAL_SET_FLAG(emu->res & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res & 0xff), F_PF);
            bc = (emu->res & (~emu->op1 | emu->op2)) | (~emu->op1 & emu->op2);
            CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_rol8:
            if(emu->op2 == 1) {
                CONDITIONAL_SET_FLAG((emu->res + (emu->res >> 7)) & 1, F_OF);
            }
        	CONDITIONAL_SET_FLAG(emu->res & 0x1, F_CF);
            break;
        case d_rol16:
            if(emu->op2 == 1) {
                CONDITIONAL_SET_FLAG((emu->res + (emu->res >> 15)) & 1, F_OF);
            }
        	CONDITIONAL_SET_FLAG(emu->res & 0x1, F_CF);
            break;
        case d_rol32:
            if(emu->op2 == 1) {
                CONDITIONAL_SET_FLAG((emu->res + (emu->res >> 31)) & 1, F_OF);
            }
        	CONDITIONAL_SET_FLAG(emu->res & 0x1, F_CF);
            break;
        case d_ror8:
            if(emu->op2 == 1) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res >> 6), F_OF);
            }
            CONDITIONAL_SET_FLAG(emu->res & (1 << 7), F_CF);
            break;
        case d_ror16:
            if(emu->op2 == 1) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res >> 14), F_OF);
            }
            CONDITIONAL_SET_FLAG(emu->res & (1 << 15), F_CF);
            break;
        case d_ror32:
            if(emu->op2 == 1) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res >> 30), F_OF);
            }
            CONDITIONAL_SET_FLAG(emu->res & (1 << 31), F_CF);
            break;

        case d_unknown:
            printf_log(LOG_NONE, "Box86: %p trying to evaluate Unknown defered Flags\n", (void*)R_EIP);
            break;
    }
    RESET_FLAGS(emu);
}

uintptr_t GetSegmentBaseEmu(x86emu_t* emu, int seg)
{
    if(emu->segs_serial[seg] != emu->context->sel_serial) {
        emu->segs_offs[seg] = (uintptr_t)GetSegmentBase(emu->segs[seg]);
        emu->segs_serial[seg] = emu->context->sel_serial;
    }
    return emu->segs_offs[seg];
}


const char* getAddrFunctionName(uintptr_t addr)
{
    static char ret[1000];
    uint32_t sz = 0;
    uintptr_t start = 0;
    elfheader_t* elf = FindElfAddress(my_context, addr);
    const char* symbname = FindNearestSymbolName(elf, (void*)addr, &start, &sz);
    if(symbname && addr>=start && (addr<(start+sz) || !sz)) {
        if(symbname[0]=='\0')
            sprintf(ret, "%s + 0x%x", ElfName(elf), addr - (uintptr_t)GetBaseAddress(elf));
        else if(addr==start)
            sprintf(ret, "%s/%s", ElfName(elf), symbname);
        else
            sprintf(ret, "%s/%s + %d", ElfName(elf), symbname, addr - start);
    } else {
        if(elf) {
            sprintf(ret, "%s + 0x%x", ElfName(elf), addr - (uintptr_t)GetBaseAddress(elf));
        } else
            sprintf(ret, "???");
    }
    return ret;
}

void printFunctionAddr(uintptr_t nextaddr, const char* text)
{
    uint32_t sz = 0;
    uintptr_t start = 0;
    const char* symbname = FindNearestSymbolName(FindElfAddress(my_context, nextaddr), (void*)nextaddr, &start, &sz);
    if(symbname && nextaddr>=start && (nextaddr<(start+sz) || !sz)) {
        if(nextaddr==start)
            printf_log(LOG_NONE, " (%s%s/%s)", text, ElfName(FindElfAddress(my_context, nextaddr)), symbname);
        else
            printf_log(LOG_NONE, " (%s%s/%s + %d)", text, ElfName(FindElfAddress(my_context, nextaddr)), symbname, nextaddr - start);
    }
}

#define PK(a)   (*(uint8_t*)(ip+a))
#define PK32(a)   (*(int32_t*)((uint8_t*)(ip+a)))

uintptr_t evalED(x86emu_t* emu, uintptr_t ip) {
    uint8_t nextop = PK(0);
    if((nextop&0xC0)==0xC0)
        return emu->regs[(nextop&7)].dword[0];
    else {
        ++ip;
        if(!(nextop&0xC0)) {
            if((nextop&7)==4) {
                uint8_t sib = PK(0);
                ++ip;
                uintptr_t base = ((sib&0x7)==5)?(uint32_t)(PK32(0)):(emu->regs[(sib&0x7)].dword[0]);
                base += (emu->sbiidx[(sib>>3)&7]->sdword[0] << (sib>>6)); \
                return *(uintptr_t*)base;
            } else if((nextop&7)==5) {
                return *(uintptr_t*)PK32(0);
            } else {
                return *(uintptr_t*)emu->regs[nextop&7].dword[0];
            }
        } else {
            uintptr_t base;
            if((nextop&7)==4) {
                uint8_t sib = PK(0);
                ++ip;
                base = emu->regs[(sib&0x7)].dword[0];
                base += (emu->sbiidx[(sib>>3)&7]->sdword[0] << (sib>>6));
            } else {
                base = emu->regs[(nextop&0x7)].dword[0];
            } \
            base+=(nextop&0x80)?(*(int32_t*)ip):(*(int8_t*)ip);
            return *(uintptr_t*)base;
        }
    }
}

#ifdef HAVE_TRACE
extern uint64_t start_cnt;

void PrintTrace(x86emu_t* emu, uintptr_t ip, int dynarec)
{
    if(start_cnt) --start_cnt;
    if(!start_cnt && my_context->dec && (
            (trace_end == 0) 
            || ((ip >= trace_start) && (ip < trace_end))) ) {
        int tid = syscall(SYS_gettid);
        mutex_lock(&my_context->mutex_trace);
#ifdef DYNAREC
        if((my_context->trace_tid != tid) || (my_context->trace_dynarec!=dynarec)) {
            printf_log(LOG_NONE, "Thread %04d| (%s) |\n", tid, dynarec?"dyn":"int");
            my_context->trace_tid = tid;
            my_context->trace_dynarec = dynarec;
        }
#else
        if(my_context->trace_tid != tid) {
            printf_log(LOG_NONE, "Thread %04d|\n", tid);
            my_context->trace_tid = tid;
        }
#endif
        printf_log(LOG_NONE, "%s", DumpCPURegs(emu, ip));
        if(PK(0)==0xcc && PK(1)=='S' && PK(2)=='C') {
            uint32_t a = *(uint32_t*)(ip+3);
            if(a==0) {
                printf_log(LOG_NONE, "%p: Exit x86emu\n", (void*)ip);
            } else {
                printf_log(LOG_NONE, "%p: Native call to %p => %s\n", (void*)ip, (void*)a, GetNativeName(*(void**)(ip+7)));
            }
        } else {
            printf_log(LOG_NONE, "%s", DecodeX86Trace(my_context->dec, ip));
            const uint8_t peek = PK(0);
            if(peek==0xC3 || peek==0xC2) {
                printf_log(LOG_NONE, " => %p", *(void**)(R_ESP));
                printFunctionAddr(*(uintptr_t*)(R_ESP), "=> ");
            } else if(peek==0x55) {
                printf_log(LOG_NONE, " => STACK_TOP: %p", *(void**)(R_ESP));
                printFunctionAddr(*(uintptr_t*)(R_ESP), "top:");
                printFunctionAddr(ip, "here: ");
            } else if(peek==0xE8) { // Call
                uintptr_t nextaddr = ip + 5 + PK32(1);
                printFunctionAddr(nextaddr, "=> ");
            } else if(peek==0xFF) {
                const uint8_t pk1 = PK(1);
                if(((pk1>>3)&7)==2 || ((pk1>>3)&7)==4) { // jmp/call near
                    uintptr_t nextaddr = (uintptr_t)getAlternate((void*)evalED(emu, ip+1));
                    printf_log(LOG_NONE, " => %p", (void*)nextaddr);
                    printFunctionAddr(nextaddr, " / ");
                }
            }
            printf_log(LOG_NONE, "\n");
        }
        mutex_unlock(&my_context->mutex_trace);
    }
}

#endif
