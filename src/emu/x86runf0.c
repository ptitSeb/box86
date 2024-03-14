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
uintptr_t TestF0(x86test_t *test, uintptr_t addr)
#else
uintptr_t RunF0(x86emu_t *emu, uintptr_t addr)
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

#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
        #define GO(B, OP)                      \
        case B+0: \
            nextop = F8;               \
            GET_EB;             \
            do {                \
            tmp8u = arm_lock_read_b(EB);     \
            tmp8u = OP##8(emu, tmp8u, GB);  \
            } while (arm_lock_write_b(EB, tmp8u));   \
            break;                              \
        case B+1: \
            nextop = F8;               \
            GET_ED;             \
            do {                \
            tmp32u = arm_lock_read_d(ED);     \
            tmp32u = OP##32(emu, tmp32u, GD.dword[0]);  \
            } while (arm_lock_write_d(ED, tmp32u));   \
            break;                              \
        case B+2: \
            nextop = F8;               \
            GET_EB;                   \
            GB = OP##8(emu, GB, EB->byte[0]); \
            break;                              \
        case B+3: \
            nextop = F8;               \
            GET_ED;         \
            GD.dword[0] = OP##32(emu, GD.dword[0], ED->dword[0]); \
            break;                              \
        case B+4: \
            R_AL = OP##8(emu, R_AL, F8); \
            break;                              \
        case B+5: \
            R_EAX = OP##32(emu, R_EAX, F32); \
            break;
#else
        #define GO(B, OP)                      \
        case B+0: \
            nextop = F8;               \
            GET_EB;             \
            pthread_mutex_lock(&emu->context->mutex_lock);\
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);  \
            pthread_mutex_unlock(&emu->context->mutex_lock);\
            break;                              \
        case B+1: \
            nextop = F8;               \
            GET_ED;             \
            pthread_mutex_lock(&emu->context->mutex_lock);\
            ED->dword[0] = OP##32(emu, ED->dword[0], GD.dword[0]); \
            pthread_mutex_unlock(&emu->context->mutex_lock);\
            break;                              \
        case B+2: \
            nextop = F8;               \
            GET_EB;                   \
            pthread_mutex_lock(&emu->context->mutex_lock);\
            GB = OP##8(emu, GB, EB->byte[0]); \
            pthread_mutex_unlock(&emu->context->mutex_lock);\
            break;                              \
        case B+3: \
            nextop = F8;               \
            GET_ED;         \
            pthread_mutex_lock(&emu->context->mutex_lock);\
            GD.dword[0] = OP##32(emu, GD.dword[0], ED->dword[0]); \
            pthread_mutex_unlock(&emu->context->mutex_lock);\
            break;                              \
        case B+4: \
            pthread_mutex_lock(&emu->context->mutex_lock);\
            R_AL = OP##8(emu, R_AL, F8); \
            pthread_mutex_unlock(&emu->context->mutex_lock);\
            break;                              \
        case B+5: \
            pthread_mutex_lock(&emu->context->mutex_lock);\
            R_EAX = OP##32(emu, R_EAX, F32); \
            pthread_mutex_unlock(&emu->context->mutex_lock);\
            break;
