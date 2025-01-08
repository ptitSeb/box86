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
#include "emu/x87emu_private.h"
#include "x86trace.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"


uintptr_t dynarecDD(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    int fixedaddress;
    int parity;
    uint8_t ed;
    int v1, v2;
    int i1, i2, i3;
    int j32;

    MAYUSE(v2);
    MAYUSE(v1);

    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            INST_NAME("FFREE STx");
            #if 1
            if((nextop&7)==0 && PK(0)==0xD9 && PK(1)==0xF7) {
                MESSAGE(LOG_DUMP, "Hack for FFREE ST0 / FINCSTP\n");
                X87_POP_OR_FAIL(dyn, ninst, x1);
                addr+=2;
                SKIPTEST(x1, x14);
            } else
                x87_free(dyn, ninst, x1, x2, x3, nextop&7);
            #else
            INST_NAME("FFREE STx");
            x87_purgecache(dyn, ninst, 0, x1, x2, x3);
            MOVW(x1, nextop&7);
            CALL(fpu_do_free, -1, 0);
            #endif
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FST ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                VMOV_32(v2, v1);
            } else {
                VMOV_64(v2, v1);
            }
            break;
        case 0xD8:
            INST_NAME("FSTP ST0, ST0");
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            INST_NAME("FSTP ST0, STx");
            // copy the cache value for st0 to stx
            x87_swapreg(dyn, ninst, x1, x2, 0, nextop&7);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
            INST_NAME("FUCOM ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
            FCOM(x1, x2);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FUCOMP ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
            FCOM(x1, x2);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
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
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FLD double");
                    X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
		            parity = getedparity(dyn, ninst, addr, nextop, 3);
		            if (parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                        VLDR_64(v1, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        LDR_IMM9(x2, ed, fixedaddress);
                        LDR_IMM9(x3, ed, fixedaddress + 4);
                        VMOVtoV_D(v1, x2, x3);
        		    }
                    break;
                case 1:
                    INST_NAME("FISTTP i64, ST0");
                    parity = getedparity(dyn, ninst, addr, nextop, 3);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(ninst
                      && dyn->insts[ninst-1].x86.addr
                      && *(uint8_t*)dyn->insts[ninst-1].x86.addr==0xDF
                      && (((*(uint8_t*)(dyn->insts[ninst-1].x86.addr+1))>>3)&7)==5)
                    {
                        if(parity) {
                            STRD_IMM8(x2, ed, 0);    // x2/x3 is 64bits
                        } else {
                            STR_IMM9(x2, ed, 0);
                            STR_IMM9(x3, ed, 4);
                        }
                    } else {
                        v1 = x87_get_st(dyn, ninst, x2, x3, 0, NEON_CACHE_ST_D);
                        //addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        fpu_get_scratch_double(dyn); // to alocate v0
                        v2 = fpu_get_scratch_double(dyn);
                        //  get TOP
                        LDR_IMM9(x14, xEmu, offsetof(x86emu_t, top));
                        int a = 0 - dyn->n.x87stack;
                        if(a<0) {
                            SUB_IMM8(x14, x14, -a);
                            AND_IMM8(x14, x14, 7);    // (emu->top + i)&7
                        } else if(a>0) {
                            ADD_IMM8(x14, x14, a);
                            AND_IMM8(x14, x14, 7);    // (emu->top + i)&7
                        }
                        ADD_REG_LSL_IMM5(x14, xEmu, x14, 4);  // each fpu_ll is 2 int64: ref than ll
                        MOVW(x2, offsetof(x86emu_t, fpu_ll));   //can be optimized?
                        ADD_REG_LSL_IMM5(x14, x14, x2, offsetof(fpu_ll_t, sref));
                        VLDR_64(v2, x14, 0);
                        VCEQ_32(v2, v2, v1);    // compare
                        VMOVfrV_D(x2, x3, v2);
                        ANDS_REG_LSL_IMM5(x2, x2, x3, 0);   // if NE then values are the same!
                        B_MARK(cEQ);    // do the i64 conversion
                        // memcpy(ed, &STll(0).ll, sizeof(int64_t));
                        LDRD_IMM8(x2, x14, offsetof(fpu_ll_t, sq));  // load ll
                        B_MARK3(c__);
                        MARK;
                        MOV32(x2, arm_fist64_3);
                        VMOV_64(0, v1);    // prepare call to log2
                        CALL_1DR_U64(x2, x2, x3, x14, (1<<x1));
                        MARK3;
                        STR_IMM9(x2, ed, 0);
                        STR_IMM9(x3, ed, 4);
                    }
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 2:
                    INST_NAME("FST double");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    parity = getedparity(dyn, ninst, addr, nextop, 3);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                        VSTR_64(v1, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        VMOVfrV_D(x2, x3, v1);
                        STR_IMM9(x2, ed, fixedaddress);
                        STR_IMM9(x3, ed, fixedaddress+4);
                    }
                    break;
                case 3:
                    INST_NAME("FSTP double");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    parity = getedparity(dyn, ninst, addr, nextop, 3);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                        VSTR_64(v1, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        VMOVfrV_D(x2, x3, v1);
                        STR_IMM9(x2, ed, fixedaddress);
                        STR_IMM9(x3, ed, fixedaddress+4);
                    }
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 4: 
                    INST_NAME("FRSTOR m108byte");
                    fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    CALL(arm_frstor, -1, 0);
                    break;
                case 6: 
                    INST_NAME("FSAVE m108byte");
                    fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    CALL(arm_fsave, -1, 0);
                    CALL(reset_fpu, -1, 0);
                    break;
                case 7:
                    INST_NAME("FNSTSW m2byte");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x14, &fixedaddress, 0, 0, 0, NULL);
                    LDR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
                    LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
                    if(dyn->n.x87stack) {
                        // update top
                        if(dyn->n.x87stack>0) {
                            SUB_IMM8(x2, x2, dyn->n.x87stack);
                        } else {
                            ADD_IMM8(x2, x2, -dyn->n.x87stack);
                        }
                        AND_IMM8(x2, x2, 7);
                    }
                    BFI(x1, x2, 11, 3); // inject top
                    STRH_IMM8(x1, ed, 0);   // store whole sw flags
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}

