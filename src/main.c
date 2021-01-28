#ifndef __USE_FILE_OFFSET64
        #define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
        #define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
        #define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BIT
        #define _FILE_OFFSET_BIT 64
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/syscall.h>
#ifdef DYNAREC
#ifdef ARM
#include <linux/auxvec.h>
#include <asm/hwcap.h>
#endif
#endif

#include "build_info.h"
#include "debug.h"
#include "box86context.h"
#include "fileutils.h"
#include "elfloader.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86trace.h"
#include "librarian.h"
#include "library.h"
#include "auxval.h"
#include "wine_tools.h"

box86context_t *my_context = NULL;
int box86_log = LOG_NONE;
int box86_nobanner = 0;
int box86_dynarec_log = LOG_NONE;
int box86_pagesize;
#ifdef DYNAREC
int box86_dynarec = 1;
int box86_dynarec_dump = 0;
int box86_dynarec_forced = 0;
int box86_dynarec_largest = 0;
int box86_dynarec_smc = 0;
#ifdef ARM
int arm_vfp = 0;     // vfp version (3 or 4), with 32 registers is mendatory
int arm_swap = 0;
int arm_div = 0;
#endif
#else   //DYNAREC
int box86_dynarec = 0;
#endif
int dlsym_error = 0;
int trace_xmm = 0;
int trace_emm = 0;
#ifdef HAVE_TRACE
uint64_t start_cnt = 0;
#ifdef DYNAREC
int box86_dynarec_trace = 0;
#endif
#endif
#ifdef PANDORA
int x11color16 = 0;
#endif
#ifdef RPI
int box86_tokitori2 = 0;
#endif
int box86_zoom = 0;
int x11threads = 0;
int x11glx = 1;
int allow_missing_libs = 0;
int fix_64bit_inodes = 0;
int box86_steam = 0;
int box86_nopulse = 0;
int box86_nogtk = 0;
int box86_novulkan = 0;
char* libGL = NULL;
uintptr_t   trace_start = 0, trace_end = 0;
char* trace_func = NULL;
uintptr_t fmod_smc_start = 0;
uintptr_t fmod_smc_end = 0;
uint32_t default_fs = 0;
int jit_gdb = 0;
int box86_tcmalloc_minimal = 0;

FILE* ftrace = NULL;
int ftrace_has_pid = 0;

void openFTrace()
{
    char* t = getenv("BOX86_TRACE_FILE");
    char tmp[500];
    char* p = t;
    if(p && strstr(t, "%pid")) {
        strcpy(tmp, p);
        char* c = strstr(tmp, "%pid");
        *c = 0; // cut
        char pid[10];
        sprintf(pid, "%d", getpid());
        strcat(tmp, pid);
        c = strstr(p, "%pid") + strlen("%pid");
        strcat(tmp, c);
        p = tmp;
        ftrace_has_pid = 1;
    }
    if(p) {
        ftrace = fopen64(p, "w");
        if(!ftrace) {
            ftrace = stdout;
            printf_log(LOG_INFO, "Cannot open trace file \"%s\" for writing (error=%s)\n", p, strerror(errno));
        } else {
            if(!box86_nobanner)
                printf("BOX86 Trace redirected to \"%s\"\n", p);
        }
    }
}

void my_child_fork()
{
    if(ftrace_has_pid) {
        // open a new ftrace...
        fclose(ftrace);
        openFTrace();
    }
}

#ifdef DYNAREC
void GatherDynarecExtensions()
{
    if(box86_dynarec==0)    // no need to check if no dynarec
        return;
#ifdef ARM
    unsigned long hwcap = real_getauxval(AT_HWCAP);
    if(!hwcap)  // no HWCap: provide a default...
        hwcap = HWCAP_HALF|HWCAP_FAST_MULT|HWCAP_EDSP|HWCAP_NEON|HWCAP_VFPv3;
    // first, check all needed extensions, lif half, edsp and fastmult
    if((hwcap&HWCAP_HALF) == 0) {
        printf_log(LOG_INFO, "Missing HALF cpu support, disabling Dynarec\n");
        box86_dynarec=0;
        return;
    }
    if((hwcap&HWCAP_FAST_MULT) == 0) {
        printf_log(LOG_INFO, "Missing FAST_MULT cpu support, disabling Dynarec\n");
        box86_dynarec=0;
        return;
    }
    if((hwcap&HWCAP_EDSP) == 0) {
        printf_log(LOG_INFO, "Missing EDSP cpu support, disabling Dynarec\n");
        box86_dynarec=0;
        return;
    }
    if((hwcap&HWCAP_NEON) == 0) {
        printf_log(LOG_INFO, "Missing NEON support, disabling Dynarec\n");
        box86_dynarec=0;
        return;
    }
    if(hwcap&HWCAP_VFPv3D16) {
        printf_log(LOG_INFO, "VFPU only have 16 registers, disabling Dynarec\n");
        box86_dynarec=0;
        return;
    }
    if(hwcap&HWCAP_VFPv3)
        arm_vfp = 3;
    if(hwcap&HWCAP_VFPv4)
        arm_vfp = 4;
    if(!arm_vfp) {
        printf_log(LOG_INFO, "VFPUv3+ not detected, disabling Dynarec\n");
        box86_dynarec=0;
        return;
    }
    if(hwcap&HWCAP_SWP)
        arm_swap = 1;
    if(hwcap&HWCAP_IDIVA)
        arm_div = 1;
    printf_log(LOG_INFO, "Dynarec for ARM, with extension: HALF FAST_MULT EDSP NEON VFPv%d", arm_vfp);
    if(arm_swap)
        printf_log(LOG_INFO, " SWP");
    if(arm_div)
        printf_log(LOG_INFO, " IDIVA");
    printf_log(LOG_INFO, " PageSize:%d\n", box86_pagesize);
#endif
}
#endif