#endif
        GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
        GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
        GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
        GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
        GO(0x20, and)                   /* AND 0x20 -> 0x25 */
        GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
        GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
        #undef GO

        case 0x0f:
            opcode = F8;
            switch (opcode) { 
                case 0xB0:                      /* CMPXCHG Eb,Gb */
                    CHECK_FLAGS(emu);
                    nextop = F8;
                    GET_EB;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp8u = arm_lock_read_b(EB);
                        cmp8(emu, R_AL, tmp8u);
                        if(ACCESS_FLAG(F_ZF)) {
                            tmp32s = arm_lock_write_b(EB, GB);
                        } else {
                            R_AL = tmp8u;
                            tmp32s = 0;
                        }
                    } while(tmp32s);
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    cmp8(emu, R_AL, EB->byte[0]);
                    if(ACCESS_FLAG(F_ZF)) {
                        EB->byte[0] = GB;
                    } else {
                        R_AL = EB->byte[0];
                    }
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                case 0xB1:                      /* CMPXCHG Ed,Gd */
                    nextop = F8;
                    GET_ED;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(((uintptr_t)ED)&3) {
                        do {
                            tmp32u = ED->dword[0] & ~0xff;
                            tmp32u |= arm_lock_read_b(ED);
                            cmp32(emu, R_EAX, tmp32u);
                            if(ACCESS_FLAG(F_ZF)) {
                                tmp32s = arm_lock_write_b(ED, GD.dword[0] & 0xff);
                                if(!tmp32s)
                                    ED->dword[0] = GD.dword[0];
                            } else {
                                R_EAX = tmp32u;
                                tmp32s = 0;
                            }
                        } while(tmp32s);
                    } else {
                        do {
                            tmp32u = arm_lock_read_d(ED);
                            cmp32(emu, R_EAX, tmp32u);
                            if(ACCESS_FLAG(F_ZF)) {
                                tmp32s = arm_lock_write_d(ED, GD.dword[0]);
                            } else {
                                R_EAX = tmp32u;
                                tmp32s = 0;
                            }
                        } while(tmp32s);
                    }
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    cmp32(emu, R_EAX, ED->dword[0]);
                    if(ACCESS_FLAG(F_ZF)) {
                        ED->dword[0] = GD.dword[0];
                    } else {
                        R_EAX = ED->dword[0];
                    }
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                case 0xB3:                      /* BTR Ed,Gd */
                    CHECK_FLAGS(emu);
                    nextop = F8;
                    GET_ED;
                    tmp8u = GD.byte[0];
                    if((nextop&0xC0)!=0xC0)
                    {
                        #ifdef TEST_INTERPRETER
                        test->memaddr=((test->memaddr)+(tmp8u>>5));
                        *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                        #else
                        ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                        #endif
                    }
                    tmp8u&=31;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp32u = arm_lock_read_d(ED);
                        if(tmp32u & (1<<tmp8u)) {
                            SET_FLAG(F_CF);
                            tmp32u ^= (1<<tmp8u);
                            tmp32s = arm_lock_write_d(ED, tmp32u);
                        } else {
                            CLEAR_FLAG(F_CF);
                            tmp32s = 0;
                        }
                    } while(tmp32s);
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    if(ED->dword[0] & (1<<tmp8u)) {
                        SET_FLAG(F_CF);
                        ED->dword[0] ^= (1<<tmp8u);
                    } else
                        CLEAR_FLAG(F_CF);
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                case 0xBA:                      
                    nextop = F8;
                    switch((nextop>>3)&7) {
                        case 4:                 /* BT Ed,Ib */
                            CHECK_FLAGS(emu);
                            GET_ED;
                            tmp8u = F8;
                            if((nextop&0xC0)!=0xC0)
                            {
                                #ifdef TEST_INTERPRETER
                                test->memaddr=((test->memaddr)+(tmp8u>>5));
                                *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                                #else
                                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                                #endif
                            }
                            tmp8u&=31;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                            if(arm_lock_read_d(ED) & (1<<tmp8u))
                                SET_FLAG(F_CF);
                            else
                                CLEAR_FLAG(F_CF);
#else
                            pthread_mutex_lock(&emu->context->mutex_lock);
                            if(ED->dword[0] & (1<<tmp8u))
                                SET_FLAG(F_CF);
                            else
                                CLEAR_FLAG(F_CF);
                            pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                            break;
                        case 5:             /* BTS Ed, Ib */
                            CHECK_FLAGS(emu);
                            GET_ED;
                            tmp8u = F8;
                            if((nextop&0xC0)!=0xC0)
                            {
                                #ifdef TEST_INTERPRETER
                                test->memaddr=((test->memaddr)+(tmp8u>>5));
                                *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                                #else
                                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                                #endif
                            }
                            tmp8u&=31;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                            do {
                                tmp32u = arm_lock_read_d(ED);
                                if(tmp32u & (1<<tmp8u)) {
                                    SET_FLAG(F_CF);
                                    tmp32s = 0;
                                } else {
                                    CLEAR_FLAG(F_CF);
                                    tmp32u ^= (1<<tmp8u);
                                    tmp32s = arm_lock_write_d(ED, tmp32u);
                                }
                            } while(tmp32s);
#else
                            pthread_mutex_lock(&emu->context->mutex_lock);
                            if(ED->dword[0] & (1<<tmp8u)) {
                                SET_FLAG(F_CF);
                            } else {
                                CLEAR_FLAG(F_CF);
                                ED->dword[0] ^= (1<<tmp8u);
                            }
                            pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                            break;
                        case 6:             /* BTR Ed, Ib */
                            CHECK_FLAGS(emu);
                            GET_ED;
                            tmp8u = F8;
                            if((nextop&0xC0)!=0xC0)
                            {
                                #ifdef TEST_INTERPRETER
                                test->memaddr=((test->memaddr)+(tmp8u>>5));
                                *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                                #else
                                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                                #endif
                            }
                            tmp8u&=31;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                            do {
                                tmp32u = arm_lock_read_d(ED);
                                if(tmp32u & (1<<tmp8u)) {
                                    SET_FLAG(F_CF);
                                    tmp32u ^= (1<<tmp8u);
                                    tmp32s = arm_lock_write_d(ED, tmp32u);
                                } else {
                                    CLEAR_FLAG(F_CF);
                                    tmp32s = 0;
                                }
                            } while(tmp32s);
#else
                            pthread_mutex_lock(&emu->context->mutex_lock);
                            if(ED->dword[0] & (1<<tmp8u)) {
                                SET_FLAG(F_CF);
                                ED->dword[0] ^= (1<<tmp8u);
                            } else
                                CLEAR_FLAG(F_CF);
                            pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                            break;

                        default:
                            return addr-3; //unfetchall 0F BA nextop but not F0, continue normal without LOCK
                    }
                    break;
                case 0xBB:                      /* BTC Ed,Gd */
                    CHECK_FLAGS(emu);
                    nextop = F8;
                    GET_ED;
                    tmp8u = GD.byte[0];
                    if((nextop&0xC0)!=0xC0)
                    {
                        #ifdef TEST_INTERPRETER
                        test->memaddr=((test->memaddr)+(tmp8u>>5));
                        *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                        #else
                        ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                        #endif
                    }
                    tmp8u&=31;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp32u = arm_lock_read_d(ED);
                        if(tmp32u & (1<<tmp8u)) {
                            SET_FLAG(F_CF);
                        } else {
                            CLEAR_FLAG(F_CF);
                        }
                        tmp32u ^= (1<<tmp8u);
                    } while(arm_lock_write_d(ED, tmp32u));
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    if(ED->dword[0] & (1<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                    ED->dword[0] ^= (1<<tmp8u);
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                case 0xC0:                      /* XADD Gb,Eb */
                    nextop = F8;
                    GET_EB;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp8u = arm_lock_read_b(EB);
                        tmp8u2 = add8(emu, tmp8u, GB);
                    } while (arm_lock_write_b(EB, tmp8u2));
                    GB = tmp8u;
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    tmp8u = add8(emu, EB->byte[0], GB);
                    GB = EB->byte[0];
                    EB->byte[0] = tmp8u;
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                case 0xC1:                      /* XADD Gd,Ed */
                    nextop = F8;
                    GET_ED;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(((uintptr_t)ED)&3) {
                        do {
                            tmp32u = ED->dword[0] & ~0xff;
                            tmp32u |= arm_lock_read_b(ED);
                            tmp32u2 = add32(emu, tmp32u, GD.dword[0]);
                        } while(arm_lock_write_b(ED, tmp32u2&0xff));
                        ED->dword[0] = tmp32u2;
                    } else {
                        do {
                            tmp32u = arm_lock_read_d(ED);
                            tmp32u2 = add32(emu, tmp32u, GD.dword[0]);
                        } while(arm_lock_write_d(ED, tmp32u2));
                    }
                    GD.dword[0] = tmp32u;
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    tmp32u = add32(emu, ED->dword[0], GD.dword[0]);
                    GD.dword[0] = ED->dword[0];
                    ED->dword[0] = tmp32u;
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                case 0xC7:                      /* CMPXCHG8B Gq */
                    nextop = F8;
                    GET_ED8;
                    switch((nextop>>3)&7) {
                        case 1:
                            CHECK_FLAGS(emu);
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                            if(((uintptr_t)ED)&7) {
                                // unaligned!!!
                                uint64_t ref = R_EDX;
                                ref = ref<<32 | R_EAX;
                                void* p = (void*)(((uintptr_t)ED + 3)&~3);
                                int d = (((uintptr_t)p - (uintptr_t)ED))*8;
                                do {
                                    uint64_t m = *(uint64_t*)ED;
                                    m&=~((0xffffffffLL)<<d);
                                    tmp32u = arm_lock_read_d(p);
                                    m|=((uint64_t)tmp32u)<<d;
                                    if(ref == m) {
                                        SET_FLAG(F_ZF);
                                        m = ((uint64_t)R_ECX)<<32 | R_EBX;
                                        tmp32u2 = m>>d;
                                        tmp32s = arm_lock_write_d(p, tmp32u2);
                                        if(!tmp32s)
                                            *(uint64_t*)ED = m;
                                    } else {
                                        CLEAR_FLAG(F_ZF);
                                        R_EAX = m&0xffffffff;
                                        R_EDX = m>>32;
                                        tmp32s = 0;
                                    }
                                } while(tmp32s);
                            } else
                            do {
                                arm_lock_read_dd(&tmp32u, &tmp32u2, ED);
                                if(R_EAX == tmp32u && R_EDX == tmp32u2) {
                                    SET_FLAG(F_ZF);
                                    tmp32s = arm_lock_write_dd(R_EBX, R_ECX, ED);
                                } else {
                                    CLEAR_FLAG(F_ZF);
                                    R_EAX = tmp32u;
                                    R_EDX = tmp32u2;
                                    tmp32s = 0;
                                }
                            } while(tmp32s);
#else
                            pthread_mutex_lock(&emu->context->mutex_lock);
                            tmp32u = ED->dword[0];
                            tmp32u2= ED->dword[1];
                            if(R_EAX == tmp32u && R_EDX == tmp32u2) {
                                SET_FLAG(F_ZF);
                                ED->dword[0] = R_EBX;
                                ED->dword[1] = R_ECX;
                            } else {
                                CLEAR_FLAG(F_ZF);
                                R_EAX = tmp32u;
                                R_EDX = tmp32u2;
                            }
                            pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                            break;
                        default:
                            printf_log(LOG_NONE, "Illegal Opcode 0xF0 0xC7 0x%02X 0x%02X\n", nextop, PK(0));
                                emu->quit=1;
                                emu->error |= ERR_ILLEGAL;
                            return 0;
                    }
                    break;
                default:
                    // trigger invalid lock?
                    return addr-2; // unfetch 0F and nextop but not F0
            }
            break;

        case 0x66:
            #ifdef TEST_INTERPRETER
            return TestF066(test, addr);
            #else
            return RunF066(emu, addr);
            #endif

        case 0x81:              /* GRP Ed,Id */
        case 0x83:              /* GRP Ed,Ib */
            nextop = F8;
            GET_ED;
            if(opcode==0x83) {
                tmp32s = F8S;
                tmp32u = (uint32_t)tmp32s;
            } else
                tmp32u = F32;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            switch((nextop>>3)&7) {
                case 0: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, add32(emu, tmp32u2, tmp32u))); break;
                case 1: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED,  or32(emu, tmp32u2, tmp32u))); break;
                case 2: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, adc32(emu, tmp32u2, tmp32u))); break;
                case 3: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, sbb32(emu, tmp32u2, tmp32u))); break;
                case 4: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, and32(emu, tmp32u2, tmp32u))); break;
                case 5: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, sub32(emu, tmp32u2, tmp32u))); break;
                case 6: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, xor32(emu, tmp32u2, tmp32u))); break;
                case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
            }
