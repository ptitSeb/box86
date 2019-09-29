#ifndef STEP
#error Meh?!
#endif

#include "arm_emitter.h"

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr+=2, addr-2)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define PK(a)   *(uint8_t*)(addr+a)

void arm_epilog();

void NAME_STEP(dynarec_arm_t* dyn, uintptr_t addr)
{
    uint8_t nextop;
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    INIT;
    while(ok) {
        ip = addr;
        nextop = F8;
        NEW_INST;
        switch(nextop) {
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x54:
            case 0x55:
            case 0x56:
            case 0x57:  /* PUSH reg */
                INST_NAME("PUSH reg");
                // can probably use strdb reg, [xESP, #-4]!
                SUB_IMM8(xESP, xESP, 4);
                STR_IMM(xEAX+(nextop&0x08), xESP, 0);
                break;
            
            //case 0x89:  /* MOV Ed,Gd */


            default:
                ok = 0;
                DEFAULT;
        }
        ++ninst;
    }
    MOV32(xEIP, ip);
    void* epilog = arm_epilog;
    MOV32(2, (uintptr_t)epilog);
    BX(2);
    FINI;
}