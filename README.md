# box86

Linux Userspace x86 Emulator with a twist

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=LU8Y2T62ZWFHU)

Box86 will let run x86 Linux program (games) on non-x86 Linux, like ARM (needs to be 32bits little-endian).

Also, Box86 use native version for some "system" libraries, like libc, libm, or SDL and OpenGL, leading to more performance and easier integration with host system.

Most x86 Games needs OpenGL, so on ARM platform, a solution like [gl4es](https://github.com/ptitSeb/gl4es) is probably needed.

Note that current version of Box86 doesn't feature any form of JIT/Dynarec: expect everything to be slow.

Current version is still experimental, so some stuff still wont run. But many do works, like for example, WorldOfGoo (at least on the Pandora), Airline Tycoon Deluxe or new FTL. Many of the GameMaker linux game also runs fine.

If you are serious about developping Box86, you should install ccache and activate it's support in the cmake project (use ccmake for example)
To have the TRACE enabled (i.e. dumping to stdout all individual x86 instruction execute, with dump of registers), you'll also need [Zydis library](https://github.com/zyantific/zydis) accessible on your system.

Some x86 internal opcode use parts of "Realmode X86 Emulator Library", see [x86primop.c](src/x86primop.c) for copyright details

Here are 3 videos, one of "Airline Tycoon Deluxe" running on an gigahertz OpenPandora, and the other 2 of "Bit.Trip.Runner" and "Neverwinter Night" running on an ODroid XU4.

[![Play on Youtube](https://img.youtube.com/vi/bLt0hMoFDLk/3.jpg)](https://www.youtube.com/watch?v=bLt0hMoFDLk) [![Play on Youtube](https://img.youtube.com/vi/8hr71S029Hg/1.jpg)](https://www.youtube.com/watch?v=8hr71S029Hg) [![Play on Youtube](https://img.youtube.com/vi/B4YN37z3-ws/1.jpg)](https://www.youtube.com/watch?v=B4YN37z3-ws)

----

Compiling
----
How to compile can be found [here](COMPILE.md)

----

Usage
----

There are a few environnement variable to control Box86 behavour.

See [here](USAGE.md) for all variables and what they do.

----

Version history
----

The change log is [here](CHANGELOG.md)

----

A note about Unity game emulation
----

Running Unity game is not possible for now. Mono itself use signal that are not well emulated enough. So the solution is to use a native version of the libmono used by unity. It can be found here: https://github.com/Unity-Technologies/mono and it needs to be build from source. When build copy `libmonosgen-2.0.so` to `libmonounity.so` and put it somewhere it can be dlopen'd (so in `usr/lib` or friend or somewhere in your `LD_LIBRARY_PATH`).
Note that libmonounity is not completly wrapped yet, and the mecanism to call x86 library from libmonounity is not done yet

----

Final word
----

(If you use Box86 in your project, please don't forget to mention Box86)
