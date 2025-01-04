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
#include <sys/prctl.h>
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
#include "rcfile.h"

box86context_t *my_context = NULL;
int box86_log = LOG_NONE;
int box86_dump = 0;
int box86_nobanner = 0;
int box86_dynarec_log = LOG_NONE;
uintptr_t box86_pagesize;
uintptr_t box86_load_addr = 0;
int box86_showbt = 0;
int box86_maxcpu = 0;
int box86_maxcpu_immutable = 0;
int box86_isglibc234 = 0;
int box86_nosandbox = 0;
int box86_malloc_hack = 0;
int box86_mutex_aligned = 0;
int box86_quit = 0;
int box86_exit_code = 0;
#ifdef DYNAREC
int box86_dynarec = 1;
int box86_dynarec_dump = 0;
int box86_dynarec_forced = 0;
int box86_dynarec_largest = 0;
int box86_dynarec_bigblock = 1;
int box86_dynarec_forward = 128;
int box86_dynarec_strongmem = 0;
int box86_dynarec_x87double = 0;
int box86_dynarec_fastnan = 1;
int box86_dynarec_fastround = 1;
int box86_dynarec_safeflags = 1;
int box86_dynarec_callret = 0;
int box86_dynarec_hotpage = 16;
int box86_dynarec_bleeding_edge = 1;
int box86_dynarec_jvm = 1;
int box86_dynarec_tbb = 1;
int box86_dynarec_wait = 1;
int box86_dynarec_fastpage = 0;
uintptr_t box86_nodynarec_start = 0;
uintptr_t box86_nodynarec_end = 0;
int box86_dynarec_test = 0;
int box86_dynarec_missing = 0;
#ifdef ARM
int arm_vfp = 0;     // vfp version (3 or 4), with 32 registers is mendatory
int arm_swap = 0;
int arm_div = 0;
int arm_v8 = 0;
int arm_aes = 0;
int arm_pmull = 0;
#endif
#else   //DYNAREC
int box86_dynarec = 0;
#endif
int box86_libcef = 1;
int box86_sdl2_jguid = 0;
int dlsym_error = 0;
int cycle_log = 0;
int trace_xmm = 0;
int trace_emm = 0;
char* trace_init = NULL;
char* box86_trace = NULL;
#ifdef HAVE_TRACE
uint64_t start_cnt = 0;
#ifdef DYNAREC
int box86_dynarec_trace = 0;
#endif
#endif
#ifdef PANDORA
int x11color16 = 0;
#endif
#if defined(RPI) || defined(RK3399) || defined(RK3288) || defined(GOA_CLONE) || defined(PYRA) || defined(PANDORA)
int box86_tokitori2 = 0;
#endif
int box86_sc3u = 0;
int box86_mapclean = 0;
int box86_zoom = 0;
int box86_x11threads = 0;
int box86_x11glx = 1;
int box86_sse_flushto0 = 0;
int box86_x87_no80bits = 0;
int allow_missing_libs = 0;
int allow_missing_symbols = 0;
int fix_64bit_inodes = 0;
int box86_prefer_wrapped = 0;
int box86_prefer_emulated = 0;
int box86_steam = 0;
int box86_wine = 0;
int box86_musl = 0;
int box86_nopulse = 0;
int box86_nogtk = 0;
int box86_novulkan = 0;
int box86_nocrashhandler = 0;
#ifdef BAD_SIGNAL
int box86_futex_waitv = 0;
#else
int box86_futex_waitv = 1;
#endif
int box86_showsegv = 0;
char* box86_libGL = NULL;
uintptr_t   trace_start = 0, trace_end = 0;
char* trace_func = NULL;
uint32_t default_fs = 0;
int jit_gdb = 0;
int box86_tcmalloc_minimal = 0;

FILE* ftrace = NULL;
int ftrace_has_pid = 0;

void openFTrace(const char* newtrace)
{
    const char* t = newtrace?newtrace:getenv("BOX86_TRACE_FILE");
    char tmp[500];
    const char* p = t;
    if(p && strstr(t, "%pid")) {
        int next = 0;
        do {
            strcpy(tmp, p);
            char* c = strstr(tmp, "%pid");
            *c = 0; // cut
            char pid[16];
            if(next)
                sprintf(pid, "%d-%d", getpid(), next);
            else
                sprintf(pid, "%d", getpid());
            strcat(tmp, pid);
            c = strstr(p, "%pid") + strlen("%pid");
            strcat(tmp, c);
            ++next;
        } while (FileExist(tmp, IS_FILE));
        p = tmp;
        ftrace_has_pid = 1;
    }
    if(p) {
        if(!strcmp(p, "stderr"))
            ftrace = stderr;
        else {
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
}

void my_child_fork()
{
    if(ftrace_has_pid) {
        // open a new ftrace...
        fclose(ftrace);
        openFTrace(NULL);
        //printf_log(/*LOG_DEBUG*/LOG_INFO, "Forked child of %s\n", GetLastApplyName());
    }
}

int getNCpu();
const char* getCpuName();
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
    #ifdef AT_HWCAP2
    unsigned long hwcap2 = real_getauxval(AT_HWCAP2);
    if(hwcap2&HWCAP2_AES)
        arm_aes = 1;
    if(hwcap2&HWCAP2_PMULL)
        arm_pmull = 1;
    if((hwcap2&HWCAP2_AES) || (hwcap2&HWCAP2_CRC32))
        arm_v8 = 1;
    #endif
    printf_log(LOG_INFO, "Dynarec for ARMv%d, with extension: HALF FAST_MULT EDSP NEON VFPv%d", 7+arm_v8, arm_vfp);
    if(arm_swap)
        printf_log(LOG_INFO, " SWP");
    if(arm_div)
        printf_log(LOG_INFO, " IDIVA");
    if(arm_aes)
        printf_log(LOG_INFO, " AES");
    if(arm_pmull)
        printf_log(LOG_INFO, " PMULL");

    printf_log(LOG_INFO, " PageSize:%zd ", box86_pagesize);
#endif
}
#endif

EXPORTDYN
void LoadLogEnv()
{
    ftrace = stdout;
    box86_nobanner = isatty(fileno(stdout))?0:1;
    const char *p = getenv("BOX86_NOBANNER");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_nobanner = p[0]-'0';
        }
    }
    // grab BOX86_TRACE_FILE envvar, and change %pid to actual pid is present in the name
    openFTrace(NULL);
    //box86_log = isatty(fileno(ftrace))?LOG_INFO:LOG_NONE; //default LOG value different if stdout is redirected or not
    p = getenv("BOX86_LOG");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0'+LOG_NONE && p[0]<='0'+LOG_NEVER) {
                box86_log = p[0]-'0';
                if(box86_log == LOG_NEVER) {
                    --box86_log;
                    box86_dump = 1;
                }
            }
        } else {
            if(!strcasecmp(p, "NONE"))
                box86_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box86_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box86_log = LOG_DEBUG;
            else if(!strcasecmp(p, "DUMP")) {
                box86_log = LOG_DEBUG;
                box86_dump = 1;
            }
        }
        if(!box86_nobanner)
            printf_log(LOG_INFO, "Debug level is %d\n", box86_log);
    }
    p = getenv("BOX86_ROLLING_LOG");
    if(p) {
        int cycle = 0;
        if(sscanf(p, "%d", &cycle)==1)
                cycle_log = cycle;
        if(cycle_log==1)
            cycle_log = 16;
        if(cycle_log<0)
            cycle_log = 0;
        if(cycle_log && box86_log>LOG_INFO) {
            cycle_log = 0;
            printf_log(LOG_NONE, "Incompatible Rolling log and Debug Log, disabling Rolling log\n");
        }
    }
    if(!box86_nobanner && cycle_log)
        printf_log(LOG_INFO, "Rolling log, showing last %d function call on signals\n", cycle_log);
    p = getenv("BOX86_DUMP");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dump = p[0]-'0';
        }
    }
    if(!box86_nobanner && box86_dump)
        printf_log(LOG_INFO, "Elf Dump if ON\n");
