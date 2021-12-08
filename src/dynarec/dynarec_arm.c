#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "debug.h"
#include "box86context.h"
#include "custommem.h"
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
#include "dynarec_arm_functions.h"
#include "elfloader.h"

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
            dynarec_log(LOG_NONE, "%s%p: %s", (box86_dynarec_dump>1)?"\e[1m":"", ip, DecodeX86Trace(dec, inst->addr));
        } else {
            dynarec_log(LOG_NONE, "%s%p: ", (box86_dynarec_dump>1)?"\e[1m":"", ip);
            for(int i=0; i<inst->size; ++i) {
                dynarec_log(LOG_NONE, "%02X ", ip[i]);
            }
            dynarec_log(LOG_NONE, " %s", name);
        }
        // print Call function name if possible
        if(ip[0]==0xE8 || ip[0]==0xE9) { // Call / Jmp
            uintptr_t nextaddr = (uintptr_t)ip + 5 + *((int32_t*)(ip+1));
            printFunctionAddr(nextaddr, "=> ");
        } else if(ip[0]==0xFF) {
            if(ip[1]==0x25) {
                uintptr_t nextaddr = (uintptr_t)ip + 6 + *((int32_t*)(ip+2));
                printFunctionAddr(nextaddr, "=> ");
            }
        }
        // end of line and colors
        dynarec_log(LOG_NONE, "%s\n", (box86_dynarec_dump>1)?"\e[m":"");
    }
}

void add_next(dynarec_arm_t *dyn, uintptr_t addr) {
    if(!box86_dynarec_bigblock)
        return;
    // exist?
    for(int i=0; i<dyn->next_sz; ++i)
        if(dyn->next[i]==addr)
            return;
    // put in a free slot
    for(int i=0; i<dyn->next_sz; ++i)
        if(!dyn->next[i]) {
            dyn->next[i] = addr;
            return;
        }
    // add slots
    if(dyn->next_sz == dyn->next_cap) {
        dyn->next_cap += 16;
        dyn->next = (uintptr_t*)realloc(dyn->next, dyn->next_cap*sizeof(uintptr_t));
    }
    dyn->next[dyn->next_sz++] = addr;
}
uintptr_t get_closest_next(dynarec_arm_t *dyn, uintptr_t addr) {
    // get closest, but no addresses befores
    uintptr_t best = 0;
    int i = 0;
    while((i<dyn->next_sz) && (best!=addr)) {
        if(dyn->next[i]<addr) { // remove the address, it's before current address
            dyn->next[i] = 0;
        } else {
            if((dyn->next[i]<best) || !best)
                best = dyn->next[i];
        }
        ++i;
    }
    return best;
}
#define PK(A) (*((uint8_t*)(addr+(A))))
int is_nops(dynarec_arm_t *dyn, uintptr_t addr, int n)
{
    if(!n)
        return 1;
    if(PK(0)==0x90)
        return is_nops(dyn, addr+1, n-1);
    if(n>1 && PK(0)==0x66)  // if opcode start with 0x66, and there is more after, than is *can* be a NOP
        return is_nops(dyn, addr+1, n-1);
    if(n>2 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x00)
        return is_nops(dyn, addr+3, n-3);
    if(n>2 && PK(0)==0x8d && PK(1)==0x76 && PK(2)==0x00)    // lea esi, [esi]
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
    if(n>6 && PK(0)==0x8d && PK(1)==0xb4 && PK(2)==0x26 && PK(3)==0x00 && PK(4)==0x00 && PK(5)==0x00 && PK(6)==0x00) // lea esi, [esi+0]
        return is_nops(dyn, addr+7, n-7);
    if(n>7 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x84 && PK(3)==0x00 && PK(4)==0x00 && PK(5)==0x00 && PK(6)==0x00 && PK(7)==0x00)
        return is_nops(dyn, addr+8, n-8);
    return 0;
}