#else
            pthread_mutex_lock(&emu->context->mutex_lock);
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp32u); break;
                case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp32u); break;
                case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp32u); break;
                case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp32u); break;
                case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp32u); break;
                case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp32u); break;
                case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
            }
            pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
            break;
        case 0x86:                      /* XCHG Eb,Gb */
        case 0x87:                      /* XCHG Ed,Gd */
            return addr-1;   // let the normal XCHG execute, it have integrated LOCK

        case 0xFE:              /* GRP 5 Eb */
            nextop = F8;
            GET_EB;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp8u = arm_lock_read_b(EB);
                    } while(arm_lock_write_b(EB, inc8(emu, tmp8u)));
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    EB->byte[0] = inc8(emu, EB->byte[0]);
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                case 1:                 /* DEC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp8u = arm_lock_read_b(EB);
                    } while(arm_lock_write_d(EB, dec8(emu, tmp8u)));
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    EB->byte[0] = dec8(emu, EB->byte[0]);
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                default:
                    printf_log(LOG_NONE, "Illegal Opcode 0xF0 0xFE 0x%02X 0x%02X\n", nextop, PK(0));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    return 0;
            }
            break;
        case 0xFF:              /* GRP 5 Ed */
            nextop = F8;
            GET_ED;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if((uintptr_t)ED&3) { 
                        //meh.
                        do {
                            tmp32u = ED->dword[0];
                            tmp32u &=~0xff;
                            tmp32u |= arm_lock_read_b(ED);
                            tmp32u = inc32(emu, tmp32u);
                        } while(arm_lock_write_b(ED, tmp32u&0xff));
                        ED->dword[0] = tmp32u;
                    } else
                        do {
                            tmp32u = arm_lock_read_d(ED);
                        } while(arm_lock_write_d(ED, inc32(emu, tmp32u)));
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    ED->dword[0] = inc32(emu, ED->dword[0]);
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                case 1:                 /* DEC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if((uintptr_t)ED&3) { 
                        //meh.
                        do {
                            tmp32u = ED->dword[0];
                            tmp32u &=~0xff;
                            tmp32u |= arm_lock_read_b(ED);
                            tmp32u = dec32(emu, tmp32u);
                        } while(arm_lock_write_b(ED, tmp32u&0xff));
                        ED->dword[0] = tmp32u;
                    } else
                        do {
                            tmp32u = arm_lock_read_d(ED);
                        } while(arm_lock_write_d(ED, dec32(emu, tmp32u)));
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    ED->dword[0] = dec32(emu, ED->dword[0]);
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;
                default:
                    printf_log(LOG_NONE, "Illegal Opcode 0xF0 0xFF 0x%02X 0x%02X\n", nextop, PK(0));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    return 0;
            }
            break;

        default:
            return 0;
    }
    return addr;
}
