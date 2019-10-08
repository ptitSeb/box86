# box86

Linux Userspace x86 Emulator with a twist

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=LU8Y2T62ZWFHU)

Box86 will let you run x86 Linux programs (games) on non-x86 Linux, like ARM (needs to be 32bit little-endian).

Also, Box86 uses the native version for some "system" libraries, like libc, libm, or SDL and OpenGL, leading to more performance and easier integration with the host system.

Most x86 Games need OpenGL, so on ARM platforms, a solution like [gl4es](https://github.com/ptitSeb/gl4es) is probably needed.

Note that the current version of Box86 doesn't feature any form of JIT/Dynarec: some things can be slow.

The current version is still experimental, so some stuff still wont run. But many do work, like for example, WorldOfGoo (at least on the Pandora), Airline Tycoon Deluxe or new FTL. Many of the GameMaker linux games also run fine.

If you are serious about developing Box86, you should install ccache and activate it's support in the cmake project (use ccmake for example)
To have TRACE enabled (i.e. dumping to stdout all individual x86 instructions executed, with dump of registers), you'll also need [Zydis library](https://github.com/zyantific/zydis) accessible on your system.

Some x86 internal opcodes use parts of "Realmode X86 Emulator Library", see [x86primop.c](src/x86primop.c) for copyright details

Here are 5 videos, one of "Airline Tycoon Deluxe" running on a gigahertz OpenPandora, and the next 2 of "Bit.Trip.Runner" and "Neverwinter Night" running on an ODroid XU4, and the last 2 on a Pi4: Shovel Knight (video from @ITotalJustice) and Freedom Planet (video from @djazz)

[![Play on Youtube](https://img.youtube.com/vi/bLt0hMoFDLk/3.jpg)](https://www.youtube.com/watch?v=bLt0hMoFDLk) [![Play on Youtube](https://img.youtube.com/vi/8hr71S029Hg/1.jpg)](https://www.youtube.com/watch?v=8hr71S029Hg) [![Play on Youtube](https://img.youtube.com/vi/B4YN37z3-ws/1.jpg)](https://www.youtube.com/watch?v=B4YN37z3-ws) [![Play on Youtube](https://img.youtube.com/vi/xk8Q30mxqPg/1.jpg)](https://www.youtube.com/watch?v=xk8Q30mxqPg) [![Play on Youtube](https://img.youtube.com/vi/_QMRMVvYrqU/1.jpg)](https://www.youtube.com/watch?v=_QMRMVvYrqU)

----

Compiling
----
How to compile can be found [here](COMPILE.md)

----

Usage
----

There are a few environment variables to control Box86 behaviour.

See [here](USAGE.md) for all variables and what they do.

----

Version history
----

The change log is [here](CHANGELOG.md)

----

A note about Unity game emulation
----

Running Unity games is not possible for now. Mono itself uses signals that are not well emulated enough. So the solution is to use a native version of the libmono used by Unity. It can be found here: https://github.com/Unity-Technologies/mono and it needs to be built from source. When built copy `libmonosgen-2.0.so` to `libmonounity.so` and put it somewhere it can be dlopen'd (so in `usr/lib` or friend or somewhere in your `LD_LIBRARY_PATH`).
Note that libmonounity is not completely wrapped yet, and the mechanism to call x86 library from libmonounity is not done yet

----

Final words
----

(If you use Box86 in your project, please don't forget to mention Box86)
