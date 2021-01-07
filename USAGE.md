Usage
----

There are many environment variables to control Box86 behaviour. 

#### BOX86_LOG
Controls the Verbosity level of the logs
 * 0
 * NONE : No message (except some fatal error)
 * 1
 * INFO : Current default. Show some log
 * 2
 * DEBUG : Details a lot of stuff (like relocations or function called)
 * 3
 * DUMP : All DEBUG plus DUMP of all ELF Info

#### BOX86_NOBANNER
Control Box86 printing its version and build
 * 0 : Enable printing
 * 1 : Disable printing 

#### BOX86_LD_LIBRARY_PATH
Path to look for x86 libs. Default is current folder and `lib` in current folder.
Also `/usr/lib/i386-linux-gnu` and `/lib/i386-linux-gnu` are added if they exists

#### BOX86_PATH
Path to look for x86 executable. Default is current folder and `bin` in current folder.

#### BOX86_DLSYM_ERROR
* 0 : default. Don't log `dlsym` error
* 1 : Log dlsym error

#### BOX86_TRACE_FILE
Send all log and trace to a file instead of `stdout`
Also, if name contains `%pid` then this is replaced by the actual PID of box86 instance

#### BOX86_TRACE
Only on build with trace enabled. Trace allow the logging of all instruction executed, along with register dump
* 0 : No trace
* 1 : Trace enabled. Trace start after the initialisation of all depending libraries is done
* symbolname : Trace only `symbolname` (trace is disable if the symbol is not found)
* 0xXXXXXXX-0xYYYYYYY : trace only between the 2 addresses

#### BOX86_TRACE_INIT
Use BOX86_TRACE_INIT instead of BOX_TRACE to start trace before the initialisation of libraries and the running program
* 0 : No trace
* 1 : Trace enabled. The trace start with the init of all depending libs is done

#### BOX86_TRACE_START
Only on build with trace enabled.
* NNNNNNN : Start trace only after NNNNNNNN opcode execute (number is an `uint64_t`)

#### BOX86_TRACE_XMM
Only on build with trace enabled.
* 0 : Default, the XMM (i.e. SSE/SSE2) register will not be logged with the general and x86 registers
* 1 : Dump the XMM registers

#### BOX86_LOAD_ADDR
Try to load at 0xXXXXXX main binary (if binary is a PIE)
* 0xXXXXXXXX the load address (only active on PIE programs)

#### BOX86_NOSIGSEGV
Disable handling of SigSEGV ( Very useful for debugging.)
* 0 : default, let x86 program set sighandler for SEGV
* 1 : disable handling of SigSEGV

#### BOX86_NOSIGILL
Disable handling of SigILL (to ease debugging mainly)
* 0 : default, let x86 program set sighandler for Illegal Instruction
* 1 : disable handling of SigILL

#### BOX86_X11COLOR16
PANDORA only: Try converting X11 color from 32 bits to 16 bits (to avoid light green on light cyan windows)
* 0 : default, don't touch X11 colors
* 1 : Change colors arguments in XSetForeground, XSetBackground and XCreateGC

#### BOX86_X11THREADS
Call XInitThreads when loading X11 (for old Loki games with Loki_Compat lib)
* 0 : default, don't force call XInitThreads
* 1 : Call XInitThreads as soon as libX11 is loaded

#### BOX86_X11GLX
Force libX11 GLX extension to be present.
* 0 : Disable the force.
* 1 : default, GLX will always be present when using XQueryExtension

#### BOX86_DYNAREC_DUMP
 Enables/disables box86's dynarec dump.
 * 0 : Disable Dynarec blocks dump (default)
 * 1 : Enable Dynarec blocks dump
 * 2 : Enable Dynarec blocks dump with some colors

#### BOX86_DYNAREC_LOG
Set level of DynaRec log
 * 0 :
 * NONE : No Log for DynaRec
 * 1 :
 * INFO : Minimum Dynarec Log (only unimplemented OpCode)
 * 2 :
 * DEBUG : Debug Log for Dynarec (with detail on block created / executed)
 * 3 :
 * VERBOSE : All above plus more

