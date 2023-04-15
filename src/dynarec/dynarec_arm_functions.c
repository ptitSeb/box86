#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

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
#include "emu/x87emu_private.h"
#include "x86trace.h"
#include "signals.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "dynarec_arm_functions.h"
#include "bridge.h"
#include "arm_printer.h"

void arm_fstp(x86emu_t* emu, void* p)
{
    if(ST0.q!=STld(0).uref)
        D2LD(&ST0.d, p);
    else
        memcpy(p, &STld(0).ld, 10);
}

void arm_print_armreg(x86emu_t* emu, uintptr_t reg, uintptr_t n)
{
    dynarec_log(LOG_DEBUG, "R%d=0x%x (%d)\n", n, reg, reg);
}

void arm_f2xm1(x86emu_t* emu)
{
    ST0.d = exp2(ST0.d) - 1.0;
}
void arm_fyl2x(x86emu_t* emu)
{
    ST(1).d = log2(ST0.d)*ST(1).d;
}
void arm_ftan(x86emu_t* emu)
{
    ST0.d = tan(ST0.d);
    emu->sw.f.F87_C2 = 0;
}
void arm_fpatan(x86emu_t* emu)
{
    ST1.d = atan2(ST1.d, ST0.d);
}
void arm_fxtract(x86emu_t* emu)
{
    int32_t tmp32s = (ST1.q&0x7ff0000000000000LL)>>52;
    tmp32s -= 1023;
    ST1.d /= exp2(tmp32s);
    ST0.d = tmp32s;
}
void arm_fprem(x86emu_t* emu)
{
    int64_t tmp64s = ST0.d / ST1.d;
    ST0.d -= ST1.d * tmp64s;
    emu->sw.f.F87_C2 = 0;
    emu->sw.f.F87_C1 = (tmp64s&1);
    emu->sw.f.F87_C3 = ((tmp64s>>1)&1);
    emu->sw.f.F87_C0 = ((tmp64s>>2)&1);
}
void arm_frndint(x86emu_t* emu)
{
    ST0.d = fpu_round(emu, ST0.d);
}
void arm_fscale(x86emu_t* emu)
{
    if(ST0.d!=0.0)
        ST0.d *= exp2(trunc(ST1.d));
}
void arm_fbld(x86emu_t* emu, uint8_t* ed)
{
    fpu_fbld(emu, ed);
}

void arm_fild64(x86emu_t* emu, int64_t* ed)
{
    int64_t tmp;
    memcpy(&tmp, ed, sizeof(tmp));
    ST0.d = tmp;
    STll(0).sq = tmp;
    STll(0).sref = ST0.sq;
}

void arm_fbstp(x86emu_t* emu, uint8_t* ed)
{
    fpu_fbst(emu, ed);
}

void arm_fistp64(x86emu_t* emu, int64_t* ed)
{
    // used of memcpy to avoid aligments issues
    if(STll(0).sref==ST(0).sq) {
        memcpy(ed, &STll(0).sq, sizeof(int64_t));
    } else {
        int64_t tmp;
        if(isgreater(ST0.d, (double)(int64_t)0x7fffffffffffffffLL) || isless(ST0.d, (double)(int64_t)0x8000000000000000LL) || !isfinite(ST0.d))
            tmp = 0x8000000000000000LL;
        else
            tmp = fpu_round(emu, ST0.d);
        memcpy(ed, &tmp, sizeof(tmp));
    }
}

int64_t arm_fist64_0(double d)
{
    int64_t tmp;
    if(isgreater(d, (double)(int64_t)0x7fffffffffffffffLL) || isless(d, (double)(int64_t)0x8000000000000000LL) || !isfinite(d))
        tmp = 0x8000000000000000LL;
    else
        tmp = nearbyint(d);
    return tmp;
}
int64_t arm_fist64_1(double d)
{
    int64_t tmp;
    if(isgreater(d, (double)(int64_t)0x7fffffffffffffffLL) || isless(d, (double)(int64_t)0x8000000000000000LL) || !isfinite(d))
        tmp = 0x8000000000000000LL;
    else
        tmp = floor(d);
    return tmp;
}
int64_t arm_fist64_2(double d)
{
    int64_t tmp;
    if(isgreater(d, (double)(int64_t)0x7fffffffffffffffLL) || isless(d, (double)(int64_t)0x8000000000000000LL) || !isfinite(d))
        tmp = 0x8000000000000000LL;
    else
        tmp = ceil(d);
    return tmp;
}
int64_t arm_fist64_3(double d)
{
    int64_t tmp;
    if(isgreater(d, (double)(int64_t)0x7fffffffffffffffLL) || isless(d, (double)(int64_t)0x8000000000000000LL) || !isfinite(d))
        tmp = 0x8000000000000000LL;
    else
        tmp = trunc(d);
    return tmp;
}

