#include <string.h>

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr+=2, addr-2)
#define F16S    *(int16_t*)(addr+=2, addr-2)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define F32S64  (uint64_t)(int64_t)F32S
#define F64     *(uint64_t*)(addr+=8, addr-8)
#define F64S    *(int64_t*)(addr+=8, addr-8)
#define PK(a)   *(uint8_t*)(addr+a)
#ifdef DYNAREC
#define STEP if(step) return 0;
#define STEP2 if(step) {R_EIP = addr; return 0;}
#define STEP3 if(*step) (*step)++;
#else
#define STEP
#define STEP2
#define STEP3
#endif

// ModRM utilities macros
#define MODREG ((nextop&0xC0)==0xC0)
#define getecommon(A, T) \
    if(!(nextop&0xC0)) { \
        if((nextop&7)==4) { \
            uint8_t sib = F8; \
            uintptr_t base = ((sib&0x7)==5)?(F32):(emu->regs[(sib&0x7)].dword[0]); \
            base += (emu->sbiidx[(sib>>3)&7]->sdword[0] << (sib>>6)); \
            A = (T*)base; \
        } else if((nextop&7)==5) { \
            A = (T*)(F32); \
        } else { \
            A = (T*)(emu->regs[nextop&7].dword[0]); \
        } \
    } else { \
        uintptr_t base; \
        if((nextop&7)==4) { \
            uint8_t sib = F8;   \
            base = emu->regs[(sib&0x7)].dword[0]; \
            base += (emu->sbiidx[(sib>>3)&7]->sdword[0] << (sib>>6));   \
        } else { \
            base = emu->regs[(nextop&0x7)].dword[0];    \
        } \
        base+=(nextop&0x80)?(F32S):(F8S); \
        A = (T*)base; \
    }
#define getecommono(A, T, O) \
    if(!(nextop&0xC0)) { \
        if((nextop&7)==4) { \
            uint8_t sib = F8; \
            uintptr_t base = ((sib&0x7)==5)?(F32):(emu->regs[(sib&0x7)].dword[0]); \
            base += (emu->sbiidx[(sib>>3)&7]->sdword[0] << (sib>>6)); \
            A = (T*)(base+O); \
        } else if((nextop&7)==5) { \
            A = (T*)(F32+O); \
        } else { \
            A = (T*)(emu->regs[nextop&7].dword[0]+O); \
        } \
    } else { \
        uintptr_t base; \
        if((nextop&7)==4) { \
            uint8_t sib = F8;   \
            base = emu->regs[(sib&0x7)].dword[0]; \
            base += (emu->sbiidx[(sib>>3)&7]->sdword[0] << (sib>>6));   \
        } else { \
            base = emu->regs[(nextop&0x7)].dword[0];    \
        } \
        base+=(nextop&0x80)?(F32S):(F8S); \
        A = (T*)(base+O); \
    }

#define  getecommon16(A, T)                     \
    {                                           \
        uint32_t m = nextop&0xC7;               \
        uint32_t base = 0;                      \
        switch(m&7) {                           \
            case 0: base = R_BX+R_SI; break;    \
            case 1: base = R_BX+R_DI; break;    \
            case 2: base = R_BP+R_SI; break;    \
            case 3: base = R_BP+R_DI; break;    \
            case 4: base =      R_SI; break;    \
            case 5: base =      R_DI; break;    \
            case 6: base =      R_BP; break;    \
            case 7: base =      R_BX; break;    \
        }                                       \
        switch((m>>6)&3) {                      \
            case 0: if(m==6) base = F16; break; \
            case 1: base += F8S; break;         \
            case 2: base += F16S; break;        \
        }                                       \
        A =  (T*)base;                          \
    }

#define  getecommon16o(A, T, O)                 \
    {                                           \
        uint32_t m = nextop&0xC7;               \
        uint32_t base = 0;                      \
        switch(m&7) {                           \
            case 0: base = R_BX+R_SI; break;    \
            case 1: base = R_BX+R_DI; break;    \
            case 2: base = R_BP+R_SI; break;    \
            case 3: base = R_BP+R_DI; break;    \
            case 4: base =      R_SI; break;    \
            case 5: base =      R_DI; break;    \
            case 6: base =      R_BP; break;    \
            case 7: base =      R_BX; break;    \
        }                                       \
        switch((m>>6)&3) {                      \
            case 0: if(m&7==6) base = F16S; break; \
            case 1: base += F8S; break;         \
            case 2: base += F16S; break;        \
        }                                       \
        A =  (T*)(base+O);                      \
    }


#define geteb(A) \
    if(MODREG) { \
        A = (reg32_t*)&emu->regs[(nextop&3)].byte[((nextop&0x4)>>2)]; \
    } else getecommon(A, reg32_t)
