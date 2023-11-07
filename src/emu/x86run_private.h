#ifndef __X86RUN_PRIVATE_H_
#define __X86RUN_PRIVATE_H_

#include <stdint.h>
#include "regs.h"
#include "x86emu_private.h"
#include "box86context.h"
typedef struct x86emu_s x86emu_t;

static inline uint8_t Fetch8(x86emu_t *emu) {return *(uint8_t*)(R_EIP++);}
static inline int8_t Fetch8s(x86emu_t *emu) {return *(int8_t*)(R_EIP++);}
static inline uint16_t Fetch16(x86emu_t *emu)
{
    uint16_t val = *(uint16_t*)R_EIP;
    R_EIP+=2;
    return val;
}
static inline int16_t Fetch16s(x86emu_t *emu)
{
    int16_t val = *(int16_t*)R_EIP;
    R_EIP+=2;
    return val;
}
static inline uint32_t Fetch32(x86emu_t *emu)
{
    uint32_t val = *(uint32_t*)R_EIP;
    R_EIP+=4;
    return val;
}
static inline int32_t Fetch32s(x86emu_t *emu)
{
    int32_t val = *(int32_t*)R_EIP;
    R_EIP+=4;
    return val;
}
static inline uint8_t Peek(x86emu_t *emu, int offset){return *(uint8_t*)(R_EIP + offset);}

static inline uint32_t Pop(x86emu_t *emu)
{
    uint32_t* st = ((uint32_t*)(R_ESP));
    R_ESP += 4;
    return *st;
}

#ifdef TEST_INTERPRETER
#define Push(E, V)  do{E->regs[_SP].dword[0] -=4; test->memsize = 4; *(uint64_t*)test->mem = (V); test->memaddr = E->regs[_SP].dword[0];}while(0)
#define Push16(E, V)  do{E->regs[_SP].dword[0] -=2; test->memsize = 2; *(uint16_t*)test->mem = (V); test->memaddr = E->regs[_SP].dword[0];}while(0)
#else
static inline void Push(x86emu_t *emu, uint32_t v)
{
    R_ESP -= 4;
    *((uint32_t*)R_ESP) = v;
}
#endif

static inline void PushExit(x86emu_t* emu)
{
    R_ESP -= 4;
    *((uint32_t*)R_ESP) = my_context->exit_bridge;
}

#if 0
// the op code definition can be found here: http://ref.x86asm.net/geek32.html
static inline reg32_t* GetECommon(x86emu_t* emu, uint32_t m)
{
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            uintptr_t base = ((sib&0x7)==5)?Fetch32(emu):(emu->regs[(sib&0x7)].dword[0]); // base
            base += (emu->sbiidx[(sib>>3)&7]->sdword[0] << (sib>>6));
            return (reg32_t*)base;
        } else if (m==0x5) { //disp32
            return (reg32_t*)Fetch32(emu);
        }
        return (reg32_t*)(emu->regs[m].dword[0]);
    } else {
        uintptr_t base;
        if((m&7)==4) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[(sib&0x7)].dword[0]; // base
            base += (emu->sbiidx[(sib>>3)&7]->sdword[0] << (sib>>6));
        } else {
            base = emu->regs[(m&0x7)].dword[0];
        }
        base+=(m&0x80)?Fetch32s(emu):Fetch8s(emu);
        return (reg32_t*)base;
    }
}

static inline reg32_t* GetEb(x86emu_t *emu, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        int lowhigh = (m&4)>>2;
         return (reg32_t *)(((char*)(&emu->regs[(m&0x03)]))+lowhigh);  //?
    } else return GetECommon(emu, m);
}

static inline reg32_t* GetEd(x86emu_t *emu, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)];
    } else return GetECommon(emu, m);
}

#define GetEw GetEd

