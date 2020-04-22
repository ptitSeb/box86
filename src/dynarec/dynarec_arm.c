#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "tools/bridge_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "emu/x86run_private.h"
#include "x86trace.h"
#include "dynablock.h"
#include "dynablock_private.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"

void printf_x86_instruction(zydis_dec_t* dec, instruction_x86_t* inst, const char* name) {
    uint8_t *ip = (uint8_t*)inst->addr;
    if(ip[0]==0xcc && ip[1]=='S' && ip[2]=='C') {
        uint32_t a = *(uint32_t*)(ip+3);
        if(a==0) {
            dynarec_log(LOG_NONE, "%s%p: Exit x86emu%s\n", (box86_dynarec_dump>1)?"\e[1m":"", (void*)ip, (box86_dynarec_dump>1)?"\e[m":"");
        } else {
            dynarec_log(LOG_NONE, "%s%p: Native call to %p%s\n", (box86_dynarec_dump>1)?"\e[1m":"", (void*)ip, (void*)a, (box86_dynarec_dump>1)?"\e[m":"");
        }
    } else {
        if(dec) {
            dynarec_log(LOG_NONE, "%s%p: %s%s\n", (box86_dynarec_dump>1)?"\e[1m":"", (void*)inst->addr, DecodeX86Trace(dec, inst->addr), (box86_dynarec_dump>1)?"\e[m":"");
        } else {
            dynarec_log(LOG_NONE, "%s%p: ", (box86_dynarec_dump>1)?"\e[1m":"", (void*)inst->addr);
            for(int i=0; i<inst->size; ++i) {
                dynarec_log(LOG_NONE, "%02X ", ip[i]);
            }
            dynarec_log(LOG_NONE, " %s%s\n", name, (box86_dynarec_dump>1)?"\e[m":"");
        }
    }
}

void add_next(dynarec_arm_t *dyn, uintptr_t addr) {
    if(dyn->next_sz == dyn->next_cap) {
        dyn->next_cap += 16;
        dyn->next = (uintptr_t*)realloc(dyn->next, dyn->next_cap*sizeof(uintptr_t));
    }
    for(int i=0; i<dyn->next_sz; ++i)
        if(dyn->next[i]==addr)
            return;
    dyn->next[dyn->next_sz++] = addr;
}
uintptr_t get_closest_next(dynarec_arm_t *dyn, uintptr_t addr) {
    // get closest, but no addresses befores
    uintptr_t best = 0;
    int i = 0;
    while((i<dyn->next_sz) && (best!=addr)) {
        if(dyn->next[i]<addr) { // remove the address, it's before current address
            memmove(dyn->next+i, dyn->next+i+1, (dyn->next_sz-i-1)*sizeof(uintptr_t));
            --dyn->next_sz;
        } else {
            if((dyn->next[i]<best) || !best)
                best = dyn->next[i];
            ++i;
        }
    }
    return best;
}
int is_nops(dynarec_arm_t *dyn, uintptr_t addr, int n)
{
    #define PK(A) (*((uint8_t*)(addr+(A))))
    if(!n)
        return 1;
    if(PK(0)==0x90)
        return is_nops(dyn, addr+1, n-1);
    if(n>1 && PK(0)==0x66)  // if opcode start with 0x66, and there is more after, than is *can* be a NOP
        return is_nops(dyn, addr+1, n-1);
    if(n>2 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x00)
        return is_nops(dyn, addr+3, n-3);
    if(n>3 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x40 && PK(3)==0x00)
        return is_nops(dyn, addr+4, n-4);
    if(n>3 && PK(0)==0x8d && PK(1)==0x74 && PK(2)==0x26 && PK(3)==0x00)
        return is_nops(dyn, addr+4, n-4);
    if(n>4 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x44 && PK(3)==0x00 && PK(4)==0x00)
        return is_nops(dyn, addr+5, n-5);
    if(n>5 && PK(0)==0x8d && PK(1)==0xb6 && PK(2)==0x00 && PK(3)==0x00 && PK(4)==0x00 && PK(5)==0x00)
        return is_nops(dyn, addr+6, n-6);
    if(n>6 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x80 && PK(3)==0x00 && PK(4)==0x00 && PK(5)==0x00 && PK(6)==0x00)
        return is_nops(dyn, addr+7, n-7);
    if(n>7 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x84 && PK(3)==0x00 && PK(4)==0x00 && PK(5)==0x00 && PK(6)==0x00 && PK(7)==0x00)
        return is_nops(dyn, addr+8, n-8);
    return 0;
    #undef PK
}

