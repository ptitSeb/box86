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
            dynarec_log(LOG_NONE, "%s%p: Exit x86emu%s\n", (box86_dynarec_dump>1)?"\e[01;33m":"", (void*)ip, (box86_dynarec_dump>1)?"\e[m":"");
        } else {
            dynarec_log(LOG_NONE, "%s%p: Native call to %p%s\n", (box86_dynarec_dump>1)?"\e[01;33m":"", (void*)ip, (void*)a, (box86_dynarec_dump>1)?"\e[m":"");
        }
    } else {
        if(dec) {
            dynarec_log(LOG_NONE, "%s%p: %s", (box86_dynarec_dump>1)?"\e[01;33m":"", ip, DecodeX86Trace(dec, inst->addr));
        } else {
            dynarec_log(LOG_NONE, "%s%p: ", (box86_dynarec_dump>1)?"\e[01;33m":"", ip);
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
        printf_log(LOG_NONE, "Warning, overallocating next\n");
    }
    dyn->next[dyn->next_sz++] = addr;
}
uintptr_t get_closest_next(dynarec_arm_t *dyn, uintptr_t addr) {
    // get closest, but no addresses befores
    uintptr_t best = 0;
    int i = 0;
    while((i<dyn->next_sz) && (best!=addr)) {
        if(dyn->next[i]) {
            if(dyn->next[i]<addr) { // remove the address, it's before current address
                dyn->next[i] = 0;
            } else {
                if((dyn->next[i]<best) || !best)
                    best = dyn->next[i];
            }
        }
        ++i;
    }
    return best;
}
void add_jump(dynarec_arm_t *dyn, int ninst) {
    // add slots
    if(dyn->jmp_sz == dyn->jmp_cap) {
        printf_log(LOG_NONE, "Warning, overallocating jmps\n");
    }
    dyn->jmps[dyn->jmp_sz++] = ninst;
}
int get_first_jump(dynarec_arm_t *dyn, int next) {
    for(int i=0; i<dyn->jmp_sz; ++i)
        if(dyn->insts[dyn->jmps[i]].x86.jmp == next)
            return dyn->jmps[i];
    return -2;
}

