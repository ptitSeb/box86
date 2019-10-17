#define INIT    
#define FINI
#define EMIT(A)     \
    if(box86_dynarec_log>=LOG_DUMP) {dynarec_log(LOG_NONE, "\t%08x\t%s\n", (A), arm_print(A));} \
    *(uint32_t*)(dyn->block) = A;   \
    dyn->block += 4; dyn->arm_size += 4

#define MESSAGE(A, ...)  dynarec_log(A, __VA_ARGS__);
#define NEW_INST        
#define INST_NAME(name) if(box86_dynarec_log>=LOG_DUMP) printf_x86_instruction(dyn->emu->dec, &dyn->insts[ninst].x86, name)

