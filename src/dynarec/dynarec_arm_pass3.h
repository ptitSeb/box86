#define INIT    
#define FINI
#define EMIT(A)     \
    if(box86_dynarec_dump) {dynarec_log(LOG_NONE, "\t%08x\t%s\n", (A), arm_print(A));} \
    *(uint32_t*)(dyn->block) = A;       \
    dyn->block += 4; dyn->arm_size += 4;\
    dyn->insts[ninst].size2 += 4

#define MESSAGE(A, ...)  if(box86_dynarec_dump) dynarec_log(LOG_NONE, __VA_ARGS__)
#define NEW_INST        
#define INST_EPILOG     
#define INST_NAME(name) \
    if(box86_dynarec_dump) {\
        printf_x86_instruction(my_context->dec, &dyn->insts[ninst].x86, name); \
        dynarec_log(LOG_NONE, "%s%p: %d emited opcodes, inst=%d, barrier=%d state=%d/%d, set=%X, use=%X, need=%X", \
            (box86_dynarec_dump>1)?"\e[32m":"", \
            (void*)(dyn->arm_start+dyn->insts[ninst].address),  \
            dyn->insts[ninst].size/4,           \
            ninst,                              \
            dyn->insts[ninst].x86.barrier,      \
            dyn->insts[ninst].x86.state_flags,  \
            dyn->state_flags,                   \
            dyn->insts[ninst].x86.set_flags,    \
            dyn->insts[ninst].x86.use_flags,    \
            dyn->insts[ninst].x86.need_flags);  \
        for(int ii=0; ii<24; ++ii) {            \
            switch(dyn->insts[ninst].n.neoncache[ii].t) {    \
                case NEON_CACHE_ST_D: dynarec_log(LOG_NONE, " D%d:ST%d", ii+8, dyn->insts[ninst].n.neoncache[ii].n); break;              \
                case NEON_CACHE_ST_F: dynarec_log(LOG_NONE, " D%d:st%d", ii+8, dyn->insts[ninst].n.neoncache[ii].n); break;              \
                case NEON_CACHE_MM: dynarec_log(LOG_NONE, " D%d:MM%d", ii+8, dyn->insts[ninst].n.neoncache[ii].n); break;                \
                case NEON_CACHE_XMMW: dynarec_log(LOG_NONE, " Q%d:XMM%d", (ii+8)/2, dyn->insts[ninst].n.neoncache[ii].n); ++ii; break;   \
                case NEON_CACHE_XMMR: dynarec_log(LOG_NONE, " Q%d:xmm%d", (ii+8)/2, dyn->insts[ninst].n.neoncache[ii].n); ++ii; break;   \
                case NEON_CACHE_SCR: dynarec_log(LOG_NONE, " D%d:Scratch", ii+8); break;                                    \
                case NEON_CACHE_NONE:           \
                default:    break;              \
            }                                   \
        }                                       \
        if(dyn->n.stack || dyn->insts[ninst].n.stack_next)                                   \
            printf_log(LOG_NONE, " X87:%d/%d", dyn->n.stack, dyn->insts[ninst].n.stack_next);\
        if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2)                   \
            printf_log(LOG_NONE, " CMB:%d/%d", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);   \
        dynarec_log(LOG_NONE, "%s\n", (box86_dynarec_dump>1)?"\e[m":"");                \
    }

#define NEW_BARRIER_INST                            \
    if(ninst) {                                     \
    dyn->sons_x86[dyn->sons_size] = (uintptr_t)ip;  \
    dyn->sons_arm[dyn->sons_size] = dyn->block;     \
    MESSAGE(LOG_DUMP, "----> potential Son here\n");\
    ++dyn->sons_size;                               \
    }

