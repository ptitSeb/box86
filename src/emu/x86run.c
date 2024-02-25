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
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

int my_setcontext(x86emu_t* emu, void* ucp);

#ifdef TEST_INTERPRETER
int RunTest(x86test_t *test)
#else
int Run(x86emu_t *emu, int step)
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
    x86emu_t* emu = test->emu;
    int step = 0;
    #endif
    #ifndef NOALIGN
    int is_nan;
    #endif
    uintptr_t addr = R_EIP;
    int rep;    // 0 none, 1=F2 prefix, 2=F3 prefix
    int unimp = 0;
    int tf_next = 0;

    if(emu->quit)
        return 0;
    if(addr==0) {
    //    emu->quit = 1;
    //    printf_log(LOG_INFO, "%04d|Ask to run at NULL, quit silently\n", GetTID());
    //    print_cycle_log(LOG_INFO);
    //    return 0;
        // Some program, like C# Vara.exe, need to trigger that segfault to actually run... (ticket #830à)
        printf_log(LOG_INFO, "%04d|Ask to run at NULL, will segfault\n", GetTID());
    }
    //ref opcode: http://ref.x64asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86 (%p), RIP=%p, Stack=%p\n", emu, (void*)addr, (void*)R_ESP);

x86emurun:
#ifndef TEST_INTERPRETER
    while(1) 