#ifdef DYNAREC
    p = getenv("BOX86_DYNAREC_DUMP");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='2')
                box86_dynarec_dump = p[0]-'0';
        }
        if (box86_dynarec_dump) printf_log(LOG_INFO, "Dynarec blocks are dumped%s\n", (box86_dynarec_dump>1)?" in color":"");
    }
    p = getenv("BOX86_DYNAREC_LOG");
    if(p) {
        if(strlen(p)==1) {
            if((p[0]>='0'+LOG_NONE) && (p[0]<='0'+LOG_VERBOSE))
                box86_dynarec_log = p[0]-'0';
        } else {
            if(!strcasecmp(p, "NONE"))
                box86_dynarec_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box86_dynarec_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box86_dynarec_log = LOG_DEBUG;
            else if(!strcasecmp(p, "VERBOSE"))
                box86_dynarec_log = LOG_VERBOSE;
        }
        printf_log(LOG_INFO, "Dynarec log level is %d\n", box86_dynarec_log);
    }
    p = getenv("BOX86_DYNAREC");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec = p[0]-'0';
        }
        printf_log(LOG_INFO, "Dynarec is %s\n", box86_dynarec?"on":"off");
    }
    p = getenv("BOX86_DYNAREC_FORCED");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_forced = p[0]-'0';
        }
        if(box86_dynarec_forced)
            printf_log(LOG_INFO, "Dynarec is forced on all addresses\n");
    }
    p = getenv("BOX86_DYNAREC_BIGBLOCK");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='2')
                box86_dynarec_bigblock = p[0]-'0';
        }
        if(!box86_dynarec_bigblock)
            printf_log(LOG_INFO, "Dynarec will not try to make big blocks\n");
        else if (box86_dynarec_bigblock>1)
            printf_log(LOG_INFO, "Dynarec will try to make bigger blocks\n");
    }
    p = getenv("BOX86_DYNAREC_FORWARD");
    if(p) {
        int val = -1;
        if(sscanf(p, "%d", &val)==1) {
            if(val>=0)
                box86_dynarec_forward = val;
        }
        if(box86_dynarec_forward)
            printf_log(LOG_INFO, "Dynarec will continue block for %d bytes on forward jump\n", box86_dynarec_forward);
        else
            printf_log(LOG_INFO, "Dynarec will not continue block on forward jump\n");
    }
    p = getenv("BOX86_DYNAREC_STRONGMEM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='3')
                box86_dynarec_strongmem = p[0]-'0';
        }
        if(box86_dynarec_strongmem)
            printf_log(LOG_INFO, "Dynarec will try to emulate a strong memory model%s\n", (box86_dynarec_strongmem==1)?" with limited performance loss":((box86_dynarec_strongmem==3)?" with more performance loss":""));
    }
    p = getenv("BOX86_DYNAREC_X87DOUBLE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_x87double = p[0]-'0';
        }
        if(box86_dynarec_x87double)
            printf_log(LOG_INFO, "Dynarec will use only double for x87 emulation\n");
    }
    p = getenv("BOX86_DYNAREC_FASTNAN");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_fastnan = p[0]-'0';
        }
        if(!box86_dynarec_fastnan)
            printf_log(LOG_INFO, "Dynarec will try to normalize generated NAN\n");
    }
    p = getenv("BOX86_DYNAREC_FASTROUND");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_fastround = p[0]-'0';
        }
        if(!box86_dynarec_fastround)
            printf_log(LOG_INFO, "Dynarec will try to generate x86 precise IEEE->int rounding and set rounding mode for computation\n");
    }
    p = getenv("BOX86_DYNAREC_SAFEFLAGS");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='2')
                box86_dynarec_safeflags = p[0]-'0';
        }
        if(!box86_dynarec_safeflags)
            printf_log(LOG_INFO, "Dynarec will not play it safe with x86 flags\n");
        else
            printf_log(LOG_INFO, "Dynarec will play %s safe with x86 flags\n", (box86_dynarec_safeflags==1)?"moderatly":"it");
    }
    p = getenv("BOX86_DYNAREC_CALLRET");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_callret = p[0]-'0';
        }
        if(box86_dynarec_callret)
            printf_log(LOG_INFO, "Dynarec will optimize CALL/RET\n");
        else
            printf_log(LOG_INFO, "Dynarec will not optimize CALL/RET\n");
    }
    p = getenv("BOX86_DYNAREC_WAIT");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_wait = p[0]-'0';
        }
        if(!box86_dynarec_wait)
            printf_log(LOG_INFO, "Dynarec will not wait for FillBlock to ready and use Interpreter instead\n");
    }
    p = getenv("BOX86_DYNAREC_BLEEDING_EDGE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_bleeding_edge = p[0]-'0';
        }
        if(!box86_dynarec_bleeding_edge)
            printf_log(LOG_INFO, "Dynarec will not detect MonoBleedingEdge\n");
    }
    p = getenv("BOX86_DYNAREC_JVM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_jvm = p[0]-'0';
        }
        if(!box86_dynarec_jvm)
            printf_log(LOG_INFO, "Dynarec will not detect libjvm\n");
    }
    p = getenv("BOX86_DYNAREC_TBB");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_tbb = p[0]-'0';
        }
        if(!box86_dynarec_tbb)
            printf_log(LOG_INFO, "Dynarec will not detect libtbb\n");
    }
    p = getenv("BOX86_DYNAREC_MISSING");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_missing = p[0]-'0';
        }
        if(box86_dynarec_missing)
            printf_log(LOG_INFO, "Dynarec will print missing opcodes\n");
    }
    p = getenv("BOX86_NODYNAREC");
    if(p) {
        if (strchr(p,'-')) {
            if(sscanf(p, "%d-%d", &box86_nodynarec_start, &box86_nodynarec_end)!=2) {
                if(sscanf(p, "0x%X-0x%X", &box86_nodynarec_start, &box86_nodynarec_end)!=2)
                    sscanf(p, "%x-%x", &box86_nodynarec_start, &box86_nodynarec_end);
            }
            printf_log(LOG_INFO, "No Dynablock creation that start in %p - %p range\n", (void*)box86_nodynarec_start, (void*)box86_nodynarec_end);
        }
    }
    p = getenv("BOX86_DYNAREC_TEST");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_dynarec_test = p[0]-'0';
        }
        if(box86_dynarec_test) {
            box86_dynarec_fastnan = 0;
            box86_dynarec_fastround = 0;
            box86_dynarec_callret = 0;
            printf_log(LOG_INFO, "Dynarec will compare it's execution with the interpreter (super slow, only for testing)\n");
        }
    }

#endif
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE_XMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                trace_xmm = p[0]-'0';
        }
    }
    p = getenv("BOX86_TRACE_EMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
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
            if(p[0]>='0' && p[0]<='0'+1)
                box86_dynarec_trace = p[0]-'0';
            if(box86_dynarec_trace)
                printf_log(LOG_INFO, "Dynarec generated code will also print a trace\n");
        }
    }
