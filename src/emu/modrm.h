// ModRM utilities macros
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
#define geteb(A) \
    if((nextop&0xC0)==0xC0) { \
        A = (reg32_t*)&emu->regs[(nextop&3)].byte[((nextop&0x4)>>2)]; \
    } else getecommon(A, reg32_t)
#define getebo(A, O)          \
    if((nextop&0xC0)==0xC0) { \
        A = (reg32_t*)&emu->regs[(nextop&3)].byte[((nextop&0x4)>>2)]; \
    } else getecommono(A, reg32_t, O)
#define geted(A) \
    if((nextop&0xC0)==0xC0) { \
        A = &emu->regs[(nextop&7)]; \
    } else getecommon(A, reg32_t)
#define getedo(A, O) \
    if((nextop&0xC0)==0xC0) { \
        A = &emu->regs[(nextop&7)]; \
    } else getecommono(A, reg32_t, O)
#define getem(A) \
    if((nextop&0xC0)==0xC0) { \
        A = &emu->mmx[(nextop&7)]; \
    } else getecommon(A, mmx_regs_t)
#define getex(A) \
    if((nextop&0xC0)==0xC0) { \
        A = &emu->xmm[(nextop&7)]; \
    } else getecommon(A, sse_regs_t)
// Macros for ModR/M gets
#define GET_EB      geteb(oped)
#define GET_ED      geted(oped)
#define GET_ED_OFFS(o) getedo(oped, o)
#define GET_EB_OFFS(o) getebo(oped, o)
#define GET_EM      getem(opem)
#define GET_EX      getex(opex)
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
#define EW          ED
#define GW          GD