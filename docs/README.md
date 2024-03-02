![Official logo](img/Box86Logo.png "Official Logo")

Linux Userspace x86 Emulator with a twist

[View changelog](https://github.com/ptitSeb/box86/blob/master/docs/CHANGELOG.md) | [中文](https://github.com/ptitSeb/box86/blob/master/docs/README_CN.md) | [Українська](https://github.com/ptitSeb/box86/blob/master/docs/README_UK.md) | [Report an error](https://github.com/ptitSeb/box86/issues/new)

![build](https://app.travis-ci.com/ptitSeb/box86.svg?branch=master) ![stars](https://img.shields.io/github/stars/ptitSeb/box86) ![forks](https://img.shields.io/github/forks/ptitSeb/box86) ![contributors](https://img.shields.io/github/contributors/ptitSeb/box86) ![prs](https://img.shields.io/github/issues-pr/ptitSeb/box86) ![issues](https://img.shields.io/github/issues/ptitSeb/box86)

----

Box86 lets you run x86 Linux programs (such as games) on non-x86 Linux systems, like ARM (host system needs to be 32bit little-endian).

You *NEED* a 32-bit subsystem to run and build Box86. Box86 is useless on 64-bit only systems. Also, you *NEED* a 32-bit toolchain to build Box86. A toolchain that only supports 64-bit will not compile Box86, and you'll get errors (typically on aarch64, you get "-marm" not recognized, and you'll need a multiarch or chroot environment).

Because Box86 uses the native versions of some "system" libraries, like libc, libm, SDL, and OpenGL, it's easy to integrate and use with most applications, and performance can be surprisingly high in many cases. Take a look at those bench analysis for an example [here](https://box86.org/index.php/2021/06/game-performances/). That also means that you will need a 32bits userspace on 64bits OS, like `armhf` on top an `aarch64` 64bits OS.

Most x86 Games need OpenGL, so on ARM platforms a solution like [gl4es](https://github.com/ptitSeb/gl4es) might be necessary. (Some ARM platforms only support OpenGL ES and/or their OpenGL implementation is dodgy. (see OpenGL on Android))

Box86 now integrates a DynaRec (dynamic recompiler) for the ARM platform, providing a speed boost between 5 to 10 times faster than only using the interpreter. Some high level information on how the Dynarec work can be found [here](https://box86.org/2021/07/inner-workings-a-high%e2%80%91level-view-of-box86-and-a-low%e2%80%91level-view-of-the-dynarec/).

Many games just work without much tweaking, for example: WorldOfGoo, Airline Tycoon Deluxe, and FTL. Many of the GameMaker Linux games also run fine. (there's a long list, among them are UNDERTALE, A Risk of Rain, and Cook Serve Delicious). Unity3D games also works fine, but the OpenGL requirement might be an issue on some ARM platforms.

If you are serious about developing Box86, you should install ccache and build Box86 with it. (Use ccmake for example.)
To enable TRACE (i.e. dumping to stdout all individual x86 instructions executed, with dump of registers), you'll also need [Zydis library](https://github.com/zyantific/zydis) available on your system.

Some x86 internal opcodes use parts of "Realmode X86 Emulator Library", see [x86primop.c](../src/emu/x86primop.c) for copyright details

----
Compiling/Installation
----
> Compilation instructions can be found [here](https://github.com/ptitSeb/box86/blob/master/docs/COMPILE.md)  
> Instructions for installing Wine for Box86 can be found [here](https://github.com/ptitSeb/box86/blob/master/docs/X86WINE.md)  

----

Here are 6 videos, the first 2 videos are videos of "Airline Tycoon Deluxe" and "Heretic 2" running on a GigaHertz OpenPandora (the second one is using  the dynarec), and the next 2 videos are videos of of "Bit.Trip.Runner" and "Neverwinter Night" running on an ODroid XU4 (without dynarec), and the last 2  videos are on on a Pi4: Shovel Knight (video from @ITotalJustice) and Freedom Planet (video from @djazz), also without dynarec.

[![Play on Youtube](https://img.youtube.com/vi/bLt0hMoFDLk/3.jpg)](https://www.youtube.com/watch?v=bLt0hMoFDLk) [![Play on Youtube](https://img.youtube.com/vi/MM7kWYts7IA/3.jpg)](https://www.youtube.com/watch?v=MM7kWYts7IA) [![Play on Youtube](https://img.youtube.com/vi/8hr71S029Hg/1.jpg)](https://www.youtube.com/watch?v=8hr71S029Hg) [![Play on Youtube](https://img.youtube.com/vi/B4YN37z3-ws/1.jpg)](https://www.youtube.com/watch?v=B4YN37z3-ws) [![Play on Youtube](https://img.youtube.com/vi/xk8Q30mxqPg/1.jpg)](https://www.youtube.com/watch?v=xk8Q30mxqPg) [![Play on Youtube](https://img.youtube.com/vi/_QMRMVvYrqU/1.jpg)](https://www.youtube.com/watch?v=_QMRMVvYrqU)

You can find many more Box86 videos on the [MicroLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ), [Pi Labs](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g) or [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA) YouTube channels.

Compatibility list is here: https://github.com/ptitSeb/box86-compatibility-list/issues

<img src="img/Box86Icon.png" width="96" height="96">

Logo and Icon made by @grayduck, thanks!

Note that this project is not to be mistaken with [86box](https://github.com/86Box/86Box), a nice "Full system" emulator specialized in early (to fairly recent) PC hardware.

----

Usage
----

There are a few environment variables to control the behaviour of Box86.

See [here](USAGE.md) for all environment variables and what they do.

Note: Box86's Dynarec uses a mechanism with Memory Protection and a SegFault signal handler to handle JIT code. In simpler terms, if you want to use GDB to debug a running program that use JIT'd code (like mono/Unity3D), you will still have many "normal" segfaults triggering. It is suggested to use something like `handle SIGSEGV nostop` in GDB to not stop at each segfault, and maybe put a breakpoint inside `my_box86signalhandler` in `signals.c` if you want to trap SegFaults.

----

Version history
----

The change log is available [here](CHANGELOG.md)

----

Notes about 64-bit platforms
----

Because Box86 works by directly translating function calls from x86 to host system, the host system (the one Box86 is running on) needs to have 32-bit libraries. Box86 doesn't include any 32-bit <-> 64-bit translation. So basically, to run Box86 on, for example, an ARM64 platform, you will need to build Box86 for ARM 32-bit, and also need to have a chroot with 32-bit libraries.

If you look at a 64bits version of box86, look at [Box64](https://github.com/ptitSeb/box64): this one is able to run x86_64 binaries on 64-bit platforms. But note that you still need Box86 (and a 32-bit chroot) to run x86 binaries (as it also happens on actual x86_64 Linux that need x86 libs and binary on multiarch to run).

----

Notes about Box86 configuration
----

Box86 now have configurations files. There are 2 files loaded. `/etc/box4.box86rc` and `~/.box86rc`. Both files have the same syntax, and is basicaly an ini files. Section in square brakets define the process name, and the rest is the env. var. to set. Looke at [Usage](USAGE.md) for detail on what parameters can be put. Box86 comes with a default file that should be installed for better stability. The file in in `system/box86.box86rc` and should be installed to `/etc/box86.box86rc` If, for some reasons, you don't want to install that file here, at least copy it to `~/.box86rc` or some game may not function correctly.
Note that the priority is: `~/.box86rc` > `/etc/box86.box86rc` > command line
So, your settings in `~/.box86rc` may override the setting from your command line...

----

Notes about Unity game emulation
----

Running Unity games should generaly work now, but you should also note that many Unity3D games require OpenGL 3+ which can be tricky to provide on ARM SBC (single-board computers) for now.
Hint: on Pi4, use `MESA_GL_VERSION_OVERRIDE=3.2` and with Panfrost use `PAN_MESA_DEBUG=gl3` to use higher profile if the game starts then quits before showing anything.

----

Notes about Steam
----

Linux Steam's can run now with box86. but you also need box64 for it to completly usable.
It is advised to run steam in the Small mode, as it use the less memory, but steamwebhelper (a 64bits process) will still be loaded even if not used.
The login screen cannot run without steamwebhelper, and will just show a blank windows without box64 properly setup in the system.
File note, Steam will use a lot of memory, and barely fit on system with 4GB of RAM. It will not work anymore on system with less memory (as a workaround, create a swap file, login and check "remember me", the use box64rc to disable steamwebhelper and run only in small mode without swap after the 1st login)
Final note: the Steam BigPicture will work, but also need steamwebhelper (and so box64), and lots of memory. It will not start on system with only 4GB of RAM without swap.
- If you have trouble installing Steam, you can find `install_steam.sh` in the root folder of the box86 repo. This simple script will download and install steam in your home folder, and then create a shortcut to steam in `/usr/local/bin` (and for this it will ask for sudo permission). Simply use `steam` to launch once it's installed. Note that the installation, being in the Home folder, will only work for a single user. Don't use this script if you need a multi-user installation.
- To avoid the "libc.so.6 is absent" message, you can use `STEAMOS=1` and `STEAM_RUNTIME=1` as environment variables (it's automatically there if you used the `install_steam.sh` script)

If you have issue with steam starting, with `steamwebhelper` not starting, you'll need to start steam with `-cef-disable-gpu` or `-cef-disable-gpu-compositor`.

----

Notes about Wine
----

Wine is now supported. Wine integrated program all runs, and many windows programs and games also runs fine. Don't forget most Windows games use Direct3D, this may require a complete OpenGL driver and a as high profile as possible (and gl4es with ES2 backend have issue with Wine for now).
Note: if you plan to use box86 with Wine on Raspberry Pi 3 or earlier, those models use a default OS that have a kernel with a 2/2 Split (meaning 2G of space for user program, and 2G of space for the Kernel). This is not compatible with Wine programs that needs to access memory > 2Gb address. So you'll need to reconfigure your kernel for a 3G/1G split.

----

Notes about Vulkan
----

Box86 already wrap Vulkan. If your system has a 32bits Vulkan driver, box86 will use it when needed. Profile 1.0, 1.1, 1.2 and 1.3, with some extensions, should be OK. DXVK, including the 2.0, works. I know some demos work on Pi4 (Sascha Willems demos build for x86 work the same as if build on armhf directly). Note that the Vulkan driver of the Pi4 DOES NOT support dxvk for now (wine DirectX->Vulkan wrapper). It's not a box86 issue, it's missing extensions (hardware support) and a few other things that make dxvk not working on pi4. On Panfrost side, PanVK is a bit young and I haven't tested dxvk with it yet.

----
Final word
----

I want to thank everyone who has contributed to box86 development.
There are many ways to contribute: code, financial, hardware and advertisement!
So, in no particular order, I want to thank:
 * For their major code contribution: rajdakin, icecream95, M-HT
 * For their major financial contribution: FlyingFathead, stormchaser3000, dennis1248, sll00, [libre-computer-project](https://libre.computer/), [CubeCoders Limited](http://cubecoders.com/)
 * For their hardware contribution: [ADLink](https://www.adlinktech.com/Products/Computer_on_Modules/COM-HPC-Server-Carrier-and-Starter-Kit/Ampere_Altra_Developer_Platform?lang=en), [Radxa](https://rockpi.org/), [Pine64](https://www.pine64.org/), [DragonBox](https://pyra-handheld.com/), [Novaspirit](https://www.youtube.com/channel/UCrjKdwxaQMSV_NDywgKXVmw), [HardKernel](https://www.hardkernel.com/), [TwisterOS team](https://twisteros.com/), [AYN](https://www.ayntec.com/)
 * For their continuous advertisement of box86 project: salva ([microLinux](https://www.youtube.com/channel/UCwFQAEj1lp3out4n7BeBatQ)), [PILab](https://www.youtube.com/channel/UCgfQjdc5RceRlTGfuthBs7g)/[TwisterOS team](https://twisteros.com/), [The Byteman](https://www.youtube.com/channel/UCEr8lpIJ3B5Ctc5BvcOHSnA), [NicoD](https://www.youtube.com/channel/UCpv7NFr0-9AB5xoklh3Snhg)

And I also thank the many other people who participated even once in the project.

(If you use Box86 in your project, please don't forget to mention it!)