EXPORTDYN
void LoadLogEnv()
{
    ftrace = stdout;
    const char *p = getenv("BOX86_NOBANNER");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='1')
                box86_nobanner = p[0]-'0';
        }
    }
    p = getenv("BOX86_LOG");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0'+LOG_NONE && p[1]<='0'+LOG_DEBUG)
                box86_log = p[0]-'0';
        } else {
            if(!strcasecmp(p, "NONE"))
                box86_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box86_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box86_log = LOG_DEBUG;
            else if(!strcasecmp(p, "DUMP"))
                box86_log = LOG_DUMP;
        }
        if(!box86_nobanner)
            printf_log(LOG_INFO, "Debug level is %d\n", box86_log);
    }
#ifdef DYNAREC
    p = getenv("BOX86_DYNAREC_DUMP");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='1')
                box86_dynarec_dump = p[0]-'0';
        }
        if (box86_dynarec_dump) printf_log(LOG_INFO, "Dynarec blocks are dumped%s\n", (box86_dynarec_dump>1)?" in color":"");
    }
    p = getenv("BOX86_DYNAREC_LOG");
    if(p) {
        if(strlen(p)==1) {
            if((p[0]>='0'+LOG_NONE) && (p[0]<='0'+LOG_DUMP))
                box86_dynarec_log = p[0]-'0';
        } else {
            if(!strcasecmp(p, "NONE"))
                box86_dynarec_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box86_dynarec_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box86_dynarec_log = LOG_DEBUG;
            else if(!strcasecmp(p, "VERBOSE"))
                box86_dynarec_log = LOG_DUMP;
        }
        printf_log(LOG_INFO, "Dynarec log level is %d\n", box86_dynarec_log);
    }
    p = getenv("BOX86_DYNAREC");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='1')
                box86_dynarec = p[0]-'0';
        }
        printf_log(LOG_INFO, "Dynarec is %s\n", box86_dynarec?"On":"Off");
    }
    p = getenv("BOX86_DYNAREC_FORCED");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='1')
                box86_dynarec_forced = p[0]-'0';
        }
        if(box86_dynarec_forced)
        printf_log(LOG_INFO, "Dynarec is Forced on all addresses\n");
    }
    p = getenv("BOX86_DYNAREC_SMC");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='1')
                box86_dynarec_smc = p[0]-'0';
        }
        if(box86_dynarec_smc)
        printf_log(LOG_INFO, "Dynarec is trying to detect SMC in same dynablock\n");
    }
#endif
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE_XMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                trace_xmm = p[0]-'0';
        }
    }
    p = getenv("BOX86_TRACE_EMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                trace_emm = p[0]-'0';
        }
    }
    p = getenv("BOX86_TRACE_START");
    if(p) {
        char* p2;
        start_cnt = strtoll(p, &p2, 10);
        printf_log(LOG_INFO, "Will start trace only after %llu instructions\n", start_cnt);
    }
#ifdef DYNAREC
    p = getenv("BOX86_DYNAREC_TRACE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                box86_dynarec_trace = p[0]-'0';
            if(box86_dynarec_trace)
                printf_log(LOG_INFO, "Dynarec generated code will also print a trace\n");
        }
    }
#endif
#endif
    // grab BOX86_TRACE_FILE envvar, and change %pid to actual pid is present in the name
    openFTrace();
    // Other BOX86 env. var.
    p = getenv("BOX86_DLSYM_ERROR");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                dlsym_error = p[0]-'0';
        }
    }
#ifdef PANDORA
    p = getenv("BOX86_X11COLOR16");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                x11color16 = p[0]-'0';
        }
        printf_log(LOG_INFO, "Try to adjust X11 Color (32->16bits) : %s\n", x11color16?"Yes":"No");
    }
#endif
    p = getenv("BOX86_X11THREADS");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                x11threads = p[0]-'0';
        }
        if(x11threads)
            printf_log(LOG_INFO, "Try to Call XInitThreads if libX11 is loaded\n");
    }
    p = getenv("BOX86_X11GLX");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                x11glx = p[0]-'0';
        }
        if(x11glx)
            printf_log(LOG_INFO, "Hack to force libX11 GLX extension present\n");
        else
            printf_log(LOG_INFO, "Disabled Hack to force libX11 GLX extension present\n");
    }
    p = getenv("BOX86_LIBGL");
    if(p)
        libGL = strdup(p);
    if(!libGL) {
        p = getenv("SDL_VIDEO_GL_DRIVER");
        if(p)
            libGL = strdup(p);
    }
    if(libGL) {
        printf_log(LOG_INFO, "BOX86 using \"%s\" as libGL.so.1\n", p);
    }
    p = getenv("BOX86_ALLOWMISSINGLIBS");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                allow_missing_libs = p[0]-'0';
        }
        if(allow_missing_libs)
            printf_log(LOG_INFO, "Allow missing needed libs\n");
    }
    p = getenv("BOX86_NOPULSE");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                box86_nopulse = p[0]-'0';
        }
        if(box86_nopulse)
            printf_log(LOG_INFO, "Disable the use of pulseaudio libs\n");
    }
    p = getenv("BOX86_NOGTK");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                box86_nogtk = p[0]-'0';
        }
        if(box86_nogtk)
            printf_log(LOG_INFO, "Disable the use of wrapped gtk libs\n");
    }
    p = getenv("BOX86_NOVULKAN");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                box86_novulkan = p[0]-'0';
        }
        if(box86_novulkan)
            printf_log(LOG_INFO, "Disable the use of wrapped vulkan libs\n");
    }
    p = getenv("BOX86_FIX_64BIT_INODES");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                fix_64bit_inodes = p[0]-'0';
        }
        if(fix_64bit_inodes)
            printf_log(LOG_INFO, "Fix 64bit inodes\n");
    }
    p = getenv("BOX86_JITGDB");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                jit_gdb = p[0]-'0';
        }
        if(jit_gdb)
            printf_log(LOG_INFO, "Launch %s on segfault\n", (jit_gdb==2)?"gdbserver":"gdb");
    }
    box86_pagesize = sysconf(_SC_PAGESIZE);
    if(!box86_pagesize)
        box86_pagesize = 4096;
