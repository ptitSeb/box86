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

#include "box86version.h"
#include "debug.h"
#include "box86context.h"
#include "fileutils.h"
#include "elfloader.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86trace.h"
#include "librarian.h"

int box86_log = LOG_INFO;//LOG_NONE;
int dlsym_error = 0;
int trace_xmm = 0;
#ifdef HAVE_TRACE
uint64_t start_cnt = 0;
#endif
#ifdef PANDORA
int x11color16 = 0;
#endif

FILE* ftrace = NULL;

void LoadLogEnv()
{
    const char *p = getenv("BOX86_LOG");
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
        printf_log(LOG_INFO, "Debug level is %d\n", box86_log);
    }
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE_XMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                trace_xmm = p[0]-'0';
        }
    }
    p = getenv("BOX86_TRACE_START");
    if(p) {
        char* p2;
        start_cnt = strtoll(p, &p2, 10);
        printf_log(LOG_INFO, "Will start trace only after %llu instructions\n", start_cnt);
    }
#endif
    p = getenv("BOX86_TRACE_FILE");
    if(p) {
        ftrace = fopen64(p, "w");
        if(!ftrace) {
            ftrace = stdout;
            printf_log(LOG_INFO, "Cannot open trace file \"%s\" for writing (error=%s)\n", p, strerror(errno));
        } else {
            printf("BOX86 Trace redirected to \"%s\"\n", p);
        }
    }
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
}

void LoadEnvPath(path_collection_t *col, const char* defpath, const char* env)
{
    const char* p = getenv(env);
    if(p) {
        printf_log(LOG_INFO, "%s: ", env);
        ParseList(p, col);
    } else {
        printf_log(LOG_INFO, "Using default %s: ", env);
        ParseList(defpath, col);
    }
    if(LOG_INFO<=box86_log) {
        for(int i=0; i<col->size; i++)
            printf_log(LOG_INFO, "%s%s", col->paths[i], (i==col->size-1)?"\n":":");
    }
}

int CountEnv(const char** env)
{
    // count, but remove all BOX86_* environnement
    // also remove PATH and LD_LIBRARY_PATH
    // but add 2 for default BOX86_PATH and BOX86_LD_LIBRARY_PATH
    const char** p = env;
    int c = 0;
    while(*p) {
        if(strncmp(*p, "BOX86_", 6)!=0)
            if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0))
                ++c;
        ++p;
    }
    return c+2;
}
int GatherEnv(char*** dest, const char** env, const char* prog)
{
    // Add all but BOX86_* environnement
    // and PATH and LD_LIBRARY_PATH
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
            int l = strlen(prog);
            char tmp[l+3];
            strcpy(tmp, "_=");
            strcat(tmp, prog);
            (*dest)[idx++] = strdup(tmp);
        } else if(strncmp(*p, "BOX86_", 6)!=0) {
            if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0)) {
                (*dest)[idx++] = strdup(*p);
            }
        }
        ++p;
    }
    if(!path) {
        (*dest)[idx++] = strdup("PATH=.:bin");
    }
    if(!ld_path) {
        (*dest)[idx++] = strdup("LD_LIBRARY_PATH=.:lib");
    }
    return 0;
}


void PrintHelp() {
    printf("\n\nThis is Box86, the Linux 86 emulator with a twist\n");
    printf("\nUsage is box86 path/to/software [args]\n");
    printf("to launch x86 software\n");
    printf("You can also set some environment variables:\n");
    printf(" BOX86_PATH to set the path used by box86 to find x86 programs (default is '.:bin')\n");
    printf(" BOX86_LD_LIBRARY_PATH to set the path where box86 will search for x86 libraries (default is '.:lib')\n");
    printf(" BOX86_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed debug info\n");
#ifdef HAVE_TRACE
    printf(" BOX86_TRACE with 1 to enable x86 execution trace\n");
    printf("    or with XXXXXX-YYYYYY to enable x86 execution trace only between address\n");
    printf("    or with FunctionName to enable x86 execution trace only in one specific function\n");
    printf("  use BOX86_TRACE_INIT instead of BOX_TRACE to start trace before init of Libs and main program\n\t (function name will probably not work then)\n");
    printf(" BOX86_TRACE_XMM with 1 to enable dump of SSE/SSE2 register along with regular registers\n");
    printf(" BOX86_TRACE_START with N to enable trace after N instructions\n");
#endif
    printf(" BOX86_TRACE_FILE with FileName to redirect logs in a file");
    printf(" BOX86_DLSYM_ERROR with 1 to log dlsym errors\n");
    printf(" BOX86_LOAD_ADDR=0xXXXXXX try to load at 0xXXXXXX main binary (if binary is a PIE)\n");
    printf(" BOX86_NOSIGSEGV=1 to disable handling of SigSEGV\n");
#ifdef PANDORA
    printf(" BOX86_X11COLOR16=1 to try convert X11 color from 32 bits to 16 bits (to avoid light green on light cyan windows\n");
#endif
}

