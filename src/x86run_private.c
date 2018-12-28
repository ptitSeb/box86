#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86run_private.h"
#include "x86emu_private.h"
#include "box86context.h"

uint8_t Fetch8(x86emu_t *emu)
{
    uint8_t val = *(uint8_t*)R_EIP;
    R_EIP++;
    return val;
}
int8_t Fetch8s(x86emu_t *emu)
{
    int8_t val = *(int8_t*)R_EIP;
    R_EIP++;
    return val;
}
uint16_t Fetch16(x86emu_t *emu)
{
    uint16_t val = *(uint16_t*)R_EIP;
    R_EIP+=2;
    return val;
}
int16_t Fetch16s(x86emu_t *emu)
{
    int16_t val = *(int16_t*)R_EIP;
    R_EIP+=2;
    return val;
}
uint32_t Fetch32(x86emu_t *emu)
{
    uint32_t val = *(uint32_t*)R_EIP;
    R_EIP+=4;
    return val;
}
int32_t Fetch32s(x86emu_t *emu)
{
    int32_t val = *(int32_t*)R_EIP;
    R_EIP+=4;
    return val;
}
uint8_t Peek(x86emu_t *emu, int offset)
{
    uint8_t val = *(uint8_t*)(R_EIP + offset);
    return val;
}


// the op code definition can be found here: http://ref.x86asm.net/geek32.html

void GetEb(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        int lowhigh = (m&04)>>3;
         *op = (reg32_t *)&emu->regs[_AX+(m&0x03)].byte[lowhigh];  //?
        return;
    } else if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            uintptr_t base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            base += emu->sbiidx[(sib>>3)&7]->dword[0] << (sib>>6);
            *op = (reg32_t*)base;
            return;
        } else if (m==0x5) { //disp32
            *op = (reg32_t*)Fetch32(emu);
            return;
        }
        *op = (reg32_t*)emu->regs[_AX+m].dword[0];
        return;
    } else if(m>=0x40 && m<=0x47) {
        uintptr_t base;
        if(m==0x44) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((v>>3)&7)==4)
                idx += Fetch8(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch8(emu);
        *op = (reg32_t*)base;
        return;
    } else if(m>=0x80 && m<0x87) {
        uintptr_t base;
        if(m==0x84) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((v>>3)&7)==4)
                idx += Fetch32(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch32(emu);
        *op = (reg32_t*)base;
        return;
    } else {
        ea->word[0] = 0;
        *op = ea;
        return;
    }
}

void GetEd(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         *op = &emu->regs[_AX+(m&0x07)];
        return;
    } else if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            uintptr_t base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            base += emu->sbiidx[(sib>>3)&7]->dword[0] << (sib>>6);
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
            if((sib&0x7)==5)
                base = Fetch32(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((sib>>3)&7)==4)
                idx += Fetch8(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch8s(emu);
        *op = (reg32_t*)base;
        return;
    } else if(m>=0x80 && m<0x87) {
        uintptr_t base;
        if(m==0x84) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((sib>>3)&7)==4)
                idx += Fetch32(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch32s(emu);
        *op = (reg32_t*)base;
        return;
    } else {
        ea->word[0] = 0;
        *op = ea;
        return;
    }
}

void GetEw(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         *op = &emu->regs[_AX+(m&0x07)];
        return;
    } else if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            uintptr_t base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch16(emu);
            base += emu->sbiidx[(sib>>3)&7]->dword[0] << (sib>>6);
            *op = (reg32_t*)base;
            return;
        } else if (m==0x5) { //disp32
            *op = (reg32_t*)Fetch32(emu);
            return;
        }
        ea->dword[0] = emu->regs[_AX+m].word[0];
        *op = (reg32_t*)ea->dword[0];
        return;
    } else if(m>=0x40 && m<=0x47) {
        uintptr_t base;
        if(m==0x44) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch16(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((sib>>3)&7)==4)
                idx += Fetch8(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch8s(emu);
        *op = (reg32_t*)base;
        return;
    } else if(m>=0x80 && m<0x87) {
        uintptr_t base;
        if(m==0x84) {
            uint8_t sib = Fetch16(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch16(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((sib>>3)&7)==4)
                idx += Fetch32(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch16s(emu);
        *op = (reg32_t*)base;
        return;
    } else {
        ea->word[0] = 0;
        *op = ea;
        return;
    }
}


void GetG(x86emu_t *emu, reg32_t **op, uint32_t v)
{
    *op = &emu->regs[_AX+((v&0x38)>>3)];
}

void GetGb(x86emu_t *emu, reg32_t **op, uint32_t v)
{
    uint8_t m = (v&0x38)>>3;
    *op = (reg32_t*)&emu->regs[m&3].byte[m>>2];
}


int32_t my___libc_start_main(x86emu_t* emu, int *(main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end))
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
    printf_log(LOG_DEBUG, "Calling main(=>%p) from __libc_start_main\n", main);
}
