#ifndef __X86EMU_PRIVATE_H_
#define __X86EMU_PRIVATE_H_

#include "regs.h"

typedef struct box86context_s box86context_t;
typedef struct i386_ucontext_s i386_ucontext_t;

#define ERR_UNIMPL  1
#define ERR_DIVBY0  2
#define ERR_ILLEGAL 4

#ifdef DYNAREC
#define CSTACK      32
#define CSTACKMASK  31
#endif

typedef struct forkpty_s {
    void*    amaster;
    void*   name;
    void*   termp;
    void*   winp;
    void*   f;  // forkpty function
} forkpty_t;

typedef struct x86emu_s x86emu_t;

typedef struct x86test_s {
    x86emu_t*   emu;
    uintptr_t   memaddr;
    int         memsize;
    int         test;
    int         clean;
    uint8_t     mem[16];
} x86test_t;

typedef struct emu_flags_s {
    uint32_t    need_jmpbuf:1;    // need a new jmpbuff for signal handling
    uint32_t    quitonlongjmp:2;  // quit if longjmp is called
    uint32_t    quitonexit:2;     // quit if exit/_exit is called
    uint32_t    longjmp:1;        // if quit because of longjmp
    uint32_t    jmpbuf_ready:1;   // the jmpbuf in the emu is ok and don't need refresh
} emu_flags_t;

#ifdef ANDROID
#include <setjmp.h>
#define JUMPBUFF sigjmp_buf
#else
#define JUMPBUFF struct __jmp_buf_tag
#endif

typedef struct x86emu_s {
    // cpu
	reg32_t     regs[8];
	x86flags_t  eflags;
    reg32_t     ip;
    uintptr_t   xSPSave;
    // fpu / mmx
	x87control_t cw;
	x87flags_t  sw;
	mmx87_regs_t x87[8];
	mmx87_regs_t mmx[8];
	uint32_t    top;        // top is part of sw, but it's faster to have it separatly
    int         fpu_stack;
	uint32_t    fpu_tags; // tags for the x87 regs, stacked, only on a 16bits anyway
    fpu_ld_t    fpu_ld[8]; // for long double emulation / 80bits fld fst
    fpu_ll_t    fpu_ll[8]; // for 64bits fild / fist sequence
    // sse
    sse_regs_t  xmm[8];
    mmxcontrol_t mxcsr;
    uintptr_t   old_ip;
    // defered flags
    defered_flags_t df;
    uint32_t    op1;
    uint32_t    op2;
    uint32_t    res;
    uint32_t    *x86emu_parity_tab; // helper
    #ifdef HAVE_TRACE
    uintptr_t   prev2_ip, prev_ip;
    #endif
    // segments
    uint16_t    segs[6];
    uint16_t    dummy_seg6, dummy_seg7; // to stay aligned
    uintptr_t   segs_offs[6];   // computed offset associate with segment
    uint32_t    segs_serial[6];  // are seg offset clean (not 0) or does they need to be re-computed (0)? For GS, serial need to be the same as context->sel_serial
    // emu control
    int         quit;
    int         error;
    int         fork;   // quit because need to fork
    int         exit;
    forkpty_t*  forkpty_info;
    emu_flags_t flags;
    x86test_t   test;       // used for dynarec testing
    // parent context
    box86context_t *context;
    // cpu helpers
    reg32_t     zero;
    reg32_t     *sbiidx[8];
    // scratch stack, used for alignement of double and 64bits ints on arm. 200 elements should be enough
    uint32_t    scratch[200];
    // local stack, do be deleted when emu is freed
    void*       stack2free; // this is the stack to free (can be NULL)
    void*       init_stack; // initial stack (owned or not)
    uint32_t    size_stack; // stack size (owned or not)
    JUMPBUFF*   jmpbuf;
    uintptr_t   old_savedsp;

    i386_ucontext_t *uc_link; // to handle setcontext

    int         type;       // EMUTYPE_xxx define

} x86emu_t;

#define EMUTYPE_NONE    0
#define EMUTYPE_MAIN    1
#define EMUTYPE_SIGNAL  2

//#define INTR_RAISE_DIV0(emu) {emu->error |= ERR_DIVBY0; emu->quit=1;}
#define INTR_RAISE_DIV0(emu) {emu->error |= ERR_DIVBY0;} // should rise a SIGFPE and not quit

void applyFlushTo0(x86emu_t* emu);

#endif //__X86EMU_PRIVATE_H_