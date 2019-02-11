Current version
=======


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

