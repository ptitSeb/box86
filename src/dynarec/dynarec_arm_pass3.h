#define INIT    
#define FINI
#define EMIT(A)     \
    if(box86_dynarec_dump) {dynarec_log(LOG_NONE, "\t%08x\t%s\n", (A), arm_print(A));} \
    *(uint32_t*)(dyn->block) = A;   \
    dyn->block += 4; dyn->arm_size += 4

#define MESSAGE(A, ...)  if(box86_dynarec_dump) dynarec_log(LOG_NONE, __VA_ARGS__)
#define NEW_INST        
#define INST_EPILOG     
#define INST_NAME(name) \
    if(box86_dynarec_dump) {\
        printf_x86_instruction(dyn->emu->dec, &dyn->insts[ninst].x86, name); \
        dynarec_log(LOG_NONE, "%s%d emited opcodes, state=%d/%d, set=%X, use=%X, need=%X%s\n", \
            (box86_dynarec_dump>1)?"\e[32m":"", \
            dyn->insts[ninst].size/4, \
            dyn->insts[ninst].x86.state_flags, \
            dyn->state_flags, \
            dyn->insts[ninst].x86.set_flags, \
            dyn->insts[ninst].x86.use_flags, \
            dyn->insts[ninst].x86.need_flags, \
            (box86_dynarec_dump>1)?"\e[m":""); \
    }

