#ifndef __ARM_LOCK_HELPER__H__
#define __ARM_LOCK_HELPER__H__
#include <stdint.h>

// LDREXB of ADDR
extern uint8_t arm_lock_read_b(void* addr);
// STREXB of ADDR, return 0 if ok, 1 if not
extern int arm_lock_write_b(void* addr, uint8_t val);

// LDREXH of ADDR
extern uint16_t arm_lock_read_h(void* addr);
// STREXH of ADDR, return 0 if ok, 1 if not
extern int arm_lock_write_h(void* addr, uint16_t val);

// LDREX of ADDR
extern uint32_t arm_lock_read_d(void* addr);
// STREX of ADDR, return 0 if ok, 1 if not
extern int arm_lock_write_d(void* addr, uint32_t val);

// LDREXD of ADDR
extern void arm_lock_read_dd(uint32_t* a, uint32_t* b, void* addr);
// STREX of ADDR, return 0 if ok, 1 if not
extern int arm_lock_write_dd(uint32_t a, uint32_t b, void* addr);

// Atomically exchange value at [p] with val, return old p
extern uintptr_t arm_lock_xchg(void* p, uintptr_t val);

// Atomically store value to [p] only if [p] is NULL. Return old [p] value
extern void* arm_lock_storeifnull(void* p, void* val);

// Atomically store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern void* arm_lock_storeifref(void* p, void* val, void* ref);

// Atomically store value to [p] only if [p] is ref. Return old [p] value
extern void* arm_lock_storeifref2(void* p, void* val, void* ref);

// decrement atomicaly the byte at [p] (but only if p not 0). Return old [p] value
extern int arm_lock_decifnot0b(void*p);

// increment atomicaly the byte at [p]
extern void arm_lock_incb(void*p);

// increment atomicaly the byte at [p] only if it was 0. Return the old value of [p]
extern int arm_lock_incif0b(void*p);

// atomic store (with memory barrier)
extern void arm_lock_stored(void*p, uint32_t d);

// atomic store (with memory barrier)
extern void arm_lock_storeb(void*p, uint8_t b);

// atomic store (with memory barrier) only if it was 0. Return the old value of [p]
extern int arm_lock_storeif0b(void*p, uint8_t b);

// atomic load (with memory barrier)
extern int arm_lock_readb(void*p);

#endif  //__ARM_LOCK_HELPER__H__