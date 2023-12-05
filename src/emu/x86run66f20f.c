#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "box86context.h"

#include "modrm.h"
#include "x86compstrings.h"

#ifdef TEST_INTERPRETER
uintptr_t Test66F20F(x86test_t *test, uintptr_t addr)
#else
uintptr_t Run66F20F(x86emu_t *emu, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u;
    int8_t tmp8s;
    uint16_t tmp16u;
    int16_t tmp16s;
    uint32_t tmp32u;
    int32_t tmp32s;
    sse_regs_t *opex, eax1, *opx2;
    mmx87_regs_t *opem;
    float tmpf;
    double tmpd;

    #ifdef TEST_INTERPRETER
    x86emu_t* emu = test->emu;
    #endif
    opcode = F8;
    switch(opcode) {

         case 0x38:  // SSE 4.x
            opcode = F8;
            switch(opcode) {
            
                case 0xF1:  // CRC32 Gd, Ew
                    nextop = F8;
                    GET_EW;
                    for(int j=0; j<2; ++j) {
                        GD.dword[0] ^=  EW->byte[j];
                        for (int i = 0; i < 8; i++) {
                            if (GD.dword[0] & 1)
                                GD.dword[0] = (GD.dword[0] >> 1) ^ 0x82f63b78;
                            else
                                GD.dword[0] = (GD.dword[0] >> 1);
                        }
                    }
                    break;

                default: 
                    return 0;
            }
        break;

    default:
        return 0;
    }

    return addr;
}
