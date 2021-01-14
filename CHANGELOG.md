Current version
======

v0.2.0
======
* Improvements on x87 Flags handling.
* A few more opcode has been added.
* A few potential BusError are now fixed.
* Added the BOX86_NOVULKAN en. var. . This is used to disable the wrapping of vulkan libraries.
* Improvments on libc O_XXXX flags handling.
* Box86 now uses a custom allocator for Dynarec and Hash tables.
* Improved the wrapping of pulse audio.
* Optimisation to a few Dynarec opcode (like SHRD/SHLD).
* Improved the tracking of memory protection.
* A few more wrapping of libraries (like libgssapi_krb5).
* More function wrapping on libturbojpeg
* Preliminary support for POWER9 (ppcle) build
* Many contributions to remove typos and rephrase the README, COMPILE and USAGE documents

v0.1.8
======
* Fixes on some float to int x86 convertions opcodes.
* Reworked all callback mecanisms.
* Added libturbojpeg wrapping (and a hack for zoom to force using native one).
* Added the BOX86_SAFEMMAP env. var.
* Reworked Dynarec's memory manager.
* Added a few opcodes.
* Improve elfloader to not force PltResolver all the time (fixing SuperHexagon and maybe other)
* Reworked Exet and Cancel Thread mecanism.
* Added wrapped libldap_r and liblber library (used by wine)
* Reworked Dynarec block handling, and remove the "AddMark" mecanism that wasn't efficient enough
* Added TokiTori 2+ detection and runtime patch on the Raspberry Pi platform
* Fixes on elfloader with TLS object.

v0.1.6
======
* Changes in Dynarec to make flags optimizations before CALL and RET opcode less aggressive.
* Added a Vulkan wrapper.
* Improved wrapping of SDL_mixer/ SDL2_mixer libraries.
* Improved wrapping of some GTK structures.
* Added a quick and dirty wrapping of GTK3 (based on current GTK2).
* Improved the signal handling, should be more stable now (the signal handler from syscall still need some works).
* Added the RK3399 profile, and some hints to build 32bits box86 on 64bits OS.
* Fixed some wrapped printf formating not handled correctly.
* Fixed some buserror with the new LOCK mecanism handling.
* A few more Dynarec opcodes added.
* If winedbg is tried to be launched, exit without launching it (it doesn't work anyways).

v0.1.4
======
* Change in Dynarec's memory handling, to simplify it (and hopefully optimized it).
* Even more opcodes added.
* And some more Dynarec opcodes.
* Fixed some issue were PltResolver was injected but should not be.
* Fixed many Dynarec and non dynarec opcodes.
* Improved Signal handling.
* Added a few more wrapped libraries (like curl).
* Gallium9 is now wrapped (thx @icecream95).
* Fixed and simplied many wrapped functions (especialy the one where a callback is involved).
* Fixed Dynarec "freeing" a Dynablock sometimes causing a corruption of the heap (generally happens in case of JIT code).
* Optimized the way LOCK prefix work when using Dynarec on ARM.
* improvements to the ARM opcode Printer (for dumping Dynarec blocks) improvement (thx @rajdakin).

v0.1.2
======
* The Dynarec now handle JIT code
* Added support for Unity games (not perfect yet).
* Added support for Wine (not perfect yet). You need an x86 build of Wine to use it.
* Added support for Steam (not perfect yet). Note that Steam have limited functionalities on 32bits (only mini-mode is available).
* More wrapped libs
* Added support for the "PltResolver". This makes the order of libraries less important and many symbols are now resolved at runtime.
* Added an option to build Box86 as a library (to wrap dynamic library).
* Better Signal handling (not perfect yet).
* More opcodes added, more opcode fixes.
* More Dynarec opcodes.
* Added support for FS:, and creating custom selector (needed by Wine).
* There is now 1 x86emu_t structure per thread (simplifying/optimising many callback handling).
* Box86 now has a logo!
* Added options handling (only version and help for now). Now it's much usefull exept for version printing.


v0.1.0
=======
* Dynarec!!! Only for ARM (note that Dynarec doesn't support JITed code for now).
* Added real support for getcontext/set/context/makecontext/swapcontext.
* Preliminary signal handling.
* Fixes to SDL(1/2) Image and SDL1 Mixer, and to SDL1.2 RWops usage.
* Fixed numerous issues in opcodes (both interpretor and dynarec). FTL works fine now, among many others.
* Added wrapped GTK support (still ongoing, many libraries involved).
* Make loading of libraries more configurable.
* If a wrapped native library is not found, try to use an emulated one.
* Added an env. var. to force the use of emulated libraries for certain libraries.
* Added an env. var. to precise which libGL to use.
* Added Install / Uninstall target (using systemd binfmt).
* Added more hardware targets (RPis, GameShell...).
* Wrapped more libraries (including FreeType, smpeg, ncurses, sndfile...).

v0.0.4
=======
* Improved the initialisation of dependent libraries. More things work now.
* Added a lot of wrapped functions.
* Added a few wrapped libraries (like libz or some other x11 related library).
* For trace enabled build, Trace can be enabled only after a certain amount of opcodes (still, a debugger would be better).
* Some fixes in a few opcodes, and implemented x87 rounding (SuperMeatBoy behaves better now).
* FTL 1.6.9 still has corrupted music, but older 1.5.13 seems fine (different set of libraries).

v0.0.2
=======
* A full commercial games runs fine on the Pandora platform: Airline Tycoon Deluxe.
* Implemented all planned subparts of Box86 except JIT support.
* CPU Emulation is at roughly 75%. This includes x87 and SSE/SSE2. MMX is barely implemented (but barely used anyway).
* x87 emulation is simplified, no real x87 State handling (but should not be mandatory, as the native libm library is used)
* No Signal handling yet
* ELF Loader is crude and probably full of bugs. Also, the initialisation of libraries are defered after all symbols are resolved for now.
* Wrapped libraries include libc, libm, rt, pthread, libdl, dllinux, libasound, GL, GLU, SDL1/mixer/image, SDL2/mixer/image/smpeg, OpenAL/ALUT, libz, libpng16, vorbisfile, x11/xrandr/xxf86vm.
* Most wrapped libraries are still partially implemented ( the SDL1 & GL  libraries should be complete).
* Implemented specific mecanism for SDL(1/2) RWops, to be able to used them both in Native and x86 world.
* WorldOfGoo works, but is painfully slow on the Pandora platform (there is too much double math, and lack of JIT).
* FTL works, but sound is broken (issue with thread? asound? or CPU core?)
* Limbo launches but crashes before main menu.