#### BOX86_DYNAREC
 * 0 : Disable Dynarec
 * 1 : Enable Dynarec (default)

#### BOX86_DYNAREC_LINKER
 * 0 : Disable Dynarec Linker (use that on debug, with dynarec log >= 2, to have detail on wich block get executed)
 * 1 : Enable Dynarec Linker (default)

#### BOX86_DYNAREC_SAFEMMAP
 * 0 : Default. Some mmp/mmap64 are consider unsafe (depending of flags mainly)
 * 1 : Consider all mmap/mmap64 safe to use linker (potential speedup, but potential crash for apps using JIT/Dynarec)

#### BOX86_DYNAREC_TRACE
Enables/Disables trace for generated code
 * 0 : Disable trace for generated code (default)
 * 1 : Enable trace for generated code (like regular Trace, this will slow down a lot and generate huge logs)

#### BOX86_LIBGL
 * libXXXX set the name for libGL (default to libGL.so.1)
 * /PATH/TO/libGLXXX set the name and path for libGL
 You can also use SDL_VIDEO_GL_DRIVER

#### BOX86_LD_PRELOAD
 * XXXX[:YYYYY] force loading XXXX (and YYYY...) libraries with the binary
 PreLoaded libs can be emulated or native, and are treated the same way as if they were comming from the binary
 
#### BOX86_EMULATED_LIBS
 * XXXX[:YYYYY] force lib XXXX (and YYYY...) to be emulated (and not wrapped)
Some games uses an old version of some libraries, with an ABI incompatible with native version.
Note that LittleInferno for example is auto detected, and libvorbis.so.0 is automatical added to emulated libs, and same for Don't Starve (and Together / Server variant) that use an old SDL2 too

#### BOX86_ALLOWMISSINGLIBS
Allow box86 to continue even if a library is missing
 * 0 : default, stop if a library cannot be loaded
 * 1 : continue even if a needed library cannot be loaded. Unadvised, this will, in most cases, crash later on.

#### BOX86_NOPULSE
Disable the load of pulseaudio libraries
 * 0 : default, load pulseaudio libraries if present
 * 1 : disable the load of pulse audio libraries (libpulse and libpulse-simple), both native and x86 version

#### BOX86_NOGTK
Disable the load of wrapped GTK libraries
 * 0 : default, load wrapped gtk libs if present
 * 1 : disable the load of wrapped gtk libs (can be used with Steam, along with STEAM_RUNTIME=1 to use i386 versio of gtk)

#### BOX86_FIX_64BIT_INODES
 * 0 : Don't fix 64bit inodes (default)
 * 1 : Fix 64bit inodes. Helps when running on filesystems with 64bit inodes, the program uses API functions which doesn't support it and the program doesn't use inodes information.

#### BOX86_JITGDB
* 0 : Just print the Segfault message on segfault (default)
* 1 : Launch `gdb` when a segfault, bus error or illegal instruction signal is trapped, attached to the offending process, and go in an endless loop, waiting.
        When in gdb, you need to find the correct thread yourself (the one with `my_box86signalhandler` in is stack)
        then probably need to `finish` 1 or 2 functions (inside `usleep(..)`) and then you'll be in `my_box86signalhandler`, 
        just before the printf of the Segfault message. Then simply `set waiting=0` to exit the infinite loop.
* 2 : Launch `gdbserver` when a segfault, bus error or illegal instruction signal is trapped, attached to the offending process, and go in an endless loop, waiting.
        Use `gdb /PATH/TO/box86` and then `target remote 127.0.0.1:1234` to connect to the gdbserver (or use actual IP if not on the machine). After that, the procedure is the same as with ` BOX86_JITGDB=1`.
        This mode can be usefull when program redirect all console output to a file (like Unity3D Games)