// return size of next instuciton, -1 is unknown
// not all instrction are setup
int next_instruction(dynarec_arm_t *dyn, uintptr_t addr)
{
    uint8_t opcode = PK(0);
    uint8_t nextop;
    switch (opcode) {
        case 0x66:
            opcode = PK(1);
            switch(opcode) {
                case 0x90:
                    return 2;
            }
            break;
        case 0x81:
            nextop = PK(1);
            return fakeed(dyn, addr+2, 0, nextop)-addr + 4;
        case 0x83:
            nextop = PK(1);
            return fakeed(dyn, addr+2, 0, nextop)-addr + 1;
        case 0x84:
        case 0x85:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
            nextop = PK(1);
            return fakeed(dyn, addr+2, 0, nextop)-addr;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
            return 1;
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
            return 5;
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
            return 2;
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            return 5;
        case 0xFF:
            nextop = PK(1);
            switch((nextop>>3)&7) {
                case 0: // INC Ed
                case 1: //DEC Ed
                case 2: // CALL Ed
                case 4: // JMP Ed
                case 6: // Push Ed
                    return fakeed(dyn, addr+2, 0, nextop)-addr;
            }
            break;
        default:
            break;
    }
    return -1;
}
#undef PK

int is_instructions(dynarec_arm_t *dyn, uintptr_t addr, int n)
{
    int i = 0;
    while(i<n) {
        int j=next_instruction(dyn, addr+i);
        if(j<=0) return 0;
        i+=j;
    }
    return (i==n)?1:0;
}

instsize_t* addInst(instsize_t* insts, size_t* size, size_t* cap, int x86_size, int arm_size)
{
    // x86 instruction is <16 bytes
    int toadd;
    if(x86_size>arm_size)
        toadd = 1 + x86_size/15;
    else
        toadd = 1 + arm_size/15;
    if((*size)+toadd>(*cap)) {
        *cap = (*size)+toadd;
        insts = (instsize_t*)realloc(insts, (*cap)*sizeof(instsize_t));
    }
    while(toadd) {
        if(x86_size>15)
            insts[*size].x86 = 15;    
        else
            insts[*size].x86 = x86_size;
        x86_size -= insts[*size].x86;
        if(arm_size>15)
            insts[*size].nat = 15;
        else
            insts[*size].nat = arm_size;
        arm_size -= insts[*size].nat;
        ++(*size);
        --toadd;
    }
    return insts;
}

static void fillPredecessors(dynarec_arm_t* dyn)
{
    int pred_sz = 0;
    // compute total size of predecessor to alocate the array
    // first compute the jumps
    for(int i=0; i<dyn->size; ++i) {
        if(dyn->insts[i].x86.jmp && dyn->insts[i].x86.jmp_insts!=-1) {
            ++pred_sz;
            ++dyn->insts[dyn->insts[i].x86.jmp_insts].pred_sz;
        }
    }
    // second the "has_next"
    for(int i=0; i<dyn->size; ++i) {
        if(i!=dyn->size-1 && dyn->insts[i].x86.has_next && (!i || dyn->insts[i].pred_sz)) {
            ++pred_sz;
            ++dyn->insts[i+1].pred_sz;
        }
    }
    dyn->predecessor = (int*)malloc(pred_sz*sizeof(int));
    // fill pred pointer
    int* p = dyn->predecessor;
    for(int i=0; i<dyn->size; ++i) {
        dyn->insts[i].pred = p;
        p += dyn->insts[i].pred_sz;
        dyn->insts[i].pred_sz=0;  // reset size, it's reused to actually fill pred[]
    }
    assert(p<dyn->predecessor+pred_sz);
    // fill pred
    for(int i=0; i<dyn->size; ++i) {
        if(i!=dyn->size-1 && dyn->insts[i].x86.has_next && (!i || dyn->insts[i].pred_sz))
            dyn->insts[i+1].pred[dyn->insts[i+1].pred_sz++] = i;
        if(dyn->insts[i].x86.jmp && dyn->insts[i].x86.jmp_insts!=-1)
            dyn->insts[dyn->insts[i].x86.jmp_insts].pred[dyn->insts[dyn->insts[i].x86.jmp_insts].pred_sz++] = i;
    }

}