#ifdef DYNAREC
    GatherDynarecExtensions();
#endif
}

EXPORTDYN
void LoadEnvPath(path_collection_t *col, const char* defpath, const char* env)
{
    const char* p = getenv(env);
    if(p) {
        printf_log(LOG_INFO, "%s: ", env);
        ParseList(p, col, 1);
    } else {
        printf_log(LOG_INFO, "Using default %s: ", env);
        ParseList(defpath, col, 1);
    }
    if(LOG_INFO<=box86_log) {
        for(int i=0; i<col->size; i++)
            printf_log(LOG_INFO, "%s%s", col->paths[i], (i==col->size-1)?"\n":":");
    }
}

EXPORTDYN
int CountEnv(const char** env)
{
    // count, but remove all BOX86_* environnement
    // also remove PATH and LD_LIBRARY_PATH
    // but add 2 for default BOX86_PATH and BOX86_LD_LIBRARY_PATH
    const char** p = env;
    int c = 0;
    while(*p) {
        if(strncmp(*p, "BOX86_", 6)!=0)
            //if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0))
                ++c;
        ++p;
    }
    return c+2;
}
EXPORTDYN
int GatherEnv(char*** dest, const char** env, const char* prog)
{
    // Add all but BOX86_* environnement
    // but add 2 for default BOX86_PATH and BOX86_LD_LIBRARY_PATH
    const char** p = env;    
    int idx = 0;
    int path = 0;
    int ld_path = 0;
    while(*p) {
        if(strncmp(*p, "BOX86_PATH=", 11)==0) {
            (*dest)[idx++] = strdup(*p+6);
            path = 1;
        } else if(strncmp(*p, "BOX86_LD_LIBRARY_PATH=", 22)==0) {
            (*dest)[idx++] = strdup(*p+6);
            ld_path = 1;
        } else if(strncmp(*p, "_=", 2)==0) {
            /*int l = strlen(prog);
            char tmp[l+3];
            strcpy(tmp, "_=");
            strcat(tmp, prog);
            (*dest)[idx++] = strdup(tmp);*/
        } else if(strncmp(*p, "BOX86_", 6)!=0) {
            (*dest)[idx++] = strdup(*p);
            /*if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0)) {
            }*/
        }
        ++p;
    }
    // update the calloc of envv when adding new variables here
    if(!path) {
        (*dest)[idx++] = strdup("BOX86_PATH=.:bin");
    }
    if(!ld_path) {
        (*dest)[idx++] = strdup("BOX86_LD_LIBRARY_PATH=.:lib");
    }
    // add "_=prog" at the end...
    if(prog) {
        int l = strlen(prog);
        char tmp[l+3];
        strcpy(tmp, "_=");
        strcat(tmp, prog);
        (*dest)[idx++] = strdup(tmp);
    }
    // and a final NULL
    (*dest)[idx++] = 0;
    return 0;
}