#endif
#endif
    // Other BOX86 env. var.
    p = getenv("BOX86_LIBCEF");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_libcef = p[0]-'0';
        }
        if(!box86_libcef)
            printf_log(LOG_INFO, "Dynarec will not detect libcef\n");
    }
    p = getenv("BOX86_NOCRASHHANDLER");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_nocrashhandler = p[0]-'0';
        }
        if(box86_nocrashhandler)
            printf_log(LOG_INFO, "Box86 will consider crashhnadler.so to not be present\n");
    }
    p = getenv("BOX86_SDL2_JGUID");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box86_sdl2_jguid = p[0]-'0';
        }
        if(!box86_sdl2_jguid)
            printf_log(LOG_INFO, "BOX86 will workaround the use of  SDL_GetJoystickGUIDInfo with 4 args instead of 5\n");
    }
    p = getenv("BOX86_LOAD_ADDR");
    if(p) {
        if(sscanf(p, "0x%zx", &box86_load_addr)!=1)
            box86_load_addr = 0;
        if(box86_load_addr)
            printf_log(LOG_INFO, "Use a starting load address of %p\n", (void*)box86_load_addr);
    }
    p = getenv("BOX86_DLSYM_ERROR");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                dlsym_error = p[0]-'0';
        }
        printf_log(LOG_INFO, "Shows details of dlopen / dlsym /dlclose : %s\n", dlsym_error?"Yes":"No");
    }
#ifdef PANDORA
    p = getenv("BOX86_X11COLOR16");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                x11color16 = p[0]-'0';
        }
        printf_log(LOG_INFO, "Try to adjust X11 Color (32->16bits) : %s\n", x11color16?"Yes":"No");
    }
#endif
    p = getenv("BOX86_X11THREADS");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box86_x11threads = p[0]-'0';
        }
        if(box86_x11threads)
            printf_log(LOG_INFO, "Try to Call XInitThreads if libX11 is loaded\n");
    }
    p = getenv("BOX86_X11GLX");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box86_x11glx = p[0]-'0';
        }
        if(box86_x11glx)
            printf_log(LOG_INFO, "Hack to force libX11 GLX extension present\n");
        else
            printf_log(LOG_INFO, "Disabled Hack to force libX11 GLX extension present\n");
    }
    p = getenv("BOX86_LIBGL");
    if(p)
        box86_libGL = box_strdup(p);
    if(!box86_libGL) {
        p = getenv("SDL_VIDEO_GL_DRIVER");
        if(p)
            box86_libGL = box_strdup(p);
    }
    if(box86_libGL) {
        printf_log(LOG_INFO, "BOX86 using \"%s\" as libGL.so.1\n", p);
    }
    p = getenv("BOX86_ALLOWMISSING_LIBS");
    if(!p) 
        p = getenv("BOX86_ALLOWMISSINGLIBS");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                allow_missing_libs = p[0]-'0';
        }
        if(allow_missing_libs)
            printf_log(LOG_INFO, "Allow missing needed libs\n");
    }
    p = getenv("BOX86_ALLOWMISSING_SYMBOLS");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                allow_missing_symbols = p[0]-'0';
        }
        if(allow_missing_symbols)
            printf_log(LOG_INFO, "Allow missing needed symbols\n");
    }
    p = getenv("BOX86_MALLOC_HACK");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+2)
                box86_malloc_hack = p[0]-'0';
        }
        if(!box86_malloc_hack) {
            if(box86_malloc_hack==1) {
                printf_log(LOG_INFO, "Malloc hook will not be redirected\n");
            } else
                printf_log(LOG_INFO, "Malloc hook will check for mmap/free occurrences\n");
        }
    }
    p = getenv("BOX86_MUTEX_ALIGNED");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box86_mutex_aligned = p[0]-'0';
        }
        if(!box86_mutex_aligned) {
            if(box86_mutex_aligned==1) {
                printf_log(LOG_INFO, "BOX86 will not aligned mutexes\n");
            } else
                printf_log(LOG_INFO, "BOX86 will wrap mutex to for them aligned\n");
        }
    }
    p = getenv("BOX86_NOPULSE");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box86_nopulse = p[0]-'0';
        }
        if(box86_nopulse)
            printf_log(LOG_INFO, "Disable the use of pulseaudio libs\n");
    }
    p = getenv("BOX86_NOGTK");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box86_nogtk = p[0]-'0';
        }
        if(box86_nogtk)
            printf_log(LOG_INFO, "Disable the use of wrapped gtk libs\n");
    }
    p = getenv("BOX86_NOVULKAN");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box86_novulkan = p[0]-'0';
        }
        if(box86_novulkan)
            printf_log(LOG_INFO, "Disable the use of wrapped vulkan libs\n");
    }
    p = getenv("BOX86_FUTEX_WAITV");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box86_futex_waitv = p[0]-'0';
        }
        #ifdef BAD_SIGNAL
        if(box86_futex_waitv)
            printf_log(LOG_INFO, "Enable the use of futex waitv syscall (if available on the system\n");
        #else
        if(!box86_futex_waitv)
            printf_log(LOG_INFO, "Disable the use of futex waitv syscall\n");
        #endif
    }
    p = getenv("BOX86_FIX_64BIT_INODES");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                fix_64bit_inodes = p[0]-'0';
        }
        if(fix_64bit_inodes)
            printf_log(LOG_INFO, "Fix 64bit inodes\n");
    }
    p = getenv("BOX86_JITGDB");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+3)
                jit_gdb = p[0]-'0';
        }
        if(jit_gdb)
            printf_log(LOG_INFO, "Launch %s on segfault\n", (jit_gdb==2)?"gdbserver":((jit_gdb==3)?"lldb":"gdb"));
    }
    p = getenv("BOX86_SHOWSEGV");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box86_showsegv = p[0]-'0';
        }
        if(box86_showsegv)
            printf_log(LOG_INFO, "Show Segfault signal even if a signal handler is present\n");
    }
    p = getenv("BOX86_SHOWBT");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box86_showbt = p[0]-'0';
        }
        if(box86_showbt)
            printf_log(LOG_INFO, "Show Backtrace for signals\n");
    }
    p = getenv("BOX86_MAXCPU");
    if(p) {
        int maxcpu = 0;
        if(sscanf(p, "%d", &maxcpu)==1)
            box86_maxcpu = maxcpu;
        if(box86_maxcpu<0)
            box86_maxcpu = 0;
        if(box86_maxcpu) {
            printf_log(LOG_INFO, "Will not expose more than %d cpu cores\n", box86_maxcpu);
        } else {
            printf_log(LOG_INFO, "Will not limit the number of cpu cores exposed\n");
        }
    }
    box86_pagesize = sysconf(_SC_PAGESIZE);
    if(!box86_pagesize)
        box86_pagesize = 4096;
#ifdef DYNAREC
    GatherDynarecExtensions();
#endif
    int ncpu = getNCpu();
    const char* cpuname = getCpuName();
    printf_log(LOG_INFO, "Running on %s with %d Cores\n", cpuname, ncpu);
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
int CountEnv(char** env)
{
    // count, but remove all BOX86_* environnement
    // also remove PATH and LD_LIBRARY_PATH
    // but add 2 for default BOX86_PATH and BOX86_LD_LIBRARY_PATH
    char** p = env;
    int c = 0;
    while(*p) {
        if(strncmp(*p, "BOX86_PATH=", 11)==0) {
        } else if(strncmp(*p, "BOX86_LD_LIBRARY_PATH=", 22)==0) {
        } else if(strncmp(*p, "BOX86_", 6)!=0) {
            ++c;
        }
        ++p;
    }
    return c+2;
}
EXPORTDYN
int GatherEnv(char*** dest, char** env, const char* prog)
{
    // Add every BOX86_* environment variables
    char** p = env;    
    int idx = 0;
    int box86_path = 0;
    int box86_ld_path = 0;
    while(*p) {
        if(strncmp(*p, "BOX86_PATH=", 11)==0) {
            (*dest)[idx++] = box_strdup(*p);
            box86_path = 1;
        } else if(strncmp(*p, "BOX86_LD_LIBRARY_PATH=", 22)==0) {
            (*dest)[idx++] = box_strdup(*p);
            box86_ld_path = 1;
        } else if(strncmp(*p, "BOX86_", 6)!=0) {
            (*dest)[idx++] = box_strdup(*p);
        }
        ++p;
    }
    // Add default values for BOX86_PATH and BOX86_LD_LIBRARY_PATH
    if(!box86_path) {
        (*dest)[idx++] = box_strdup("BOX86_PATH=.:bin");
    }
    if(!box86_ld_path) {
        (*dest)[idx++] = box_strdup("BOX86_LD_LIBRARY_PATH=.:lib");
    }
    // Add "_=prog" at the end...
    if(prog) {
        int l = strlen(prog);
        char tmp[l+3];
        strcpy(tmp, "_=");
        strcat(tmp, prog);
        (*dest)[idx++] = box_strdup(tmp);
    }
    // and a final NULL
    (*dest)[idx++] = 0;
    return 0;
}


