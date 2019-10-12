#define INIT    
#define FINI     \
    dyn->insts[ninst].x86.addr = addr; \
    if(ninst) dyn->insts[ninst-1].x86.size = dyn->insts[ninst].x86.addr - dyn->insts[ninst-1].x86.addr;
#define MESSAGE(A, ...)  
#define EMIT(A)     
#define UFLAGS(A)   dyn->insts[ninst+(A?1:0)].x86.flags = X86_FLAGS_CHANGE
#define USEFLAG(A)  dyn->insts[ninst].x86.flags = X86_FLAGS_USE
#define JUMP(A)     dyn->insts[ninst].x86.jmp = A
#define NEW_INST \
    dyn->insts[ninst].x86.addr = ip; \
    if(ninst) dyn->insts[ninst-1].x86.size = dyn->insts[ninst].x86.addr - dyn->insts[ninst-1].x86.addr;
#define INST_NAME(name) 
#define DEFAULT         

