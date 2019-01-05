#ifndef __SDL1RWOPS_H__
#define __SDL1RWOPS_H__

typedef struct sdl1rwops_s sdl1rwops_t;     // box86 wrappers collection
typedef struct SDL1_RWops_s SDL1_RWops_t;   // the actual SDL1 SDL_RWops
typedef struct x86emu_s x86emu_t;

typedef struct SDLRWSave_s {
    int   anyEmu;
    void* seek;
    void* read;
    void* write;
    void* close;
    void* s1;
    void* s2;
} SDLRWSave_t;

sdl1rwops_t* NewSDL1RWops();
void FreeSDL1RWops(sdl1rwops_t **rw);

// each function will be added to dictionary, and each native functions will be wrapped so they run in emulated world
void AddNativeRW(x86emu_t* emu, SDL1_RWops_t* ops);
void RWNativeStart(x86emu_t* emu, SDL1_RWops_t* ops, SDLRWSave_t* save);   // put Native RW function, wrapping emulated (callback style) ones if needed
void RWNativeEnd(x86emu_t* emu, SDL1_RWops_t* ops, SDLRWSave_t* save);     // put back emulated function back in place

#endif