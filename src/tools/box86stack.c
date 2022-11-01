#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

#include "box86stack.h"
#include "box86context.h"
#include "elfloader.h"
#include "debug.h"
#include "emu/x86emu_private.h"
#include "emu/x86run_private.h"
#include "auxval.h"

EXPORTDYN
int CalcStackSize(box86context_t *context)
{
    printf_log(LOG_DEBUG, "Calc stack size, based on %d elf(s)\n", context->elfsize);
    context->stacksz = 8*1024*1024; context->stackalign=4;
    for (int i=0; i<context->elfsize; ++i)
        CalcStack(context->elfs[i], &context->stacksz, &context->stackalign);

    context->stack = mmap(NULL, context->stacksz, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
    if (context->stack==(void*)-1/*posix_memalign((void**)&context->stack, context->stackalign, context->stacksz)*/) {
        printf_log(LOG_NONE, "Cannot allocate aligned memory (0x%x/0x%x) for stack\n", context->stacksz, context->stackalign);
        return 1;
    }
    //smemset(context->stack, 0, context->stacksz);
    printf_log(LOG_DEBUG, "Stack is @%p size=0x%x align=0x%x\n", context->stack, context->stacksz, context->stackalign);

    return 0;
}

uint16_t Pop16(x86emu_t *emu)
{
    uint16_t* st = ((uint16_t*)(R_ESP));
    R_ESP += 2;
    return *st;
}

void Push16(x86emu_t *emu, uint16_t v)
{
    R_ESP -= 2;
    *((uint16_t*)R_ESP) = v;
}
uint32_t Pop32(x86emu_t *emu)
{
    uint32_t* st = ((uint32_t*)(R_ESP));
    R_ESP += 4;
    return *st;
}

void Push32(x86emu_t *emu, uint32_t v)
{
    R_ESP -= 4;
    *((uint32_t*)R_ESP) = v;
}

void PushString(x86emu_t *emu, const char* s)
{
    int sz = strlen(s) + 1;
    // round to 4 bytes boundary
    R_ESP -= sz;
    memcpy((void*)R_ESP, s, sz);
}

EXPORTDYN
void SetupInitialStack(x86emu_t *emu)
{
    // start with 0
    Push(emu, 0);
    // push program executed
    PushString(emu, emu->context->argv[0]);
    uintptr_t p_arg0 = R_ESP;
    // push envs
    uintptr_t p_envv[emu->context->envc];
    for (int i=emu->context->envc-1; i>=0; --i) {
        PushString(emu, emu->context->envv[i]);
        p_envv[i] = R_ESP;
    }
    // push args, also, free the argv[] string and point to the one in the main stack
    uintptr_t p_argv[emu->context->argc];
    for (int i=emu->context->argc-1; i>=0; --i) {
        PushString(emu, emu->context->argv[i]);
        p_argv[i] = R_ESP;
        box_free(emu->context->argv[i]);
        emu->context->argv[i] = (char*)p_argv[i];
    }
    // align
    uintptr_t tmp = (R_ESP)&~(emu->context->stackalign-1);
    memset((void*)tmp, 0, R_ESP-tmp);
    R_ESP=tmp;

    // push some AuxVector stuffs
    PushString(emu, "i686");
    uintptr_t p_i686 = R_ESP;
    uintptr_t p_random = real_getauxval(25);
    if(!p_random) {
        for (int i=0; i<4; ++i)
            Push(emu, random());
        p_random = R_ESP;
    }
    // align
    tmp = (R_ESP)&~(emu->context->stackalign-1);
    memset((void*)tmp, 0, R_ESP-tmp);
    R_ESP=tmp;

    // push the AuxVector themselves
    /*
    00: 00000000
    03: 08048034
    04: 00000020
    05: 0000000b
    06: 00001000
    07: f7fc0000
    08: 00000000
    09: 08049060
    11: 000003e8
    12: 000003e8
    13: 000003e8
    14: 000003e8
    15: ffd8aa5b/i686
    16: bfebfbff
    17: 00000064
    23: 00000000
    25: ffd8aa4b
    26: 00000000
    31: ffd8bfeb/./testAuxVec
    32: f7fbfb40
    33: f7fbf000
    */
    Push(emu, 0); Push(emu, 0);                            //AT_NULL(0)=0
    //Push(emu, ); Push(emu, 3);                             //AT_PHDR(3)=address of the PH of the executable
    //Push(emu, ); Push(emu, 4);                             //AT_PHENT(4)=size of PH entry
    //Push(emu, ); Push(emu, 5);                             //AT_PHNUM(5)=number of elf headers
    Push(emu, box86_pagesize); Push(emu, 6);               //AT_PAGESZ(6)
    //Push(emu, real_getauxval(7)); Push(emu, 7);            //AT_BASE(7)=ld-2.27.so start (in memory)
    Push(emu, 0); Push(emu, 8);                            //AT_FLAGS(8)=0
    Push(emu, R_EIP); Push(emu, 9);                        //AT_ENTRY(9)=entrypoint
    Push(emu, real_getauxval(11)); Push(emu, 11);          //AT_UID(11)
    Push(emu, real_getauxval(12)); Push(emu, 12);          //AT_EUID(12)
    Push(emu, real_getauxval(13)); Push(emu, 13);          //AT_GID(13)
    Push(emu, real_getauxval(14)); Push(emu, 14);          //AT_EGID(14)
    Push(emu, p_i686); Push(emu, 15);                      //AT_PLATFORM(15)=&"i686"
    // Push HWCAP:
    //  FPU: 1<<0 ; VME: 1<<1 ; DE : 1<<2 ; PSE: 1<<3 ; TSC: 1<<4 ; MSR: 1<<5 ; PAE: 1<<6 ; MCE: 1<<7
    //  CX8: 1<<8 ; APIC:1<<9 ;             SEP: 1<<11; MTRR:1<<12; PGE: 1<<13; MCA: 1<<14; CMOV:1<<15
    // FCMOV:1<<16;                                                                         MMX: 1<<23
    // OSFXR:1<<24; XMM: 1<<25;XMM2: 1<<26;                                                AMD3D:1<<31
    Push(emu, (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<8)  | (1<<15) | (1<<16) | (1<<23) | (1<<25) | (1<<26));
    Push(emu, 16);                                         //AT_HWCAP(16)=...
    //Push(emu, sysconf(_SC_CLK_TCK)); Push(emu, 17);        //AT_CLKTCK(17)=times() frequency
    Push(emu, real_getauxval(23)); Push(emu, 23);          //AT_SECURE(23)
    Push(emu, p_random); Push(emu, 25);                    //AT_RANDOM(25)=p_random
    Push(emu, 0); Push(emu, 26);                           //AT_HWCAP2(26)=0
    Push(emu, p_arg0); Push(emu, 31);                      //AT_EXECFN(31)=p_arg0
    Push(emu, emu->context->vsyscall); Push(emu, 32); //AT_SYSINFO(32)=vsyscall
    //Push(emu, ); Push(emu, 33);                            //AT_SYSINFO_EHDR(33)=address of vDSO
    if(!emu->context->auxval_start) // store auxval start if needed
        emu->context->auxval_start = (uintptr_t*)R_ESP;

    // push nil / envs / nil / args / argc
    Push(emu, 0);
    for (int i=emu->context->envc-1; i>=0; --i)
        Push(emu, p_envv[i]);
    box_free(emu->context->envv);
    emu->context->envv = (char**)R_ESP;
    Push(emu, 0);
    for (int i=emu->context->argc-1; i>=0; --i)
        Push(emu, p_argv[i]);
    box_free(emu->context->argv);
    emu->context->argv = (char**)R_ESP;
    Push(emu, emu->context->argc);
}