#define testeb(A, O) \
    if(MODREG) { \
        A = (reg32_t*)&emu->regs[(nextop&3)].byte[((nextop&0x4)>>2)]; \
    } else { \
        reg32_t* ret; \
        getecommono(ret, reg32_t, O);\
        test->memsize = 1; \
        test->memaddr = (uintptr_t)ret;\
        test->mem[0] = ret->byte[0];\
        A = (reg32_t*)test->mem;\
    }
#define getebo(A, O)          \
    if(MODREG) { \
        A = (reg32_t*)&emu->regs[(nextop&3)].byte[((nextop&0x4)>>2)]; \
    } else getecommono(A, reg32_t, O)
#define geted(A) \
    if(MODREG) { \
        A = &emu->regs[(nextop&7)]; \
    } else getecommon(A, reg32_t)
#define getedo(A, O) \
    if(MODREG) { \
        A = &emu->regs[(nextop&7)]; \
    } else getecommono(A, reg32_t, O)
#define tested(A, SZ, O) \
    if(MODREG) { \
        A = &emu->regs[(nextop&7)]; \
    } else {\
        reg32_t* ret; \
        getecommono(ret, reg32_t, O);\
        test->memsize = SZ; \
        test->memaddr = (uintptr_t)ret;\
        memcpy(test->mem, ret->dword, SZ);\
        A = (reg32_t*)test->mem;\
    }
#define getem(A) \
    if(MODREG) { \
        A = &emu->mmx[(nextop&7)]; \
    } else getecommon(A, mmx87_regs_t)
#define testem(A) \
    if(MODREG) { \
        A = &emu->mmx[(nextop&7)]; \
    } else {\
        mmx87_regs_t* ret; \
        getecommon(ret, mmx87_regs_t) \
        test->memsize = 8; \
        test->memaddr = (uintptr_t)ret;\
        memcpy(test->mem, ret->ub, 8);\
        A = (mmx87_regs_t*)test->mem;\
    }
#define getex(A) \
    if(MODREG) { \
        A = &emu->xmm[(nextop&7)]; \
    } else getecommon(A, sse_regs_t)
#define testex(A) \
    if(MODREG) { \
        A = &emu->xmm[(nextop&7)]; \
    } else {\
        sse_regs_t* ret; \
        getecommon(ret, sse_regs_t);\
        test->memsize = 16; \
        test->memaddr = (uintptr_t)ret;\
        memcpy(test->mem, ret->ub, 16);\
        A = (sse_regs_t*)test->mem;\
    }
#define getew16(A)  \
    if(MODREG) { \
        A = &emu->regs[(nextop&7)]; \
    } else getecommon16(A, reg32_t)
#define getew16o(A, O)              \
    if(MODREG) {       \
        A = &emu->regs[(nextop&7)]; \
    } else getecommon16o(A, reg32_t, O)
#define testtew16(A, O)                 \
    if(MODREG) {           \
        A = &emu->regs[(nextop&7)];     \
    } else {                            \
        reg32_t* ret;                   \
        getecommon16o(A, reg32_t, O);   \
        test->memsize = 2;              \
        test->memaddr = (uintptr_t)ret; \
        *(uint16_t*)test->mem = ret->word[0];\
        A = (reg32_t*)test->mem;        \
    }

// Macros for ModR/M gets
#ifdef TEST_INTERPRETER
#define GET_EB      testeb(oped, 0)
#define GET_ED      tested(oped, 4, 0)
#define GET_ED8     tested(oped, 8, 0)
#define GET_EDT     tested(oped, 10, 0)
#define GET_ED_OFFS(o) tested(oped, 4, o)
#define GET_EB_OFFS(o) testeb(oped, o)
#define GET_EM      testem(opem)
#define GET_EX      testex(opex)
#define GET_EW16    testew16(oped, 0)
#define GET_EW16_OFFS(o)    testtew16(oped, o)
#else
#define GET_EB      geteb(oped)
#define GET_ED      geted(oped)
#define GET_ED8     geted(oped)
#define GET_EDT     geted(oped)
#define GET_ED_OFFS(o) getedo(oped, o)
#define GET_EB_OFFS(o) getebo(oped, o)
#define GET_EM      getem(opem)
#define GET_EX      getex(opex)
#define GET_EW16    getew16(oped)
#define GET_EW16_OFFS(o)    getew16o(oped, o)
#endif
#define GET_ED_     geted(oped)
#define GET_EW16_   getew16(oped)
#define EB          oped
#define ED          oped
#define EM          opem
#define EX          opex
#define GB          emu->regs[(nextop>>3)&3].byte[(nextop>>5)&0x1]
#define GD          emu->regs[((nextop&0x38)>>3)]
#define GM          emu->mmx[((nextop&0x38)>>3)]
#define GX          emu->xmm[((nextop&0x38)>>3)]

// Alias
#define GET_EW      GET_ED
#define GET_EW_OFFS GET_ED_OFFS
#define EW          ED
#define GW          GD