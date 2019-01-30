#ifndef __X86RUN_PRIVATE_H_
#define __X86RUN_PRIVATE_H_

#include <stdint.h>
#include "regs.h"
#include "x86emu_private.h"
typedef struct x86emu_s x86emu_t;

inline uint8_t Fetch8(x86emu_t *emu) {return *(uint8_t*)(R_EIP++);}
inline int8_t Fetch8s(x86emu_t *emu) {return *(int8_t*)(R_EIP++);}
inline uint16_t Fetch16(x86emu_t *emu)
{
    uint16_t val = *(uint16_t*)R_EIP;
    R_EIP+=2;
    return val;
}
inline int16_t Fetch16s(x86emu_t *emu)
{
    int16_t val = *(int16_t*)R_EIP;
    R_EIP+=2;
    return val;
}
inline uint32_t Fetch32(x86emu_t *emu)
{
    uint32_t val = *(uint32_t*)R_EIP;
    R_EIP+=4;
    return val;
}
inline int32_t Fetch32s(x86emu_t *emu)
{
    int32_t val = *(int32_t*)R_EIP;
    R_EIP+=4;
    return val;
}
inline uint8_t Peek(x86emu_t *emu, int offset){return *(uint8_t*)(R_EIP + offset);}

inline uint32_t Pop(x86emu_t *emu)
{
    uint32_t* st = ((uint32_t*)(R_ESP));
    R_ESP += 4;
    return *st;
}

inline void Push(x86emu_t *emu, uint32_t v)
{
    R_ESP -= 4;
    *((uint32_t*)R_ESP) = v;
}


// the op code definition can be found here: http://ref.x86asm.net/geek32.html

static inline void GetECommon(x86emu_t* emu, reg32_t **op, uint32_t m)
{
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            uintptr_t base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            base += (emu->sbiidx[(sib>>3)&7]->sdword[0] << (sib>>6));
            *op = (reg32_t*)base;
            return;
        } else if (m==0x5) { //disp32
            *op = (reg32_t*)Fetch32(emu);
            return;
        }
        *op = (reg32_t*)(emu->regs[_AX+m].dword[0]);
        return;
    } else if(m>=0x40 && m<=0x47) {
        uintptr_t base;
        if(m==0x44) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            int32_t idx = emu->sbiidx[(sib>>3)&7]->sdword[0];
            base += (idx << (sib>>6));
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch8s(emu);
        *op = (reg32_t*)base;
        return;
    } else /*if(m>=0x80 && m<=0x87)*/ {
        uintptr_t base;
        if(m==0x84) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            int32_t idx = emu->sbiidx[(sib>>3)&7]->sdword[0];
            base += (idx << (sib>>6));
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch32s(emu);
        *op = (reg32_t*)base;
        return;
    }
}

static inline void GetEb(x86emu_t *emu, reg32_t **op, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        int lowhigh = (m&4)>>2;
         *op = (reg32_t *)(((char*)(&emu->regs[_AX+(m&0x03)]))+lowhigh);  //?
        return;
    } else GetECommon(emu, op, m);
}

static inline void GetEd(x86emu_t *emu, reg32_t **op, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         *op = &emu->regs[_AX+(m&0x07)];
        return;
    } else GetECommon(emu, op, m);
}

#define GetEw GetEd

static inline void GetEw16(x86emu_t *emu, reg32_t **op, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         *op = &emu->regs[_AX+(m&0x07)];
        return;
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
        *op = (reg32_t*)base;
        return;
    }
}

static inline void GetEm(x86emu_t *emu, mmx_regs_t **op, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         *op = &emu->mmx[m&0x07];
        return;
    } else GetECommon(emu, (reg32_t**)op, m);
}

static inline void GetEx(x86emu_t *emu, sse_regs_t **op, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         *op = &emu->xmm[m&0x07];
        return;
    } else GetECommon(emu, (reg32_t**)op, m);
}


static inline void GetG(x86emu_t *emu, reg32_t **op, uint32_t v)
{
    *op = &emu->regs[_AX+((v&0x38)>>3)];
}

static inline void GetGb(x86emu_t *emu, reg32_t **op, uint32_t v)
{
    uint8_t m = (v&0x38)>>3;
    *op = (reg32_t*)&emu->regs[m&3].byte[m>>2];
}

static inline void GetGm(x86emu_t *emu, mmx_regs_t **op, uint32_t v)
{
    uint8_t m = (v&0x38)>>3;
    *op = &emu->mmx[m&7];
}

static inline void GetGx(x86emu_t *emu, sse_regs_t **op, uint32_t v)
{
    uint8_t m = (v&0x38)>>3;
    *op = &emu->xmm[m&7];
}

void UpdateFlags(x86emu_t *emu);

#define CHECK_FLAGS(emu) if(emu->df) UpdateFlags(emu)
#define RESET_FLAGS(emu) emu->df = d_none

void Run67(x86emu_t *emu);
void Run0F(x86emu_t *emu);
void Run660F(x86emu_t *emu);
void Run66D9(x86emu_t *emu);    // x87
void Run6766(x86emu_t *emu);

void x86Syscall(x86emu_t *emu);
void x86Int3(x86emu_t* emu);
x86emu_t* x86emu_fork(x86emu_t* e);

const char* GetNativeName(void* p);


#endif //__X86RUN_PRIVATE_H_