#define PK(A) (*((uint8_t*)(addr+(A))))
int is_nops(dynarec_arm_t *dyn, uintptr_t addr, int n)
{
    if(!n)
        return 1;
    if(PK(0)==0x90)
        return is_nops(dyn, addr+1, n-1);
    if(n>1 && PK(0)==0x2E)  // if opcode start with 0x2E, and there is more after, than this *can* be a NOP
        return is_nops(dyn, addr+1, n-1);
    if(n>1 && PK(0)==0x66)  // if opcode start with 0x66, and there is more after, than this *can* be a NOP
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
                case 1: // DEC Ed
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

void addInst(instsize_t* insts, size_t* size, int x86_size, int native_size)
{
    // x86 instruction is <16 bytes
    int toadd;
    if(x86_size>native_size)
        toadd = 1 + x86_size/15;
    else
        toadd = 1 + native_size/15;
    while(toadd) {
        if(x86_size>15)
            insts[*size].x86 = 15;    
        else
            insts[*size].x86 = x86_size;
        x86_size -= insts[*size].x86;
        if(native_size>15)
            insts[*size].nat = 15;
        else
            insts[*size].nat = native_size;
        native_size -= insts[*size].nat;
        ++(*size);
        --toadd;
    }
}

static void recurse_mark_alive(dynarec_arm_t* dyn, int i)
{
    if(dyn->insts[i].x86.alive)
        return;
    dyn->insts[i].x86.alive = 1;
    if(dyn->insts[i].x86.jmp && dyn->insts[i].x86.jmp_insts!=-1)
        recurse_mark_alive(dyn, dyn->insts[i].x86.jmp_insts);
    if(i<dyn->size-1 && dyn->insts[i].x86.has_next)
        recurse_mark_alive(dyn, i+1);
}

static int sizePredecessors(dynarec_arm_t* dyn)
{
    int pred_sz = 1;    // to be safe
    // compute total size of predecessor to allocate the array
    // mark alive...
    recurse_mark_alive(dyn, 0);
    // first compute the jumps
    int jmpto;
    for(int i=0; i<dyn->size; ++i) {
        if(dyn->insts[i].x86.alive && dyn->insts[i].x86.jmp && ((jmpto=dyn->insts[i].x86.jmp_insts)!=-1)) {
            pred_sz++;
            dyn->insts[jmpto].pred_sz++;
        }
    }
    // remove "has_next" from orphan branch
    for(int i=0; i<dyn->size-1; ++i) {
        if(dyn->insts[i].x86.has_next && !dyn->insts[i+1].x86.alive)
            dyn->insts[i].x86.has_next = 0;
    }
    // second the "has_next"
    for(int i=0; i<dyn->size-1; ++i) {
        if(dyn->insts[i].x86.has_next) {
            pred_sz++;
            dyn->insts[i+1].pred_sz++;
        }
    }
    return pred_sz;
}
static void fillPredecessors(dynarec_arm_t* dyn)
{
    // fill pred pointer
    int* p = dyn->predecessor;
    for(int i=0; i<dyn->size; ++i) {
        dyn->insts[i].pred = p;
        p += dyn->insts[i].pred_sz;
        dyn->insts[i].pred_sz=0;  // reset size, it's reused to actually fill pred[]
    }
    // fill pred
    for(int i=0; i<dyn->size; ++i) if(dyn->insts[i].x86.alive) {
        if((i!=dyn->size-1) && dyn->insts[i].x86.has_next)
            dyn->insts[i+1].pred[dyn->insts[i+1].pred_sz++] = i;
        if(dyn->insts[i].x86.jmp && (dyn->insts[i].x86.jmp_insts!=-1)) {
            int j = dyn->insts[i].x86.jmp_insts;
            dyn->insts[j].pred[dyn->insts[j].pred_sz++] = i;
        }
    }
}

// updateNeed goes backward, from last intruction to top
static int updateNeed(dynarec_arm_t* dyn, int ninst, uint8_t need) {
    while (ninst>=0) {
        // need pending but instruction is only a subset: remove pend and use an X_ALL instead
        need |= dyn->insts[ninst].x86.need_after;
        if((need&X_PEND) && (dyn->insts[ninst].x86.state_flags==SF_SUBSET || dyn->insts[ninst].x86.state_flags==SF_SET || dyn->insts[ninst].x86.state_flags==SF_SET_NODF)) {
            need &=~X_PEND;
            need |= X_ALL;
        }
        if((need&X_PEND) && dyn->insts[ninst].x86.state_flags==SF_SUBSET_PENDING) {
            need |= X_ALL&~(dyn->insts[ninst].x86.set_flags);
        }
        dyn->insts[ninst].x86.gen_flags = need&dyn->insts[ninst].x86.set_flags;
        if((need&X_PEND) && (dyn->insts[ninst].x86.state_flags&SF_PENDING))
            dyn->insts[ninst].x86.gen_flags |= X_PEND;
        dyn->insts[ninst].x86.need_after = need;
        need = dyn->insts[ninst].x86.need_after&~dyn->insts[ninst].x86.gen_flags;

        if(dyn->insts[ninst].x86.may_set)
            need |= dyn->insts[ninst].x86.gen_flags;    // forward the flags
        else if((need&X_PEND) && (dyn->insts[ninst].x86.set_flags&SF_PENDING))
            need &=~X_PEND;         // Consume X_PEND if relevant
        need |= dyn->insts[ninst].x86.use_flags;
        if(dyn->insts[ninst].x86.need_before == need)
            return ninst - 1;
        dyn->insts[ninst].x86.need_before = need;
        if(dyn->insts[ninst].x86.barrier&BARRIER_FLAGS) {
            need = need?X_PEND:0;
        }
        int ok = 0;
        for(int i=0; i<dyn->insts[ninst].pred_sz; ++i) {
            if(dyn->insts[ninst].pred[i] == ninst-1)
                ok = 1;
            else
                updateNeed(dyn, dyn->insts[ninst].pred[i], need);
        }
        --ninst;
        if(!ok)
            return ninst;
    }
    return ninst;
}

void* current_helper = NULL;
static int static_jmps[MAX_INSTS+2];
static uintptr_t static_next[MAX_INSTS+2];
static instruction_arm_t static_insts[MAX_INSTS+2] = {0};
// TODO: ninst could be a uint16_t instead of an int, that could same some temp. memory

void CancelBlock(int need_lock)
{
    if(need_lock)
        mutex_lock(&my_context->mutex_dyndump);
    dynarec_arm_t* helper = (dynarec_arm_t*)current_helper;
    if(helper) {
        if(helper->dynablock && helper->dynablock->actual_block) {
            FreeDynarecMap((uintptr_t)helper->dynablock->actual_block);
            helper->dynablock->actual_block = NULL;
        }
    }
    current_helper = NULL;
    if(need_lock)
        mutex_unlock(&my_context->mutex_dyndump);
}

uintptr_t arm_pass0(dynarec_arm_t* dyn, uintptr_t addr);
uintptr_t arm_pass1(dynarec_arm_t* dyn, uintptr_t addr);
uintptr_t arm_pass2(dynarec_arm_t* dyn, uintptr_t addr);
uintptr_t arm_pass3(dynarec_arm_t* dyn, uintptr_t addr);
void arm_epilog();
void arm_next();

void* CreateEmptyBlock(dynablock_t* block, uintptr_t addr) {
    block->isize = 0;
    block->done = 0;
    size_t sz = 8*sizeof(void*);
    void* actual_p = (void*)AllocDynarecMap(sz);
    void* p = actual_p + sizeof(void*);
    if(actual_p==NULL) {
        dynarec_log(LOG_INFO, "AllocDynarecMap(%p, %zu) failed, cancelling block\n", block, sz);
        CancelBlock(0);
        return NULL;
    }
    block->size = sz;
    block->actual_block = actual_p;
    block->block = p;
    block->jmpnext = p;
    *(dynablock_t**)actual_p = block;
    *(void**)(p+4*sizeof(void*)) = arm_epilog;
    CreateJmpNext(block->jmpnext, p+4*sizeof(void*));
    // all done...
    __clear_cache(actual_p, actual_p+sz);   // need to clear the cache before execution...
    return block;
}

void* FillBlock(dynablock_t* block, uintptr_t addr) {
    /*
        A Block must have this layout:

        0x0000..0x0003  : dynablock_t* : self
        0x0004..4+4*n   : actual Native instructions, (n is the total number)
        B ..    B+3     : dynablock_t* : self (as part of JmpNext, that simulate another block)
        B+4 ..  B+15    : 3 Native code for jmpnext (or jmp epilog in case of empty block)
        B+16 .. B+19    : jmpnext (or jmp_epilog) address
        B+20 .. B+31    : empty (in case an architecture needs more than 3 opcodes)
        B+32 .. B+32+sz : instsize (compressed array with each instruction lenght on x86 and native side)

    */
dynarec_log(LOG_DEBUG, "Asked to Fill block %p with %p\n", block, (void*)addr);
    if(addr>=box86_nodynarec_start && addr<box86_nodynarec_end) {
        dynarec_log(LOG_DEBUG, "Asked to fill a block in fobidden zone\n");
        return CreateEmptyBlock(block, addr);
    }
    if(current_helper) {
        dynarec_log(LOG_DEBUG, "Cancelling dynarec FillBlock at %p as anothor one is going on\n", (void*)addr);
        return NULL;
    }
    // protect the 1st page
    protectDB(addr, 1);
    // init the helper
    dynarec_arm_t helper = {0};
    current_helper = &helper;
    helper.dynablock = block;
    helper.start = addr;
    uintptr_t start = addr;
    helper.cap = MAX_INSTS;
    helper.insts = static_insts;
    helper.jmps = static_jmps;
    helper.jmp_cap = MAX_INSTS;
    helper.next = static_next;
    helper.next_cap = MAX_INSTS;
    // pass 0, addresses, x86 jump addresses, overall size of the block
    uintptr_t end = arm_pass0(&helper, addr);
    if(helper.abort) {
        if(box86_dynarec_dump || box86_dynarec_log)dynarec_log(LOG_NONE, "Abort dynablock on pass0\n");
        CancelBlock(0);
        return NULL;
    }
    // basic checks
    if(!helper.size) {
        dynarec_log(LOG_DEBUG, "Warning, null-sized dynarec block (%p)\n", (void*)addr);
        CancelBlock(0);
        return CreateEmptyBlock(block, addr);
    }
    if(!isprotectedDB(addr, 1)) {
        dynarec_log(LOG_INFO, "Warning, write on current page on pass0, aborting dynablock creation (%p)\n", (void*)addr);
        CancelBlock(0);
        return NULL;
    }
    // already protect the block and compute hash signature
    // protect the block of it goes over the 1st page
    if((addr&~box86_pagesize)!=(end&~box86_pagesize)) // need to protect some other pages too
        protectDB(addr, end-addr);  //end is 1byte after actual end
    uint32_t hash = X31_hash_code((void*)addr, end-addr);
    // calculate barriers
    for(int ii=0; ii<helper.jmp_sz; ++ii) {
        int i = helper.jmps[ii];
        uintptr_t j = helper.insts[i].x86.jmp;
        helper.insts[i].x86.jmp_insts = -1;
        if(j<start || j>=end || j==helper.insts[i].x86.addr) {
            if(j==helper.insts[i].x86.addr) // if there is a loop on some opcode, make the block "always to tested"
                helper.always_test = 1;
            helper.insts[i].x86.need_after |= X_PEND;
        } else {
            // find jump address instruction
            int k=-1;
            int search = ((j>=helper.insts[0].x86.addr) && j<helper.insts[0].x86.addr+helper.isize)?1:0;
            int imin = 0;
            int imax = helper.size-1;
            int i2 = helper.size/2;
            // dichotomy search
            while(search) {
                if(helper.insts[i2].x86.addr == j) {
                    k = i2;
                    search = 0;
                } else if(helper.insts[i2].x86.addr>j) {
                    imax = i2;
                    i2 = (imax+imin)/2;
                } else {
                    imin = i2;
                    i2 = (imax+imin)/2;
                }
                if(search && (imax-imin)<2) {
                    search = 0;
                    if(helper.insts[imin].x86.addr==j)
                        k = imin;
                    else if(helper.insts[imax].x86.addr==j)
                        k = imax;
                }
            }
            /*for(int i2=0; i2<helper.size && k==-1; ++i2) {
                if(helper.insts[i2].x86.addr==j)
                    k=i2;
            }*/
            if(k!=-1) {
                if(k!=-1 && !helper.insts[i].barrier_maybe)
                    helper.insts[k].x86.barrier |= BARRIER_FULL;
                helper.insts[i].x86.jmp_insts = k;
            }
        }
    }
    // no need for next and jmps anymore
    helper.next_sz = helper.next_cap = 0;
    helper.next = NULL;
    helper.jmp_sz = helper.jmp_cap = 0;
    helper.jmps = NULL;
    // fill predecessors with the jump address
    int alloc_size = sizePredecessors(&helper);
    helper.predecessor = (int*)alloca(alloc_size*sizeof(int));
    fillPredecessors(&helper);

    int pos = helper.size;
    while (pos>=0)
        pos = updateNeed(&helper, pos, 0);
    // remove fpu stuff on non-executed code
    for(int i=1; i<helper.size-1; ++i)
        if(!helper.insts[i].pred_sz) {
            int ii = i;
            while(ii<helper.size && !helper.insts[ii].pred_sz)
                fpu_reset_ninst(&helper, ii++);
            i = ii;
        }

    // pass 1, float optimisations, first pass for flags
    arm_pass1(&helper, addr);
    if(helper.abort) {
        if(box86_dynarec_dump || box86_dynarec_log)dynarec_log(LOG_NONE, "Abort dynablock on pass0\n");
        CancelBlock(0);
        return NULL;
    }

    // pass 2, instruction size
    arm_pass2(&helper, addr);
    if(helper.abort) {
        if(box86_dynarec_dump || box86_dynarec_log)dynarec_log(LOG_NONE, "Abort dynablock on pass0\n");
        CancelBlock(0);
        return NULL;
    }
    // ok, now allocate mapped memory, with executable flag on
    size_t insts_rsize = (helper.insts_size+2)*sizeof(instsize_t);
    insts_rsize = (insts_rsize+7)&~7;   // round the size...
    size_t arm_size = (helper.arm_size+7)&~7;   // round the size...
    // ok, now allocate mapped memory, with executable flag on
    size_t sz = sizeof(void*) + arm_size + 8*sizeof(void*) + insts_rsize;
    //           dynablock_t* block (arm insts) jmpnext code    instsize
    void* actual_p = (void*)AllocDynarecMap(sz);
    void* p = (void*)(((uintptr_t)actual_p) + sizeof(void*));
    void* next = p + arm_size;
    void* instsize = next + 8*sizeof(void*);
    if(actual_p==NULL) {
        dynarec_log(LOG_INFO, "AllocDynarecMap(%p, %zu) failed, cancelling block\n", block, sz);
        CancelBlock(0);
        return NULL;
    }
    helper.block = p;
    block->actual_block = actual_p;
    helper.arm_start = (uintptr_t)p;
    helper.jmp_next = (uintptr_t)next+sizeof(void*);
    helper.instsize = (instsize_t*)instsize;
    *(dynablock_t**)actual_p = block;
    // pass 3, emit (log emit native opcode)
    if(box86_dynarec_dump) {
        dynarec_log(LOG_NONE, "%s%04d|Emitting %d bytes for %d x86 bytes", (box86_dynarec_dump>1)?"\e[01;36m":"", GetTID(), helper.arm_size, helper.isize); 
        printFunctionAddr(helper.start, " => ");
        dynarec_log(LOG_NONE, "%s\n", (box86_dynarec_dump>1)?"\e[m":"");
    }
    size_t oldarmsize = helper.arm_size;
    size_t oldinstsize = helper.insts_size;
    helper.arm_size = 0;
    helper.insts_size = 0;  // reset
    arm_pass3(&helper, addr);
    if(helper.abort) {
        if(box86_dynarec_dump || box86_dynarec_log)dynarec_log(LOG_NONE, "Abort dynablock on pass0\n");
        CancelBlock(0);
        return NULL;
    }
    // keep size of instructions for signal handling
    block->instsize = instsize;
    // ok, free the helper now
    helper.insts = NULL;
    helper.instsize = NULL;
    helper.predecessor = NULL;
    block->size = sz;
    block->isize = helper.size;
    block->block = p;
    block->jmpnext = next+sizeof(void*);
    block->always_test = helper.always_test;
    block->dirty = block->always_test;
    *(dynablock_t**)next = block;
    *(void**)(next+5*sizeof(void*)) = arm_next;
    CreateJmpNext(block->jmpnext, next+5*sizeof(void*));
    //block->x86_addr = (void*)start;
    block->x86_size = end-start;
    // all done...
    __clear_cache(actual_p, actual_p+sz);   // need to clear the cache before execution...
    block->hash = X31_hash_code(block->x86_addr, block->x86_size);
    // Check if something changed, to abbort if it as
    if((block->hash != hash)) {
        dynarec_log(LOG_DEBUG, "Warning, a block changed while being processed hash(%p:%zu)=%x/%x\n", block->x86_addr, block->x86_size, block->hash, hash);
        CancelBlock(0);
        return NULL;
    }
    if((oldarmsize!=helper.arm_size)) {
        printf_log(LOG_NONE, "BOx86: Warning, size difference in block between pass2 (%zu) & pass3 (%zu)!\n", sz, helper.arm_size);
        uint8_t *dump = (uint8_t*)helper.start;
        printf_log(LOG_NONE, "Dump of %d x86 opcodes:\n", helper.size);
        for(int i=0; i<helper.size; ++i) {
            printf_log(LOG_NONE, "%p:", dump);
            for(; dump<(uint8_t*)helper.insts[i+1].x86.addr; ++dump)
                printf_log(LOG_NONE, " %02X", *dump);
            printf_log(LOG_NONE, "\t%d -> %d\n", helper.insts[i].size2, helper.insts[i].size);
        }
        printf_log(LOG_NONE, " ------------\n");
        CancelBlock(0);
        return NULL;
    }
    // ok, free the helper now
    helper.insts = NULL;
    if(insts_rsize/sizeof(instsize_t)<helper.insts_size) {
        printf_log(LOG_NONE, "BOX86: Warning, ists_size difference in block between pass2 (%zu) and pass3 (%zu), allocated: %zu\n", oldinstsize, helper.insts_size, insts_rsize/sizeof(instsize_t));
    }
    if(!isprotectedDB(addr, end-addr)) {
        dynarec_log(LOG_DEBUG, "Warning, block unprotected while being processed %p:%zu, marking as need_test\n", block->x86_addr, block->x86_size);
        block->dirty = 1;
        //protectDB(addr, end-addr);
    }
    if(getProtection(addr)&PROT_NEVERCLEAN) {
        block->dirty = 1;
        block->always_test = 1;
    }
    if(block->always_test) {
        dynarec_log(LOG_DEBUG, "Note: block marked as always dirty %p:%ld\n", block->x86_addr, block->x86_size);
    }
    current_helper = NULL;
    //block->done = 1;
    return (void*)block;
}