int main(int argc, const char **argv, const char **env) {

    // trying to open and load 1st arg
    if(argc==1) {
        printf("Box86%s v%d.%d.%d\n", 
        #ifdef USE_FLOAT
            " (float only)",
        #else
            "",
        #endif
            BOX86_MAJOR, BOX86_MINOR, BOX86_REVISION);
        PrintHelp();
        return 1;
    }
    ftrace = stdout;

    // init random seed
    srandom(time(NULL));

    // check BOX86_LOG debug level
    LoadLogEnv();
    
    // Create a new context
    box86context_t *context = NewBox86Context(argc - 1);

    const char *p;
    const char* prog = argv[1];
    // check BOX86_LD_LIBRARY_PATH and load it
    LoadEnvPath(&context->box86_ld_lib, ".:lib", "BOX86_LD_LIBRARY_PATH");
#ifdef PANDORA
    if(FileExist("/mnt/utmp/codeblocks/usr/lib/i386-linux-gnu", 0))
        AddPath("/mnt/utmp/codeblocks/usr/lib/i386-linux-gnu", &context->box86_ld_lib);
    if(FileExist("/mnt/utmp/box86/lib/i386-linux-gnu", 0))
        AddPath("/mnt/utmp/box86/lib/i386-linux-gnu", &context->box86_ld_lib);
    //TODO: add relative path to box86 location
#else
    if(FileExist("/lib/i386-linux-gnu", 0))
        AddPath("/lib/i386-linux-gnu", &context->box86_ld_lib);
    if(FileExist("/usr/lib/i386-linux-gnu", 0))
        AddPath("/usr/lib/i386-linux-gnu", &context->box86_ld_lib);
#endif
    if(getenv("BOX86_NOSIGSEGV")) {
        if (strcmp(getenv("BOX86_NOSIGSEGV"), "1")==0)
            context->no_sigsegv = 1;
            printf_log(LOG_INFO, "BOX86: Disabling handling of SigSEGV\n");
    }
    // check BOX86_PATH and load it
    LoadEnvPath(&context->box86_path, ".:bin", "BOX86_PATH");
    // prepare all other env. var
    context->envc = CountEnv(env);
    printf_log(LOG_INFO, "Counted %d Env var\n", context->envc);
    context->envv = (char**)calloc(context->envc, sizeof(char*));
    GatherEnv(&context->envv, env, prog);
    if(box86_log>=LOG_DUMP) {
        for (int i=0; i<context->envc; ++i)
            printf_log(LOG_DUMP, " Env[%02d]: %s\n", i, context->envv[i]);
    }
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE");
    if(p) {
        if (strcmp(p, "0"))
            context->x86trace = 1;
    }
    p = getenv("BOX86_TRACE_INIT");
    if(p) {
        if (strcmp(p, "0"))
            context->x86trace = 1;
    }
    if(context->x86trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if(InitX86Trace(context)) {
            printf_log(LOG_INFO, "Zydis init failed, no x86 trace activated\n");
            context->x86trace = 0;
        }
    }
#endif
    // lets build argc/argv stuff
    printf_log(LOG_INFO, "Looking for %s\n", prog);
    if(strchr(prog, '/'))
        context->argv[0] = strdup(prog);
    else
        context->argv[0] = ResolveFile(prog, &context->box86_path);

    for(int i=1; i<context->argc; ++i)
        context->argv[i] = strdup(argv[i+1]);
    // check if file exist
    if(!context->argv[0]) {
        printf_log(LOG_NONE, "Error: file is not found (check BOX86_PATH)\n");
        FreeBox86Context(&context);
        return -1;
    }
    if(!FileExist(context->argv[0], IS_FILE|IS_EXECUTABLE)) {
        printf_log(LOG_NONE, "Error: file %s is not found\n", context->argv[0]);
        FreeBox86Context(&context);
        return -1;
    }
    context->fullpath = (char*)calloc(PATH_MAX, 1);
    if(!realpath(context->argv[0], context->fullpath))
        strcpy(context->fullpath, context->argv[0]);
    FILE *f = fopen(context->argv[0], "rb");
    if(!f) {
        printf_log(LOG_NONE, "Error: Cannot open %s\n", context->argv[0]);
        FreeBox86Context(&context);
        return -1;
    }
    elfheader_t *elf_header = LoadAndCheckElfHeader(f, context->argv[0], 1);
    if(!elf_header) {
        printf_log(LOG_NONE, "Error: reading elf header of %s\n", context->argv[0]);
        fclose(f);
        FreeBox86Context(&context);
        return -1;
    }
    int mainelf = AddElfHeader(context, elf_header);

    if(CalcLoadAddr(elf_header)) {
        printf_log(LOG_NONE, "Error: reading elf header of %s\n", context->argv[0]);
        fclose(f);
        FreeBox86Context(&context);
        return -1;
    }
    // allocate memory
    if(AllocElfMemory(elf_header, 1)) {
        printf_log(LOG_NONE, "Error: allocating memory for elf %s\n", context->argv[0]);
        fclose(f);
        FreeBox86Context(&context);
        return -1;
    }
    // Load elf into memory
    if(LoadElfMemory(f, elf_header)) {
        printf_log(LOG_NONE, "Error: loading in memory elf %s\n", context->argv[0]);
        fclose(f);
        FreeBox86Context(&context);
        return -1;
    }
    // can close the file now
    fclose(f);
    // get and alloc stack size and align
    if(CalcStackSize(context)) {
        printf_log(LOG_NONE, "Error: allocating stack\n");
        FreeBox86Context(&context);
        return -1;
    }
    // init x86 emu
    context->emu = NewX86Emu(context, context->ep, (uintptr_t)context->stack, context->stacksz, 0);
    // stack setup is much more complicated then just that!
    SetupInitialStack(context);
    // this is probably useless
    SetupX86Emu(context->emu);
    SetEAX(context->emu, context->argc);
    SetEBX(context->emu, (uint32_t)context->argv);
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE_INIT");
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t trace_start=0, trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(context->emu, 0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%d-%d", &trace_start, &trace_end)!=2) {
                if(sscanf(p, "0x%X-0x%X", &trace_start, &trace_end)!=2)
                    sscanf(p, "%x-%x", &trace_start, &trace_end);
            }
            if(trace_start)
                SetTraceEmu(context->emu, trace_start, trace_end);
        } else {
            if (GetSymbolStartEnd(GetMapSymbol(context->maplib), p, &trace_start, &trace_end)) {
                SetTraceEmu(context->emu, trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)trace_start, (void*)trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to Traced (\"%s\") not found, disabling trace\n", p);
                SetTraceEmu(context->emu, 0, 100);  // disabling trace, mostly
            }
        }
    } else {
        p = getenv("BOX86_TRACE");
        if(p)
            if (strcmp(p, "0"))
                SetTraceEmu(context->emu, 0, 1);
    }