void arm_fistt64(x86emu_t* emu, int64_t* ed)
{
    // used of memcpy to avoid aligments issues
    int64_t tmp = ST0.d;
    memcpy(ed, &tmp, sizeof(tmp));
}

void arm_fld(x86emu_t* emu, uint8_t* ed)
{
    memcpy(&STld(0).ld, ed, 10);
    LD2D(&STld(0).ld, &ST(0).d);
    STld(0).uref = ST0.q;
}

void arm_ud(x86emu_t* emu)
{
    emit_signal(emu, SIGILL, (void*)R_EIP, 0);
}

void arm_fsave(x86emu_t* emu, uint8_t* ed)
{
    fpu_savenv(emu, (char*)ed, 0);

    uint8_t* p = ed;
    p += 28;
    for (int i=0; i<8; ++i) {
        D2LD(&ST(i).d, p);
        p+=10;
    }
}
void arm_frstor(x86emu_t* emu, uint8_t* ed)
{
    fpu_loadenv(emu, (char*)ed, 0);

    uint8_t* p = ed;
    p += 28;
    for (int i=0; i<8; ++i) {
        LD2D(p, &ST(i).d);
        p+=10;
    }

}

void arm_fprem1(x86emu_t* emu)
{
    // simplified version
    int64_t tmp64s = lrint(ST0.d / ST1.d);
    ST0.d -= ST1.d*tmp64s;
    emu->sw.f.F87_C2 = 0;
    emu->sw.f.F87_C1 = (tmp64s&1);
    emu->sw.f.F87_C3 = ((tmp64s>>1)&1);
    emu->sw.f.F87_C0 = ((tmp64s>>2)&1);
}


// Get a FPU single scratch reg
int fpu_get_scratch_single(dynarec_arm_t* dyn)
{
    return dyn->n.fpu_scratch++;  // return an Sx
}
// Get a FPU double scratch reg
int fpu_get_scratch_double(dynarec_arm_t* dyn)
{
    int i = (dyn->n.fpu_scratch+1)&(~1);
    dyn->n.fpu_scratch = i+2;
    return i/2; // return a Dx
}
// Get a FPU quad scratch reg
int fpu_get_scratch_quad(dynarec_arm_t* dyn)
{
    if(dyn->n.fpu_scratch>4) {
        if(dyn->n.fpu_extra_qscratch) {
            dynarec_log(LOG_NONE, "Warning, Extra QScratch slot taken and need another one!\n");
        } else
            dyn->n.fpu_extra_qscratch = fpu_get_reg_quad(dyn, NEON_CACHE_SCR, 0);
        return dyn->n.fpu_extra_qscratch;
    }
    int i = (dyn->n.fpu_scratch+3)&(~3);
    dyn->n.fpu_scratch = i+4;
    return i/2; // Return a Dx, not a Qx
}
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_arm_t* dyn)
{
    dyn->n.fpu_scratch = 0;
    if(dyn->n.fpu_extra_qscratch) {
        fpu_free_reg_quad(dyn, dyn->n.fpu_extra_qscratch);
        dyn->n.fpu_extra_qscratch = 0;
    }
}
// Get a FPU double reg
int fpu_get_reg_double(dynarec_arm_t* dyn, unsigned int t, unsigned int n)
{
    // TODO: check upper limit?
    int i=0;
    while (dyn->n.fpuused[i] && (i<24)) ++i;
    assert(i<24);

    dyn->n.fpuused[i] = 1;
    dyn->n.neoncache[i].n = n;
    dyn->n.neoncache[i].t = t;
    dyn->n.news |= (1<<i);
    return i+FPUFIRST; // return a Dx
}
// Free a FPU double reg
void fpu_free_reg_double(dynarec_arm_t* dyn, int reg)
{
    // TODO: check upper limit?
    int i=reg-FPUFIRST;
    dyn->n.fpuused[i] = 0;
    if(dyn->n.neoncache[i].t!=NEON_CACHE_ST_F && dyn->n.neoncache[i].t!=NEON_CACHE_ST_D)
        dyn->n.neoncache[i].v = 0;
}
// Get a FPU quad reg
int fpu_get_reg_quad(dynarec_arm_t* dyn, unsigned int t, unsigned int n)
{
    int i=0;
    while ((dyn->n.fpuused[i] || dyn->n.fpuused[i+1]) && (i<24)) i+=2;
    assert(i<24);
    dyn->n.fpuused[i] = dyn->n.fpuused[i+1] = 1;
    dyn->n.neoncache[i].t = t;
    dyn->n.neoncache[i].n = n;
    dyn->n.neoncache[i+1].t = t;
    dyn->n.neoncache[i+1].n = n;
    dyn->n.news |= (3<<i);
    return i+FPUFIRST; // Return a Dx, not a Qx
}
// Free a FPU quad reg
void fpu_free_reg_quad(dynarec_arm_t* dyn, int reg)
{
    int i=reg-FPUFIRST;
    dyn->n.fpuused[i] = dyn->n.fpuused[i+1] = 0;
    dyn->n.neoncache[i].v = 0;
    dyn->n.neoncache[i+1].v = 0;
}
// Reset fpu regs counter
void fpu_reset_reg(dynarec_arm_t* dyn)
{
    dyn->n.fpu_reg = 0;
    for (int i=0; i<24; ++i) {
        dyn->n.fpuused[i]=0;
        dyn->n.neoncache[i].v = 0;
    }
}