uintptr_t arm_pass0(dynarec_arm_t* dyn, uintptr_t addr);
uintptr_t arm_pass1(dynarec_arm_t* dyn, uintptr_t addr);
uintptr_t arm_pass2(dynarec_arm_t* dyn, uintptr_t addr);
uintptr_t arm_pass3(dynarec_arm_t* dyn, uintptr_t addr);

__thread void* current_helper = NULL;

void CancelBlock()
{
    dynarec_arm_t* helper = (dynarec_arm_t*)current_helper;
    current_helper = NULL;
    if(!helper)
        return;
    free(helper->next);
    free(helper->insts);
    free(helper->predecessor);
    free(helper->sons_x86);
    free(helper->sons_arm);
    if(helper->dynablock && helper->dynablock->block)
        FreeDynarecMap(helper->dynablock, (uintptr_t)helper->dynablock->block, helper->dynablock->size);
}

void* FillBlock(dynablock_t* block, uintptr_t addr) {
dynarec_log(LOG_DEBUG, "Asked to Fill block %p with %p\n", block, (void*)addr);
    if(addr>=box86_nodynarec_start && addr<box86_nodynarec_end) {
        dynarec_log(LOG_DEBUG, "Asked to fill a block in fobidden zone\n");
        return NULL;
    }
    if(!isJumpTableDefault((void*)addr)) {
        dynarec_log(LOG_DEBUG, "Asked to fill a block at %p, but JumpTable is not default\n", (void*)addr);
        return NULL;
    }
    // init the helper
    dynarec_arm_t helper = {0};
    current_helper = &helper;
    helper.dynablock = block;
    helper.start = addr;
    uintptr_t start = addr;
    helper.cap = 64; // needs epilog handling
    helper.insts = (instruction_arm_t*)calloc(helper.cap, sizeof(instruction_arm_t));
    // pass 0, addresses, x86 jump addresses, overall size of the block
    uintptr_t end = arm_pass0(&helper, addr);
    // no need for next anymore
    free(helper.next);
    helper.next_sz = helper.next_cap = 0;
    helper.next = NULL;
    // basic checks
    if(!helper.size) {
        dynarec_log(LOG_DEBUG, "Warning, null-sized dynarec block (%p)\n", (void*)addr);
        CancelBlock();
        return (void*)block;
    }
    // already protect the block and compute hash signature
    protectDB(addr, end-addr);  //end is 1byte after actual end
    uint32_t hash = X31_hash_code((void*)addr, end-addr);
    // Compute flag_need, without taking into account any barriers
    uint32_t last_need = X_PEND;
    for(int i = helper.size; i-- > 0;) {
        last_need |= helper.insts[i].x86.use_flags;
        if((helper.insts[i].x86.state_flags == SF_SUBSET) && (last_need&X_PEND))
            last_need = (X_ALL & (~helper.insts[i].x86.set_flags) ) | helper.insts[i].x86.use_flags;
        if (last_need == (X_PEND | X_ALL)) {
            last_need = X_ALL;
        }
        helper.insts[i].x86.need_flags = last_need;
        if ((helper.insts[i].x86.set_flags) && !(helper.insts[i].x86.state_flags & SF_MAYSET)) {
            if (last_need & X_PEND) {
                last_need = (~helper.insts[i].x86.set_flags) & X_ALL;
            } else {
                last_need &= ~helper.insts[i].x86.set_flags;
            }
        }
    }
    // calculate barriers
    for(int i=0; i<helper.size; ++i)
        if(helper.insts[i].x86.jmp) {
            uintptr_t j = helper.insts[i].x86.jmp;
            if(j<start || j>=end) {
                helper.insts[i].x86.jmp_insts = -1;
            } else {
                // find jump address instruction
                int k=-1;
                for(int i2=0; i2<helper.size && k==-1; ++i2) {
                    if(helper.insts[i2].x86.addr==j)
                        k=i2;
                }
                if(k!=-1)   // -1 if not found, mmm, probably wrong, exit anyway
                    helper.insts[k].x86.barrier = BARRIER_FULL;
                helper.insts[i].x86.jmp_insts = k;
            }
        }
    // fill predecessors with the jump address
    fillPredecessors(&helper);
    // check for the optionnal barriers now
    for(int i=helper.size-1; i>=0; --i) {
        if(helper.insts[i].barrier_maybe)
            if(helper.insts[i].x86.jmp_insts == -1) {
                if(i==helper.size-1 || helper.insts[i+1].x86.barrier)
                    helper.insts[i].x86.barrier=BARRIER_FLOAT;  // nope, end of block or barrier just after
                else
                    helper.insts[i].x86.barrier=BARRIER_NONE;  // ok, no need for a barrier here after all
            } else 
                helper.insts[i].x86.barrier=BARRIER_FLOAT;
    }
    // check to remove useless barrier, in case of jump when destination doesn't needs flags
    for(int i=helper.size-1; i>=0; --i) {
        if(helper.insts[i].x86.jmp
        && helper.insts[i].x86.jmp_insts>=0
        && helper.insts[helper.insts[i].x86.jmp_insts].x86.barrier==BARRIER_FULL) {
            int k = helper.insts[i].x86.jmp_insts;
            //TODO: optimize FPU barrier too
            if((!helper.insts[k].x86.need_flags)
             ||(helper.insts[k].x86.set_flags==X_ALL
                  && helper.insts[k].x86.state_flags==SF_SET)
             ||(helper.insts[k].x86.state_flags==SF_SET_PENDING)) {
                //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "Removed barrier for inst %d\n", k);
                helper.insts[k].x86.barrier = BARRIER_FLOAT; // remove barrier (keep FPU barrier, and still reset state flag)
             }
        }
    }
    // reset need_flags and compute again, now taking barrier into account (because barrier change use_flags)
    for(int i = helper.size; i-- > 0;) {
        if(helper.insts[i].x86.barrier==BARRIER_FULL)
            // immediate barrier
            helper.insts[i].x86.use_flags |= X_PEND;
        else if(helper.insts[i].x86.jmp 
        && helper.insts[i].x86.jmp_insts>=0
        ) {
            if(helper.insts[helper.insts[i].x86.jmp_insts].x86.barrier==BARRIER_FULL)
                // jumpto barrier
                helper.insts[i].x86.use_flags |= X_PEND;
            else
                helper.insts[i].x86.use_flags |= helper.insts[helper.insts[i].x86.jmp_insts].x86.need_flags;
        }
    }
    for(int i = helper.size; i-- > 0;)
        helper.insts[i].x86.need_flags = 0;
    last_need = X_PEND;
    for(int i = helper.size; i-- > 0;) {
        helper.insts[i].x86.need_flags = last_need;
        if((helper.insts[i].x86.state_flags == SF_SUBSET) && (last_need&X_PEND))
            last_need = (X_ALL & (~helper.insts[i].x86.set_flags) ) | helper.insts[i].x86.use_flags;
        if ((helper.insts[i].x86.set_flags) && !(helper.insts[i].x86.state_flags & SF_MAYSET)) {
            if (last_need & X_PEND) {
                last_need = (~helper.insts[i].x86.set_flags) & X_ALL;
            } else {
                last_need &= ~helper.insts[i].x86.set_flags;
            }
        }
        last_need |= helper.insts[i].x86.use_flags;
        if (last_need == (X_PEND | X_ALL)) {
            last_need = X_ALL;
        }
    }

    // pass 1, float optimisations
    arm_pass1(&helper, addr);
    
    // pass 2, instruction size
    arm_pass2(&helper, addr);
    // ok, now allocate mapped memory, with executable flag on
    int sz = helper.arm_size;
    void* p = (void*)AllocDynarecMap(block, sz);
    if(p==NULL) {
        dynarec_log(LOG_DEBUG, "AllocDynarecMap(%p, %d) failed, cancelling block\n", block, sz);
        CancelBlock();
        return NULL;
    }
    helper.block = p;
    helper.arm_start = (uintptr_t)p;
    if(helper.sons_size) {
        helper.sons_x86 = (uintptr_t*)calloc(helper.sons_size, sizeof(uintptr_t));
        helper.sons_arm = (void**)calloc(helper.sons_size, sizeof(void*));
    }
    // pass 3, emit (log emit arm opcode)
    if(box86_dynarec_dump) {
        dynarec_log(LOG_NONE, "%s%04d|Emitting %d bytes for %d x86 bytes", (box86_dynarec_dump>1)?"\e[01;36m":"", GetTID(), helper.arm_size, helper.isize); 
        printFunctionAddr(helper.start, " => ");
        dynarec_log(LOG_NONE, "%s\n", (box86_dynarec_dump>1)?"\e[m":"");
    }
    helper.arm_size = 0;
    arm_pass3(&helper, addr);
    if(sz!=helper.arm_size) {
        printf_log(LOG_NONE, "BOX86: Warning, size difference in block between pass2 (%d) & pass3 (%d)!\n", sz, helper.arm_size);
        uint8_t *dump = (uint8_t*)helper.start;
        printf_log(LOG_NONE, "Dump of %d x86 opcodes:\n", helper.size);
        for(int i=0; i<helper.size; ++i) {
            printf_log(LOG_NONE, "%p:", dump);
            for(; dump<(uint8_t*)helper.insts[i+1].x86.addr; ++dump)
                printf_log(LOG_NONE, " %02X", *dump);
            printf_log(LOG_NONE, "\t%d -> %d\n", helper.insts[i].size2, helper.insts[i].size);
        }
        printf_log(LOG_NONE, " ------------\n");
    }
    // all done...
    __clear_cache(p, p+sz);   // need to clear the cache before execution...
    // keep size of instructions for signal handling
    {
        size_t cap = 1;
        for(int i=0; i<helper.size; ++i)
            cap += 1 + ((helper.insts[i].x86.size>helper.insts[i].size)?helper.insts[i].x86.size:helper.insts[i].size)/15;
        size_t size = 0;
        block->instsize = (instsize_t*)calloc(cap, sizeof(instsize_t));
        for(int i=0; i<helper.size; ++i)
            block->instsize = addInst(block->instsize, &size, &cap, helper.insts[i].x86.size, helper.insts[i].size/4);
        block->instsize = addInst(block->instsize, &size, &cap, 0, 0);    // add a "end of block" mark, just in case
    }
    // ok, free the helper now
    free(helper.insts);
    helper.insts = NULL;
    free(helper.next);
    helper.next = NULL;
    block->size = sz;
    block->isize = helper.size;
    block->block = p;
    block->need_test = 0;
    //block->x86_addr = (void*)start;
    block->x86_size = end-start;
    if(box86_dynarec_largest<block->x86_size)
        box86_dynarec_largest = block->x86_size;
    block->hash = X31_hash_code(block->x86_addr, block->x86_size);
    // Check if something changed, to abbort if it as
    if(block->hash != hash) {
        dynarec_log(LOG_INFO, "Warning, a block changed while beeing processed hash(%p:%d)=%x/%x\n", block->x86_addr, block->x86_size, block->hash, hash);
        CancelBlock();
        return NULL;
    }
    // fill sons if any
    dynablock_t** sons = NULL;
    int sons_size = 0;
    if(helper.sons_size) {
        sons = (dynablock_t**)calloc(helper.sons_size, sizeof(dynablock_t*));
        for (int i=0; i<helper.sons_size; ++i) {
            int created = 1;
            dynablock_t *son = AddNewDynablock(block->parent, helper.sons_x86[i], &created);
            if(created) {    // avoid breaking a working block!
                son->block = helper.sons_arm[i];
                son->x86_addr = (void*)helper.sons_x86[i];
                son->x86_size = end-helper.sons_x86[i];
                if(!son->x86_size) {printf_log(LOG_NONE, "Warning, son with null x86 size! (@%p / ARM=%p)", son->x86_addr, son->block);}
                son->father = block;
                son->done = 1;
                sons[sons_size++] = son;
                if(!son->parent)
                    son->parent = block->parent;
            }
        }
        if(sons_size) {
            block->sons = sons;
            block->sons_size = sons_size;
        } else
            free(sons);
    }
    free(helper.predecessor);
    helper.predecessor = NULL;
    free(helper.sons_x86);
    helper.sons_x86 = NULL;
    free(helper.sons_arm);
    helper.sons_arm = NULL;
    current_helper = NULL;
    block->done = 1;
    return (void*)block;
}