void PrintHelp() {
    printf("\n\nThis is Box86, the Linux x86 emulator with a twist\n");
    printf("\nUsage is box86 [options] path/to/software [args]\n");
    printf("to launch x86 software\n");
    printf(" options can be :\n");
    printf("    '-v'|'--version' to print box86 version and quit\n");
    printf("    '-h'|'--help'    to print box86 help and quit\n");
    printf("You can also set some environment variables:\n");
    printf(" BOX86_PATH is the box86 version of PATH (default is '.:bin')\n");
    printf(" BOX86_LD_LIBRARY_PATH is the box86 version LD_LIBRARY_PATH (default is '.:lib')\n");
    printf(" BOX86_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed debug info\n");
    printf(" BOX86_NOBANNER with 0/1 to enable/disable the printing of box86 version and build at start\n");
#ifdef DYNAREC
    printf(" BOX86_DYNAREC_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed dynarec info\n");
    printf(" BOX86_DYNAREC with 0/1 to disable or enable Dynarec (On by default)\n");
#endif
#ifdef HAVE_TRACE
    printf(" BOX86_TRACE with 1 to enable x86 execution trace\n");
    printf("    or with XXXXXX-YYYYYY to enable x86 execution trace only between address\n");
    printf("    or with FunctionName to enable x86 execution trace only in one specific function\n");
    printf("  use BOX86_TRACE_INIT instead of BOX_TRACE to start trace before init of Libs and main program\n\t (function name will probably not work then)\n");
    printf(" BOX86_TRACE_XMM with 1 to enable dump of SSE/SSE2 register along with regular registers\n");
    printf(" BOX86_TRACE_START with N to enable trace after N instructions\n");
#ifdef DYNAREC
    printf(" BOX86_DYNAREC_TRACE with 0/1 to disable or enable Trace on generated code too\n");
#endif
#endif
    printf(" BOX86_TRACE_FILE with FileName to redirect logs in a file");
    printf(" BOX86_DLSYM_ERROR with 1 to log dlsym errors\n");
    printf(" BOX86_LOAD_ADDR=0xXXXXXX try to load at 0xXXXXXX main binary (if binary is a PIE)\n");
    printf(" BOX86_NOSIGSEGV=1 to disable handling of SigSEGV\n");
    printf(" BOX86_NOSIGILL=1  to disable handling of SigILL\n");
#ifdef PANDORA
    printf(" BOX86_X11COLOR16=1 to try convert X11 color from 32 bits to 16 bits (to avoid light green on light cyan windows\n");
#endif
    printf(" BOX86_X11THREADS=1 to call XInitThreads when loading X11 (for old Loki games with Loki_Compat lib)");
    printf(" BOX86_LIBGL=libXXXX set the name (and optionnaly full path) for libGL.so.1\n");
    printf(" BOX86_LD_PRELOAD=XXXX[:YYYYY] force loading XXXX (and YYYY...) libraries with the binary\n");
    printf(" BOX86_ALLOWMISSINGLIBS with 1 to allow to continue even if a lib is missing (unadvised, will probably  crash later)\n");
    printf(" BOX86_NOPULSE=1 to disable the loading of pulseaudio libs\n");
    printf(" BOX86_NOGTK=1 to disable the loading of wrapped gtk libs\n");
    printf(" BOX86_NOVULKAN=1 to disable the loading of wrapped vulkan libs\n");
    printf(" BOX86_JITGDB with 1 to launch \"gdb\" when a segfault is trapped, attached to the offending process\n");
}

EXPORTDYN
void LoadEnvVars(box86context_t *context)
{
    // check BOX86_LD_LIBRARY_PATH and load it
    LoadEnvPath(&context->box86_ld_lib, ".:lib:lib32:x86", "BOX86_LD_LIBRARY_PATH");
#ifdef PANDORA
    if(FileExist("/mnt/utmp/codeblocks/usr/lib/i386-linux-gnu", 0))
        AddPath("/mnt/utmp/codeblocks/usr/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/mnt/utmp/box86/lib/i386-linux-gnu", 0))
        AddPath("/mnt/utmp/box86/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    //TODO: add relative path to box86 location
#endif
    if(FileExist("/lib/i386-linux-gnu", 0))
        AddPath("/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/usr/lib/i386-linux-gnu", 0))
        AddPath("/usr/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/lib/i686-pc-linux-gnu", 0))
        AddPath("/lib/i686-pc-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/usr/lib/i686-pc-linux-gnu", 0))
        AddPath("/usr/lib/i686-pc-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/usr/lib32", 0))
        AddPath("/usr/lib32", &context->box86_ld_lib, 1);
    if(getenv("LD_LIBRARY_PATH"))
        PrependList(&context->box86_ld_lib, getenv("LD_LIBRARY_PATH"), 1);   // in case some of the path are for x86 world
    if(getenv("BOX86_EMULATED_LIBS")) {
        char* p = getenv("BOX86_EMULATED_LIBS");
        ParseList(p, &context->box86_emulated_libs, 0);
        if (my_context->box86_emulated_libs.size && box86_log) {
            printf_log(LOG_INFO, "BOX86 will force the used of emulated libs for ");
            for (int i=0; i<context->box86_emulated_libs.size; ++i)
                printf_log(LOG_INFO, "%s ", context->box86_emulated_libs.paths[i]);
            printf_log(LOG_INFO, "\n");
        }
    }

    if(getenv("BOX86_NOSIGSEGV")) {
        if (strcmp(getenv("BOX86_NOSIGSEGV"), "1")==0)
            context->no_sigsegv = 1;
            printf_log(LOG_INFO, "BOX86: Disabling handling of SigSEGV\n");
    }
    if(getenv("BOX86_NOSIGILL")) {
        if (strcmp(getenv("BOX86_NOSIGILL"), "1")==0)
            context->no_sigill = 1;
            printf_log(LOG_INFO, "BOX86: Disabling handling of SigILL\n");
    }
    // check BOX86_PATH and load it
    LoadEnvPath(&context->box86_path, ".:bin", "BOX86_PATH");
    if(getenv("PATH"))
        AppendList(&context->box86_path, getenv("PATH"), 1);   // in case some of the path are for x86 world
#ifdef HAVE_TRACE
    char* p = getenv("BOX86_TRACE");
    if(p) {
        if (strcmp(p, "0"))
            context->x86trace = 1;
    }
    p = getenv("BOX86_TRACE_INIT");
    if(p) {
        if (strcmp(p, "0"))
            context->x86trace = 1;
    }
    if(my_context->x86trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if(InitX86Trace(my_context)) {
            printf_log(LOG_INFO, "Zydis init failed, no x86 trace activated\n");
            context->x86trace = 0;
        }
    }
#endif
#ifdef BUILD_LIB
    context->argc = 1;  // need 1
    context->argv = (char**)malloc(sizeof(char*));
    context->argv[0] = strdup("dummy");