int neoncache_get_st(dynarec_arm_t* dyn, int ninst, int a)
{
    if (dyn->insts[ninst].n.swapped) {
        if(dyn->insts[ninst].n.combined1 == a)
            a = dyn->insts[ninst].n.combined2;
        else if(dyn->insts[ninst].n.combined2 == a)
            a = dyn->insts[ninst].n.combined1;
    }
    for(int i=0; i<24; ++i)
        if((dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
         || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_D)
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return dyn->insts[ninst].n.neoncache[i].t;
    // not in the cache yet, so will be fetched...
    return NEON_CACHE_ST_D;
}

int neoncache_get_current_st(dynarec_arm_t* dyn, int ninst, int a)
{
    (void)ninst;
    if(!dyn->insts)
        return NEON_CACHE_ST_D;
    for(int i=0; i<24; ++i)
        if((dyn->n.neoncache[i].t==NEON_CACHE_ST_F
         || dyn->n.neoncache[i].t==NEON_CACHE_ST_D)
         && dyn->n.neoncache[i].n==a)
            return dyn->n.neoncache[i].t;
    // not in the cache yet, so will be fetched...
    return NEON_CACHE_ST_D;
}

int neoncache_get_st_f(dynarec_arm_t* dyn, int ninst, int a)
{
    /*if(a+dyn->insts[ninst].n.stack_next-st<0)
        // The STx has been pushed at the end of instructon, so stop going back
        return -1;*/
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return i;
    return -1;
} 
int neoncache_get_st_f_noback(dynarec_arm_t* dyn, int ninst, int a)
{
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return i;
    return -1;
} 
int neoncache_get_current_st_f(dynarec_arm_t* dyn, int a)
{
    for(int i=0; i<24; ++i)
        if(dyn->n.neoncache[i].t==NEON_CACHE_ST_F
         && dyn->n.neoncache[i].n==a)
            return i;
    return -1;
} 
static void neoncache_promote_double_forward(dynarec_arm_t* dyn, int ninst, int maxinst, int a);
static void neoncache_promote_double_internal(dynarec_arm_t* dyn, int ninst, int maxinst, int a);
static void neoncache_promote_double_combined(dynarec_arm_t* dyn, int ninst, int maxinst, int a)
{
    if(a == dyn->insts[ninst].n.combined1 || a == dyn->insts[ninst].n.combined2) {
        if(a == dyn->insts[ninst].n.combined1) {
            a = dyn->insts[ninst].n.combined2;
        } else 
            a = dyn->insts[ninst].n.combined1;
        int i = neoncache_get_st_f_noback(dyn, ninst, a);
        //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_combined, ninst=%d combined%c %d i=%d (stack:%d/%d)\n", ninst, (a == dyn->insts[ninst].n.combined2)?'2':'1', a ,i, dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop);
        if(i>=0) {
            dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
            if(!dyn->insts[ninst].n.barrier)
                neoncache_promote_double_internal(dyn, ninst-1, maxinst, a-dyn->insts[ninst].n.stack_push);
            // go forward is combined is not pop'd
            if(a-dyn->insts[ninst].n.stack_pop>=0)
                if(!dyn->insts[ninst+1].n.barrier)
                    neoncache_promote_double_forward(dyn, ninst+1, maxinst, a-dyn->insts[ninst].n.stack_pop);
        }
    }
}
static void neoncache_promote_double_internal(dynarec_arm_t* dyn, int ninst, int maxinst, int a)
{
    if(dyn->insts[ninst+1].n.barrier)
        return;
    while(ninst>=0) {
        a+=dyn->insts[ninst].n.stack_pop;    // adjust Stack depth: add pop'd ST (going backward)
        int i = neoncache_get_st_f(dyn, ninst, a);
        //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_internal, ninst=%d, a=%d st=%d:%d, i=%d\n", ninst, a, dyn->insts[ninst].n.stack, dyn->insts[ninst].n.stack_next, i);
        if(i<0) return;
        dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
        // check combined propagation too
        if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) {
            if(dyn->insts[ninst].n.swapped) {
                //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_internal, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
                if (a==dyn->insts[ninst].n.combined1)
                    a = dyn->insts[ninst].n.combined2;
                else if (a==dyn->insts[ninst].n.combined2)
                    a = dyn->insts[ninst].n.combined1;
            } else {
                //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_internal, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
                neoncache_promote_double_combined(dyn, ninst, maxinst, a);
            }
        }
        a-=dyn->insts[ninst].n.stack_push;  // // adjust Stack depth: remove push'd ST (going backward)
        --ninst;
        if(ninst<0 || a<0 || dyn->insts[ninst].n.barrier)
            return;
    }
}