void PrintFlags() {
    printf("Environment Variables:\n");
    printf(" BOX86_PATH is the box86 version of PATH (default is '.:bin')\n");
    printf(" BOX86_LD_LIBRARY_PATH is the box86 version LD_LIBRARY_PATH (default is '.:lib')\n");
    printf(" BOX86_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed debug info (level 3 is level 2 + BOX86_DUMP)\n");
    printf(" BOX86_ROLLING_LOG 0/1 use a rolling level 2 log of 16 entries\n");
    printf(" BOX86_DUMP with 0/1 to dump elf infos\n");
    printf(" BOX86_NOBANNER with 0/1 to enable/disable the printing of box86 version and build at start\n");
#ifdef DYNAREC
    printf(" BOX86_DYNAREC_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed dynarec info\n");
    printf(" BOX86_DYNAREC with 0/1 to disable or enable Dynarec (On by default)\n");
    printf(" BOX86_NODYNAREC with address interval (0x1234-0x4567) to forbid dynablock creation in the interval specified\n");
    printf(" BOX86_DYNAREC_BIGBLOCK 0/1/2 to control Dynarec building BigBlock or not (default: 1)\n");
    printf(" BOX86_DYNAREC_STRONGMEM 0/1/2 to control Dynarec emulation attempt of Stong memory model (default: 0)\n");
    printf(" BOX86_DYNAREC_X87DOUBLE 0/1 to force Dynarec to use Double for x87 emulation (default: 0)\n");
    printf(" BOX86_DYNAREC_FASTNAN 0/1 to control if NaN are x86 accurate or not (default: 1)\n");
    printf(" BOX86_DYNAREC_SAFEFLAGS 0/1/2 to control flags generation on RET/CALL opcodes (default: 1)\n");
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
    printf(" BOX86_TRACE_FILE with FileName to redirect logs in a file (or stderr to use stderr instead of stdout)\n");
    printf(" BOX86_DLSYM_ERROR with 1 to log dlsym errors\n");
    printf(" BOX86_LOAD_ADDR=0xXXXXXX try to load at 0xXXXXXX main binary (if binary is a PIE)\n");
    printf(" BOX86_NOSIGSEGV=1 to disable handling of SigSEGV\n");
    printf(" BOX86_NOSIGILL=1  to disable handling of SigILL\n");
    printf(" BOX86_SHOWSEGV=1 to show Segfault signal even if a signal handler is present\n");
#ifdef PANDORA
    printf(" BOX86_X11COLOR16=1 to try convert X11 color from 32 bits to 16 bits (to avoid light green on light cyan windows\n");
#endif
    printf(" BOX86_X11THREADS=1 to call XInitThreads when loading X11 (for old Loki games with Loki_Compat lib)\n");
    printf(" BOX86_LIBGL=libXXXX set the name (and optionnaly full path) for libGL.so.1\n");
    printf(" BOX86_LD_PRELOAD=XXXX[:YYYYY] force loading XXXX (and YYYY...) libraries with the binary\n");
    printf(" BOX86_ALLOWMISSINGLIBS with 1 to allow to continue even if a lib is missing (unadvised, will probably crash later)\n");
    printf(" BOX86_PREFER_EMULATED=1 to prefer emulated libs first (execpt for glibc, alsa, pulse, GL, vulkan and X11)\n");
    printf(" BOX86_PREFER_WRAPPED if box86 will use wrapped libs even if the lib is specified with absolute path\n");
    printf(" BOX86_NOPULSE=1 to disable the loading of pulseaudio libs\n");
    printf(" BOX86_NOGTK=1 to disable the loading of wrapped gtk libs\n");
    printf(" BOX86_NOVULKAN=1 to disable the loading of wrapped vulkan libs\n");
    printf(" BOX86_ENV='XXX=yyyy' will add XXX=yyyy env. var.\n");
    printf(" BOX86_ENV1='XXX=yyyy' will add XXX=yyyy env. var. and continue with BOX86_ENV2 ... until var doesn't exist\n");
    printf(" BOX86_JITGDB with 1 to launch \"gdb\" when a segfault is trapped, attached to the offending process\n");
}

void PrintHelp() {
    printf("This is Box86, the Linux x86 emulator with a twist\n");
    printf("\nUsage is 'box86 [options] path/to/software [args]' to launch x86 software.\n");
    printf(" options are:\n");
    printf("    '-v'|'--version' to print box86 version and quit\n");
    printf("    '-h'|'--help' to print this and quit\n");
    printf("    '-f'|'--flags' to print box86 flags and quit\n");
}

void addNewEnvVar(const char* s)
{
    if(!s)
        return;
    char* p = box_strdup(s);
    char* e = strchr(p, '=');
    if(!e) {
        printf_log(LOG_INFO, "Invalid specific env. var. '%s'\n", s);
        box_free(p);
        return;
    }
    *e='\0';
    ++e;
    setenv(p, e, 1);
    box_free(p);
}

EXPORTDYN
void LoadEnvVars(box86context_t *context)
{
    char *p;
    // Check custom env. var. and add them if needed
    p = getenv("BOX86_ENV");
    if(p)
        addNewEnvVar(p);
    int i = 1;
    char box86_env[50];
    do {
        sprintf(box86_env, "BOX86_ENV%d", i);
        p = getenv(box86_env);
        if(p) {
            addNewEnvVar(p);
            ++i;
        }
    } while(p);
    // check BOX86_LD_LIBRARY_PATH and load it
    LoadEnvPath(&context->box86_ld_lib, ".:lib:lib32:x86:i686", "BOX86_LD_LIBRARY_PATH");
#ifdef PANDORA
    if(FileExist("/mnt/utmp/codeblocks/usr/lib/i386-linux-gnu", 0))
        AddPath("/mnt/utmp/codeblocks/usr/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/mnt/utmp/box86/lib/i386-linux-gnu", 0))
        AddPath("/mnt/utmp/box86/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/mnt/utmp/codeblocks/usr/lib/box86-i386-linux-gnu", 0))
        AddPath("/mnt/utmp/codeblocks/usr/lib/box86-i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/mnt/utmp/box86/lib/box86-i386-linux-gnu", 0))
        AddPath("/mnt/utmp/box86/lib/box86-i386-linux-gnu", &context->box86_ld_lib, 1);
    //TODO: add relative path to box86 location
#endif
#ifndef TERMUX
    if(FileExist("/lib/box86", 0))
        AddPath("/lib/box86", &context->box86_ld_lib, 1);
    if(FileExist("/usr/lib/box86", 0))
        AddPath("/usr/lib/box86", &context->box86_ld_lib, 1);
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
    if(FileExist("/lib/box86-i386-linux-gnu", 0))
        AddPath("/lib/box86-i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/usr/lib/box86-i386-linux-gnu", 0))
        AddPath("/usr/lib/box86-i386-linux-gnu", &context->box86_ld_lib, 1);