static inline reg32_t* GetEw16(x86emu_t *emu, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)];
    } else {
        uint32_t base = 0;
        switch(m&7) {
            case 0: base = R_BX+R_SI; break;
            case 1: base = R_BX+R_DI; break;
            case 2: base = R_BP+R_SI; break;
            case 3: base = R_BP+R_DI; break;
            case 4: base =      R_SI; break;
            case 5: base =      R_DI; break;
            case 6: base =      R_BP; break;
            case 7: base =      R_BX; break;
        }
        switch((m>>6)&3) {
            case 0: if(m==6) base = Fetch16(emu); break;
            case 1: base += Fetch8s(emu); break;
            case 2: base += Fetch16s(emu); break;
            // case 3 is C0..C7, already dealt with
        }
        return (reg32_t*)base;
    }
}

static inline reg32_t* GetEw16off(x86emu_t *emu, uint32_t v, uintptr_t offset)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)];
    } else {
        uint32_t base = 0;
        switch(m&7) {
            case 0: base = R_BX+R_SI; break;
            case 1: base = R_BX+R_DI; break;
            case 2: base = R_BP+R_SI; break;
            case 3: base = R_BP+R_DI; break;
            case 4: base =      R_SI; break;
            case 5: base =      R_DI; break;
            case 6: base =      R_BP; break;
            case 7: base =      R_BX; break;
        }
        switch((m>>6)&3) {
            case 0: if(m==6) base = Fetch16(emu); break;
            case 1: base += Fetch8s(emu); break;
            case 2: base += Fetch16s(emu); break;
            // case 3 is C0..C7, already dealt with
        }
        return (reg32_t*)(base+offset);
    }
}

static inline mmx87_regs_t* GetEm(x86emu_t *emu, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->mmx[m&0x07];
    } else return (mmx87_regs_t*)GetECommon(emu, m);
}

static inline sse_regs_t* GetEx(x86emu_t *emu, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->xmm[m&0x07];
    } else return (sse_regs_t*)GetECommon(emu, m);
}


static inline reg32_t* GetG(x86emu_t *emu, uint32_t v)
{
    return &emu->regs[((v&0x38)>>3)];
}

static inline reg32_t* GetGb(x86emu_t *emu, uint32_t v)
{
    uint8_t m = (v&0x38)>>3;
    return (reg32_t*)&emu->regs[m&3].byte[m>>2];
}

static inline mmx87_regs_t* GetGm(x86emu_t *emu, uint32_t v)
{
    uint8_t m = (v&0x38)>>3;
    return &emu->mmx[m&7];
}

static inline sse_regs_t* GetGx(x86emu_t *emu, uint32_t v)
{
    uint8_t m = (v&0x38)>>3;
    return &emu->xmm[m&7];
}
#endif
void UpdateFlags(x86emu_t *emu);

#define CHECK_FLAGS(emu) if(emu->df) UpdateFlags(emu)
#define RESET_FLAGS(emu) emu->df = d_none

uintptr_t Run0F(x86emu_t *emu, uintptr_t addr, int *step);
uintptr_t Run64(x86emu_t *emu, int seg, uintptr_t addr);
uintptr_t Run640F(x86emu_t *emu, uintptr_t tlsdata, uintptr_t addr);
uintptr_t Run6466(x86emu_t *emu, uintptr_t tlsdata, uintptr_t addr);
uintptr_t Run6467(x86emu_t *emu, uintptr_t tlsdata, uintptr_t addr);
uintptr_t Run66(x86emu_t *emu, int rep, uintptr_t addr);
uintptr_t Run660F(x86emu_t *emu, uintptr_t addr);
uintptr_t Run66F20F(x86emu_t *emu, uintptr_t addr);
uintptr_t Run6664(x86emu_t *emu, int seg, uintptr_t addr);
uintptr_t Run66D9(x86emu_t *emu, uintptr_t addr);
uintptr_t Run66DD(x86emu_t *emu, uintptr_t addr);
uintptr_t Run66F0(x86emu_t *emu, uintptr_t addr);
uintptr_t Run67(x86emu_t *emu, int rep, uintptr_t addr);
uintptr_t Run670F(x86emu_t *emu, int rep, uintptr_t addr);
uintptr_t Run6766(x86emu_t *emu, int rep, uintptr_t addr);
uintptr_t Run67660F(x86emu_t *emu, uintptr_t addr);
uintptr_t RunD8(x86emu_t *emu, uintptr_t addr);
uintptr_t RunD9(x86emu_t *emu, uintptr_t addr);
uintptr_t RunDA(x86emu_t *emu, uintptr_t addr);
uintptr_t RunDB(x86emu_t *emu, uintptr_t addr);
uintptr_t RunDC(x86emu_t *emu, uintptr_t addr);
uintptr_t RunDD(x86emu_t *emu, uintptr_t addr);
uintptr_t RunDE(x86emu_t *emu, uintptr_t addr);
uintptr_t RunDF(x86emu_t *emu, uintptr_t addr);
uintptr_t RunF0(x86emu_t *emu, uintptr_t addr);
uintptr_t RunF066(x86emu_t *emu, uintptr_t addr);
uintptr_t RunF20F(x86emu_t *emu, uintptr_t addr, int *step);
uintptr_t RunF30F(x86emu_t *emu, uintptr_t addr);

