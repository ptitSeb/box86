#define _GNU_SOURCE
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
#include "x87emu_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "x86run.h"

#define F8      *(uint8_t*)(ip++)
#define F8S     *(int8_t*)(ip++)
#define F16     *(uint16_t*)(ip+=2, ip-2)
#define F16S    *(int16_t*)(ip+=2, ip-2)
#define F32     *(uint32_t*)(ip+=4, ip-4)
#define F32S    *(int32_t*)(ip+=4, ip-4)
#define PK(a)   *(uint8_t*)(ip+a)

#include "modrm.h"

void Run66D9(x86emu_t *emu)
{
    uintptr_t ip = R_EIP;
    uint8_t nextop = F8;
    reg32_t *oped;
    switch (nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
        case 0xD0:
        case 0xE0:
        case 0xE5:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xFC:
        case 0xE1:
        case 0xE4:
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            ip = R_EIP-2;
            UnimpOpcode(emu);
            break;
        default:
        switch((nextop>>3)&7) {
            case 4:     /* FLDENV m */
                // warning, incomplete
                GET_EW;
                fpu_loadenv(emu, (char*)ED, 1);
                break;
            case 6:     /* FNSTENV m */
                // warning, incomplete
                GET_EW;
                fpu_savenv(emu, (char*)ED, 1);
                break;
            default:
                ip = R_EIP-2;
                UnimpOpcode(emu);
        }
    }
    R_EIP = ip;
}

void Run66DD(x86emu_t *emu)
{
    uintptr_t ip = R_EIP;
    uint8_t nextop = F8;
    reg32_t *oped;
    switch(nextop) {
    
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
        case 0xD0:
        case 0xE0:
        case 0xE5:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xFC:
        case 0xE1:
        case 0xE4:
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            ip = R_EIP-2;
            UnimpOpcode(emu);
        break;

    default:
        switch((nextop>>3)&7) {
            case 4: /* FRSTOR m94byte */
                GET_EW;
                fpu_loadenv(emu, (char*)ED, 1);
                // get the STx
                {
                    char* p =(char*)ED;
                    p += 14;
                    for (int i=0; i<8; ++i) {
                        LD2D(p, &ST(i).d);
                        p+=10;
                    }
                }
                break;
            case 6: /* FNSAVE m94byte */
                GET_EW;
                // ENV first...
                fpu_savenv(emu, (char*)ED, 1);
                // save the STx
                {
                    char* p =(char*)ED;
                    p += 14;
                    for (int i=0; i<8; ++i) {
                        D2LD(&ST(i).d, p);
                        p+=10;
                    }
                }
                reset_fpu(emu);
                break;
            default:
                ip = R_EIP-2; // unfetch
                UnimpOpcode(emu);
        }
    }
    R_EIP = ip;
}
