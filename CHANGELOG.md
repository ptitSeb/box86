Current version
======

v0.1.8
======
* Fixes on some float to int x87 convertions opcodes
* Reworked all callback mecanisms
* Added libturbojpeg wrapping (and a hack for zoom to force using native one)
* Added BOX86_SAFEMMAP env. var.
* Reworked dynarec memory manager
* Added a few opcodes
* Improve elfloader to not force PltResolver all the time (fixing SuperHexagon and maybe other)
* Reworked Exet and Cancel Thread mecanism
* Added wrapped libldap_r and liblber library (used by wine)
* Reworked Dynarec block handling, and remove the "AddMark" mecanism that wasn't efficient enough
* Added TokiTori 2+ detection and runtime patch on PI
* Fixes on elfloader with TLS object

v0.1.6
======
* Changes in Dynarec to make flags optimizations before CALL and RET opcode less aggressive
* Added Vulkan wrapper
* Improved wrapping of SDL_mixer/ SDL2_mixer lib
* Improved wrapping of some gtk structures
* Added a quick and dirty wrapping of Gtk3 (based on current gtk2)
* Improved the signal handling, should be more stable now (the signal handler from syscall still need some works)
* Added RK3399 profile, and some hints to build the 32bits box86 on 64bits OS
* Fixed some wrapped printf formating not handled correctly
* Fixed some buserror with the new LOCK mecanism handling
* A few more Dynarec opcodes added
* If winedbg is tried to be launched, exit without launching it (it doesn't work anyway)

v0.1.4
======
* Change in Dynarec memory handling, to simplify it (and hopefully optimized it)
* Even more opcodes added.
* And some more Dynarec opcodes.
* Fixed some issue were PltResolver was injected but should not
* Fixed many Dynarec and non dynarec opcodes
* Improved Signal handling
* Added a few more wrapped lib (like curl)
* Gallium9 is wrapped (thx @icecream95)
* Fixed and simplied many wrapped functions (especialy the one were a callback is involved)
* Fixed Dynarec "freeing" a Dynablock somthime corrupting the heap (happens in case of JIT mainly)
* Optimized the way LOCK prefix work when using Dynarec on ARM
* Printer of ARM opcode (for dumping Dynarec blocks) improvement (thx @rajdakin)

v0.1.2
======
* The Dynarec now handle JIT code
* Added support for Unity games (not perfect yet)
* Added support for Wine (not perfect yet). You need an x86 build of Wine to use it
* Added support for Steam (not perfect yet). Note that steam have limited functionnalities on 32bits (only mini-mode is available)
* More wrapped libs
* Added support for "PltResolver", so order of library is less important and many symbol are resolved at runtime
* Added an option to build box86 as a lib (to wrapped dynamic library)
* Better Signal handling (not perfect yet)
* More opcodes added, more opcode fixes.
* More Dynarec opcodes.
* Added support for FS:, and creating custom selector (needed by Wine)
* There is now 1 x86emu_t structure per thread (simplifying/optimising many callback handling)
* Box86 now have a logo!
* Added options handling (only version and help for now). Now it's much usefull exept for version printing.


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

