#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "x87emu_private.h"
#include "box86context.h"
#include "my_cpuid.h"
#include "bridge.h"
#include "signals.h"
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestF066(x86test_t *test, uintptr_t addr)
#else
uintptr_t RunF066(x86emu_t *emu, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u, tmp8u2;
    int8_t tmp8s;
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u, tmp32u2, tmp32u3;
    int32_t tmp32s, tmp32s2;
    uint64_t tmp64u;
    int64_t tmp64s;
    double d;
    float f;
    int64_t ll;
    sse_regs_t *opex, eax1;
    mmx87_regs_t *opem, eam1;

#ifdef TEST_INTERPRETER
    x86emu_t *emu = test->emu;
#endif
    opcode = F8;

    switch(opcode) {

        case 0x0f:
            opcode = F8;
            switch (opcode) { 
                case 0xB1:                      /* CMPXCHG Ew,Gw */
                    nextop = F8;
                    GET_EW;
#ifdef DYNAREC
                    do {
                        tmp16u = arm_lock_read_h(ED);
                        cmp16(emu, R_AX, tmp16u);
                        if(ACCESS_FLAG(F_ZF)) {
                            tmp32s = arm_lock_write_h(ED, GW.word[0]);
                        } else {
                            R_AX = tmp16u;
                            tmp32s = 0;
                        }
                    } while(tmp32s);
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    GET_EW;
                    cmp16(emu, R_AX, EW->word[0]);
                    if(ACCESS_FLAG(F_ZF)) {
                        EW->word[0] = GW.word[0];
                    } else {
                        R_AX = EW->word[0];
                    }
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
            default:
                return 0;
            }
            break;
        case 0x81:              /* GRP Ed,Iw */
        case 0x83:              /* GRP Ed,Ib */
            nextop = F8;
            GET_EW;
            if(opcode==0x83) {
                tmp16s = F8S;
                tmp16u = (uint32_t)tmp16s;
            } else
                tmp16u = F16;
#ifdef DYNAREC
            switch((nextop>>3)&7) {
                case 0: do { tmp16u2 = arm_lock_read_h(EW);} while(arm_lock_write_h(EW, add16(emu, tmp16u2, tmp16u))); break;
                case 1: do { tmp16u2 = arm_lock_read_h(EW);} while(arm_lock_write_h(EW,  or16(emu, tmp16u2, tmp16u))); break;
                case 2: do { tmp16u2 = arm_lock_read_h(EW);} while(arm_lock_write_h(EW, adc16(emu, tmp16u2, tmp16u))); break;
                case 3: do { tmp16u2 = arm_lock_read_h(EW);} while(arm_lock_write_h(EW, sbb16(emu, tmp16u2, tmp16u))); break;
                case 4: do { tmp16u2 = arm_lock_read_h(EW);} while(arm_lock_write_h(EW, and16(emu, tmp16u2, tmp16u))); break;
                case 5: do { tmp16u2 = arm_lock_read_h(EW);} while(arm_lock_write_h(EW, sub16(emu, tmp16u2, tmp16u))); break;
                case 6: do { tmp16u2 = arm_lock_read_h(EW);} while(arm_lock_write_h(EW, xor16(emu, tmp16u2, tmp16u))); break;
                case 7:                cmp16(emu, ED->dword[0], tmp16u); break;
            }
#else
            pthread_mutex_lock(&emu->context->mutex_lock);
            switch((nextop>>3)&7) {
                case 0: EW->word[0] = add16(emu, EW->word[0], tmp16u); break;
                case 1: EW->word[0] =  or16(emu, EW->word[0], tmp16u); break;
                case 2: EW->word[0] = adc16(emu, EW->word[0], tmp16u); break;
                case 3: EW->word[0] = sbb16(emu, EW->word[0], tmp16u); break;
                case 4: EW->word[0] = and16(emu, EW->word[0], tmp16u); break;
                case 5: EW->word[0] = sub16(emu, EW->word[0], tmp16u); break;
                case 6: EW->word[0] = xor16(emu, EW->word[0], tmp16u); break;
                case 7:               cmp16(emu, EW->word[0], tmp16u); break;
            }
            pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
            break;
        
        case 0xFF:
            nextop = F8;
            GET_EW;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
#ifdef DYNAREC
                    if((uintptr_t)ED&1) { 
                        //meh.
                        do {
                            tmp16u = ED->dword[0];
                            tmp16u &=~0xff;
                            tmp16u |= arm_lock_read_b(ED);
                            tmp16u = inc16(emu, tmp16u);
                        } while(arm_lock_write_b(ED, tmp16u&0xff));
                        ED->dword[0] = tmp16u;
                    } else
                        do {
                            tmp16u = arm_lock_read_h(ED);
                        } while(arm_lock_write_h(ED, inc16(emu, tmp16u)));
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    ED->dword[0] = inc32(emu, ED->dword[0]);
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                case 1:                 /* DEC Ed */
#ifdef DYNAREC
                    if((uintptr_t)ED&1) { 
                        //meh.
                        do {
                            tmp16u = ED->dword[0];
                            tmp16u &=~0xff;
                            tmp16u |= arm_lock_read_b(ED);
                            tmp16u = dec16(emu, tmp16u);
                        } while(arm_lock_write_b(ED, tmp16u&0xff));
                        ED->dword[0] = tmp16u;
                    } else
                        do {
                            tmp16u = arm_lock_read_h(ED);
                        } while(arm_lock_write_h(ED, dec16(emu, tmp16u)));
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    ED->dword[0] = dec32(emu, ED->dword[0]);
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                default:
                    return 0;
            }
            break;

        default:
            return 0;
    }
    return addr;
}
