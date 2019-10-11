
#define INIT    uintptr_t sav_addr=addr
#define FINI    dyn->isize = addr-sav_addr
#define MESSAGE(A, ...)  
#define EMIT(A)     
#define UFLAGS(A)          {}
#define NEW_INST        ++dyn->size
#define INST_NAME(name) 
#define DEFAULT         \
        --dyn->size;    \
        dynarec_log(LOG_INFO, "%p: Dynarec stopped because of Opcode %02X %02X %02X %02X %02X %02X %02X\n", \
        ip, PKip(0),                   \
        PKip(1), PKip(2), PKip(3),     \
        PKip(4), PKip(5), PKip(6))