#endif
    // export symbols
    AddSymbols(context->maplib, GetMapSymbol(context->maplib), GetWeakSymbol(context->maplib), GetLocalSymbol(context->maplib), elf_header);
    // Call librarian to load all dependant elf
    if(LoadNeededLibs(elf_header, context->maplib, NULL, context, context->emu)) {
        printf_log(LOG_NONE, "Error: loading needed libs in elf %s\n", context->argv[0]);
        FreeBox86Context(&context);
        return -1;
    }
    if(FinalizeNeededLibs(elf_header, context->maplib, context, context->emu)) {
        printf_log(LOG_NONE, "Error: finalizing needed libs in elf %s\n", context->argv[0]);
        FreeBox86Context(&context);
        return -1;
    }
    // reloc...
    printf_log(LOG_DEBUG, "And now export symbols / relocation for %s...\n", ElfName(elf_header));
    if(RelocateElf(context->maplib, elf_header)) {
        printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", context->argv[0]);
        FreeBox86Context(&context);
        return -1;
    }
    // and handle PLT
    RelocateElfPlt(context, context->maplib, elf_header);
    // defered init
    RunDeferedElfInit(context->emu);
    // init...
//    RunElfInit(elf_header, context->emu); //=> this seems to be done in the startup code itself
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE");
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t trace_start=0, trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(context->emu, 0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%d-%d", &trace_start, &trace_end)!=2) {
                if(sscanf(p, "0x%X-0x%X", &trace_start, &trace_end)!=2)
                    sscanf(p, "%x-%x", &trace_start, &trace_end);
            }
            if(trace_start)
                SetTraceEmu(context->emu, trace_start, trace_end);
        } else {
            if (GetGlobalSymbolStartEnd(context->maplib, p, &trace_start, &trace_end)) {
                SetTraceEmu(context->emu, trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)trace_start, (void*)trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to Traced (\"%s\") not found, disabling trace\n", p);
                SetTraceEmu(context->emu, 0, 100);  // disabling trace, mostly
            }
        }
    }
#endif
    // get entrypoint
    context->ep = GetEntryPoint(context->maplib, elf_header);

    // emulate!
    printf_log(LOG_DEBUG, "Start x86emu on Main\n");
    SetEAX(context->emu, context->argc);
    SetEBX(context->emu, (uint32_t)context->argv);
    SetEIP(context->emu, context->ep);
    ResetFlags(context->emu);
    Run(context->emu);
    // Get EAX
    int ret = GetEAX(context->emu);
    printf_log(LOG_DEBUG, "Emulation finished, EAX=%d\n", ret);


    // all done, free context
    FreeBox86Context(&context);

    return ret;
}
