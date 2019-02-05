# box86

Linux Userspace x86 Emulator

Box86 will let run x86 Linux program (games) on non-x86 Linux, like ARM (needs to be 32bits little-endian).

Also, Box86 use native version for some "system" libraries, like libc, libm, or SDL and OpenGL, leading to more performance and easier integration with host system.

Most x86 Games needs OpenGL, so on ARM platform, a solution like gl4es is probably needed.

Current version is higly experimental and early, and most stuff wont run and run correctly. For now, WorldOfGoo does run correctly. FTL runs but sound is distorted...

If you are serious about developping Box86, you should install ccache and activate it's support in the cmake project (use ccmake for example)
To have the TRACE enabled (i.e. dumping to stdout all individual x86 instruction execute, with dump of registers), you'll also need Zydis library (https://github.com/zyantific/zydis) accessible on your system.