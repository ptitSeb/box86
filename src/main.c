#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "box86version.h"
#include "debug.h"
#include "box86context.h"
#include "fileutils.h"
#include "elfloader.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86trace.h"
#include "librarian.h"

int box86_log = LOG_INFO;//LOG_NONE;

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
            printf("%s%s", col->paths[i], (i==col->size-1)?"\n":":");
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
}


void PrintHelp() {
    printf("\n\nThis is Box86, the Linux 86 emulator with a twist\n");
    printf("\nUsage is box86 path/to/software [args]\n");
    printf("to launch x86 software\n");
    printf("You can also set some env. var.:\n");
    printf(" BOX86_PATH to set the PATH used by box86 to find x86 programs (default is '.:bin')\n");
    printf(" BOX86_LD_LIBRARY_PATH to set the path were x86 lib are searched (default is '.:lib')\n");
    printf(" BOX86_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set printed debug info\n");
    printf(" BOX86_TRACE with 1 to enable x86 execution trace\n");
}

int main(int argc, const char **argv, const char **env) {

    // trying to open and load 1st arg
    if(argc==1) {
        printf("Box86 v%d.%d.%d\n", BOX86_MAJOR, BOX86_MINOR, BOX86_REVISION);
        PrintHelp();
        return 1;
    }

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

    p = getenv("BOX86_TRACE");
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
    if(AllocElfMemory(elf_header)) {
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
    // Call librarian to load all dependant elf
    if(LoadNeededLib(elf_header, context->maplib)) {
        printf_log(LOG_NONE, "Error: loading needed libs in elf %s\n", context->argv[0]);
        FreeBox86Context(&context);
        return -1;
    }
    // finalize relocations
    AddGlobalsSymbols(GetMapSymbol(context->maplib), elf_header);
    if(RelocateElf(context->maplib, elf_header)) {
        printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", context->argv[0]);
        FreeBox86Context(&context);
        return -1;
    }
    // get and alloc stack size and align
    if(CalcStackSize(context)) {
        printf_log(LOG_NONE, "Error: allocating stack\n");
        FreeBox86Context(&context);
        return -1;
    }
    // set entrypoint
    context->ep = GetEntryPoint(context->maplib, elf_header);
    // init x86 emu
    context->emu = NewX86Emu(context, context->ep, (uintptr_t)context->stack, context->stacksz);
    // stack setup is much more complicated then just that!
    // setup the stack...
    Push(context->emu, (uint32_t)context->argv);
    Push(context->emu, context->argc);
    SetupX86Emu(context->emu, NULL, NULL);
    SetEAX(context->emu, context->argc);
    SetEBX(context->emu, (uint32_t)context->argv);

    p = getenv("BOX86_TRACE");
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t trace_start, trace_end;
        if (strcmp(p, "1")==0)
            SetTraceEmu(context->emu, 0, 0);
        else if (GetSymbolStartEnd(GetMapSymbol(context->maplib), p, &trace_start, &trace_end))
            SetTraceEmu(context->emu, trace_start, trace_end);
    }

    // emulate!
    printf_log(LOG_DEBUG, "Start x86emu on Main\n");
    Run(context->emu);
    // Get EAX
    int ret = GetEAX(context->emu);
    printf_log(LOG_DEBUG, "Emulation finished, EAX=%d\n", ret);


    // all done, free context
    FreeBox86Context(&context);

    return ret;
}