uint32_t needed_flags(dynarec_arm_t *dyn, int ninst, uint32_t setf, int recurse)
{
    if(ninst == dyn->size || (recurse==5))
        return X_ALL; // no more instructions, or too many jmp loop, stop
    uint32_t needed = dyn->insts[ninst].x86.use_flags;
    if(needed) {
        setf &= ~needed;
        if(!setf)   // all flags already used, no need to continue
            return needed;
    }

    if(!needed && !dyn->insts[ninst].x86.set_flags && !dyn->insts[ninst].x86.jmp_insts) {
        int start = ninst;
        int end = ninst;
        while(end<dyn->size && !dyn->insts[end].x86.use_flags && !dyn->insts[end].x86.set_flags && !dyn->insts[end].x86.jmp_insts)
            ++end;
        needed = needed_flags(dyn, end, setf, recurse);
        for(int i=start; i<end; ++i)
            dyn->insts[i].x86.need_flags = needed;
        return needed;
    }

    if(dyn->insts[ninst].x86.set_flags && (dyn->insts[ninst].x86.state_flags!=SF_MAYSET)) {
        if((setf & ~dyn->insts[ninst].x86.set_flags) == 0)
            return needed;    // all done, gives all the flags needed
        setf |= dyn->insts[ninst].x86.set_flags;    // add new flags to continue
    }

    int jinst = dyn->insts[ninst].x86.jmp_insts;
    if(dyn->insts[ninst].x86.jmp) {
        dyn->insts[ninst].x86.need_flags = (jinst==-1)?X_PEND:needed_flags(dyn, jinst, setf, recurse+1);
        if(dyn->insts[ninst].x86.use_flags)  // conditionnal jump
             dyn->insts[ninst].x86.need_flags |= needed_flags(dyn, ninst+1, setf, recurse);
    } else
        dyn->insts[ninst].x86.need_flags = needed_flags(dyn, ninst+1, setf, recurse);
    if(dyn->insts[ninst].x86.state_flags==SF_MAYSET)
        needed |= dyn->insts[ninst].x86.need_flags;
    else
        needed |= (dyn->insts[ninst].x86.need_flags & ~dyn->insts[ninst].x86.set_flags);
    if(needed == (X_PEND|X_ALL))
        needed = X_ALL;
    return needed;
}

void arm_pass0(dynarec_arm_t* dyn, uintptr_t addr);
void arm_pass1(dynarec_arm_t* dyn, uintptr_t addr);
void arm_pass2(dynarec_arm_t* dyn, uintptr_t addr);
void arm_pass3(dynarec_arm_t* dyn, uintptr_t addr);

void FillBlock(x86emu_t* emu, dynablock_t* block, uintptr_t addr) {
    // init the helper
    dynarec_arm_t helper = {0};
    helper.emu = emu;
    helper.nolinker = box86_dynarec_linker?(block->parent->nolinker):1;
    helper.start = addr;
    arm_pass0(&helper, addr);
    if(!helper.size) {
        dynarec_log(LOG_DEBUG, "Warning, null-sized dynarec block (%p)\n", (void*)addr);
        block->done = 1;
        free(helper.next);
        return;
    }
    helper.cap = helper.size+3; // needs epilog handling
    helper.insts = (instruction_arm_t*)calloc(helper.cap, sizeof(instruction_arm_t));
    // pass 1, addresses, x86 jump addresses, flags
    arm_pass1(&helper, addr);
    // calculate barriers
    uintptr_t start = helper.insts[0].x86.addr;
    uintptr_t end = helper.insts[helper.size].x86.addr+helper.insts[helper.size].x86.size;
    for(int i=0; i<helper.size; ++i)
        if(helper.insts[i].x86.jmp) {
            uintptr_t j = helper.insts[i].x86.jmp;
            if(j<start || j>=end)
                helper.insts[i].x86.jmp_insts = -1;
            else {
                // find jump address instruction
                int k=-1;
                for(int i2=0; i2<helper.size && k==-1; ++i2) {
                    if(helper.insts[i2].x86.addr==j)
                        k=i2;
                }
                if(k!=-1)   // -1 if not found, mmm, probably wrong, exit anyway
                    helper.insts[k].x86.barrier = 1;
                helper.insts[i].x86.jmp_insts = k;
            }
        }
    for(int i=0; i<helper.size; ++i)
        if(helper.insts[i].x86.set_flags && !helper.insts[i].x86.need_flags) {
            helper.insts[i].x86.need_flags = needed_flags(&helper, i+1, helper.insts[i].x86.set_flags, 0);
            if((helper.insts[i].x86.need_flags&X_PEND) && (helper.insts[i].x86.state_flags==SF_MAYSET))
                helper.insts[i].x86.need_flags = X_ALL;
        }
    
    // pass 2, instruction size
    arm_pass2(&helper, addr);
    // ok, now allocate mapped memory, with executable flag on
    int sz = helper.arm_size;
    void* p = (void*)AllocDynarecMap(sz, block->parent->nolinker);
    if(p==NULL) {
        free(helper.insts);
        free(helper.next);
        return;
    }
    helper.block = p;
    helper.tablesz = helper.tablei;
    if(helper.tablesz)
        helper.table = (uintptr_t*)calloc(helper.tablesz, sizeof(uintptr_t));
    // pass 3, emit (log emit arm opcode)
    if(box86_dynarec_dump) dynarec_log(LOG_NONE, "Emitting %d bytes for %d x86 bytes\n", helper.arm_size, helper.isize);
    helper.arm_size = 0;
    arm_pass3(&helper, addr);
    // all done...
    __builtin___clear_cache(p, p+helper.arm_size);   // need to clear the cache before execution...
    free(helper.insts);
    free(helper.next);
    block->table = helper.table;
    block->tablesz = helper.tablesz;
    block->size = sz;
    block->block = p;
    block->done = 1;
}