#ifdef DYNAREC
uintptr_t Test0F(x86test_t *test, uintptr_t addr, int *step);
uintptr_t Test64(x86test_t *test, int seg, uintptr_t addr);
uintptr_t Test640F(x86test_t *test, uintptr_t tlsdata, uintptr_t addr);
uintptr_t Test6466(x86test_t *test, uintptr_t tlsdata, uintptr_t addr);
uintptr_t Test6467(x86test_t *test, uintptr_t tlsdata, uintptr_t addr);
uintptr_t Test66(x86test_t *test, int rep, uintptr_t addr);
uintptr_t Test660F(x86test_t *test, uintptr_t addr);
uintptr_t Test66F20F(x86test_t *test, uintptr_t addr);
uintptr_t Test6664(x86test_t *test, int seg, uintptr_t addr);
uintptr_t Test66D9(x86test_t *test, uintptr_t addr);
uintptr_t Test66DD(x86test_t *test, uintptr_t addr);
uintptr_t Test66F0(x86test_t *test, uintptr_t addr);
uintptr_t Test67(x86test_t *test, int rep, uintptr_t addr);
uintptr_t Test670F(x86test_t *test, int rep, uintptr_t addr);
uintptr_t Test6766(x86test_t *test, int rep, uintptr_t addr);
uintptr_t Test67660F(x86test_t *test, uintptr_t addr);
uintptr_t TestD8(x86test_t *test, uintptr_t addr);
uintptr_t TestD9(x86test_t *test, uintptr_t addr);
uintptr_t TestDA(x86test_t *test, uintptr_t addr);
uintptr_t TestDB(x86test_t *test, uintptr_t addr);
uintptr_t TestDC(x86test_t *test, uintptr_t addr);
uintptr_t TestDD(x86test_t *test, uintptr_t addr);
uintptr_t TestDE(x86test_t *test, uintptr_t addr);
uintptr_t TestDF(x86test_t *test, uintptr_t addr);
uintptr_t TestF0(x86test_t *test, uintptr_t addr);
uintptr_t TestF066(x86test_t *test, uintptr_t addr);
uintptr_t TestF20F(x86test_t *test, uintptr_t addr, int *step);
uintptr_t TestF30F(x86test_t *test, uintptr_t addr);
#endif

void x86Syscall(x86emu_t *emu);
void x86Int3(x86emu_t* emu);
x86emu_t* x86emu_fork(x86emu_t* e, int forktype);

uintptr_t GetSegmentBaseEmu(x86emu_t* emu, int seg);
#define GetGSBaseEmu(emu)    GetSegmentBaseEmu(emu, _GS)
#define GetFSBaseEmu(emu)    GetSegmentBaseEmu(emu, _FS)
#define GetESBaseEmu(emu)    GetSegmentBaseEmu(emu, _ES)
#define GetDSBaseEmu(emu)    GetSegmentBaseEmu(emu, _DS)

const char* GetNativeName(void* p);

#ifdef HAVE_TRACE
void PrintTrace(x86emu_t* emu, uintptr_t ip, int dynarec);
#endif

#endif //__X86RUN_PRIVATE_H_