static void neoncache_promote_double_forward(dynarec_arm_t* dyn, int ninst, int maxinst, int a)
{
    while((ninst!=-1) && (ninst<maxinst) && (a>=0)) {
        a+=dyn->insts[ninst].n.stack_push;  // // adjust Stack depth: add push'd ST (going forward)
        if((dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) && dyn->insts[ninst].n.swapped) {
            //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_forward, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
            if (a==dyn->insts[ninst].n.combined1)
                a = dyn->insts[ninst].n.combined2;
            else if (a==dyn->insts[ninst].n.combined2)
                a = dyn->insts[ninst].n.combined1;
        }
        int i = neoncache_get_st_f_noback(dyn, ninst, a);
        //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_forward, ninst=%d, a=%d st=%d:%d(%d/%d), i=%d\n", ninst, a, dyn->insts[ninst].n.stack, dyn->insts[ninst].n.stack_next, dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop, i);
        if(i<0) return;
        dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
        // check combined propagation too
        if((dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) && !dyn->insts[ninst].n.swapped) {
            //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_forward, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
            neoncache_promote_double_combined(dyn, ninst, maxinst, a);
        }
        a-=dyn->insts[ninst].n.stack_pop;    // adjust Stack depth: remove pop'd ST (going forward)
        if(dyn->insts[ninst].x86.has_next && !dyn->insts[ninst].n.barrier)
            ++ninst;
        else
            ninst=-1;
    }
    if(ninst==maxinst)
        neoncache_promote_double(dyn, ninst, a);
}

void neoncache_promote_double(dynarec_arm_t* dyn, int ninst, int a)
{
    int i = neoncache_get_current_st_f(dyn, a);
    //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double, ninst=%d a=%d st=%d i=%d\n", ninst, a, dyn->n.stack, i);
    if(i<0) return;
    dyn->n.neoncache[i].t = NEON_CACHE_ST_D;
    dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
    // check combined propagation too
    if(dyn->n.combined1 || dyn->n.combined2) {
        if(dyn->n.swapped) {
            //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double, ninst=%d swapped! %d/%d vs %d\n", ninst, dyn->n.combined1 ,dyn->n.combined2, a);
            if(dyn->n.combined1 == a)
                a = dyn->n.combined2;
            else if(dyn->n.combined2 == a)
                a = dyn->n.combined1;
        } else {
            //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double, ninst=%d combined! %d/%d vs %d\n", ninst, dyn->n.combined1 ,dyn->n.combined2, a);
            if(dyn->n.combined1 == a)
                neoncache_promote_double(dyn, ninst, dyn->n.combined2);
            else if(dyn->n.combined2 == a)
                neoncache_promote_double(dyn, ninst, dyn->n.combined1);
        }
    }
    a-=dyn->insts[ninst].n.stack_push;  // // adjust Stack depth: remove push'd ST (going backward)
    if(!ninst || a<0) return;
    neoncache_promote_double_internal(dyn, ninst-1, ninst, a);
}