#else
    if(FileExist("/data/data/com.termux/files/usr/lib/i386-linux-gnu", 0))
        AddPath("/data/data/com.termux/files/usr/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/data/data/com.termux/files/usr/lib/i686-linux-gnu", 0))
        AddPath("/data/data/com.termux/files/usr/lib/i686-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/data/data/com.termux/files/usr/lib/box86-i386-linux-gnu", 0))
        AddPath("/data/data/com.termux/files/usr/lib/box86-i386-linux-gnu", &context->box86_ld_lib, 1);
#endif
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
    // add libssl and libcrypto, prefer emulated version because of multiple version exist
    AddPath("libssl.so.1", &context->box86_emulated_libs, 0);
    AddPath("libssl.so.1.0.0", &context->box86_emulated_libs, 0);
    AddPath("libcrypto.so.1", &context->box86_emulated_libs, 0);
    AddPath("libcrypto.so.1.0.0", &context->box86_emulated_libs, 0);
    // check BOX86_PATH and load it
    LoadEnvPath(&context->box86_path, ".:bin", "BOX86_PATH");
    if(getenv("PATH"))
        AppendList(&context->box86_path, getenv("PATH"), 1);   // in case some of the path are for x86 world

#define READENV(v, name, dest, eqn) \
    do {                         \
        p = getenv(name);        \
        if (p) {                 \
            if (!strcmp(p, v)) { \
                dest = 1;        \
                eqn;             \
            }                    \
        }                        \
    } while (0)
#define READENV0(name, dest, eqn) READENV("0", name, dest, eqn)
#define READENV1(name, dest, log) READENV("1", name, dest, printf_log(LOG_INFO, log))
    READENV1("BOX86_SSE_FLUSHTO0", box86_sse_flushto0, "BOX86: Direct apply of SSE Flush to 0 flag\n");
    READENV1("BOX86_X87_NO80BITS", box86_x87_no80bits, "BOX86: all 80bits x87 long double will be handle as double\n");
    READENV1("BOX86_PREFER_WRAPPED", box86_prefer_wrapped, "BOX86: Prefer Wrapped libs\n");
    READENV1("BOX86_PREFER_EMULATED", box86_prefer_emulated, "BOX86: Prefer Emulated libs\n");
    READENV1("BOX86_NOSIGSEGV", context->no_sigsegv, "BOX86: Disabling handling of SigSEGV\n");
    READENV1("BOX86_NOSIGILL", context->no_sigill, "BOX86: Disabling handling of SigILL\n");
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE");
    if(p && strcmp(p, "0")) {
        context->x86trace = 1;
        box86_trace = p;
    };
        
    READENV0("BOX86_TRACE_INIT", context->x86trace, trace_init = p);
    if(my_context->x86trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if(InitX86Trace(my_context)) {
            printf_log(LOG_INFO, "Zydis init failed, no x86 trace activated\n");
            context->x86trace = 0;
        }
    }
#endif
#undef READENV
#undef READENV0
#undef READENV1
#ifdef BUILD_LIB
    context->argc = 1;  // need 1
    context->argv = (char**)box_malloc(sizeof(char*));
    context->argv[0] = box_strdup("dummy");
#endif
}

EXPORTDYN
void setupTraceInit()
{
#ifdef HAVE_TRACE
    char* p = trace_init;
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t s_trace_start=0, s_trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%zd-%zd", &s_trace_start, &s_trace_end)!=2) {
                if(sscanf(p, "0x%zX-0x%zX", &s_trace_start, &s_trace_end)!=2)
                    sscanf(p, "%zx-%zx", &s_trace_start, &s_trace_end);
            }
            if(s_trace_start || s_trace_end)
                SetTraceEmu(s_trace_start, s_trace_end);
        } else {
            if (my_context->elfs && GetGlobalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL, NULL, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else if(my_context->elfs && GetLocalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL, NULL, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to trace (\"%s\") not found, disabling trace\n", p);
                SetTraceEmu(0, 100);  // disabling trace, mostly
            }
        }
    } else {
        p = box86_trace;
        if(p)
            if (strcmp(p, "0"))
                SetTraceEmu(0, 1);
    }
#endif
}

EXPORTDYN
void setupTrace()
{
#ifdef HAVE_TRACE
    char* p = box86_trace;
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t s_trace_start=0, s_trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%zd-%zd", &s_trace_start, &s_trace_end)!=2) {
                if(sscanf(p, "0x%zX-0x%zX", &s_trace_start, &s_trace_end)!=2)
                    sscanf(p, "%zx-%zx", &s_trace_start, &s_trace_end);
            }
            if(s_trace_start || s_trace_end) {
                SetTraceEmu(s_trace_start, s_trace_end);
                if(!s_trace_start && s_trace_end==1) {
                    printf_log(LOG_INFO, "TRACE enabled but inactive\n");
                } else {
                    printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
                }
            }
        } else {
            if (GetGlobalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL, NULL, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else if(GetLocalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL, NULL, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to trace (\"%s\") not found, trying to set trace later\n", p);
                SetTraceEmu(0, 1);  // disabling trace, mostly
                if(trace_func)
                    box_free(trace_func);
                trace_func = box_strdup(p);
            }
        }
    }
#endif
}
void endMallocHook();

void endBox86()
{
    if(!my_context || box86_quit)
        return;

    box86_quit = 1;
    endMallocHook();
    x86emu_t* emu = thread_get_emu();
    //atexit first
    printf_log(LOG_DEBUG, "Calling atexit registered functions\n");
    CallAllCleanup(emu);
    printf_log(LOG_DEBUG, "Calling fini for all loaded elfs and unload native libs\n");
    RunElfFini(my_context->elfs[0], emu);
    FreeLibrarian(&my_context->local_maplib, emu);    // unload all libs
    FreeLibrarian(&my_context->maplib, emu);    // unload all libs
    void closeAllDLOpenned();
    closeAllDLOpenned();    // close residual dlopenned libs
    #if 0
    // waiting for all thread except this one to finish
    int this_thread = GetTID();
    // int pid = getpid();
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
                        //syscall(__NR_tgkill, pid, tid, SIGABRT);
                        running = 0;
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
    #endif
    // all done, free context
    FreeBox86Context(&my_context);
    #ifdef DYNAREC
    // disable dynarec now
    box86_dynarec = 0;
    #endif
    if(box86_libGL) {
        box_free(box86_libGL);
        box86_libGL = NULL;
    }
}

static void add_argv(const char* what) {
    int there = 0;
    for(int i=1; i<my_context->argc && !there; ++i)
        if(!strcmp(my_context->argv[i], what))
            there = 1;
    if(!there) {
        my_context->argv = (char**)box_realloc(my_context->argv, (my_context->argc+1)*sizeof(char*));
        my_context->argv[my_context->argc] = box_strdup(what);
        my_context->argc++;
    }
}

