#ifndef STEP
#error Meh?!
#endif

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr+=2, addr-2)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define PK(a)   *(uint8_t*)(addr+a)

void NAME_STEP(dynarec_arm_t* dyn, uintptr_t addr)
{
    uint8_t nextop;
    int ok = 1;
    int ninst = 0;
    while(ok) {
        uintptr_t ip = addr;
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
                break;
            
            //case 0x89:  /* MOV Ed,Gd */


            default:
                ok = 0;
                DEFAULT;
        }
        ++ninst;
    }
}