int neoncache_combine_st(dynarec_arm_t* dyn, int ninst, int a, int b)
{
    dyn->n.combined1=a;
    dyn->n.combined2=b;
    if( neoncache_get_current_st(dyn, ninst, a)==NEON_CACHE_ST_F
     && neoncache_get_current_st(dyn, ninst, b)==NEON_CACHE_ST_F )
        return NEON_CACHE_ST_F;
    return NEON_CACHE_ST_D;
}

int isPred(dynarec_arm_t* dyn, int ninst, int pred) {
    for(int i=0; i<dyn->insts[ninst].pred_sz; ++i)
        if(dyn->insts[ninst].pred[i]==pred)
            return pred;
    return -1;
}
int getNominalPred(dynarec_arm_t* dyn, int ninst) {
    if((ninst<=0) || !dyn->insts[ninst].pred_sz)
        return -1;
    if(isPred(dyn, ninst, ninst-1)!=-1)
        return ninst-1;
    return dyn->insts[ninst].pred[0];
}

static int fpuCacheNeedsTransform(dynarec_arm_t* dyn, int ninst) {
    int i2 = dyn->insts[ninst].x86.jmp_insts;
    if(i2<0)
        return 1;
    if((dyn->insts[i2].x86.barrier&BARRIER_FLOAT))
        return ((dyn->insts[ninst].x86.barrier&BARRIER_FLOAT))?0:1; // if the barrier as already been apply, no transform needed
    int ret = 0;
    if(!i2) { // just purge
        if(dyn->insts[ninst].n.stack_next)  {
            return 1;
        }
        for(int i=0; i<24 && !ret; ++i)
            if(dyn->insts[ninst].n.neoncache[i].v) {       // there is something at ninst for i
                if(!(
                (dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_D)
                && dyn->insts[ninst].n.neoncache[i].n<dyn->insts[ninst].n.stack_pop))
                    ret = 1;
            }
        return ret;
    }
    // Check if ninst can be compatible to i2
    if(dyn->insts[ninst].n.stack_next != dyn->insts[i2].n.stack-dyn->insts[i2].n.stack_push) {
        return 1;
    }
    neoncache_t cache_i2 = dyn->insts[i2].n;
    neoncacheUnwind(&cache_i2);

    for(int i=0; i<24; ++i) {
        if(dyn->insts[ninst].n.neoncache[i].v) {       // there is something at ninst for i
            if(!cache_i2.neoncache[i].v) {    // but there is nothing at i2 for i
                ret = 1;
            } else if(dyn->insts[ninst].n.neoncache[i].v!=cache_i2.neoncache[i].v) {  // there is something different
                if(dyn->insts[ninst].n.neoncache[i].n!=cache_i2.neoncache[i].n) {   // not the same x86 reg
                    ret = 1;
                }
                else if(dyn->insts[ninst].n.neoncache[i].t == NEON_CACHE_XMMR && cache_i2.neoncache[i].t == NEON_CACHE_XMMW)
                    {/* nothing */ }
                else
                    ret = 1;
            }
        } else if(cache_i2.neoncache[i].v)
            ret = 1;
    }
    return ret;
}

