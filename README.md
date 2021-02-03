# box86

![Official logo](Box86Logo.png "Official Logo")
Linux Userspace x86 Emulator with a twist

Box86 lets you run x86 Linux programs (such as games) on non-x86 Linux, like ARM (host system needs to be 32bit little-endian).

You *NEED* a 32-bit subsystem to run and build Box86. Box86 is useless on 64-bit only systems. Also, you *NEED* a 32-bit toolchain to build Box86. A toolchain that only support 64-bit will not compile Box86, and you'll get errors (typically on aarch64, you get "-marm" not recognized, and you'll need a multiarch or chroot environnement).

Because Box86 uses the native versions of some "system" libraries, like libc, libm, SDL, and OpenGL, it's easy to integrate and use, and performance can be surprisingly high in some cases.

Most x86 Games need OpenGL, so on ARM platforms a solution like [gl4es](https://github.com/ptitSeb/gl4es) is usually necessary. (Most ARM platforms only support OpenGL ES and/or their OpenGL implementation is dodgy (see OpenGL on Android).)

Box86 now integrates a DynaRec (dynamic recompiler) for the ARM platform, providing a speed boost between 5 to 10 times faster than only using the interpreter.

Many games already work, for example: WorldOfGoo, Airline Tycoon Deluxe, and FTL. Many of the GameMaker Linux games also run fine. (there's a long list, among them are UNDERTALE, A Risk of Rain, and Cook Serve Delicious)

If you are serious about developing Box86, you should install ccache and build Box86 with it. (Use ccmake for example.)
To enable TRACE (i.e. dumping to stdout all individual x86 instructions executed, with dump of registers), you'll also need [Zydis library](https://github.com/zyantific/zydis) available on your system.

Some x86 internal opcodes use parts of "Realmode X86 Emulator Library", see [x86primop.c](src/x86primop.c) for copyright details

Here's  6 videos, the first 2 videos are videos of "Airline Tycoon Deluxe" and "Heretic 2" running on a GigaHertz OpenPandora (the second one is using  the dynarec), and the next 2 videos are videos of of "Bit.Trip.Runner" and "Neverwinter Night" running on an ODroid XU4 (without dynarec), and the last 2  videos are on on a Pi4: Shovel Knight (video from @ITotalJustice) and Freedom Planet (video from @djazz), also without dynarec.

[![Play on Youtube](https://img.youtube.com/vi/bLt0hMoFDLk/3.jpg)](https://www.youtube.com/watch?v=bLt0hMoFDLk) [![Play on Youtube](https://img.youtube.com/vi/MM7kWYts7IA/3.jpg)](https://www.youtube.com/watch?v=MM7kWYts7IA) [![Play on Youtube](https://img.youtube.com/vi/8hr71S029Hg/1.jpg)](https://www.youtube.com/watch?v=8hr71S029Hg) [![Play on Youtube](https://img.youtube.com/vi/B4YN37z3-ws/1.jpg)](https://www.youtube.com/watch?v=B4YN37z3-ws) [![Play on Youtube](https://img.youtube.com/vi/xk8Q30mxqPg/1.jpg)](https://www.youtube.com/watch?v=xk8Q30mxqPg) [![Play on Youtube](https://img.youtube.com/vi/_QMRMVvYrqU/1.jpg)](https://www.youtube.com/watch?v=_QMRMVvYrqU)

You can find many more Box86 video on [PI Lab Channel](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g) [![PI Lab Channel](https://yt3.ggpht.com/a/AATXAJyMeWrgCjs78gr6To6yX4KtDPUCS7hsbX1rRA=s100-c-k-c0xffffffff-no-rj-mo)](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g).

Compatibility list is there: https://github.com/ptitSeb/box86-compatibility-list/issues

<img src="Box86Icon.png" width="96" height="96">

Logo and Icon made by @grayduck, thanks!

Note that this project is not to be mistaken with [86box](https://github.com/86Box/86Box), a nice "Full system" emulator specialized in early (to fairly recent) PC hardware.

----

Discord
----

If you want to discuss(or have any problems) about box86, there is a Discord friendly server there: [PI Lab Discord](https://discord.gg/Fh8sjmu)


----

Compiling
----
Compilation instructions can be found [here](COMPILE.md)

----

Usage
----

There are a few environment variables to control the behaviour of Box86.

See [here](USAGE.md) for all environment variables and what they do.

Note: Box86's Dynarec uses a mechanism with Memory Protection and a SegFault signal handler to handle JIT code. In simpler terms, if you want to use GDB to debug a running program that use JIT'd code (like mono/Unity3D), you will still have many "normal" segfaults triggering. It is suggested to use something like `handle SIGSEGV nostop` in GDB to not stop at each segfault, and maybe put a breakpoint inside `my_memprotectionhandler` in `signals.c` if you want to trap SegFaults.

----

Version history
----

The change log is available [here](CHANGELOG.md)

----

Notes about 64-bit platforms
----

Because Box86 works by directly translating function calls from x86 to host system, the host system (the one Box86 is running on) needs to have 32-bit libraries. Box86 doesn't include any 32-bit <-> 64-bit translation. So basically, to run Box86 on, for example, an ARM64 platform, you will need to build Box86 for ARM 32-bit, and also need to have a chroot with 32-bit libraries.

Also note that, even if, on day, there is a Box86_64, this one will only be able to run x86_64 binaries on 64-bit platforms. You will still need Box86 (and a 32-bit chroot) to run x86 binaries (in fact, the same is the case on actual x86_64 Linux).

----

A note about Unity game emulation
----

Running Unity games is a hit or miss for now. Unity uses Mono (which uses signals that are not well emulated enough), and a runtime embedded in the main binary. A solution would be to use a native version of the libmono library used by Unity (it can be found here: https://github.com/Unity-Technologies/mono and needs to be built from source). But the wrapping of this lib is tricky, and not done for now, so the only solution is to emulate everything. The tricky part is to emulate the "JIT" code emitted by Mono, however with the new "protected memory" mechanism implemented it should be running with correct performance now.
You should also note that some Unity3D games require OpenGL 3+ which can be tricky to provide on ARM SBC (single-board computers) for now.

TL;DR: Not all Unity games work and can require a high OpenGL profile, but the speed, for the ones running, should be correct now.

----

Notes about Steam
----

Linux Steam's can run now with box86. But it's still a bit unstable, and not everything works:
- First problem is Steam crashing after the sign-in window, if you encounter this issue, you may need to add libappindicator. To install it on Debian, run `sudo apt install libappindicator1`.
- If you select to "Remember password", Steam is crashing on subsequent starts, unless you have libnm intalled. To install it on Debian, run `sudo apt install libnm0`.
- Once open, Steam will only work on "Small Mode" and in "Big Picture", not in the regular "Large Mode". This is because some Steam components used in the browser view are only 64-bit now. So go in the "View" menu and switch to "Small view", else the list will stay empty. Alternatively, Steam can be started in small mode directly by using `+open steam://open/minigameslist` command line arguments.
- To avoid the "libc.so.6 is absent" message, you can use `STEAMOS=1` and `STEAM_RUNTIME=1` as environment variables. 
- Some Steam games (most Source engine games, like "Portal" or "Half-Life 2") use libtcmalloc. Box86 will detect it and will try to LD_PRELOAD it, for better compatibility. While it should work without the aformentionned feature, it is safer to add it to your system if you intend to play those game. To install it on Debian, run `sudo apt install libtcmalloc-minimal4`.

Steam for Windows installs fine but doesn't work yet.

----

Notes about Wine
----

Wine is now partly supported. Wine integrated program all runs, and some windows programs and games now runs fine. Don't forget most Windows games use Direct3D, this may require a complete OpenGL driver and as high profile as possible (and gl4es with ES2 backend have issue with Wine for now). Also, vulkan is not wrapped on box86, so vk3d is not usable yet, even if supported by the hardware.
Note: if you plan to use box86 with Wine on Raspberry Pi 3 or earlier, those models use a default OS that have a kernel with a 2/2 Split (meaning 2G of space for user program, and 2G of space for the Kernel). This is not compatible with Wine programs that needs to access memory > 2Gb address. So you'll need to reconfigure your kernel for a 3G/1G split. Use your favorite search engine for instructions on how to do that.

----

Final words
----

(If you use Box86 in your project, please don't forget to mention Box86)
