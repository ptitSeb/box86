#define INIT    
#define FINI     \
    dyn->insts[ninst].x86.addr = addr; \
    if(ninst) dyn->insts[ninst-1].x86.size = dyn->insts[ninst].x86.addr - dyn->insts[ninst-1].x86.addr;
#define MESSAGE(A, ...)  
#define EMIT(A)     
#define READFLAGS(A)    dyn->insts[ninst].x86.use_flags = A
#define SETFLAGS(A,B)   {dyn->insts[ninst].x86.set_flags = A; dyn->insts[ninst].x86.state_flags = B;}
#define JUMP(A)         dyn->insts[ninst].x86.jmp = A
#define BARRIER(A)      dyn->insts[ninst].x86.barrier = A
#define BARRIER_NEXT(A) if(ninst<dyn->size) dyn->insts[ninst+1].x86.barrier = A

#define NEW_INST \
    dyn->insts[ninst].x86.addr = ip; \
    if(ninst) dyn->insts[ninst-1].x86.size = dyn->insts[ninst].x86.addr - dyn->insts[ninst-1].x86.addr;
#define INST_EPILOG 
#define INST_NAME(name)  