void neoncacheUnwind(neoncache_t* cache)
{
    if(cache->swapped) {
        // unswap
        int a = -1; 
        int b = -1;
        for(int j=0; j<24 && ((a==-1) || (b==-1)); ++j)
            if((cache->neoncache[j].t == NEON_CACHE_ST_D || cache->neoncache[j].t == NEON_CACHE_ST_F)) {
                if(cache->neoncache[j].n == cache->combined1)
                    a = j;
                else if(cache->neoncache[j].n == cache->combined2)
                    b = j;
            }
        if(a!=-1 && b!=-1) {
            int tmp = cache->neoncache[a].n;
            cache->neoncache[a].n = cache->neoncache[b].n;
            cache->neoncache[b].n = tmp;
        }
        cache->swapped = 0;
        cache->combined1 = cache->combined2 = 0;
    }
    if(cache->news) {
        // reove the newly created neoncache
        for(int i=0; i<24; ++i)
            if(cache->news&(1<<i))
                cache->neoncache[i].v = 0;
        cache->news = 0;
    }
    if(cache->stack_push) {
        // unpush
        for(int j=0; j<24; ++j) {
            if((cache->neoncache[j].t == NEON_CACHE_ST_D || cache->neoncache[j].t == NEON_CACHE_ST_F)) {
                if(cache->neoncache[j].n<cache->stack_push)
                    cache->neoncache[j].v = 0;
                else
                    cache->neoncache[j].n-=cache->stack_push;
            }
        }
        cache->x87stack-=cache->stack_push;
        cache->stack-=cache->stack_push;
        cache->stack_push = 0;
    }
    cache->x87stack+=cache->stack_pop;
    cache->stack_next = cache->stack;
    cache->stack_pop = 0;
    cache->barrier = 0;
    // And now, rebuild the x87cache info with neoncache
    cache->mmxcount = 0;
    cache->fpu_scratch = 0;
    cache->fpu_extra_qscratch = 0;
    cache->fpu_reg = 0;
    for(int i=0; i<8; ++i) {
        cache->x87cache[i] = -1;
        cache->mmxcache[i] = -1;
        cache->ssecache[i].v = -1;
        cache->x87reg[i] = 0;
    }
    int x87reg = 0;
    for(int i=0; i<24; ++i) {
        if(cache->neoncache[i].v) {
            cache->fpuused[i] = 1;
            switch (cache->neoncache[i].t) {
                case NEON_CACHE_MM:
                    cache->mmxcache[cache->neoncache[i].n] = i+FPUFIRST;
                    ++cache->mmxcount;
                    ++cache->fpu_reg;
                    break;
                case NEON_CACHE_XMMR:
                case NEON_CACHE_XMMW:
                    cache->ssecache[cache->neoncache[i].n].reg = i+FPUFIRST;
                    cache->ssecache[cache->neoncache[i].n].write = (cache->neoncache[i].t==NEON_CACHE_XMMW)?1:0;
                    cache->neoncache[i] = cache->neoncache[i+1];
                    ++cache->fpu_reg;
                    ++i;    // next next
                    break;
                case NEON_CACHE_ST_F:
                case NEON_CACHE_ST_D:
                    cache->x87cache[x87reg] = cache->neoncache[i].n;
                    cache->x87reg[x87reg] = i+FPUFIRST;
                    ++x87reg;
                    ++cache->fpu_reg;
                    break;
                case NEON_CACHE_SCR:
                    cache->fpuused[i] = 0;
                    cache->neoncache[i].v = 0;
                    break;
            }
        } else {
            cache->fpuused[i] = 0;
        }
    }
}

#define F8      *(uint8_t*)(addr++)
#define F32     *(uint32_t*)(addr+=4, addr-4)
// Get if ED will have the correct parity. Not emiting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_arm_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity)
{

    uint32_t tested = (1<<parity)-1;
    if((nextop&0xC0)==0xC0)
        return 0;   // direct register, no parity...
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint32_t tmp = F32;
                if (sib_reg!=4) {
                    // if XXXXXX+reg<<N then check parity of XXXXX and N should be enough
                    return ((tmp&tested)==0 && (sib>>6)>=parity)?1:0;
                } else {
                    // just a constant...
                    return (tmp&tested)?0:1;
                }
            } else {
                if(sib_reg==4 && parity<3)
                    return 0;   // simple [reg]
                // don't try [reg1 + reg2<<N], unless reg1 is ESP
                return ((sib&0x7)==4 && (sib>>6)>=parity)?1:0;
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            return (tmp&tested)?0:1;
        } else {
            return 0;
        }
    } else {
        return 0; //Form [reg1 + reg2<<N + XXXXXX]
    }
}

// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop) 
{
    if((nextop&0xC0)==0xC0)
        return addr;
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            if((sib&0x7)==5) {
                addr+=4;
            }
        } else if((nextop&7)==5) {
            addr+=4;
        }
    } else {
        if((nextop&7)==4) {
            ++addr;
        }
        if(nextop&0x80) {
            addr+=4;
        } else {
            ++addr;
        }
    }
    return addr;
}
#undef F8
#undef F32

