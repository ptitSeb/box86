#ifndef __DYNAREC_ARM_PRIVATE_H_
#define __DYNAREC_ARM_PRIVATE_H_

#include "dynarec_private.h"

typedef struct x86emu_s x86emu_t;

#define NEON_CACHE_NONE 0
#define NEON_CACHE_ST   1
#define NEON_CACHE_MM   2
#define NEON_CACHE_XMMW 3
#define NEON_CACHE_XMMR 4
#define NEON_CACHE_SCR  5
typedef union neon_cache_s {
    int8_t           v;
    struct {
        unsigned int t:4;   // reg type
        unsigned int n:4;   // reg number
    };
} neon_cache_t;
typedef struct neoncache_s {
    neon_cache_t neoncache[24];
    int8_t       ststack;
} neoncache_t;

typedef struct instruction_arm_s {
    instruction_x86_t   x86;
    uintptr_t           address;    // (start) address of the arm emited instruction
    uintptr_t           epilog;     // epilog of current instruction (can be start of next, of barrier stuff)
    int                 size;       // size of the arm emited instruction
    int                 size2;      // size of the arm emited instrucion after pass2
    uintptr_t           mark, mark2, mark3;
    uintptr_t           markf;
    uintptr_t           markseg;
    uintptr_t           marklock;
    int                 pass2choice;// value for choices that are fixed on pass2 for pass3
    uintptr_t           natcall;
    int                 retn;
    neoncache_t         n;
} instruction_arm_t;

typedef union sse_cache_s {
    int     v;
    struct {
        uint8_t reg;
        uint8_t write;
    };
} sse_cache_t;

typedef struct dynarec_arm_s {
    instruction_arm_t   *insts;
    int32_t             size;
    int32_t             cap;
    uintptr_t           start;      // start of the block
    uint32_t            isize;      // size in byte of x86 instructions included
    void*               block;      // memory pointer where next instruction is emited
    uintptr_t           arm_start;  // start of the arm code
    int                 arm_size;   // size of emitted arm code
    int                 state_flags;// actual state for on-demand flags
    int                 x87cache[8];// cache status for the 8 x87 register behind the fpu stack
    int                 x87reg[8];  // reg used for x87cache entry
    int                 mmxcache[8];// cache status for the 8 MMX registers
    sse_cache_t         ssecache[8];// cache status for the 8 SSE(2) registers
    int                 fpuused[24];// all 8..31 double reg from fpu, used by x87, sse and mmx
    neoncache_t         n;          // cache for the 8..31 double reg from fpu, plus x87 stack delta
    int                 x87stack;   // cache stack counter
    int                 mmxcount;   // number of mmx register used (not both mmx and x87 at the same time)
    int                 fpu_scratch;// scratch counter
    int                 fpu_extra_qscratch; // some opcode need an extra quad scratch register
    int                 fpu_reg;    // x87/sse/mmx reg counter
    int                 dfnone;     // if defered flags is already set to df_none
    uintptr_t*          next;       // variable array of "next" jump address
    int                 next_sz;
    int                 next_cap;
    uintptr_t*          sons_x86;   // the x86 address of potential dynablock sons
    void**              sons_arm;   // the arm address of potential dynablock sons
    int                 sons_size;  // number of potential dynablock sons
} dynarec_arm_t;

void add_next(dynarec_arm_t *dyn, uintptr_t addr);
uintptr_t get_closest_next(dynarec_arm_t *dyn, uintptr_t addr);
int is_nops(dynarec_arm_t *dyn, uintptr_t addr, int n);
int is_instructions(dynarec_arm_t *dyn, uintptr_t addr, int n);

#endif //__DYNAREC_ARM_PRIVATE_H_