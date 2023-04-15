#define INIT    
#define FINI
#define EMIT(A)     \
    if(box86_dynarec_dump) print_opcode(dyn, ninst, (uint32_t)(A)); \
    *(uint32_t*)(dyn->block) = A;                                   \
    dyn->block += 4; dyn->arm_size += 4;                            \
    dyn->insts[ninst].size2 += 4

#define MESSAGE(A, ...)  if(box86_dynarec_dump) dynarec_log(LOG_NONE, __VA_ARGS__)
// warning, there is some logic, handing of sons, in newinst_pass3
#define NEW_INST  newinst_pass3(dyn, ninst, (uintptr_t)ip)
#define INST_EPILOG     
#define INST_NAME(name) inst_name_pass3(dyn, ninst, name)