int isNativeCall(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn)
{
#define PK(a)       *(uint8_t*)(addr+a)
#define PK32(a)     *(uint32_t*)(addr+a)

    if(!addr)
        return 0;
    if(PK(0)==0xff && PK(1)==0x25) {  // absolute jump, maybe the GOT
        uintptr_t a1 = (PK32(2));   // need to add a check to see if the address is from the GOT !
        addr = (uintptr_t)getAlternate(*(void**)a1); 
    }
    if(addr<0x10000)    // too low, that is suspicious
        return 0;
    onebridge_t *b = (onebridge_t*)(addr);
    if(b->CC==0xCC && b->S=='S' && b->C=='C' && b->w!=(wrapper_t)0 && b->f!=(uintptr_t)PltResolver) {
        // found !
        if(retn) *retn = (b->C3==0xC2)?b->N:0;
        if(calladdress) *calladdress = addr+1;
        return 1;
    }
    return 0;
#undef PK32
#undef PK
}

const char* getCacheName(int t, int n)
{
    static char buff[20];
    switch(t) {
        case NEON_CACHE_ST_D: sprintf(buff, "ST%d", n); break;
        case NEON_CACHE_ST_F: sprintf(buff, "st%d", n); break;
        case NEON_CACHE_MM: sprintf(buff, "MM%d", n); break;
        case NEON_CACHE_XMMW: sprintf(buff, "XMM%d", n); break;
        case NEON_CACHE_XMMR: sprintf(buff, "xmm%d", n); break;
        case NEON_CACHE_SCR: sprintf(buff, "Scratch"); break;
        case NEON_CACHE_NONE: buff[0]='\0'; break;
    }
    return buff;
}

// is inst clean for a son branch?
int isInstClean(dynarec_arm_t* dyn, int ninst)
{
    // check flags cache
    if(dyn->insts[ninst].f_entry.dfnone || dyn->insts[ninst].f_entry.pending)
        return 0;
    if(dyn->insts[ninst].x86.state_flags)
        return 0;
    // check neoncache
    neoncache_t* n = &dyn->insts[ninst].n;
    if(n->news || n->stack || n->stack_next)
        return 0;
    for(int i=0; i<24; ++i)
        if(n->neoncache[i].v)
            return 0;
    return 1;
}

static int flagsCacheNeedsTransform(dynarec_arm_t* dyn, int ninst) {
    int jmp = dyn->insts[ninst].x86.jmp_insts;
    if(jmp<0)
        return 0;
    if(dyn->insts[ninst].f_exit.dfnone)  // flags are fully known, nothing we can do more
        return 0;
/*    if((dyn->f.pending!=SF_SET)
    && (dyn->f.pending!=SF_SET_PENDING)) {
        if(dyn->f.pending!=SF_PENDING) {*/
    switch (dyn->insts[jmp].f_entry.pending) {
        case SF_UNKNOWN: return 0;
        case SF_SET: 
            if(dyn->insts[ninst].f_exit.pending!=SF_SET && dyn->insts[ninst].f_exit.pending!=SF_SET_PENDING) 
                return 1; 
            else 
                return 0;
        case SF_SET_PENDING:
            if(dyn->insts[ninst].f_exit.pending!=SF_SET 
            && dyn->insts[ninst].f_exit.pending!=SF_SET_PENDING
            && dyn->insts[ninst].f_exit.pending!=SF_PENDING) 
                return 1; 
            else 
                return 0;
        case SF_PENDING:
            if(dyn->insts[ninst].f_exit.pending!=SF_SET 
            && dyn->insts[ninst].f_exit.pending!=SF_SET_PENDING
            && dyn->insts[ninst].f_exit.pending!=SF_PENDING)
                return 1;
            else
                return (dyn->insts[jmp].f_entry.dfnone  == dyn->insts[ninst].f_exit.dfnone)?0:1;
    }
    if(dyn->insts[jmp].f_entry.dfnone && !dyn->insts[ninst].f_exit.dfnone)
        return 1;
    return 0;
}
int CacheNeedsTransform(dynarec_arm_t* dyn, int ninst) {
    int ret = 0;
    if (fpuCacheNeedsTransform(dyn, ninst)) ret|=1;
    if (flagsCacheNeedsTransform(dyn, ninst)) ret|=2;
    return ret;
}

