v0.1.0
=======
* Dynarec!!! Only for ARM (note that dynarec doesn't support JITed code for now)
* Added real support for getcontext/set/context/makecontext/swapcontext
* Preliminary signal handling
* Fixes to SDL(1/2) Image and SDL1 Mixer, and to SDL1.2 RWops usage
* Fixed numerous issues in opcodes (both interpretor and dynarec). FTL works fine now, among many others
* Added wrapped gtk support (still ongoing, many libs involved)
* Make loading of libs more configurable
* If a wrapped native libs is not found, try to use emulated one
* Add en env. var. to force use of emulated lib for certain libs
* Add an env. var. to define wich libGL to use
* Added Install / Uninstall target (using systemd binfmt)
* Added more hardware target (RPis, GameShell...)
* Wrapped more libs (including FreeType, smpeg, ncurses, sndfile...)

v0.0.4
=======
* Improved the Init of dependant Libs. More things work now.
* Added a lot of wrapped functions.
* Added a few wrapped libs (like libz or some other x11 related libs)
* For trace enabled build, Trace can be enabled only after a certain amount of opcodes (still, a debugger would be better)
* Some fixes in a few opcodes, and implemented x87 rounding (SuperMeatBoy behaves better now)
* FTL 1.6.9 still have corrupted music, but older 1.5.13 seems fine (different set of libs)

v0.0.2
=======
* A full commercial games runs fine on the Pandora: Airline Tycoon Deluxe
* Implemented all planed subpart of Box86 except JIT
* CPU Emulation is at 75%, roughly, including x87 and SSE/SSE2. MMX is barely implemented, but barely used anyway
* x87 emulation is simplified, no real x87 State handling (but should not be mandatory, as native libm is used)
* No Signal handling yet
* ELF Loader is crude and probably full of bug. Also, the Init of libs is defered after all symbols are resolved for now.
* Wrapped lib include libc, libm, rt, pthread, libdl, dllinux, libasound, GL, GLU, SDL1/mixer/image, SDL2/mixer/image/smpeg, OpenAL/ALUT, libz, libpng16, vorbisfile, x11/xrandr/xxf86vm.
* Most wrapped libs are still partially implemented (SDL1 & GL should be complete)
* Implemented specific mecanism for SDL(1/2) RWops, to be able to used them both in Native and x86 world
* WorldOfGoo works, but painfully slow on th Pandora (too much double math, and lack of JIT)
* FTL works, but sound is broken (issue with thread? asound? or CPU core?)
* Limbo launch but crash before main menu