#endif
}

EXPORTDYN
void setupTraceInit(box86context_t* context)
{
#ifdef HAVE_TRACE
    char* p = getenv("BOX86_TRACE_INIT");
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t trace_start=0, trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%d-%d", &trace_start, &trace_end)!=2) {
                if(sscanf(p, "0x%X-0x%X", &trace_start, &trace_end)!=2)
                    sscanf(p, "%x-%x", &trace_start, &trace_end);
            }
            if(trace_start || trace_end)
                SetTraceEmu(trace_start, trace_end);
        } else {
            if (GetSymbolStartEnd(GetMapSymbol(my_context->maplib), p, &trace_start, &trace_end)) {
                SetTraceEmu(trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)trace_start, (void*)trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to Traced (\"%s\") not found, disabling trace\n", p);
                SetTraceEmu(0, 100);  // disabling trace, mostly
            }
        }
    } else {
        p = getenv("BOX86_TRACE");
        if(p)
            if (strcmp(p, "0"))
                SetTraceEmu(0, 1);
    }
#endif
}

EXPORTDYN
void setupTrace(box86context_t* context)
{
#ifdef HAVE_TRACE
    char* p = getenv("BOX86_TRACE");
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t trace_start=0, trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%d-%d", &trace_start, &trace_end)!=2) {
                if(sscanf(p, "0x%X-0x%X", &trace_start, &trace_end)!=2)
                    sscanf(p, "%x-%x", &trace_start, &trace_end);
            }
            if(trace_start || trace_end)
                SetTraceEmu(trace_start, trace_end);
        } else {
            if (GetGlobalSymbolStartEnd(my_context->maplib, p, &trace_start, &trace_end)) {
                SetTraceEmu(trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)trace_start, (void*)trace_end);
            } else if(GetLocalSymbolStartEnd(my_context->maplib, p, &trace_start, &trace_end, NULL)) {
                SetTraceEmu(trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)trace_start, (void*)trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to Traced (\"%s\") not found, trying to set trace later\n", p);
                SetTraceEmu(0, 1);  // disabling trace, mostly
                trace_func = strdup(p);
            }
        }
    }
#endif
}

void endBox86()
{
    if(!my_context)
        return;
    x86emu_t* emu = thread_get_emu();
    //atexit first
    printf_log(LOG_DEBUG, "Calling atexit registered functions\n");
    CallAllCleanup(emu);
    // than call all the Fini (some "smart" ordering of the fini may be needed, but for now, callign in this order should be good enough)
    printf_log(LOG_DEBUG, "Calling fini for all loaded elfs and unload native libs\n");
    RunElfFini(my_context->elfs[0], emu);
    FreeLibrarian(&my_context->maplib);    // unload all libs
    FreeLibrarian(&my_context->local_maplib);    // unload all libs
    // waiting for all thread except this one to finish
    int this_thread = GetTID();
    int pid = getpid();
    int running = 1;
    int attempt = 0;
    printf_log(LOG_DEBUG, "Waiting for all threads to finish before unloading box86context\n");
    while(running) {
        DIR *proc_dir;
        char dirname[100];
        snprintf(dirname, sizeof dirname, "/proc/self/task");
        proc_dir = opendir(dirname);
        running = 0;
        if (proc_dir)
        {
            struct dirent *entry;
            while ((entry = readdir(proc_dir)) != NULL && !running)
            {
                if(entry->d_name[0] == '.')
                    continue;

                int tid = atoi(entry->d_name);
                // tid != pthread_t, so no pthread functions are available here
                if(tid!=this_thread) {
                    if(attempt>4000) {
                        printf_log(LOG_INFO, "Stop waiting for remaining thread %04d\n", tid);
                        // enough wait, kill all thread!
                        syscall(__NR_tgkill, pid, tid, SIGABRT);
                    } else {
                        running = 1;
                        ++attempt;
                        sched_yield();
                    }
                }
            }
            closedir(proc_dir);
        }
    }
    // all done, free context
    FreeBox86Context(&my_context);
    if(libGL) {
        free(libGL);
        libGL = NULL;
    }
}

#ifdef BUILD_LIB
#ifdef BUILD_DYNAMIC
EXPORTDYN
box86context_t* GetBox86Context()
{
    return my_context;
}

EXPORTDYN
x86emu_t* GetX86Emu()
{
    return thread_get_emu();
}

__attribute__((constructor))
void init_library() 
{
    // init random seed
    srandom(time(NULL));

    // check BOX86_LOG debug level
    LoadLogEnv();

    printf_log(LOG_INFO, "Initializing libbox86\n");
    // create a global box86 context
    my_context = NewBox86Context(0);
    LoadEnvVars(my_context);
    CalcStackSize(my_context); // with no elf, so default size of 8M
    // init x86 emu
    x86emu_t *emu = NewX86Emu(my_context, my_context->ep, (uintptr_t)my_context->stack, my_context->stacksz, 0);
    // stack setup is much more complicated then just that!
    SetupInitialStack(emu);
    SetupX86Emu(emu);
    thread_set_emu(emu);
}
#endif

#else

static void free_contextargv()
{
    for(int i=0; i<my_context->argc; ++i)
        free(my_context->argv[i]);
}

