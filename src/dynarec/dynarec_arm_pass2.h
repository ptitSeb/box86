#define INIT        dyn->arm_size = 0
#define FINI                                                                                            \
        if(ninst) {                                                                                     \
                dyn->insts[ninst].address = (dyn->insts[ninst-1].address+dyn->insts[ninst-1].size);     \
                dyn->insts_size += 1+((dyn->insts[ninst].x86.size>dyn->insts[ninst].size)?dyn->insts[ninst].x86.size:dyn->insts[ninst].size)/15; \
        }

#define MESSAGE(A, ...)  
#define EMIT(A)     do{dyn->insts[ninst].size+=4; dyn->arm_size+=4;}while(0)
#define NEW_INST                                                                                        \
        if(ninst) {                                                                                     \
                dyn->insts[ninst].address = (dyn->insts[ninst-1].address+dyn->insts[ninst-1].size);     \
                dyn->insts_size += 1+((dyn->insts[ninst-1].x86.size>dyn->insts[ninst-1].size)?dyn->insts[ninst-1].x86.size:dyn->insts[ninst-1].size)/15; \
        }
#define INST_EPILOG dyn->insts[ninst].epilog = dyn->arm_size; 
#define INST_NAME(name) 
