#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "librarian.h"
#include "librarian_private.h"
#include "x86emu.h"

#include "bridge.h"

KHASH_MAP_IMPL_STR(maplib_t, onelib_t);

lib_t *NewLibrarian()
{
    lib_t *maplib = (lib_t*)calloc(1, sizeof(lib_t));
    maplib->maplib = kh_init(maplib_t);
    maplib->bridge = NewBridge();

    return maplib;
}
void FreeLibrarian(lib_t **maplib)
{
    kh_destroy(maplib_t, (*maplib)->maplib);

    if((*maplib)->bridge)
        FreeBridge(&(*maplib)->bridge);

    free(*maplib);
    *maplib = NULL;

}

void my__stack_chk_fail(x86emu_t* emu)
{
    StopEmu(emu, "Stack is corrupted, abborting");
}

void my__libc_start_main()
{
    // nothing here
}
uint32_t LibSyscall(x86emu_t *emu); // implemented in x86syscall.c

uintptr_t CreateSymbol(lib_t *maplib, const char* name)
{
    // look for symbols that can be created
    uintptr_t addr = 0;
    if(strcmp(name, "__stack_chk_fail")==0) {
        addr = AddBridge(maplib->bridge, vFE, &my__stack_chk_fail);
    } else if(strcmp(name, "__libc_start_main")==0) {
        addr = AddBridge(maplib->bridge, vFv, &my__libc_start_main);
    } else if(strcmp(name, "syscall")==0) {
        addr = AddBridge(maplib->bridge, uFE, &LibSyscall);
    } else if(strcmp(name, "puts")==0) {
        addr = AddBridge(maplib->bridge, iFp, &puts);
    }
    if(addr)
        AddSymbol(maplib, name, addr);
    return addr;
}

void AddSymbol(lib_t *maplib, const char* name, uintptr_t addr)
{
    int ret;
    khint_t k = kh_put(maplib_t, maplib->maplib, name, &ret);
    kh_value(maplib->maplib, k).offs = addr;
}
uintptr_t FindSymbol(lib_t *maplib, const char* name)
{
    khint_t k = kh_get(maplib_t, maplib->maplib, name);
    if(k==kh_end(maplib->maplib))
        return CreateSymbol(maplib, name);
    return kh_val(maplib->maplib, k).offs;
}
