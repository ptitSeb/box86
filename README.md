# box86

Linux Userspace x86 Emulator with a twist

Box86 will let you run x86 Linux programs (games) on non-x86 Linux, like ARM (host system needs to be 32bit little-endian).

Because Box86 uses the native version for some "system" libraries, like libc, libm, or SDL and OpenGL, it's easy to integrate and use, and performances can be surprinsigly high in some cases.

Most x86 Games need OpenGL, so on ARM platforms, a solution like [gl4es](https://github.com/ptitSeb/gl4es) is probably needed.

Box86 now integrate a DynaRec for ARM platform, providing a speed boost between 5 to 10 times compared to only the interpretor.

Many games already work, like for example: WorldOfGoo, Airline Tycoon Deluxe or FTL. Many of the GameMaker linux games also run fine (there a long list, among them are UNDERTALE, A Risk of Rain, and Cook Server Delicious)

If you are serious about developing Box86, you should install ccache and activate it's support in the cmake project (use ccmake for example)
To have TRACE enabled (i.e. dumping to stdout all individual x86 instructions executed, with dump of registers), you'll also need [Zydis library](https://github.com/zyantific/zydis) accessible on your system.

Some x86 internal opcodes use parts of "Realmode X86 Emulator Library", see [x86primop.c](src/x86primop.c) for copyright details

Here are 6 videos, the first 2 of "Airline Tycoon Deluxe" and "Heretic 2" running on a gigahertz OpenPandora (the second one use the dynarec), and the next 2 of "Bit.Trip.Runner" and "Neverwinter Night" running on an ODroid XU4 (without dynarec), and the last 2 on a Pi4: Shovel Knight (video from @ITotalJustice) and Freedom Planet (video from @djazz), also without dynarec.

[![Play on Youtube](https://img.youtube.com/vi/bLt0hMoFDLk/3.jpg)](https://www.youtube.com/watch?v=bLt0hMoFDLk) [![Play on Youtube](https://img.youtube.com/vi/MM7kWYts7IA/3.jpg)](https://www.youtube.com/watch?v=MM7kWYts7IA) [![Play on Youtube](https://img.youtube.com/vi/8hr71S029Hg/1.jpg)](https://www.youtube.com/watch?v=8hr71S029Hg) [![Play on Youtube](https://img.youtube.com/vi/B4YN37z3-ws/1.jpg)](https://www.youtube.com/watch?v=B4YN37z3-ws) [![Play on Youtube](https://img.youtube.com/vi/xk8Q30mxqPg/1.jpg)](https://www.youtube.com/watch?v=xk8Q30mxqPg) [![Play on Youtube](https://img.youtube.com/vi/_QMRMVvYrqU/1.jpg)](https://www.youtube.com/watch?v=_QMRMVvYrqU)

You can find many more box86 video on [PI Lab Channel](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g) [![PI Lab Channel](https://yt3.ggpht.com/a/AATXAJyMeWrgCjs78gr6To6yX4KtDPUCS7hsbX1rRA=s100-c-k-c0xffffffff-no-rj-mo)](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g).

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

A note about 64bits platform
----

Because box86 works by directly translating function calls from x86 to host system, the host system (the one box86 is running on) needs to have 32bits library. Box86 doesn't include any 32bits <-> 64bits translation. So basically, to run box86 on, for example, an ARM64 platform, you will need to build box86 for arm 32bits, and also need to have a chroot with 32bits library.

Also note that, even if, on day, there is a box86_64, this one will only be able to run x86_64 binary on 64bits platform. You will still need box86 (and see 32bits chroot) to run x86 binary (in fact, like it is the case on actual x86_64 linux)

----

A note about Unity game emulation
----

Running Unity games is not possible for now. Mono itself uses signals that are not well emulated enough. So the solution is to use a native version of the libmono used by Unity. It can be found here: https://github.com/Unity-Technologies/mono and it needs to be built from source. When built copy `libmonosgen-2.0.so` to `libmonounity.so` and put it somewhere it can be dlopen'd (so in `usr/lib` or friend or somewhere in your `LD_LIBRARY_PATH`).
Note that libmonounity is not completely wrapped yet, and the mechanism to call x86 library from libmonounity is not done yet, so the use of libmonounity is not enable for now. 

TL;DR: mono games are not working for now anyway.

----

Final words
----

(If you use Box86 in your project, please don't forget to mention Box86)
