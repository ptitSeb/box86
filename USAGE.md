Usage
----

There are many environment variable to control Box86 behaviour. 

##### BOX86_LOG
Controls the Verbose level of the log
 * 0
 * NONE : No message (except some fatal error)
 * 1
 * INFO : Current default. Show some log
 * 2
 * DEBUG : Verbose a lot of stuffs (like relocations or function called)
 * 3
 * DUMP : All DEBUG plus DUMP of all ELF Info

#### BOX86_LD_LIBRARY_PATH
Controls the folder scanned to look for x86 libs. Default is current folder and `lib` in current folder.
Also `/usr/lib/i386-linux-gnu` and `/lib/i386-linux-gnu` are added if they exists

#### BOX86_PATH
Controls the folder scanned to look for x86 executable. Default is current folder and `bin` in current folder.

#### BOX86_DLSYM_ERROR
* 0 : default. Don't log `dlsym` error
* 1 : Log dlsym error

#### BOX86_TRACE_FILE
Send all log and trace to a file instead of `stdout`

#### BOX86_TRACE
Only on build with trace enabled. Trace allow the logging of all instruction execute, along with register dump
* 0 : No trace
* 1 : Trace enabled. The trace start after the init of all depending libs is done
* symbolname : Trace only `symbolname` (trace is disable if the symbol is not found)
* 0xXXXXXXX-0xYYYYYYY : trace only between the 2 addresses

#### BOX86_TRACE_INIT
Use BOX86_TRACE_INIT instead of BOX_TRACE to start trace before init of Libs and main program
* 0 : No trace
* 1 : Trace enabled. The trace start with the init of all depending libs is done

#### BOX86_TRACE_START
Only on build with trace enabled.
* NNNNNNN : Start trace only after NNNNNNNN opcode execute (number is an `uint64_t`)

#### BOX86_TRACE_XMM
Only on build with trace enabled.
* 0 : Default, the XMM (i.e. SSE/SSE2) register will not be logged with the general and x87 registers
* 1 : Dump the XMM registers

#### BOX86_LOAD_ADDR
Try to load at 0xXXXXXX main binaray (if binary is a PIE)
* 0xXXXXXXXX the load address (only active on PIE programs)

#### BOX86_NOSIGSEGV
To disable handling of SigSEGV (to ease debugging mainly)
* 0 : default, let x86 program set sighandler for SEGV
* 1 : disable handling of SigSEGV

#### BOX86_X11COLOR16
PANDORA only: to try convert X11 color from 32 bits to 16 bits (to avoid light green on light cyan windows
* 0 : default, don't touch X11 colors
* 1 : Change colors arguments in XSetForeground, XSetBackground and XCreateGC

#### BOX86_X11THREADS
To call XInitThreads when loading X11 (for old Loki games with Loki_Compat lib)
* 0 : default, don't force call XInitThreads
* 1 : Call XInitThreads as soon as libX11 is loaded

#### BOX86_DYNAREC_DUMP
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

#### BOX86_DYNAREC_TRACE
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
Some games use old version of some lib, with an ABI incompatible with native version.
Note that LittleInferno for example is auto detected, and libvorbis.so.0 is automatical added to emulated libs, and same for Don't Starve (and Together / Server variant) that use an old SDL2 too

#### BOX86_ALLOWMISSINGLIBS
Allow box86 to continue even if a lib is missing
 * 0 : default, stop if a lib cannot be loaded
 * 1 : continue even if a needed lib cannot be loaded. Unadvised, this will, in most cases, crash later on.

