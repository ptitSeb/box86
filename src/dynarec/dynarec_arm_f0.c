#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "emu/x86run_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "emu/x86run_private.h"
#include "x86trace.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"

#include "dynarec_arm_helper.h"


uintptr_t dynarecF0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = PK(0); // don't increment addr
    int locked = 0;
    switch(nextop) {
        // generic case
        #define GO(A)   \
        case A+0x00:    \
        case A+0x01:    \
        case A+0x02:    \
        case A+0x03:    \
        case A+0x04:    \
        case A+0x05:
        GO(0x00);
        GO(0x08);
        GO(0x10);
        GO(0x18);
        GO(0x20);
        GO(0x28);
        GO(0x30);
        #undef GO

        case 0x81:
        case 0x83:
            MESSAGE(LOG_DUMP, "LOCK\n");
            locked = 1;
            LOCK;
            addr = dynarec00(dyn, addr, ip, ninst, ok, need_epilog);
            break;

        case 0x86:  // for this two, the lock is already done by the opcode, so just ignoring it
        case 0x87:
            addr = dynarec00(dyn, addr, ip, ninst, ok, need_epilog);
            break;

        case 0x0F:
            nextop = PK(1);
            switch(nextop) {
                case 0xB0:
                case 0xB1:
                case 0xB3:
                case 0xBA:
                case 0xBB:
                case 0xC0:
                case 0xC1:
                case 0xC7:
                    MESSAGE(LOG_DUMP, "LOCK\n");
                    locked = 1;
                    LOCK;
                    addr = dynarec0F(dyn, addr+1, ip, ninst, ok, need_epilog);
                    break;

                default:
                    addr = dynarec0F(dyn, addr+1, ip, ninst, ok, need_epilog);    // no lock, regular instruction...
            }
            break;

        case 0xFF:
            nextop = PK(1);
            switch((nextop>>3)&7)
            {
                case 0:
                case 1:
                    MESSAGE(LOG_DUMP, "LOCK\n");
                    locked = 1;
                    LOCK;
                    addr = dynarec00(dyn, addr, ip, ninst, ok, need_epilog);
                    break;
                default:
                addr = dynarec00(dyn, addr, ip, ninst, ok, need_epilog);    // no lock, regular instruction...
            }
            break;
       
        default:
            addr = dynarec00(dyn, addr, ip, ninst, ok, need_epilog);    // no lock, regular instruction...
    }
    if(locked) {UNLOCK;}

    return addr;
}