void inst_name_pass3(dynarec_arm_t* dyn, int ninst, const char* name)
{
    if(box86_dynarec_dump) {
        printf_x86_instruction(my_context->dec, &dyn->insts[ninst].x86, name);
        dynarec_log(LOG_NONE, "%s%p: %d emited opcodes, inst=%d, barrier=%d state=%d/%d(%d), %s=%X/%X, use=%X, need=%X/%X sm=%d/%d",
            (box86_dynarec_dump>1)?"\e[32m":"",
            (void*)(dyn->arm_start+dyn->insts[ninst].address),
            dyn->insts[ninst].size/4,
            ninst,
            dyn->insts[ninst].x86.barrier,
            dyn->insts[ninst].x86.state_flags,
            dyn->f.pending,
            dyn->f.dfnone,
            dyn->insts[ninst].x86.may_set?"may":"set",
            dyn->insts[ninst].x86.set_flags,
            dyn->insts[ninst].x86.gen_flags,
            dyn->insts[ninst].x86.use_flags,
            dyn->insts[ninst].x86.need_before,
            dyn->insts[ninst].x86.need_after,
            dyn->smread, dyn->smwrite);
        if(dyn->insts[ninst].pred_sz) {
            dynarec_log(LOG_NONE, ", pred=");
            for(int ii=0; ii<dyn->insts[ninst].pred_sz; ++ii)
                dynarec_log(LOG_NONE, "%s%d", ii?"/":"", dyn->insts[ninst].pred[ii]);
        }
        if(dyn->insts[ninst].x86.jmp && dyn->insts[ninst].x86.jmp_insts>=0)
            dynarec_log(LOG_NONE, ", jmp=%d", dyn->insts[ninst].x86.jmp_insts);
        if(dyn->insts[ninst].x86.jmp && dyn->insts[ninst].x86.jmp_insts==-1)
            dynarec_log(LOG_NONE, ", jmp=out");
        for(int ii=0; ii<24; ++ii) {
            switch(dyn->insts[ninst].n.neoncache[ii].t) {
                case NEON_CACHE_ST_D: dynarec_log(LOG_NONE, " D%d:%s", ii+8, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
                case NEON_CACHE_ST_F: dynarec_log(LOG_NONE, " S%d:%s", (ii+8)*2, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
                case NEON_CACHE_MM: dynarec_log(LOG_NONE, " D%d:%s", ii+8, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
                case NEON_CACHE_XMMW: dynarec_log(LOG_NONE, " Q%d:%s", (ii+8)/2, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); ++ii; break;
                case NEON_CACHE_XMMR: dynarec_log(LOG_NONE, " Q%d:%s", (ii+8)/2, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); ++ii; break;
                case NEON_CACHE_SCR: dynarec_log(LOG_NONE, " D%d:%s", ii+8, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
                case NEON_CACHE_NONE:
                default:    break;
            }
        }
        if(dyn->n.stack || dyn->insts[ninst].n.stack_next || dyn->insts[ninst].n.x87stack)
            dynarec_log(LOG_NONE, " X87:%d/%d(+%d/-%d)%d", dyn->n.stack, dyn->insts[ninst].n.stack_next, dyn->insts[ninst].n.stack_push, dyn->insts[ninst].n.stack_pop, dyn->insts[ninst].n.x87stack);
        if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2)
            dynarec_log(LOG_NONE, " %s:%d/%d", dyn->insts[ninst].n.swapped?"SWP":"CMB", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);
        dynarec_log(LOG_NONE, "%s\n", (box86_dynarec_dump>1)?"\e[m":"");
    }
}

void print_opcode(dynarec_arm_t* dyn, int ninst, uint32_t opcode)
{
    dynarec_log(LOG_NONE, "\t%08x\t%s\n", opcode, arm_print(opcode));
}

void newinst_pass3(dynarec_arm_t* dyn, int ninst, uintptr_t ip)
{
    if(ninst && isInstClean(dyn, ninst)) {
        dyn->sons_x86[dyn->sons_size] = ip;
        dyn->sons_arm[dyn->sons_size] = dyn->block;
        if(box86_dynarec_dump) dynarec_log(LOG_NONE, "----> potential Son here %p/%p\n", (void*)ip, dyn->block);
        ++dyn->sons_size;
    }
}