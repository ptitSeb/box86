#define INIT    
#define FINI     \
    dyn->insts[ninst].x86.addr = addr; \
    if(ninst) dyn->insts[ninst-1].x86.size = dyn->insts[ninst].x86.addr - dyn->insts[ninst-1].x86.addr;
#define MESSAGE(A, ...)  
#define EMIT(A)     
#define READFLAGS(A)    dyn->insts[ninst].x86.use_flags = A
#define SETFLAGS(A,B)   {dyn->insts[ninst].x86.set_flags = A; dyn->insts[ninst].x86.state_flags = B;}

#define NEW_INST \
    dyn->insts[ninst].x86.addr = ip;\
    dyn->n.combined1 = dyn->n.combined2 = 0;\
    if(ninst) {dyn->insts[ninst-1].x86.size = dyn->insts[ninst].x86.addr - dyn->insts[ninst-1].x86.addr;}

#define INST_EPILOG dyn->insts[ninst].n = dyn->n

#define INST_NAME(name)  