static void load_rcfiles()
{
 char* rcpath = getenv("BOX86_RCFILE");

    if(rcpath && FileExist(rcpath, IS_FILE))
	LoadRCFile(rcpath);
        
    #ifndef TERMUX
    else if(FileExist("/etc/box86.box86rc", IS_FILE))
        LoadRCFile("/etc/box86.box86rc");
    #else
    else if(FileExist("/data/data/com.termux/files/usr/etc/box86.box86rc", IS_FILE))
        LoadRCFile("/data/data/com.termux/files/usr/etc/box86.box86rc");
    #endif
    #ifdef PANDORA
    else if(FileExist("/mnt/utmp/codeblocks/usr/etc/box86.box86rc", IS_FILE))
        LoadRCFile("/mnt/utmp/codeblocks/usr/etc/box86.box86rc");
    #endif
    // else
    //     LoadRCFile(NULL);   // load default rcfile
    char* p = getenv("HOME");
    if(p) {
        char tmp[4096];
        strncpy(tmp, p, 4096);
        strncat(tmp, "/.box86rc", 4095);
        if(FileExist(tmp, IS_FILE))
            LoadRCFile(tmp);
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
        box_free(my_context->argv[i]);
}

extern char **environ;

int main(int argc, const char **argv, char **env)
{
    init_malloc_hook();
    init_auxval(argc, argv, environ?environ:env);
    // trying to open and load 1st arg
    if(argc==1) {
        /*PrintBox86Version();
        PrintHelp();
        return 1;*/
        printf("BOX86: Missing operand after 'box86'\n");
        printf("See 'box86 --help' for more information.\n");
        exit(0);
    }
    if(argc>1 && !strcmp(argv[1], "/usr/bin/gdb"))
        exit(0);
    // uname -m is redirected to box86 -m
    if(argc==2 && (!strcmp(argv[1], "-m") || !strcmp(argv[1], "-p") || !strcmp(argv[1], "-i")))
    {
        // try box64 first
        execlp("box64", "box64", argv[1], (char*)NULL);
        // fallthru if box64 is not installed
        printf("i686\n");
        exit(0);
    }
    // init random seed
    srandom(time(NULL));

    // check BOX86_LOG debug level
    LoadLogEnv();
    if(!getenv("BOX86_NORCFILES")) {
        load_rcfiles();
    }
    char* bashpath = NULL;
    {
        char* p = getenv("BOX86_BASH");
        if(p) {
            if(FileIsX86ELF(p) || FileIsX64ELF(p)) {
                bashpath = p;
            } else {
                printf_log(LOG_INFO, "the x86 bash \"%s\" is not an x86 binary\n", p);
            }
        }
        if(!bashpath && (p=getenv("BOX64_BASH"))) {
            if(FileIsX86ELF(p) || FileIsX64ELF(p)) {
                bashpath = p;
            } else {
                printf_log(LOG_INFO, "the x86_64 bash \"%s\" is not an x86_64 binary\n", p);
            }
        }
    }
    if(bashpath)
        printf_log(LOG_INFO, "Using bash \"%s\"\n", bashpath);

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
        if(!strcmp(prog, "-f") || !strcmp(prog, "--flags")) {
            PrintFlags();
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
    if(strstr(prog, "wine-preloader")==(prog+strlen(prog)-strlen("wine-preloader")) 
     || strstr(prog, "wine64-preloader")==(prog+strlen(prog)-strlen("wine64-preloader"))) {
        // wine-preloader detecter, skipping it if next arg exist and is an x86 binary
        int x86 = (nextarg<argc)?FileIsX86ELF(argv[nextarg]):0;
        if(x86) {
            prog = argv[++nextarg];
            printf_log(LOG_INFO, "BOX86: Wine preloader detected, loading \"%s\" directly\n", prog);
            //wine_preloaded = 1;
        }
        box86_wine = 1;
    }
    int wine_steam = 0;
    // check if this is wine
    if(!strcmp(prog, "wine") || (strlen(prog)>5 && !strcmp(prog+strlen(prog)-strlen("/wine"), "/wine"))) {
        const char* prereserve = getenv("WINEPRELOADRESERVE");
        printf_log(LOG_INFO, "BOX86: Wine detected, WINEPRELOADRESERVE=\"%s\"\n", prereserve?prereserve:"");
        if(wine_preloaded)
            wine_prereserve(prereserve);
        // special case for winedbg, doesn't work anyway
        if(argv[nextarg+1] && strstr(argv[nextarg+1], "winedbg")==argv[nextarg+1]) {
            if(getenv("BOX86_WINEDBG")) {
                box86_nobanner = 1;
                box86_log = 0;
            } else {
                printf_log(LOG_NONE, "winedbg detected, not launching it!\n");
                exit(0);    // exiting, it doesn't work anyway
            }
        }
        box86_wine = 1;
        // if program being called is wine_steam (rudimentary check...) and if no other argument are there
        if(argv[nextarg+1] && argv[nextarg+1][0]!='-' /*&& argc==(nextarg+2)*/) {
            if(!strcasecmp(argv[nextarg+1], "steam.exe"))
                wine_steam = 1;
            else if(!strcasecmp(argv[nextarg+1], "steam"))
                wine_steam = 1;
            if(!wine_steam) {
                const char* pp = strrchr(argv[nextarg+1], '/');
                if(pp && !strcasecmp(pp+1, "steam.exe"))
                    wine_steam = 1;
                else {
                    pp = strrchr(argv[nextarg+1], '\\');
                    if(pp && !strcasecmp(pp+1, "steam.exe"))
                        wine_steam = 1;
                }
            }
        }
    }
    // check if this is wineserver
    if(!strcmp(prog, "wineserver") || (strlen(prog)>9 && !strcmp(prog+strlen(prog)-strlen("/wineserver"), "/wineserver"))) {
        box86_wine = 1;
    }
    // Create a new context
    my_context = NewBox86Context(argc - nextarg);

    // check BOX86_LD_LIBRARY_PATH and load it
    LoadEnvVars(my_context);

    my_context->box86path = ResolveFile(argv[0], &my_context->box86_path);
    // check if box64 is present
    {
        my_context->box64path = getenv("BOX86_BOX64");
        if(!my_context->box64path) {
            my_context->box64path = box_strdup(my_context->box86path);
            if (strstr(my_context->box64path, "box86")) {
                char* p = strrchr(my_context->box64path, '8');  // get the 8 of box86
                p[0] = '6'; p[1] = '4'; // change 86 to 64
            }
        }
        if(!FileExist(my_context->box64path, IS_FILE)) {
            box_free(my_context->box64path);
            my_context->box64path = ResolveFile("box64", &my_context->box86_path);
        }
        if(!FileExist(my_context->box64path, IS_FILE)) {
            box_free(my_context->box64path);
            my_context->box64path = NULL;
            if(bashpath && FileIsX64ELF(bashpath)) {
                printf_log(LOG_INFO, "The bash binary given is an x86_64 version, but box64 is not found\n");
                bashpath = NULL;
            }
        }
    }

    // prepare all other env. var
    my_context->envc = CountEnv(environ?environ:env);
    printf_log(LOG_INFO, "Counted %d Env var\n", my_context->envc);
    // allocate extra space for the new environment variable _ and the null terminator
    my_context->envv = (char**)box_calloc(my_context->envc+2, sizeof(char*));
    GatherEnv(&my_context->envv, environ?environ:env, my_context->box86path);
    if(box86_dump) {
        for (int i=0; i<my_context->envc; ++i)
            printf_dump(LOG_NEVER, " Env[%02d]: %s\n", i, my_context->envv[i]);
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
            if(strstr(p, "libtcmalloc_minimal.so.0"))
                box86_tcmalloc_minimal = 1;
            if(strstr(p, "libtcmalloc_minimal.so"))
                box86_tcmalloc_minimal = 1;
            if(strstr(p, "libtcmalloc_minimal_debug.so.4"))
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
        my_context->argv[0] = box_strdup(prog);
    else
        my_context->argv[0] = ResolveFile(prog, &my_context->box86_path);

    const char* prgname = strrchr(prog, '/');
    if(!prgname)
        prgname = prog;
    else
        ++prgname;
    if(box86_wine) {
        #ifdef ANDROID
            AddPath("libdl.so", &ld_preload, 0);
        #else
            AddPath("libdl.so.2", &ld_preload, 0);
        #endif
    }
    // special case for steam that somehow seems to alter libudev opaque pointer (udev_monitor)
    if(strstr(prgname, "steam")==prgname) {
        printf_log(LOG_INFO, "steam detected\n");
        box86_steam = 1;
        unsetenv("BOX86_NOGTK");
    }
    // special case for steam-runtime-check-requirements to fake 64bits suport
    if(strstr(prgname, "steam-runtime-check-requirements")==prgname) {
        printf_log(LOG_INFO, "steam-runtime-check-requirements detected, faking All is good!\n");
        exit(0);    // exiting, not testing anything
    }
    // special case for UnrealLinux.bin, it doesn't like "full path resolution"
    if(!strcmp(prog, "UnrealLinux.bin") && my_context->argv[0]) {
        box_free(my_context->argv[0]);
        my_context->argv[0] = box_strdup("./UnrealLinux.bin");
    }
    #if defined(RPI) || defined(RK3399) || defined(RK3288) || defined(GOA_CLONE) || defined(PYRA) || defined(PANDORA)
    // special case for TokiTori 2+, that check if texture max size is > = 8192
    if(strstr(prgname, "TokiTori2.bin.x86")==prgname) {
        printf_log(LOG_INFO, "TokiTori 2+ detected, runtime patch to fix GPU non-power-of-two faillure\n");
        box86_tokitori2 = 1;
    }
    #endif
    // special case for SimCity3k Demo, that have a bug were it do feof after fclose on the same fd
    if(strstr(prgname, "sc3u_demo.x86")==prgname) {
        printf_log(LOG_INFO, "SimCity3000 Demo detected, runtime patch to fix buf feof after fclose\n");
        box86_sc3u = 1;
    }
    if(strstr(prgname, "sc3u")==prgname) {
        printf_log(LOG_INFO, "SimCity3000 detected, runtime patch to fix buf feof after fclose\n");
        box86_sc3u = 1;
    }
    // special case for zoom
    if(strstr(prgname, "zoom")==prgname) {
        printf_log(LOG_INFO, "Zoom detected, trying to use system libturbojpeg if possible\n");
        box86_zoom = 1;
    }
    // special case for bash (add BOX86_NOBANNER=1 if not there)
    if(!strcmp(prgname, "bash")) {
        printf_log(LOG_INFO, "bash detected, disabling banner\n");
        if (!box86_nobanner) {
            setenv("BOX86_NOBANNER", "1", 0);
            setenv("BOX64_NOBANNER", "1", 0);
        }
        if (!bashpath) {
            bashpath = (char*)prog;
            setenv("BOX86_BASH", prog, 1);
        }
    }
    if(bashpath)
        my_context->bashpath = box_strdup(bashpath);

    /*if(strstr(prgname, "awesomium_process")==prgname) {
        printf_log(LOG_INFO, "awesomium_process detected, forcing emulated libpng12\n");
        AddPath("libpng12.so.0", &my_context->box86_emulated_libs, 0);
    }*/
    /*if(!strcmp(prgname, "gdb")) {
        exit(-1);
    }*/
    ApplyParams("*", &ld_preload);   // [*] is a special setting for all process
    ApplyParams(prgname, &ld_preload);

    for(int i=1; i<my_context->argc; ++i) {
        my_context->argv[i] = box_strdup(argv[i+nextarg]);
        printf_log(LOG_INFO, "argv[%i]=\"%s\"\n", i, my_context->argv[i]);
    }
    if(box86_wine) {
        box86_maxcpu_immutable = 1; // cannot change once wine is loaded
    }
    if(box86_nosandbox)
    {
        add_argv("--no-sandbox");
    }
    if(wine_steam /*|| box86_steam*/) {
        printf_log(LOG_INFO, "Steam%s detected, adding -cef-single-process -cef-in-process-gpu -cef-disable-sandbox -no-cef-sandbox -cef-disable-breakpad to parameters", wine_steam?".exe":"");
        add_argv("-cef-single-process");
        add_argv("-cef-in-process-gpu");
        add_argv("-cef-disable-sandbox");
        add_argv("-no-cef-sandbox");
        add_argv("-cef-disable-breakpad");
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
    if(!(my_context->fullpath = box_realpath(my_context->argv[0], NULL)))
        my_context->fullpath = box_strdup(my_context->argv[0]);
    if(getenv("BOX86_ARG0"))
        my_context->argv[0] = box_strdup(getenv("BOX86_ARG0"));
    FILE *f = fopen64(my_context->fullpath, "rb");
    if(!f) {
        printf_log(LOG_NONE, "Error: Cannot open %s\n", my_context->fullpath);
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    elfheader_t *elf_header = LoadAndCheckElfHeader(f, my_context->fullpath, 1);
    if(!elf_header) {
        int x64 = my_context->box64path?FileIsX64ELF(my_context->fullpath):0;
        int script = my_context->bashpath?FileIsShell(my_context->fullpath):0;
        printf_log(LOG_NONE, "Error: reading elf header of %s, trying to launch %s instead\n", my_context->fullpath, x64?"using box64":(script?"using bash":"natively"));
        fclose(f);
        FreeCollection(&ld_preload);
        int ret;
        if(x64) {
           // duplicate the array and insert 1st arg as box86
            const char** newargv = (const char**)box_calloc(my_context->argc+2, sizeof(char*));
            newargv[0] = my_context->box64path;
            for(int i=0; i<my_context->argc; ++i)
                newargv[i+1] = my_context->argv[i];
            FreeBox86Context(&my_context);
            ret = execvp(newargv[0], (char * const*)newargv);
        } else if (script) {
            // duplicate the array and insert 1st arg as box64, 2nd is bash
            const char** newargv = (const char**)box_calloc(my_context->argc+3, sizeof(char*));
            newargv[0] = my_context->box86path;
            newargv[1] = my_context->bashpath;
            for(int i=0; i<my_context->argc; ++i)
                newargv[i+2] = my_context->argv[i];
            ret = execvp(newargv[0], (char * const*)newargv);
        } else {
            const char** newargv = (const char**)box_calloc(my_context->argc+1, sizeof(char*));
            for(int i=0; i<my_context->argc; ++i)
                newargv[i] = my_context->argv[i];
            ret = execvp(newargv[0], (char * const*)newargv);
        }
        free_contextargv();
        FreeBox86Context(&my_context);
        return ret;
    }
    AddElfHeader(my_context, elf_header);

    if(CalcLoadAddr(elf_header)) {
        printf_log(LOG_NONE, "Error: reading elf header of %s\n", my_context->fullpath);
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        FreeElfHeader(&elf_header);
        return -1;
    }
    // allocate memory
    if(AllocLoadElfMemory(my_context, elf_header, 1)) {
        printf_log(LOG_NONE, "Error: loading elf %s\n", my_context->fullpath);
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        FreeElfHeader(&elf_header);
        return -1;
    }
    my_context->brk = ElfGetBrk(elf_header);
    if(ElfCheckIfUseTCMallocMinimal(elf_header)) {
        if(!box86_tcmalloc_minimal) {
            // need to reload with tcmalloc_minimal as a LD_PRELOAD!
            printf_log(LOG_INFO, "BOX86: tcmalloc_minimal.so.4 used, reloading box86 with the lib preladed\n");
            // need to get a new envv variable. so first count it and check if LD_PRELOAD is there
            int preload=(getenv("LD_PRELOAD"))?1:0;
            int nenv = 0;
            while(env[nenv]) nenv++;
            // alloc + "LD_PRELOAD" if needd + last NULL ending
            char** newenv = (char**)box_calloc(nenv+1+((preload)?0:1), sizeof(char*));
            // copy strings
            for (int i=0; i<nenv; ++i)
                newenv[i] = box_strdup(env[i]);
            // add ld_preload
            if(preload) {
                // find the line
                int l = 0;
                while(l<nenv) {
                    if(strstr(newenv[l], "LD_PRELOAD=")==newenv[l]) {
                        // found it!
                        char *old = newenv[l];
                        newenv[l] = (char*)box_calloc(strlen(old)+strlen("libtcmalloc_minimal.so.4:")+1, sizeof(char));
                        strcpy(newenv[l], "LD_PRELOAD=libtcmalloc_minimal.so.4:");
                        strcat(newenv[l], old + strlen("LD_PRELOAD="));
                        box_free(old);
                        // done, end loop
                        l = nenv;
                    } else ++l;
                }
            } else {
                //move last one
                newenv[nenv] = box_strdup(newenv[nenv-1]);
                box_free(newenv[nenv-1]);
                newenv[nenv-1] = box_strdup("LD_PRELOAD=libtcmalloc_minimal.so.4");
            }
            // duplicate argv too
            char** newargv = box_calloc(argc+1, sizeof(char*));
            int narg = 0;
            while(argv[narg]) {newargv[narg] = box_strdup(argv[narg]); narg++;}
            // launch with new env...
            if(execve(newargv[0], newargv, newenv)<0)
                printf_log(LOG_NONE, "Failed to relaunch, error is %d/%s\n", errno, strerror(errno));
        } else {
            printf_log(LOG_INFO, "BOX86: Using tcmalloc_minimal.so.4, and it's in the LD_PRELOAD command\n");
        }
    }
#if defined(RPI) || defined(RK3399) || defined(GOA_CLONE) || defined(PYRA)
    // before launching emulation, let's check if this is a mojosetup from GOG
    if (((strstr(prog, "bin/linux/x86/mojosetup") && getenv("MOJOSETUP_BASE")) || strstr(prog, ".mojosetup/mojosetup"))
       && getenv("GTK2_RC_FILES")) {
        sanitize_mojosetup_gtk_background();
    }
#endif
    // change process name
    {
        char* p = strrchr(my_context->fullpath, '/');
        if(p)
            ++p;
        else
            p = my_context->fullpath;
        if(prctl(PR_SET_NAME, p)==-1)
            printf_log(LOG_NONE, "Error setting process name (%s)\n", strerror(errno));
        else
            printf_log(LOG_INFO, "Rename process to \"%s\"\n", p);
        // and now all change the argv (so libs libs mesa find the correct program names)
        char* endp = (char*)argv[argc-1];
        while(*endp)
            ++endp;    // find last argv[] address
        uintptr_t diff = prog - argv[0]; // this is the difference we need to compensate
        for(p=(char*)prog; p<=endp; ++p)
            *(p - diff) = *p;  // copy all element at argv[nextarg] to argv[0]
        memset(endp - diff, 0, diff); // fill reminder with NULL
        for(int i=nextarg; i<argc; ++i)
            argv[i] -= diff;    // adjust strings
        my_context->orig_argc = argc;
        my_context->orig_argv = (char**)argv;
    }
    box86_isglibc234 = GetNeededVersionForLib(elf_header, "libc.so.6", "GLIBC_2.34");
    if(box86_isglibc234)
        printf_log(LOG_DEBUG, "Program linked with GLIBC 2.34+\n");
    // get and alloc stack size and align
    if(CalcStackSize(my_context)) {
        printf_log(LOG_NONE, "Error: allocating stack\n");
        free_contextargv();
        FreeBox86Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    #if defined(RPI) || defined(RK3399) || defined(RK3288) || defined(GOA_CLONE) || defined(PYRA) || defined(PANDORA)
    if(box86_tokitori2) {
        uint32_t *patch = (uint32_t*)0x85897f4;
        if(*patch==0x2000) {
            *patch = 0x1000;
            printf_log(LOG_NONE, "Runtime patching the game\n");
        } else
            printf_log(LOG_NONE, "Cannot patch the game\n");
    }
    #endif
    if(box86_sc3u) {
        uint32_t *patch1 = (uint32_t*)0x8400684;
        uint32_t *patch2 = (uint32_t*)0x8406644;
        if(*patch1==0x57f4c483) {
            *patch1=0x57f407eb;
            printf_log(LOG_NONE, "Runtime patching the game\n");
        } else if(*patch2==0x57f4c483) {
            *patch2=0x57f407eb;
            printf_log(LOG_NONE, "Runtime patching the game\n");
        } else

            printf_log(LOG_NONE, "Cannot patch the game\n");
    }
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
    AddSymbols(my_context->maplib, GetMapSymbols(elf_header), GetWeakSymbols(elf_header), GetLocalSymbols(elf_header), elf_header);
    if(wine_preloaded) {
        uintptr_t wineinfo = FindSymbol(GetMapSymbols(elf_header), "wine_main_preload_info", -1, NULL, 1, NULL);
        if(!wineinfo) wineinfo = FindSymbol(GetWeakSymbols(elf_header), "wine_main_preload_info", -1, NULL, 1, NULL);
        if(!wineinfo) wineinfo = FindSymbol(GetLocalSymbols(elf_header), "wine_main_preload_info", -1, NULL, 1, NULL);
        if(!wineinfo) {printf_log(LOG_NONE, "Warning, Symbol wine_main_preload_info not found\n");}
        else {
            *(void**)wineinfo = get_wine_prereserve();
            printf_log(LOG_DEBUG, "WINE wine_main_preload_info found and updated\n");
        }
        #ifdef DYNAREC
        dynarec_wine_prereserve();
        #endif
    }
    AddMainElfToLinkmap(elf_header);
    // pre-load lib if needed
    if(ld_preload.size) {
        my_context->preload = new_neededlib(0);
        for(int i=0; i<ld_preload.size; ++i) {
            needed_libs_t* tmp = new_neededlib(1);
            tmp->names[0] = ld_preload.paths[i];
            if(AddNeededLib(my_context->maplib, 0, 0, tmp, elf_header, my_context, emu)) {
                printf_log(LOG_INFO, "Warning, cannot pre-load %s\n", tmp->names[0]);
                RemoveNeededLib(my_context->maplib, 0, tmp, my_context, emu);
            } else {
                for(int j=0; j<tmp->size; ++j)
                    add1lib_neededlib(my_context->preload, tmp->libs[j], tmp->names[j]);
                free_neededlib(tmp);
            }
        }
    }
    FreeCollection(&ld_preload);
    // Call librarian to load all dependant elf
    if(LoadNeededLibs(elf_header, my_context->maplib, 0, 0, my_context, emu)) {
        printf_log(LOG_NONE, "Error: loading needed libs in elf %s\n", my_context->argv[0]);
        FreeBox86Context(&my_context);
        return -1;
    }
    // reloc...
    printf_log(LOG_DEBUG, "And now export symbols / relocation for %s...\n", ElfName(elf_header));
    if(RelocateElf(my_context->maplib, NULL, 0, elf_header)) {
        printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", my_context->fullpath);
        FreeBox86Context(&my_context);
        return -1;
    }
    // and handle PLT
    RelocateElfPlt(my_context->maplib, NULL, 0, elf_header);
    // defered init
    setupTraceInit();
    RunDeferredElfInit(emu);
    // update TLS of main elf
    RefreshElfTLS(elf_header);
    // do some special case check, _IO_2_1_stderr_ and friends, that are setup by libc, but it's already done here, so need to do a copy
    ResetSpecialCaseMainElf(elf_header);
    // init...
    setupTrace();
    // get entrypoint
    my_context->ep = GetEntryPoint(my_context->maplib, elf_header);

    atexit(endBox86);
    loadProtectionFromMap();
    
    // emulate!
    printf_log(LOG_DEBUG, "Start x86emu on Main\n");
    // Stack is ready, with stacked: NULL env NULL argv argc
    SetEIP(emu, my_context->ep);
    ResetFlags(emu);
    Push32(emu, my_context->exit_bridge);  // push to pop it just after
    SetEDX(emu, Pop32(emu));    // EDX is exit function
    DynaRun(emu);
    // Get EAX
    int ret = GetEAX(emu);
    printf_log(LOG_DEBUG, "Emulation finished, EAX=%d\n", ret);
    endBox86();

    if(trace_func)  {
        box_free(trace_func);
        trace_func = NULL;
    }

    return ret;
}
#endif  //BUILD_LIB