const char **environ __attribute__((weak)) = NULL;
int main(int argc, const char **argv, const char **env) {

    init_auxval(argc, argv, environ?environ:env);
    // trying to open and load 1st arg
    if(argc==1) {
        PrintBox86Version();
        PrintHelp();
        return 1;
    }

    // init random seed
    srandom(time(NULL));

    // check BOX86_LOG debug level
    LoadLogEnv();
    
    const char* prog = argv[1];
    int nextarg = 1;
    // check if some options are passed
    while(prog && prog[0]=='-') {
        if(!strcmp(prog, "-v") || !strcmp(prog, "--version")) {
            PrintBox86Version();
            exit(0);
        }
        if(!strcmp(prog, "-h") || !strcmp(prog, "--help")) {
            PrintHelp();
            exit(0);
        }
        // other options?
        if(!strcmp(prog, "--")) {
            prog = argv[++nextarg];
            break;
        }
        printf("Warning, unrecognized option '%s'\n", prog);
        prog = argv[++nextarg];
    }
    if(!prog || nextarg==argc) {
        printf("Box86: nothing to run\n");
        exit(0);
    }
    if(!box86_nobanner)
        PrintBox86Version();
    // precheck, for win-preload
    if(strstr(prog, "wine-preloader")==(prog+strlen(prog)-strlen("wine-preloader"))) {
        // wine-preloader detecter, skipping it if next arg exist and is an x86 binary
        int x86 = (nextarg<argc)?FileIsX86ELF(argv[nextarg]):0;
        if(x86) {
            prog = argv[++nextarg];
            printf_log(LOG_INFO, "BOX86: Wine preloader detected, loading \"%s\" directly\n", prog);
        }
    }
    // check if this is wine
    if(!strcmp(prog, "wine") || (strlen(prog)>5 && !strcmp(prog+strlen(prog)-strlen("/wine"), "/wine"))) {
        const char* prereserve = getenv("WINEPRELOADRESERVE");
        printf_log(LOG_INFO, "BOX86: Wine detected, WINEPRELOADRESERVE=\"%s\"\n", prereserve?prereserve:"");
            //wine_prereserve(prereserve);
            // special case for winedbg, doesn't work anyway
        if(argv[nextarg+1] && strstr(argv[nextarg+1], "winedbg")==argv[nextarg+1]) {
            printf_log(LOG_NONE, "winedbg detected, not launching it!\n");
            exit(0);    // exiting, it doesn't work anyway
        }
    }
    // Create a new context
    my_context = NewBox86Context(argc - nextarg);

    // check BOX86_LD_LIBRARY_PATH and load it
    LoadEnvVars(my_context);

    if(argv[0][0]=='/')
        my_context->box86path = strdup(argv[0]);
    else
        my_context->box86path = ResolveFile(argv[0], &my_context->box86_path);
    // prepare all other env. var
    my_context->envc = CountEnv(environ?environ:env);
    printf_log(LOG_INFO, "Counted %d Env var\n", my_context->envc);
    // allocate extra space for new environment variables such as BOX86_PATH
    my_context->envv = (char**)calloc(my_context->envc+4, sizeof(char*));
    GatherEnv(&my_context->envv, environ?environ:env, my_context->box86path);
    if(box86_log>=LOG_DUMP) {
        for (int i=0; i<my_context->envc; ++i)
            printf_log(LOG_DUMP, " Env[%02d]: %s\n", i, my_context->envv[i]);
    }

    path_collection_t ld_preload = {0};
    if(getenv("BOX86_LD_PRELOAD")) {
        char* p = getenv("BOX86_LD_PRELOAD");
        ParseList(p, &ld_preload, 0);
        if (ld_preload.size && box86_log) {
            printf_log(LOG_INFO, "BOX86 try to Preload ");
            for (int i=0; i<ld_preload.size; ++i)
                printf_log(LOG_INFO, "%s ", ld_preload.paths[i]);
            printf_log(LOG_INFO, "\n");
        }
    } else {
        if(getenv("LD_PRELOAD")) {
            char* p = getenv("LD_PRELOAD");
            if(strstr(p, "libtcmalloc_minimal.so.4"))
                box86_tcmalloc_minimal = 1;
            if(strstr(p, "libasan.so"))
                box86_tcmalloc_minimal = 1; // it seems Address Sanitizer doesn't handle dlsym'd malloc very well
            ParseList(p, &ld_preload, 0);
            if (ld_preload.size && box86_log) {
                printf_log(LOG_INFO, "BOX86 try to Preload ");
                for (int i=0; i<ld_preload.size; ++i)
                    printf_log(LOG_INFO, "%s ", ld_preload.paths[i]);
                printf_log(LOG_INFO, "\n");
            }
        }
    }
    // lets build argc/argv stuff
    printf_log(LOG_INFO, "Looking for %s\n", prog);
    if(strchr(prog, '/'))
        my_context->argv[0] = strdup(prog);
    else
        my_context->argv[0] = ResolveFile(prog, &my_context->box86_path);

    const char* prgname = strrchr(prog, '/');
    if(!prgname)
        prgname = prog;
    else
        ++prgname;
    // special case for LittleInferno that use an old libvorbis
    if(strstr(prgname, "LittleInferno.bin.x86")==prgname) {
        printf_log(LOG_INFO, "LittleInferno detected, forcing emulated libvorbis\n");
        AddPath("libvorbis.so.0", &my_context->box86_emulated_libs, 0);
    }
    // special case for dontstarve that use an old SDL2
    if(strstr(prgname, "dontstarve")) {
        printf_log(LOG_INFO, "Dontstarve* detected, forcing emulated SDL2\n");
        AddPath("libSDL2-2.0.so.0", &my_context->box86_emulated_libs, 0);
    }
    // special case for steam that somehow seems to alter libudev opaque pointer (udev_monitor)
    if(strstr(prgname, "steam")==prgname) {
        printf_log(LOG_INFO, "steam detected, forcing emulated libudev\n");
        AddPath("libudev.so.0", &my_context->box86_emulated_libs, 0);
        box86_steam = 1;
    }
    // special case for steam-runtime-check-requirements to fake 64bits suport
    if(strstr(prgname, "steam-runtime-check-requirements")==prgname) {
        printf_log(LOG_INFO, "steam-runtime-check-requirements detected, faking All is good!\n");
        exit(0);    // exiting, not testing anything
    }
    // special case for UnrealLinux.bin, it doesn't like "full path resolution"
    if(!strcmp(prog, "UnrealLinux.bin") && my_context->argv[0]) {
        free(my_context->argv[0]);
        my_context->argv[0] = strdup("./UnrealLinux.bin");
    }
    #ifdef RPI
    // special case for TokiTori 2+, that check if texture max size is > = 8192
    if(strstr(prgname, "TokiTori2.bin.x86")==prgname) {
        printf_log(LOG_INFO, "TokiTori 2+ detected, runtime patch to fix GPU non-power-of-two faillure\n");
        box86_tokitori2 = 1;
    }
    #endif
    // special case for zoom
    if(strstr(prgname, "zoom")==prgname) {
        printf_log(LOG_INFO, "Zoom detected, trying to use system libturbojpeg if possible\n");
        box86_zoom = 1;
    }
    /*if(strstr(prgname, "awesomium_process")==prgname) {
        printf_log(LOG_INFO, "awesomium_process detected, forcing emulated libpng12\n");
        AddPath("libpng12.so.0", &my_context->box86_emulated_libs, 0);
    }*/
    /*if(!strcmp(prgname, "gdb")) {
        exit(-1);
    }*/

    for(int i=1; i<my_context->argc; ++i) {
        my_context->argv[i] = strdup(argv[i+nextarg]);
        printf_log(LOG_INFO, "argv[%i]=\"%s\"\n", i, my_context->argv[i]);
    }

    // check if file exist
    if(!my_context->argv[0] || !FileExist(my_context->argv[0], IS_FILE)) {
        printf_log(LOG_NONE, "Error: file is not found (check BOX86_PATH)\n");
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(!FileExist(my_context->argv[0], IS_FILE|IS_EXECUTABLE)) {
        printf_log(LOG_NONE, "Error: %s is not an executable file\n", my_context->argv[0]);
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(!(my_context->fullpath = realpath(my_context->argv[0], NULL)))
        my_context->fullpath = strdup(my_context->argv[0]);
    FILE *f = fopen64(my_context->argv[0], "rb");
    if(!f) {
        printf_log(LOG_NONE, "Error: Cannot open %s\n", my_context->argv[0]);
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    elfheader_t *elf_header = LoadAndCheckElfHeader(f, my_context->argv[0], 1);
    if(!elf_header) {
        printf_log(LOG_NONE, "Error: reading elf header of %s, try to launch natively instead\n", my_context->argv[0]);
        fclose(f);
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return execvp(argv[1], (char * const*)(argv+1));
    }
    AddElfHeader(my_context, elf_header);

    if(CalcLoadAddr(elf_header)) {
        printf_log(LOG_NONE, "Error: reading elf header of %s\n", my_context->argv[0]);
        fclose(f);
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // allocate memory
    if(AllocElfMemory(my_context, elf_header, 1)) {
        printf_log(LOG_NONE, "Error: allocating memory for elf %s\n", my_context->argv[0]);
        fclose(f);
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // Load elf into memory
    if(LoadElfMemory(f, my_context, elf_header)) {
        printf_log(LOG_NONE, "Error: loading in memory elf %s\n", my_context->argv[0]);
        fclose(f);
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // can close the file now
    fclose(f);
    if(ElfCheckIfUseTCMallocMinimal(elf_header)) {
        if(!box86_tcmalloc_minimal) {
            // need to reload with tcmalloc_minimal as a LD_PRELOAD!
            printf_log(LOG_INFO, "BOX86: tcmalloc_minimal.so.4 used, reloading box86 with the lib preladed\n");
            // need to get a new envv variable. so first count it and check if LD_PRELOAD is there
            int preload=(getenv("LD_PRELOAD"))?1:0;
            int nenv = 0;
            while(env[nenv]) nenv++;
            // alloc + "LD_PRELOAD" if needd + last NULL ending
            char** newenv = (char**)calloc(nenv+1+((preload)?0:1), sizeof(char*));
            // copy strings
            for (int i=0; i<nenv; ++i)
                newenv[i] = strdup(env[i]);
            // add ld_preload
            if(preload) {
                // find the line
                int l = 0;
                while(l<nenv) {
                    if(strstr(newenv[l], "LD_PRELOAD=")==newenv[l]) {
                        // found it!
                        char *old = newenv[l];
                        newenv[l] = (char*)calloc(strlen(old)+strlen("libtcmalloc_minimal.so.4:")+1, sizeof(char));
                        strcpy(newenv[l], "LD_PRELOAD=libtcmalloc_minimal.so.4:");
                        strcat(newenv[l], old + strlen("LD_PRELOAD="));
                        free(old);
                        // done, end loop
                        l = nenv;
                    } else ++l;
                }
            } else {
                //move last one
                newenv[nenv] = strdup(newenv[nenv-1]);
                free(newenv[nenv-1]);
                newenv[nenv-1] = strdup("LD_PRELOAD=libtcmalloc_minimal.so.4");
            }
            // duplicate argv too
            char** newargv = calloc(argc+1, sizeof(char*));
            int narg = 0;
            while(argv[narg]) {newargv[narg] = strdup(argv[narg]); narg++;}
            // launch with new env...
            if(execve(newargv[0], newargv, newenv)<0)
                printf_log(LOG_NONE, "Failed to relaunch, error is %d/%s\n", errno, strerror(errno));
        } else {
            printf_log(LOG_INFO, "BOX86: Using tcmalloc_minimal.so.4, and it's in the LD_PRELOAD command\n");
        }
    }
    // get and alloc stack size and align
    if(CalcStackSize(my_context)) {
        printf_log(LOG_NONE, "Error: allocating stack\n");
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    #ifdef RPI
    if(box86_tokitori2) {
        uint32_t *patch = (uint32_t*)0x85897f4;
        if(*patch==0x2000) {
            *patch = 0x1000;
            printf_log(LOG_NONE, "Runtime patching the game\n");
        } else
            printf_log(LOG_NONE, "Cannot patch the game\n");
    }
    #endif
    // init x86 emu
    x86emu_t *emu = NewX86Emu(my_context, my_context->ep, (uintptr_t)my_context->stack, my_context->stacksz, 0);
    // stack setup is much more complicated then just that!
    SetupInitialStack(emu); // starting here, the argv[] don't need free anymore
    SetupX86Emu(emu);
    SetEAX(emu, my_context->argc);
    SetEBX(emu, (uint32_t)my_context->argv);

    // child fork to handle traces
    pthread_atfork(NULL, NULL, my_child_fork);

    thread_set_emu(emu);

    setupTraceInit(my_context);
    // export symbols
    AddSymbols(my_context->maplib, GetMapSymbol(my_context->maplib), GetWeakSymbol(my_context->maplib), GetLocalSymbol(my_context->maplib), elf_header);
    /*if(wine_preloaded) {
        uintptr_t wineinfo = FindSymbol(GetMapSymbol(my_context->maplib), "wine_main_preload_info");
        if(!wineinfo) wineinfo = FindSymbol(GetWeakSymbol(my_context->maplib), "wine_main_preload_info");
        if(!wineinfo) wineinfo = FindSymbol(GetLocalSymbol(my_context->maplib), "wine_main_preload_info");
        if(!wineinfo) {printf_log(LOG_NONE, "Warning, Symbol wine_main_preload_info not found\n");}
        else {
            *(void**)wineinfo = get_wine_prereserve();
            printf_log(LOG_DEBUG, "WINE wine_main_preload_info found and updated\n");
        }
        #ifdef DYNAREC
        dynarec_wine_prereserve();
        #endif
    }*/
    // pre-load lib if needed
    if(ld_preload.size) {
        for (int i=0; i<ld_preload.size; ++i) {
            if(AddNeededLib(NULL, NULL, 0, ld_preload.paths[i], my_context, emu)) {
                printf_log(LOG_INFO, "Warning, cannot pre-load lib: \"%s\"\n", ld_preload.paths[i]);
            }            
        }
    }
    FreeCollection(&ld_preload);
    // Call librarian to load all dependant elf
    if(LoadNeededLibs(elf_header, my_context->maplib, &my_context->neededlibs, 0, my_context, emu)) {
        printf_log(LOG_NONE, "Error: loading needed libs in elf %s\n", my_context->argv[0]);
        FreeBox86Context(&my_context);
        return -1;
    }
    // reloc...
    printf_log(LOG_DEBUG, "And now export symbols / relocation for %s...\n", ElfName(elf_header));
    if(RelocateElf(my_context->maplib, NULL, elf_header)) {
        printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", my_context->argv[0]);
        FreeBox86Context(&my_context);
        return -1;
    }
    // and handle PLT
    RelocateElfPlt(my_context->maplib, NULL, elf_header);
    // defered init
    RunDeferedElfInit(emu);
    // do some special case check, _IO_2_1_stderr_ and friends, that are setup by libc, but it's already done here, so need to do a copy
    ResetSpecialCaseMainElf(elf_header);
    // init...
    setupTrace(my_context);
    // get entrypoint
    my_context->ep = GetEntryPoint(my_context->maplib, elf_header);
#if defined(RPI) || defined(RK3399)
    // before launching emulation, let's check if this is a mojosetup from GOG
    if (((strstr(prog, "bin/linux/x86/mojosetup") && getenv("MOJOSETUP_BASE")) || strstr(prog, ".mojosetup/mojosetup"))
       && getenv("GTK2_RC_FILES")) {
        sanitize_mojosetup_gtk_background();
    }
#endif

    atexit(endBox86);
    
    // emulate!
    printf_log(LOG_DEBUG, "Start x86emu on Main\n");
    SetEAX(emu, my_context->argc);
    SetEBX(emu, (uint32_t)my_context->argv);
    SetEIP(emu, my_context->ep);
    ResetFlags(emu);
    Run(emu, 0);
    // Get EAX
    int ret = GetEAX(emu);
    printf_log(LOG_DEBUG, "Emulation finished, EAX=%d\n", ret);

    if(trace_func)  {
        free(trace_func);
        trace_func = NULL;
    }

    return ret;
}
#endif  //BUILD_LIB