#endif
    {
#if defined(HAVE_TRACE)
        __builtin_prefetch((void*)addr, 0, 0); 
        emu->prev2_ip = emu->old_ip;
        if(my_context->dec && (
            (trace_end == 0) 
            || ((addr >= trace_start) && (addr < trace_end))) )
                PrintTrace(emu, addr, 0);
#endif
        emu->old_ip = addr;

        opcode = F8;
        
        rep = 0;
        while((opcode==0xF2) || (opcode==0xF3) || (opcode==0x3E)) {
            switch (opcode) {
                case 0xF2: rep = 1; break;
                case 0xF3: rep = 2; break;
                case 0x3E: /* ignored*/ break;
            }
            opcode = F8;
        }

        switch(opcode) {

        #define GO(B, OP)                               \
        case B+0:                                       \
            nextop = F8;                                \
            GET_EB;                                     \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);  \
            break;                                      \
        case B+1:                                       \
            nextop = F8;                                \
            GET_ED;                                     \
            ED->dword[0] = OP##32(emu, ED->dword[0], GD.dword[0]); \
            break;                                      \
        case B+2:                                       \
            nextop = F8;                                \
            GET_EB;                                     \
            GB = OP##8(emu, GB, EB->byte[0]);           \
            break;                                      \
        case B+3:                                       \
            nextop = F8;                                \
            GET_ED;                                     \
            GD.dword[0] = OP##32(emu, GD.dword[0], ED->dword[0]); \
            break;                                      \
        case B+4:                                       \
            R_AL = OP##8(emu, R_AL, F8);                \
            break;                                      \
        case B+5:                                       \
            R_EAX = OP##32(emu, R_EAX, F32);            \
            break;


        GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
        GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
        GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
        GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
        GO(0x20, and)                   /* AND 0x20 -> 0x25 */
        GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
        GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
        //GO(0x38, cmp)                   /* CMP 0x38 -> 0x3D */    avoid affectation

        #undef GO
        case 0x38:
            nextop = F8;
            GET_EB;
            cmp8(emu, EB->byte[0], GB);
            break;
        case 0x39:
            nextop = F8;
            GET_ED;
            cmp32(emu, ED->dword[0], GD.dword[0]);
            break;
        case 0x3A:
            nextop = F8;
            GET_EB;
            cmp8(emu, GB, EB->byte[0]);
            break;
        case 0x3B:
            nextop = F8;
            GET_ED;
            cmp32(emu, GD.dword[0], ED->dword[0]);
            break;
        case 0x3C:
            cmp8(emu, R_AL, F8);
            break;
        case 0x3D:
            cmp32(emu, R_EAX, F32);
            break;

        case 0x06:                      /* PUSH ES */
            Push(emu, emu->segs[_ES]);    // even if a segment is a 16bits, a 32bits push/pop is done
            break;
        case 0x07:                      /* POP ES */
            emu->segs[_ES] = Pop(emu);    // no check, no use....
            emu->segs_serial[_ES] = 0;
            break;

        case 0x0E:                      /* PUSH CS */
            Push(emu, emu->segs[_CS]);
            break;
        case 0x0F:                      /* More instructions */
            switch(rep) {
                case 1:
                    #ifdef TEST_INTERPRETER 
                    if(!(addr = TestF20F(test, addr, &step)))
                        unimp = 1;
                    #else
                    if(!(addr = RunF20F(emu, addr, &step))) {
                        unimp = 1;
                        goto fini;
                    }
                    if(step==2) STEP2;
                    #endif
                    break;
                case 2:
                    #ifdef TEST_INTERPRETER 
                    if(!(addr = TestF30F(test, addr)))
                        unimp = 1;
                    #else
                    if(!(addr = RunF30F(emu, addr))) {
                        unimp = 1;
                        goto fini;
                    }
                    #endif
                    break;
                default:
                    #ifdef TEST_INTERPRETER 
                    if(!(addr = Test0F(test, addr, &step)))
                        unimp = 1;
                    #else
                    if(!(addr = Run0F(emu, addr, &step))) {
                        unimp = 1;
                        goto fini;
                    }
                    if(step==2) STEP2;
                    #endif
                    break;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            break;

        case 0x16:                      /* PUSH SS */
            Push(emu, emu->segs[_SS]);    // even if a segment is a 16bits, a 32bits push/pop is done
            break;
        case 0x17:                      /* POP SS */
            emu->segs[_SS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_SS] = 0;
            break;

        case 0x1E:                      /* PUSH DS */
            Push(emu, emu->segs[_DS]);  // even if a segment is a 16bits, a 32bits push/pop is done
            break;
        case 0x1F:                      /* POP DS */
            emu->segs[_DS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_DS] = 0;
            break;

        case 0x26:                      /* ES: */
            break;  //ignored...
        case 0x27:                      /* DAA */
            R_AL = daa8(emu, R_AL);
            break;

        case 0x2E:                      /* CS: */
            // Can also happens before a condition branch: Branch Not Taken Hint
            break;  //ignored...
        case 0x2F:                      /* DAS */
            R_AL = das8(emu, R_AL);
            break;

        case 0x36:                      /* SS: */
            break;
        case 0x37:                      /* AAA */
            R_AX = aaa16(emu, R_AX);
            break;

        case 0x3E:                      /* DS: */
            // Can also happens before a condition branch: Branch Taken Hint
            break;
        case 0x3F:                      /* AAS */
            R_AX = aas16(emu, R_AX);
            break;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:                      /* INC Reg */
            tmp8u = opcode&7;
            emu->regs[tmp8u].dword[0] = inc32(emu, emu->regs[tmp8u].dword[0]);
            break;
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:                      /* DEC Reg */
            tmp8u = opcode&7;
            emu->regs[tmp8u].dword[0] = dec32(emu, emu->regs[tmp8u].dword[0]);
            break;
        case 0x54:                      /* PUSH ESP */
            tmp32u = R_ESP;
            Push(emu, tmp32u);
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x55:
        case 0x56:
        case 0x57:                      /* PUSH Reg */
            tmp8u = opcode&7;
            Push(emu, emu->regs[tmp8u].dword[0]);
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:                      /* POP ESP */
        case 0x5D:
        case 0x5E:
        case 0x5F:                      /* POP Reg */
            tmp8u = opcode&7;
            emu->regs[tmp8u].dword[0] = Pop(emu);
            break;
        case 0x60:                      /* PUSHAD */
            tmp32u = R_ESP;
            Push(emu, R_EAX);
            Push(emu, R_ECX);
            Push(emu, R_EDX);
            Push(emu, R_EBX);
            Push(emu, tmp32u);
            Push(emu, R_EBP);
            Push(emu, R_ESI);
            Push(emu, R_EDI);
            break;
        case 0x61:                      /* POPAD */
            R_EDI = Pop(emu);
            R_ESI = Pop(emu);
            R_EBP = Pop(emu);
            R_ESP+=4;   // POP ESP
            R_EBX = Pop(emu);
            R_EDX = Pop(emu);
            R_ECX = Pop(emu);
            R_EAX = Pop(emu);
            break;
        case 0x62:                      /* BOUND Gd, Ed */
            nextop = F8;
            GET_ED;
            // ignoring the test for now
            break;
        case 0x63:                      /* ARPL Ew, Gw */
            nextop = F8;
            GET_EW;
            // faking to always happy...
            SET_FLAG(F_ZF);
            break;
        case 0x64:                      /* FS: */
            #ifdef TEST_INTERPRETER
            if(!(addr = Test64(test, _FS, addr)))
                unimp = 1;
            #else
            if(!(addr = Run64(emu, _FS, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0x65:                      /* GS: */
            #ifdef TEST_INTERPRETER
            if(!(addr = Test64(test, _GS, addr)))
                unimp = 1;
            #else
            if(!(addr = Run64(emu, _GS, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0x66:                      /* Prefix to change width of intructions, so here, down to 16bits */
            #ifdef TEST_INTERPRETER
            if(!(addr = Test66(test, rep, addr)))
                unimp = 1;
            #else
            if(!(addr = Run66(emu, rep, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0x67:                      /* Prefix to change width of registers */
            #ifdef TEST_INTERPRETER
            if(!(addr = Test67(test, rep, addr)))
                unimp = 1;
            #else
            if(!(addr = Run67(emu, rep, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0x68:                      /* Push Id */
            Push(emu, F32);
            break;
        case 0x69:                      /* IMUL Gd,Ed,Id */
            nextop = F8;
            GET_ED;
            tmp32u = F32;
            GD.dword[0] = imul32(emu, ED->dword[0], tmp32u);
            break;
        case 0x6A:                      /* Push Ib */
            tmp32s = F8S;
            Push(emu, (uint32_t)tmp32s);
            break;
        case 0x6B:                      /* IMUL Gd,Ed,Ib */
            nextop = F8;
            GET_ED;
            tmp32s = F8S;
            GD.dword[0] = imul32(emu, ED->dword[0], (uint32_t)tmp32s);
            break;
        case 0x6C:                      /* INSB */
        case 0x6D:                      /* INSD */
        case 0x6E:                      /* OUTSB */
        case 0x6F:                      /* OUTSD */
            #ifndef TEST_INTERPRETER
            // this is a privilege opcode
            emit_signal(emu, SIGSEGV, (void*)R_EIP, 128);
            STEP;
            #endif
            break;

        #define GOCOND(BASE, PREFIX, CONDITIONAL) \
        case BASE+0x00:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_OF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x01:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_OF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x02:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_CF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x03:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_CF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x04:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x05:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x06:                         \
            PREFIX                              \
            if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x07:                         \
            PREFIX                              \
            if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x08:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x09:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_SF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0A:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_PF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0B:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_PF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0C:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0D:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0E:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0F:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            break;
        GOCOND(0x70
            ,   tmp8s = F8S; CHECK_FLAGS(emu);
            ,   addr += tmp8s;
            )                           /* Jxx Ib */
        #undef GOCOND

        
        case 0x80:                      /* GRP Eb,Ib */
        case 0x82:                      // 0x82 and 0x80 are the same opcodes it seems?
            nextop = F8;
            GET_EB;
            tmp8u = F8;
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = add8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] =  or8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = adc8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = sbb8(emu, EB->byte[0], tmp8u); break;
                case 4: EB->byte[0] = and8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = sub8(emu, EB->byte[0], tmp8u); break;
                case 6: EB->byte[0] = xor8(emu, EB->byte[0], tmp8u); break;
                case 7:               cmp8(emu, EB->byte[0], tmp8u); break;
            }
            break;
        case 0x81:                      /* GRP Ed,Id */
        case 0x83:                      /* GRP Ed,Ib */
            nextop = F8;
            GET_ED;
            tmp32s = (opcode==0x83)?(F8S):(F32S);
            tmp32u = (uint32_t)tmp32s;
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
            break;
        case 0x84:                      /* TEST Eb,Gb */
            nextop = F8;
            GET_EB;
            test8(emu, EB->byte[0], GB);
            break;
        case 0x85:                      /* TEST Ed,Gd */
            nextop = F8;
            GET_ED;
            test32(emu, ED->dword[0], GD.dword[0]);
            break;
        case 0x86:                      /* XCHG Eb,Gb */
            nextop = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            GET_EB;
            if(MODREG) { // reg / reg: no lock
                tmp8u = GB;
                GB = EB->byte[0];
                EB->byte[0] = tmp8u;
            } else {
                do {
                    tmp8u = arm_lock_read_b(EB);
                } while(arm_lock_write_b(EB, GB));
                GB = tmp8u;
            }
            // dynarec use need it's own mecanism
#else
            GET_EB;
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_lock(&emu->context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
            tmp8u = GB;
            GB = EB->byte[0];
            EB->byte[0] = tmp8u;
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_unlock(&emu->context->mutex_lock);
#endif                
            break;
        case 0x87:                      /* XCHG Ed,Gd */
            nextop = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            GET_ED;
            if(MODREG) {
                tmp32u = GD.dword[0];
                GD.dword[0] = ED->dword[0];
                ED->dword[0] = tmp32u;
            } else {
                if(((uintptr_t)ED)&3)
                {
                    // not aligned, dont't try to "LOCK"
                    tmp32u = ED->dword[0];
                    ED->dword[0] = GD.dword[0];
                    GD.dword[0] = tmp32u;
                } else {
                    // XCHG is supposed to automaticaly LOCK memory bus
                    GD.dword[0] = arm_lock_xchg(ED, GD.dword[0]);
                }
            }
#else
            GET_ED;
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_lock(&emu->context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
            tmp32u = GD.dword[0];
            GD.dword[0] = ED->dword[0];
            ED->dword[0] = tmp32u;
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
            break;
        case 0x88:                      /* MOV Eb,Gb */
            nextop = F8;
            GET_EB;
            EB->byte[0] = GB;
            break;
        case 0x89:                      /* MOV Ed,Gd */
            nextop = F8;
            GET_ED;
            ED->dword[0] = GD.dword[0];
            break;
        case 0x8A:                      /* MOV Gb,Eb */
            nextop = F8;
            GET_EB;
            GB = EB->byte[0];
            break;
        case 0x8B:                      /* MOV Gd,Ed */
            nextop = F8;
            GET_ED;
            GD.dword[0] = ED->dword[0];
            break;
        case 0x8C:                      /* MOV Ed, Seg */
            nextop = F8;
            GET_ED;
            if(MODREG)
                ED->dword[0] = emu->segs[((nextop&0x38)>>3)];
            else
                ED->word[0] = emu->segs[((nextop&0x38)>>3)];
            break;
        case 0x8D:                      /* LEA Gd,M */
            nextop = F8;
            GET_ED_;
            GD.dword[0] = (uint32_t)ED;
            break;
        case 0x8E:                      /* MOV Seg,Ew */
            nextop = F8;
            GET_EW;
            emu->segs[((nextop&0x38)>>3)] = EW->word[0];
            emu->segs_serial[((nextop&0x38)>>3)] = 0;
            if(((nextop&0x38)>>3)==_FS)
                default_fs = EW->word[0];
            break;
        case 0x8F:                      /* POP Ed */
            nextop = F8;
            if(MODREG) {
                emu->regs[(nextop&7)].dword[0] = Pop(emu);
            } else {
                tmp32u = Pop(emu);  // this order allows handling POP [ESP] and variant
                GET_ED;
                R_ESP -= 4; // to prevent issue with SEGFAULT
                ED->dword[0] = tmp32u;
                R_ESP += 4;
            }
            break;
        case 0x90:                      /* NOP */
            break;
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:                      /* XCHG reg,EAX */
            tmp32u = R_EAX;
            R_EAX = emu->regs[opcode&7].dword[0];
            emu->regs[opcode&7].dword[0] = tmp32u;
            break;

        case 0x98:                      /* CWDE */
            emu->regs[_AX].sdword[0] = emu->regs[_AX].sword[0];
            break;
        case 0x99:                      /* CDQ */
            R_EDX=(R_EAX & 0x80000000)?0xFFFFFFFF:0x00000000;
            break;

        case 0x9B:                      /* FWAIT */
            break;
        case 0x9C:                      /* PUSHF */
            CHECK_FLAGS(emu);
            Push(emu, emu->eflags.x32);
            break;
        case 0x9D:                      /* POPF */
            if(ACCESS_FLAG(F_TF) && !tf_next)
                --tf_next;
            emu->eflags.x32 = ((Pop(emu) & 0x3F7FD7)/* & (0xffff-40)*/ ) | 0x2; // mask off res2 and res3 and on res1
            RESET_FLAGS(emu);
            if(ACCESS_FLAG(F_TF))
                ++tf_next;
            break;
        case 0x9E:                      /* SAHF */
            tmp8u = emu->regs[_AX].byte[1];
            CONDITIONAL_SET_FLAG(tmp8u&0x01, F_CF);
            CONDITIONAL_SET_FLAG(tmp8u&0x04, F_PF);
            CONDITIONAL_SET_FLAG(tmp8u&0x10, F_AF);
            CONDITIONAL_SET_FLAG(tmp8u&0x40, F_ZF);
            CONDITIONAL_SET_FLAG(tmp8u&0x80, F_SF);
            RESET_FLAGS(emu);
            break;
        case 0x9F:                      /* LAHF */
            CHECK_FLAGS(emu);
            R_AH = (uint8_t)emu->eflags.x32;
            break;

        case 0xA0:                      /* MOV AL,Ob */
            R_AL = *(uint8_t*)F32;
            break;
        case 0xA1:                      /* MOV EAX,Od */
            R_EAX = *(uint32_t*)F32;
            break;
        case 0xA2:                      /* MOV Ob,AL */
            #ifdef TEST_INTERPRETER
            test->memaddr = F32;
            test->memsize = 1;
            *(uint8_t*)(test->mem) = R_AL;
            #else
            *(uint8_t*)F32 = R_AL;
            #endif
            break;
        case 0xA3:                      /* MOV Od,EAX */
            #ifdef TEST_INTERPRETER
            test->memaddr = F32;
            test->memsize = 4;
            *(uint32_t*)(test->mem) = R_EAX;
            #else
            *(uint32_t*)F32 = R_EAX;
            #endif
            break;
        case 0xA4:                      /* MOVSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            tmp32u = rep?R_ECX:1;
            while(tmp32u--) {
                #ifndef TEST_INTERPRETER
                *(uint8_t*)R_EDI = *(uint8_t*)R_ESI;
                #endif
                R_EDI += tmp8s;
                R_ESI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;
        case 0xA5:                      /* MOVSD */
            tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            tmp32u = rep?R_ECX:1;
            while(tmp32u--) {
                #ifndef TEST_INTERPRETER
                *(uint32_t*)R_EDI = *(uint32_t*)R_ESI;
                #endif
                R_EDI += tmp8s;
                R_ESI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;
        case 0xA6:                      /* CMPSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            switch (rep) {
                case 0:
                    tmp8u  = *(uint8_t*)R_EDI;
                    tmp8u2 = *(uint8_t*)R_ESI;
                    R_EDI += tmp8s;
                    R_ESI += tmp8s;
                    cmp8(emu, tmp8u2, tmp8u);
                    break;
                case 1:
                    tmp8u = 0;
                    tmp8u2 = 0;
                    tmp32u = R_ECX;
                    while(tmp32u) {
                        --tmp32u;
                        tmp8u  = *(uint8_t*)R_EDI;
                        tmp8u2 = *(uint8_t*)R_ESI;
                        R_EDI += tmp8s;
                        R_ESI += tmp8s;
                        if(tmp8u==tmp8u2)
                            break;
                    }
                    if(R_ECX) cmp8(emu, tmp8u2, tmp8u);
                    R_ECX = tmp32u;
                    break;
                case 2:
                    tmp8u = 0;
                    tmp8u2 = 0;
                    tmp32u = R_ECX;
                    while(tmp32u) {
                        --tmp32u;
                        tmp8u  = *(uint8_t*)R_EDI;
                        tmp8u2 = *(uint8_t*)R_ESI;
                        R_EDI += tmp8s;
                        R_ESI += tmp8s;
                        if(tmp8u!=tmp8u2)
                            break;
                    }
                    if(R_ECX) cmp8(emu, tmp8u2, tmp8u);
                    R_ECX = tmp32u;
                    break;
            }
            break;
        case 0xA7:                      /* CMPSD */
            tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            switch (rep) {
                case 0:
                    tmp32u  = *(uint32_t*)R_EDI;
                    tmp32u2 = *(uint32_t*)R_ESI;
                    R_EDI += tmp8s;
                    R_ESI += tmp8s;
                    cmp32(emu, tmp32u2, tmp32u);
                    break;
                case 1:
                    tmp32u2 = 0;
                    tmp32u3 = 0;
                    tmp32u = R_ECX;
                    while(tmp32u) {
                        --tmp32u;
                        tmp32u3 = *(uint32_t*)R_EDI;
                        tmp32u2 = *(uint32_t*)R_ESI;
                        R_EDI += tmp8s;
                        R_ESI += tmp8s;
                        if(tmp32u3==tmp32u2)
                            break;
                    }
                    if(R_ECX) cmp32(emu, tmp32u2, tmp32u3);
                    R_ECX = tmp32u;
                    break;
                case 2:
                    tmp32u2 = 0;
                    tmp32u3 = 0;
                    tmp32u = R_ECX;
                    while(tmp32u) {
                        --tmp32u;
                        tmp32u3 = *(uint32_t*)R_EDI;
                        tmp32u2 = *(uint32_t*)R_ESI;
                        R_EDI += tmp8s;
                        R_ESI += tmp8s;
                        if(tmp32u3!=tmp32u2)
                            break;
                    }
                    if(R_ECX) cmp32(emu, tmp32u2, tmp32u3);
                    R_ECX = tmp32u;
                    break;
            }
            break;
        case 0xA8:                      /* TEST AL, Ib */
            test8(emu, R_AL, F8);
            break;
        case 0xA9:                      /* TEST EAX, Id */
            test32(emu, R_EAX, F32);
            break;
        case 0xAA:                      /* STOSB */
            tmp32u = rep?R_ECX:1;
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            while(tmp32u--) {
                #ifndef TEST_INTERPRETER
                *(uint8_t*)R_EDI = R_AL;
                #endif
                R_EDI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;
        case 0xAB:                      /* STOSD */
            tmp32u = rep?R_ECX:1;
            tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            while(tmp32u--) {
                #ifndef TEST_INTERPRETER
                *(uint32_t*)R_EDI = R_EAX;
                #endif
                R_EDI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;
        case 0xAC:                      /* LODSB */
            tmp32u = rep?R_ECX:1;
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            while(tmp32u--) {
                R_AL = *(uint8_t*)R_ESI;
                R_ESI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;
        case 0xAD:                      /* LODSD */
            tmp32u = rep?R_ECX:1;
            tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            while(tmp32u--) {
                R_EAX = *(uint32_t*)R_ESI;
                R_ESI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;
        case 0xAE:                      /* SCASB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            switch(rep) {
                case 0:
                    cmp8(emu, R_AL, *(uint8_t*)R_EDI);
                    R_EDI += tmp8s;
                    break;
                case 1:
                    tmp32u = R_ECX;
                    tmp8u = 0;
                    while(tmp32u) {
                        --tmp32u;
                        tmp8u = *(uint8_t*)R_EDI;
                        R_EDI += tmp8s;
                        if(R_AL==tmp8u)
                            break;
                    }
                    if(R_ECX) cmp8(emu, R_AL, tmp8u);
                    R_ECX = tmp32u;
                    break;
                case 2:
                    tmp32u = R_ECX;
                    tmp8u = 0;
                    while(tmp32u) {
                        --tmp32u;
                        tmp8u = *(uint8_t*)R_EDI;
                        R_EDI += tmp8s;
                        if(R_AL!=tmp8u)
                            break;
                    }
                    if(R_ECX) cmp8(emu, R_AL, tmp8u);
                    R_ECX = tmp32u;
                    break;
            }
            break;
        case 0xAF:                      /* SCASD */
            tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            switch(rep) {
                case 0:
                    cmp32(emu, R_EAX, *(uint32_t*)R_EDI);
                    R_EDI += tmp8s;
                    break;
                case 1:
                    tmp32u = R_ECX;
                    tmp32u2 = 0;
                    while(tmp32u) {
                        --tmp32u;
                        tmp32u2 = *(uint32_t*)R_EDI;
                        R_EDI += tmp8s;
                        if(R_EAX==tmp32u2)
                            break;
                    }
                    if(R_ECX) cmp32(emu, R_EAX, tmp32u2);
                    R_ECX = tmp32u;
                    break;
                case 2:
                    tmp32u = R_ECX;
                    tmp32u2 = 0;
                    while(tmp32u) {
                        --tmp32u;
                        tmp32u2 = *(uint32_t*)R_EDI;
                        R_EDI += tmp8s;
                        if(R_EAX!=tmp32u2)
                            break;
                    }
                    if(R_ECX) cmp32(emu, R_EAX, tmp32u2);
                    R_ECX = tmp32u;
                    break;
            }
            break;
        case 0xB0:                      /* MOV AL,Ib */
        case 0xB1:                      /* MOV CL,Ib */
        case 0xB2:                      /* MOV DL,Ib */
        case 0xB3:                      /* MOV BL,Ib */
            emu->regs[opcode&3].byte[0] = F8;
            break;
        case 0xB4:                      /* MOV AH,Ib */
        case 0xB5:                      /*    ...    */
        case 0xB6:
        case 0xB7:
            emu->regs[opcode&3].byte[1] = F8;
            break;
        case 0xB8:                      /* MOV EAX,Id */
        case 0xB9:                      /* MOV ECX,Id */
        case 0xBA:                      /* MOV EDX,Id */
        case 0xBB:                      /* MOV EBX,Id */
        case 0xBC:                      /*    ...     */
        case 0xBD:
        case 0xBE:
        case 0xBF:
            emu->regs[opcode&7].dword[0] = F32;
            break;
        case 0xC0:                      /* GRP2 Eb,Ib */
            nextop = F8;
            GET_EB;
            tmp8u = F8/* & 0x1f*/; // masking done in each functions
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = rol8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] = ror8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = rcl8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = rcr8(emu, EB->byte[0], tmp8u); break;
                case 4:
                case 6: EB->byte[0] = shl8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = shr8(emu, EB->byte[0], tmp8u); break;
                case 7: EB->byte[0] = sar8(emu, EB->byte[0], tmp8u); break;
            }
            break;
        case 0xC1:                      /* GRP2 Ed,Ib */
            nextop = F8;
            GET_ED;
            tmp8u = F8/* & 0x1f*/; // masking done in each functions
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = rol32(emu, ED->dword[0], tmp8u); break;
                case 1: ED->dword[0] = ror32(emu, ED->dword[0], tmp8u); break;
                case 2: ED->dword[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                case 3: ED->dword[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                case 4:
                case 6: ED->dword[0] = shl32(emu, ED->dword[0], tmp8u); break;
                case 5: ED->dword[0] = shr32(emu, ED->dword[0], tmp8u); break;
                case 7: ED->dword[0] = sar32(emu, ED->dword[0], tmp8u); break;
            }
            break;
        case 0xC2:                      /* RETN Iw */
            tmp16u = F16;
            addr = Pop(emu);
            R_ESP += tmp16u;
            STEP2;
            break;
        case 0xC3:                      /* RET */
            addr = Pop(emu);
            STEP2;
            break;
        case 0xC4:                      /* LES Gd,Ed */
            nextop = F8;
            if(MODREG) {
                UnimpOpcode(emu);
                emit_signal(emu, SIGILL, (void*)R_EIP, 0);
                emu->quit=1;
                emu->error |= ERR_UNIMPL;
            }
            GET_ED;
            emu->segs[_ES] = *(__uint16_t*)(((char*)ED)+4);
            emu->segs_serial[_ES] = 0;
            GD.dword[0] = *(uint32_t*)ED;
            break;
        case 0xC5:                      /* LDS Gd,Ed */
            nextop = F8;
            if(MODREG) {
                UnimpOpcode(emu);
                emit_signal(emu, SIGILL, (void*)R_EIP, 0);
                emu->quit=1;
                emu->error |= ERR_UNIMPL;
            }
            GET_ED;
            emu->segs[_DS] = *(__uint16_t*)(((char*)ED)+4);
            emu->segs_serial[_DS] = 0;
            GD.dword[0] = *(uint32_t*)ED;
            break;
        case 0xC6:                      /* MOV Eb,Ib */
            nextop = F8;
            GET_EB;
            EB->byte[0] = F8;
            break;
        case 0xC7:                      /* MOV Ed,Id */
            nextop = F8;
            GET_ED;
            ED->dword[0] = F32;
            break;
        case 0xC8:                      /* ENTER Iw,Ib */
            tmp16u = F16;
            tmp8u = (F8) & 0x1f;
            tmp32u = R_EBP;
            Push(emu, R_EBP);
            R_EBP = R_ESP;
            if (tmp8u) {
                for (tmp8u2 = 1; tmp8u2 < tmp8u; tmp8u2++) {
                    tmp32u -= 4;
                    Push(emu, *((uint32_t*)tmp32u));
                }
                Push(emu, R_EBP);
            }
            R_ESP -= tmp16u;
            break;
        case 0xC9:                      /* LEAVE */
            R_ESP = R_EBP;
            R_EBP = Pop(emu);
            break;
        case 0xCA:                      /* FAR RETN */
            tmp16u = F16;
            addr = Pop(emu);
            emu->segs[_CS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_CS] = 0;
            R_ESP += tmp16u;
            // need to check status of CS register!
            STEP2;
            break;
        case 0xCB:                      /* FAR RET */
            addr = Pop(emu);
            emu->segs[_CS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_CS] = 0;
            // need to check status of CS register!
            STEP2;
            break;
        case 0xCC:                      /* INT 3 */
            emu->old_ip = R_EIP;
            R_EIP = addr;
            #ifndef TEST_INTERPRETER
            x86Int3(emu);
            #endif
            addr = R_EIP;
            if(emu->quit) goto fini;
            break;
        case 0xCD:                      /* INT Ib */
            nextop = F8;
            if(nextop == 0x80) {
                emu->old_ip = R_EIP;
                R_EIP = addr;
                #ifndef TEST_INTERPRETER
                x86Syscall(emu);
                #endif
                addr = R_EIP;
                if(emu->quit) goto fini;
            } else {
                if(box86_wine && nextop==0x2D) {
                    // actually ignoring this
                    printf_log(LOG_DEBUG, "INT 2D called\n");
                } else {
                    #ifndef TEST_INTERPRETER
                    emit_signal(emu, SIGSEGV, (void*)R_EIP, 128);
                    if(emu->quit) goto fini;
                    STEP;
                    #endif
                }
            }
            break;
        case 0xCE:                      /* INTO */
            emu->old_ip = R_EIP;
            #ifndef TEST_INTERPRETER
            CHECK_FLAGS(emu);
            if(ACCESS_FLAG(F_OF))
                emit_signal(emu, SIGSEGV, (void*)R_EIP, 128);
            STEP;
            #endif
            break;
        case 0xCF:                      /* IRET */
            addr = Pop(emu);
            emu->segs[_CS] = Pop(emu)&0xffff;
            emu->segs_serial[_CS] = 0;
            emu->eflags.x32 = ((Pop(emu) & 0x3F7FD7)/* & (0xffff-40)*/ ) | 0x2; // mask off res2 and res3 and on res1
            RESET_FLAGS(emu);
            break;
        case 0xD0:                      /* GRP2 Eb,1 */
        case 0xD2:                      /* GRP2 Eb,CL */
            nextop = F8;
            GET_EB;
            tmp8u = (opcode==0xD0)?1:R_CL;
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = rol8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] = ror8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = rcl8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = rcr8(emu, EB->byte[0], tmp8u); break;
                case 4: 
                case 6: EB->byte[0] = shl8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = shr8(emu, EB->byte[0], tmp8u); break;
                case 7: EB->byte[0] = sar8(emu, EB->byte[0], tmp8u); break;
            }
            break;
        case 0xD1:                      /* GRP2 Ed,1 */
        case 0xD3:                      /* GRP2 Ed,CL */
            nextop = F8;
            GET_ED;
            tmp8u = (opcode==0xD1)?1:R_CL;
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = rol32(emu, ED->dword[0], tmp8u); break;
                case 1: ED->dword[0] = ror32(emu, ED->dword[0], tmp8u); break;
                case 2: ED->dword[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                case 3: ED->dword[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                case 4: 
                case 6: ED->dword[0] = shl32(emu, ED->dword[0], tmp8u); break;
                case 5: ED->dword[0] = shr32(emu, ED->dword[0], tmp8u); break;
                case 7: ED->dword[0] = sar32(emu, ED->dword[0], tmp8u); break;
            }
            break;
        case 0xD4:                      /* AAM Ib */
            R_AX = aam16(emu, R_AL, F8);
            break;
        case 0xD5:                      /* AAD Ib */
            R_AX = aad16(emu, R_AX, F8);
            break;
        case 0xD6:
            CHECK_FLAGS(emu);
            R_AL = ACCESS_FLAG(F_CF)?0xFF:0x00;
            break;
        case 0xD7:                      /* XLAT */
            R_AL = *(uint8_t*)(R_EBX + R_AL);
            break;
        
        case 0xD8:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestD8(test, addr)))
                unimp = 1;
            #else
            if(!(addr = RunD8(emu, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xD9:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestD9(test, addr)))
                unimp = 1;
            #else
            if(!(addr = RunD9(emu, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDA:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDA(test, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDA(emu, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDB:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDB(test, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDB(emu, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDC:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDC(test, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDC(emu, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDD:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDD(test, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDD(emu, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDE:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDE(test, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDE(emu, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDF:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDF(test, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDF(emu, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xE0:                      /* LOOPNZ */
            CHECK_FLAGS(emu);
            tmp8s = F8S;
            --R_ECX; // don't update flags
            if(R_ECX && !ACCESS_FLAG(F_ZF))
                addr += tmp8s;
            STEP2;
            break;
        case 0xE1:                      /* LOOPZ */
            CHECK_FLAGS(emu);
            tmp8s = F8S;
            --R_ECX; // don't update flags
            if(R_ECX && ACCESS_FLAG(F_ZF))
                addr += tmp8s;
            STEP2;
            break;
        case 0xE2:                      /* LOOP */
            tmp8s = F8S;
            --R_ECX; // don't update flags
            if(R_ECX)
                addr += tmp8s;
            STEP2;
            break;
        case 0xE3:                      /* JECXZ */
            tmp8s = F8S;
            if(!R_ECX)
                addr += tmp8s;
            STEP2;
            break;
        case 0xE4:                      /* IN AL, Ib */
        case 0xE5:                      /* IN EAX, Ib */
        case 0xE6:                      /* OUT Ib, AL */
        case 0xE7:                      /* OUT Ib, EAX */
            #ifndef TEST_INTERPRETER
            // this is a privilege opcode
            emit_signal(emu, SIGSEGV, (void*)R_EIP, 128);
            STEP;
            #endif
            break;
        case 0xE8:                      /* CALL Id */
            tmp32s = F32S; // call is relative
            Push(emu, addr);
            addr += tmp32s;
            addr = (uintptr_t)getAlternate((void*)addr);
            STEP2;
            break;
        case 0xE9:                      /* JMP Id */
            tmp32s = F32S; // jmp is relative
            addr += tmp32s;
            addr = (uintptr_t)getAlternate((void*)addr);
            STEP2;
            break;

        case 0xEB:                      /* JMP Ib */
            tmp32s = F8S; // jump is relative
            addr += tmp32s;
            STEP2;
            break;
        case 0xEC:                      /* IN AL, DX */
        case 0xED:                      /* IN EAX, DX */
        case 0xEE:                      /* OUT DX, AL */
        case 0xEF:                      /* OUT DX, EAX */
            #ifndef TEST_INTERPRETER
            // this is a privilege opcode
            emit_signal(emu, SIGSEGV, (void*)R_EIP, 128);
            STEP;
            #endif
            break;

        case 0xF0:                      /* LOCK */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestF0(test, addr)))
                unimp = 1;
            #else
            if(!(addr = RunF0(emu, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_EIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xF1:                      /* INT1 */
            // does nothing for now
            break;

        case 0xF4:                      /* HLT */
            #ifndef TEST_INTERPRETER
            // this is a privilege opcode
            emit_signal(emu, SIGSEGV, (void*)R_EIP, 128);
            STEP;
            #endif
            break;
        case 0xF5:                      /* CMC */
            CHECK_FLAGS(emu);
            CONDITIONAL_SET_FLAG(!ACCESS_FLAG(F_CF), F_CF);
            break;

        case 0xF6:                      /* GRP3 Eb(,Ib) */
            nextop = F8;
            GET_EB;
            switch((nextop>>3)&7) {
                case 0: 
                case 1:                 /* TEST Eb,Ib */
                    tmp8u = F8;
                    test8(emu, EB->byte[0], tmp8u);
                    break;
                case 2:                 /* NOT Eb */
                    EB->byte[0] = not8(emu, EB->byte[0]);
                    break;
                case 3:                 /* NEG Eb */
                    EB->byte[0] = neg8(emu, EB->byte[0]);
                    break;
                case 4:                 /* MUL AL,Eb */
                    mul8(emu, EB->byte[0]);
                    break;
                case 5:                 /* IMUL AL,Eb */
                    imul8(emu, EB->byte[0]);
                    break;
                case 6:                 /* DIV Eb */
                    if(!EB->byte[0])
                        emit_div0(emu, (void*)R_EIP, 0);
                    div8(emu, EB->byte[0]);
                    break;
                case 7:                 /* IDIV Eb */
                    if(!EB->byte[0])
                        emit_div0(emu, (void*)R_EIP, 0);
                    idiv8(emu, EB->byte[0]);
                    break;
            }
            break;
        case 0xF7:                      /* GRP3 Ed(,Id) */
            nextop = F8;
            GET_ED;
            switch((nextop>>3)&7) {
                case 0: 
                case 1:                 /* TEST Ed,Id */
                    tmp32u = F32;
                    test32(emu, ED->dword[0], tmp32u);
                    break;
                case 2:                 /* NOT Ed */
                    ED->dword[0] = not32(emu, ED->dword[0]);
                    break;
                case 3:                 /* NEG Ed */
                    ED->dword[0] = neg32(emu, ED->dword[0]);
                    break;
                case 4:                 /* MUL EAX,Ed */
                    mul32_eax(emu, ED->dword[0]);
                    break;
                case 5:                 /* IMUL EAX,Ed */
                    imul32_eax(emu, ED->dword[0]);
                    break;
                case 6:                 /* DIV Ed */
                    if(!ED->dword[0])
                        emit_div0(emu, (void*)R_EIP, 0);
                    div32(emu, ED->dword[0]);
                    break;
                case 7:                 /* IDIV Ed */
                    if(!ED->dword[0])
                        emit_div0(emu, (void*)R_EIP, 0);
                    idiv32(emu, ED->dword[0]);
                    break;
            }
            break;

        case 0xF8:                      /* CLC */
            CHECK_FLAGS(emu);
            CLEAR_FLAG(F_CF);
            break;
        case 0xF9:                      /* STC */
            CHECK_FLAGS(emu);
            SET_FLAG(F_CF);
            break;
        case 0xFA:                      /* CLI */
        case 0xFB:                      /* STI */
            #ifndef TEST_INTERPRETER
            // this is a privilege opcode
            emit_signal(emu, SIGSEGV, (void*)R_EIP, 128);
            STEP;
            #endif
            break;
        case 0xFC:                      /* CLD */
            CLEAR_FLAG(F_DF);
            break;
        case 0xFD:                      /* STD */
            SET_FLAG(F_DF);
            break;
        case 0xFE:                      /* GRP 5 Eb */
            nextop = F8;
            GET_EB;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Eb */
                    EB->byte[0] = inc8(emu, EB->byte[0]);
                    break;
                case 1:                 /* DEC Eb */
                    EB->byte[0] = dec8(emu, EB->byte[0]);
                    break;
                default:
                    unimp = 1;
                    goto fini;
            }
            break;
        case 0xFF:                      /* GRP 5 Ed */
            nextop = F8;
            tmp32u2 = addr;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    GET_ED;
                    ED->dword[0] = inc32(emu, ED->dword[0]);
                    break;
                case 1:                 /* DEC Ed */
                    GET_ED;
                    ED->dword[0] = dec32(emu, ED->dword[0]);
                    break;
                case 2:                 /* CALL NEAR Ed */
                    GET_ED_;
                    tmp32u = (uintptr_t)getAlternate((void*)ED->dword[0]);
                    Push(emu, addr);
                    addr = tmp32u;
                    STEP2;
                    break;
                case 3:                 /* CALL FAR Ed */
                    if(nextop>0xc0) {
                        emu->old_ip = R_EIP;
                        R_EIP = addr = tmp32u2;
                        printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X\n", (void*)addr, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        GET_ED_;
                        Push(emu, R_CS);
                        Push(emu, addr);
                        addr = (uintptr_t)getAlternate((void*)ED->dword[0]);
                        R_CS = (ED+1)->word[0];
                        goto fini;  // in case there is a change of CS
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    GET_ED_;
                    addr = (uintptr_t)getAlternate((void*)ED->dword[0]);
                    STEP2;
                    break;
                case 5:                 /* JMP FAR Ed */
                    if(nextop>0xc0) {
                        emu->old_ip = R_EIP;
                        R_EIP = addr = tmp32u2;
                        printf_log(LOG_NONE, "Illegal Opcode %p: 0x%02X 0x%02X %02X %02X\n", (void*)addr, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        GET_ED_;
                        addr = (uintptr_t)getAlternate((void*)ED->dword[0]);
                        R_CS = (ED+1)->word[0];
                        STEP2;
                    }
                    break;
                case 6:                 /* Push Ed */
                    GET_ED_;
                    tmp32u = ED->dword[0];
                    Push(emu, tmp32u);  // avoid potential issue with push [esp+...]
                    break;
                default:
                    emu->old_ip = R_EIP;
                    R_EIP = addr = tmp32u2;
                    printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X %02X %02X\n",(void*)addr, opcode, nextop, PK(2), PK(3), PK(4), PK(5));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    goto fini;
            }
            break;

        default:
            unimp = 1;
            goto fini;
        }
        #ifndef TEST_INTERPRETER
        if(ACCESS_FLAG(F_TF)) {
            if(tf_next) {
                tf_next = 0;
            } else {
                R_EIP = addr;
                emit_signal(emu, SIGTRAP, (void*)addr, 1);
                if(emu->quit) goto fini;
            }
        }
        #endif
        R_EIP = addr;
    }

fini:
#ifndef TEST_INTERPRETER
    // check the TRACE flag before going to out, in case it's a step by step scenario
    if(!emu->quit && !emu->fork && !emu->uc_link && ACCESS_FLAG(F_TF)) {
        R_EIP = addr;
        emit_signal(emu, SIGTRAP, (void*)addr, 1);
        if(emu->quit) goto fini;
    }
#endif
#ifndef TEST_INTERPRETER
    printf_log(LOG_DEBUG, "End of X86 run (%p), RIP=%p, Stack=%p, unimp=%d, emu->fork=%d, emu->uc_link=%p, emu->quit=%d\n", emu, (void*)R_EIP, (void*)R_ESP, unimp, emu->fork, emu->uc_link, emu->quit);
    if(unimp) {
        UnimpOpcode(emu);
        emit_signal(emu, SIGILL, (void*)R_EIP, 0);
        emu->quit=1;
        emu->error |= ERR_UNIMPL;
    }
    // fork handling
    if(emu->fork) {
        addr = R_EIP;
        if(step)
            return 0;
        int forktype = emu->fork;
        emu->quit = 0;
        emu->fork = 0;
        emu = x86emu_fork(emu, forktype);
        goto x86emurun;
    }
    // setcontext handling
    else if(emu->quit && emu->uc_link) {
        emu->quit = 0;
        my_setcontext(emu, emu->uc_link);
        addr = R_EIP;
        goto x86emurun;
    }
#else
    if(unimp) {
        printf_log(LOG_INFO, "Warning, inimplemented opcode in Test Interpreter\n");
    } else
        addr = R_EIP;
#endif
    return 0;
}
