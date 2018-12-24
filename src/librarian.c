#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

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

int32_t my__libc_start_main(x86emu_t* emu, int *(main) (int, char * *, char * *), 
    int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), 
    void (*rtld_fini) (void), void (* stack_end)); // implemented in x86run_private.c
uint32_t LibSyscall(x86emu_t *emu); // implemented in x86syscall.c
int my_pthread_create(x86emu_t *emu, void* t, void* attr, void* start_routine, void* arg); //implemented in thread.c

uintptr_t CreateSymbol(lib_t *maplib, const char* name)
{
    // look for symbols that can be created
    uintptr_t addr = 0;
    // libc
    if(strcmp(name, "__stack_chk_fail")==0) {
        addr = AddBridge(maplib->bridge, vFE, my__stack_chk_fail);
    } else if(strcmp(name, "__libc_start_main")==0) {
        addr = AddBridge(maplib->bridge, iFEpippppp, my__libc_start_main);
    } else if(strcmp(name, "syscall")==0) {
        addr = AddBridge(maplib->bridge, uFE, LibSyscall);
    } else if(strcmp(name, "puts")==0) {
        addr = AddBridge(maplib->bridge, iFp, puts);
    } else if(strcmp(name, "printf")==0) {
        addr = AddBridge(maplib->bridge, iFopV, vfprintf);
    } else if(strcmp(name, "__printf_chk")==0) {
        addr = AddBridge(maplib->bridge, iFvopV, vfprintf);
    } else if(strcmp(name, "calloc")==0) {
        addr = AddBridge(maplib->bridge, pFuu, calloc);
    } else if(strcmp(name, "free")==0) {
        addr = AddBridge(maplib->bridge, vFp, free);
    } else if(strcmp(name, "putchar")==0) {
        addr = AddBridge(maplib->bridge, iFi, putchar);
    } else if(strcmp(name, "strtol")==0) {
        addr = AddBridge(maplib->bridge, iFppi, strtol);
    } else if(strcmp(name, "strerror")==0) {
        addr = AddBridge(maplib->bridge, pFv, strerror);
    } //pthread
    else if(strcmp(name, "pthread_self")==0) {
        addr = AddBridge(maplib->bridge, uFv, pthread_self);
    } else if(strcmp(name, "pthread_create")==0) {
        addr = AddBridge(maplib->bridge, iFEpppp, my_pthread_create);
    } else if(strcmp(name, "pthread_equal")==0) {
        addr = AddBridge(maplib->bridge, iFuu, pthread_equal);
    } else if(strcmp(name, "pthread_join")==0) {
        addr = AddBridge(maplib->bridge, iFup, pthread_join);
    }
    
    if(addr)
        AddSymbol(maplib, name, addr, 12);
    return addr;
}

void AddSymbol(lib_t *maplib, const char* name, uintptr_t addr, uint32_t sz)
{
    int ret;
    khint_t k = kh_put(maplib_t, maplib->maplib, name, &ret);
    kh_value(maplib->maplib, k).offs = addr;
    kh_value(maplib->maplib, k).sz = sz;
}
uintptr_t FindSymbol(lib_t *maplib, const char* name)
{
    khint_t k = kh_get(maplib_t, maplib->maplib, name);
    if(k==kh_end(maplib->maplib))
        return CreateSymbol(maplib, name);
    return kh_val(maplib->maplib, k).offs;
}

int GetSymbolStartEnd(lib_t* maplib, const char* name, uintptr_t* start, uintptr_t* end)
{
    khint_t k = kh_get(maplib_t, maplib->maplib, name);
    if(k==kh_end(maplib->maplib))
        return 0;
    *start = kh_val(maplib->maplib, k).offs;
    *end = *start + kh_val(maplib->maplib, k).sz;
    return 1;
}