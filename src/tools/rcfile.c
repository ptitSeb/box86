#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "debug.h"
#include "rcfile.h"
#include "box86context.h"
#include "pathcoll.h"
#include "fileutils.h"
#ifdef HAVE_TRACE
#include "x86trace.h"
#endif
#include "custommem.h"
#include "khash.h"

// This file handle the box86rc files
// file are basicaly ini file, with section [XXXX] defining the name of the process
// and BOX86_XXXX=YYYY entry like the env. var. variables

// list of all entries
#define SUPER1()                                        \
ENTRYINTPOS(BOX86_ROLLING_LOG, new_cycle_log)           \
ENTRYSTRING_(BOX86_LD_LIBRARY_PATH, ld_library_path)    \
ENTRYSTRING_(BOX86_PATH, box86_path)                    \
ENTRYSTRING_(BOX86_TRACE_FILE, trace_file)              \
ENTRYADDR(BOX86_LOAD_ADDR, box86_load_addr)             \
ENTRYINT(BOX86_LOG, box86_log, 0, 3, 2)                 \
ENTRYBOOL(BOX86_DUMP, box86_dump)                       \
ENTRYBOOL(BOX86_DLSYM_ERROR, dlsym_error)               \
CENTRYBOOL(BOX86_NOSIGSEGV, no_sigsegv)                 \
CENTRYBOOL(BOX86_NOSIGILL, no_sigill)                   \
ENTRYBOOL(BOX86_SHOWSEGV, box86_showsegv)               \
ENTRYBOOL(BOX86_SHOWBT, box86_showbt)                   \
ENTRYBOOL(BOX86_X11THREADS, box86_x11threads)           \
ENTRYBOOL(BOX86_X11GLX, box86_x11glx)                   \
ENTRYDSTRING(BOX86_LIBGL, box86_libGL)                  \
ENTRYBOOL(BOX86_SSE_FLUSHTO0, box86_sse_flushto0)       \
ENTRYBOOL(BOX86_X87_NO80BITS, box86_x87_no80bits)       \
ENTRYSTRING_(BOX86_EMULATED_LIBS, emulated_libs)        \
ENTRYBOOL(BOX86_ALLOWMISSINGLIBS, allow_missing_libs)   \
ENTRYBOOL(BOX86_ALLOWMISSING_SYMBOLS, allow_missing_symbols)    \
ENTRYBOOL(BOX86_PREFER_WRAPPED, box86_prefer_wrapped)   \
ENTRYBOOL(BOX86_PREFER_EMULATED, box86_prefer_emulated) \
ENTRYBOOL(BOX86_NOPULSE, box86_nopulse)                 \
ENTRYBOOL(BOX86_NOGTK, box86_nogtk)                     \
ENTRYBOOL(BOX86_NOVULKAN, box86_novulkan)               \
ENTRYBOOL(BOX86_FUTEX_WAITV, box86_futex_waitv)         \
ENTRYBOOL(BOX86_FIX_64BIT_INODES, fix_64bit_inodes)     \
ENTRYSTRING_(BOX86_BASH, bash)                          \
ENTRYINT(BOX86_JITGDB, jit_gdb, 0, 3, 2)                \
ENTRYSTRING_(BOX86_BOX64, box64)                        \
ENTRYSTRING_(BOX86_LD_PRELOAD, ld_preload)              \
ENTRYBOOL(BOX86_NOSANDBOX, box86_nosandbox)             \
ENTRYBOOL(BOX86_LIBCEF, box86_libcef)                   \
ENTRYBOOL(BOX86_NOCRASHHANDLER, box86_nocrashhandler)   \
ENTRYBOOL(BOX86_SDL2_JGUID, box86_sdl2_jguid)           \
ENTRYBOOL(BOX86_MUTEX_ALIGNED, box86_mutex_aligned)     \
ENTRYINT(BOX86_MALLOC_HACK, box86_malloc_hack, 0, 2, 2) \
ENTRYINTPOS(BOX86_MAXCPU, new_maxcpu)                   \

#ifdef HAVE_TRACE
#define SUPER2()                                        \
ENTRYSTRING_(BOX86_TRACE, trace)                        \
ENTRYULONG(BOX86_TRACE_START, start_cnt)                \
ENTRYSTRING_(BOX86_TRACE_INIT, trace_init)              \
ENTRYBOOL(BOX86_TRACE_XMM, trace_xmm)                   \
ENTRYBOOL(BOX86_TRACE_EMM, trace_emm)                   \

#else
#define SUPER2()                                        \
IGNORE(BOX86_TRACE)                                     \
IGNORE(BOX86_TRACE_START)                               \
IGNORE(BOX86_TRACE_INIT)                                \
IGNORE(BOX86_TRACE_XMM)                                 \
IGNORE(BOX86_TRACE_EMM)                                 \

#endif

#ifdef DYNAREC
#define SUPER3()                                                    \
ENTRYBOOL(BOX86_DYNAREC, box86_dynarec)                             \
ENTRYINT(BOX86_DYNAREC_DUMP, box86_dynarec_dump, 0, 2, 2)           \
ENTRYINT(BOX86_DYNAREC_LOG, box86_dynarec_log, 0, 3, 2)             \
ENTRYINT(BOX86_DYNAREC_BIGBLOCK, box86_dynarec_bigblock, 0, 3, 2)   \
ENTRYSTRING_(BOX86_DYNAREC_FORWARD, box86_dynarec_forward)          \
ENTRYINT(BOX86_DYNAREC_STRONGMEM, box86_dynarec_strongmem, 0, 3, 2) \
ENTRYBOOL(BOX86_DYNAREC_X87DOUBLE, box86_dynarec_x87double)         \
ENTRYBOOL(BOX86_DYNAREC_FASTNAN, box86_dynarec_fastnan)             \
ENTRYBOOL(BOX86_DYNAREC_FASTROUND, box86_dynarec_fastround)         \
ENTRYINT(BOX86_DYNAREC_SAFEFLAGS, box86_dynarec_safeflags, 0, 2, 2) \
ENTRYBOOL(BOX86_DYNAREC_CALLRET, box86_dynarec_callret)             \
IGNORE(BOX86_DYNAREC_HOTPAGE)                                       \
IGNORE(BOX86_DYNAREC_FASTPAGE)                                      \
ENTRYBOOL(BOX86_DYNAREC_WAIT, box86_dynarec_wait)                   \
ENTRYBOOL(BOX86_DYNAREC_BLEEDING_EDGE, box86_dynarec_bleeding_edge) \
ENTRYBOOL(BOX86_DYNAREC_JVM, box86_dynarec_jvm)                     \
ENTRYBOOL(BOX86_DYNAREC_TBB, box86_dynarec_tbb)                     \
ENTRYSTRING_(BOX86_NODYNAREC, box86_nodynarec)                      \
ENTRYBOOL(BOX86_DYNAREC_TEST, box86_dynarec_test)                   \
ENTRYBOOL(BOX86_DYNAREC_MISSING, box86_dynarec_missing)             \

#else
#define SUPER3()                                                    \
IGNORE(BOX86_DYNAREC)                                               \
IGNORE(BOX86_DYNAREC_DUMP)                                          \
IGNORE(BOX86_DYNAREC_LOG)                                           \
IGNORE(BOX86_DYNAREC_BIGBLOCK)                                      \
IGNORE(BOX86_DYNAREC_FORWARD)                                       \
IGNORE(BOX86_DYNAREC_STRONGMEM)                                     \
IGNORE(BOX86_DYNAREC_X87DOUBLE)                                     \
IGNORE(BOX86_DYNAREC_FASTNAN)                                       \
IGNORE(BOX86_DYNAREC_FASTROUND)                                     \
IGNORE(BOX86_DYNAREC_SAFEFLAGS)                                     \
IGNORE(BOX86_DYNAREC_CALLRET)                                       \
IGNORE(BOX86_DYNAREC_HOTPAGE)                                       \
IGNORE(BOX86_DYNAREC_FASTPAGE)                                      \
IGNORE(BOX86_DYNAREC_WAIT)                                          \
IGNORE(BOX86_DYNAREC_BLEEDING_EDGE)                                 \
IGNORE(BOX86_DYNAREC_JVM)                                           \
IGNORE(BOX86_DYNAREC_TBB)                                           \
IGNORE(BOX86_NODYNAREC)                                             \
IGNORE(BOX86_DYNAREC_TEST)                                          \
IGNORE(BOX86_DYNAREC_MISSING)                                       \

#endif

#if defined(HAVE_TRACE) && defined(DYNAREC)
#define SUPER4()                                                    \
ENTRYBOOL(BOX86_DYNAREC_TRACE, box86_dynarec_trace)                 \

#else
#define SUPER4()                                                    \
IGNORE(BOX86_DYNAREC_TRACE)                                         \

#endif


#ifdef PANDORA
#define SUPER5()                            \
ENTRYBOOL(BOX86_X11COLOR16, x11color16)            
#else
#define SUPER5()                            \
IGNORE(BOX86_X11COLOR16)       
#endif

#define SUPER() \
SUPER1()        \
SUPER2()        \
SUPER3()        \
SUPER4()        \
SUPER5()

typedef struct my_params_s {
// is present part
#define ENTRYBOOL(NAME, name) uint8_t is_##name##_present:1;
#define CENTRYBOOL(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYINT(NAME, name, minval, maxval, bits) uint8_t is_##name##_present:1;
#define ENTRYINTPOS(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYSTRING(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYSTRING_(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYDSTRING(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYADDR(NAME, name) uint8_t is_##name##_present:1;
#define ENTRYULONG(NAME, name) uint8_t is_##name##_present:1;
#define IGNORE(NAME) 
SUPER()
// done
#undef ENTRYBOOL
#undef CENTRYBOOL
#undef ENTRYINT
#undef ENTRYINTPOS
#undef ENTRYSTRING
#undef ENTRYSTRING_
#undef ENTRYDSTRING
#undef ENTRYADDR
#undef ENTRYULONG
// the actual fields
#define ENTRYBOOL(NAME, name) uint8_t name:1;
#define CENTRYBOOL(NAME, name) uint8_t name:1;
#define ENTRYINT(NAME, name, minval, maxval, bits) uint8_t name:bits;
#define ENTRYINTPOS(NAME, name) uint32_t name;
#define ENTRYSTRING(NAME, name) char* name;
#define ENTRYSTRING_(NAME, name) char* name;
#define ENTRYDSTRING(NAME, name) char* name;
#define ENTRYADDR(NAME, name) uintptr_t name;
#define ENTRYULONG(NAME, name) uint64_t name;
SUPER()
// done
#undef ENTRYBOOL
#undef CENTRYBOOL
#undef ENTRYINT
#undef ENTRYINTPOS
#undef ENTRYSTRING
#undef ENTRYSTRING_
#undef ENTRYDSTRING
#undef ENTRYADDR
#undef ENTRYULONG
} my_params_t;

KHASH_MAP_INIT_STR(params, my_params_t)

static kh_params_t *params = NULL;

#ifdef PANDORA
extern int x11color16;
#endif
extern int fix_64bit_inodes;

static void clearParam(my_params_t* param)
{
    #define ENTRYBOOL(NAME, name) 
    #define CENTRYBOOL(NAME, name) 
    #define ENTRYINT(NAME, name, minval, maxval, bits) 
    #define ENTRYINTPOS(NAME, name) 
    #define ENTRYSTRING(NAME, name) free(param->name); 
    #define ENTRYSTRING_(NAME, name) free(param->name); 
    #define ENTRYDSTRING(NAME, name) free(param->name); 
    #define ENTRYADDR(NAME, name) 
    #define ENTRYULONG(NAME, name) 
    SUPER()
    #undef ENTRYBOOL
    #undef CENTRYBOOL
    #undef ENTRYINT
    #undef ENTRYINTPOS
    #undef ENTRYSTRING
    #undef ENTRYSTRING_
    #undef ENTRYDSTRING
    #undef ENTRYADDR
    #undef ENTRYULONG
}

static void addParam(const char* name, my_params_t* param)
{
    khint_t k;
    k = kh_get(params, params, name);
    if(k==kh_end(params)) {
        int ret;
        k = kh_put(params, params, strdup(name), &ret);
    } else {
        clearParam(&kh_value(params, k));
    }
    my_params_t *p = &kh_value(params, k);
    memcpy(p, param, sizeof(my_params_t));
}

static void trimString(char* s)
{
    if(!s)
        return;
    // trim right space/tab
    size_t len = strlen(s);
    while(len && (s[len-1]==' ' || s[len-1]=='\t' || s[len-1]=='\n'))
        s[--len] = '\0';
    // trim left space/tab
    while(s[0]==' ' || s[0]=='\t')
        memmove(s, s+1, strlen(s));
}

void LoadRCFile(const char* filename)
{
    FILE *f = fopen(filename, "r");
    if(!f) {
        printf_log(LOG_INFO, "Cannot open RC file %s\n", filename);
        return;
    }
    // init the hash table if needed
    if(!params)
        params = kh_init(params);
    // prepare to parse the file
    char* line = NULL;
    size_t lsize = 0;
    my_params_t current_param = {0};
    char* current_name = NULL;
    int dummy;
    size_t len;
    char* p;
    // parsing
    while ((dummy = getline(&line, &lsize, f)) != -1) {
        // remove comments
        if((p=strchr(line, '#')))
            *p = '\0';
        trimString(line);
        len = strlen(line);
        // check the line content
        if(line[0]=='[' && strchr(line, ']')) {
            // new entry, will need to add current one
            if(current_name)
                addParam(current_name, &current_param);
            // prepare a new entry
            memset(&current_param, 0, sizeof(current_param));
            free(current_name);
            current_name = LowerCase(line+1);
            *strchr(current_name, ']') = '\0';
            trimString(current_name);
        } else if(strchr(line, '=')) {
            // actual parameters
            //get the key and val
            char* key = line;
            char* val = strchr(key, '=')+1;
            *strchr(key, '=') = '\0';
            trimString(key);
            trimString(val);
            // extract, check and set arg
            #define ENTRYBOOL(NAME, name) ENTRYINT(NAME, name, 0, 1, 1)
            #define CENTRYBOOL(NAME, name) ENTRYBOOL(NAME, name)
            #define ENTRYINT(NAME, name, minval, maxval, bits)          \
                else if(!strcmp(key, #NAME)) {                          \
                    int v = strtol(val, &p, 0);                         \
                    if(p!=val && v>=minval && v<=maxval) {              \
                        current_param.is_##name##_present = 1;          \
                        current_param.name = v;                         \
                    }                                                   \
                }
            #define ENTRYINTPOS(NAME, name)                             \
                else if(!strcmp(key, #NAME)) {                          \
                    int v = strtol(val, &p, 0);                         \
                    if(p!=val) {                                        \
                        current_param.is_##name##_present = 1;          \
                        current_param.name = v;                         \
                    }                                                   \
                }
            #define ENTRYSTRING(NAME, name)                             \
                else if(!strcmp(key, #NAME)) {                          \
                    current_param.is_##name##_present = 1;              \
                    if(current_param.name) free(current_param.name);    \
                    current_param.name = strdup(val);                   \
                }
            #define ENTRYSTRING_(NAME, name) ENTRYSTRING(NAME, name)
            #define ENTRYDSTRING(NAME, name) ENTRYSTRING(NAME, name)
            #define ENTRYADDR(NAME, name)                               \
                else if(!strcmp(key, #NAME)) {                          \
                    uintptr_t v = strtoul(val, &p, 0);                  \
                    if(p!=val) {                                        \
                        current_param.is_##name##_present = 1;          \
                        current_param.name = v;                         \
                    }                                                   \
                }
            #define ENTRYULONG(NAME, name)                              \
                else if(!strcmp(key, #NAME)) {                          \
                    uint64_t v = strtoull(val, &p, 0);                  \
                    if(p!=val) {                                        \
                        current_param.is_##name##_present = 1;          \
                        current_param.name = v;                         \
                    }                                                   \
                }
            #undef IGNORE
            #define IGNORE(NAME) else if(!strcmp(key, #NAME)) ;
            if(0) ;
            SUPER()
            else if(len && current_name) {
                printf_log(LOG_INFO, "Warning, unsupported %s=%s for [%s] in %s", key, val, current_name, filename);
            }
            #undef ENTRYBOOL
            #undef CENTRYBOOL
            #undef ENTRYINT
            #undef ENTRYINTPOS
            #undef ENTRYSTRING
            #undef ENTRYSTRING_
            #undef ENTRYDSTRING
            #undef ENTRYADDR
            #undef ENTRYULONG
            #undef IGNORE
            #define IGNORE(NAME) 
        }
    }
    // last entry to be pushed too
    if(current_name)
        addParam(current_name, &current_param);
    free(line);
    fclose(f);
    printf_log(LOG_INFO, "Params database has %d entries\n", kh_size(params));
}

void DeleteParams()
{
    if(!params)
        return;
    
    // free strings
    my_params_t* p;
    // need to free duplicated strings
    kh_foreach_value_ref(params, p, clearParam(p));
    const char* key;
    kh_foreach_key(params, key, free((void*)key));
    // free the hash itself
    kh_destroy(params, params);
    params = NULL;
}

extern int ftrace_has_pid;
void openFTrace(const char* newtrace);
#ifdef DYNAREC
void GatherDynarecExtensions();
#endif
#ifdef HAVE_TRACE
void setupTraceInit();
void setupTrace();
#endif
void ApplyParams(const char* name, path_collection_t* preload)
{
    if(!name || !params)
        return;
    static const char* old_name = NULL;
    int new_cycle_log = cycle_log;
    int new_maxcpu = box86_maxcpu;
    if(old_name && !strcmp(name, old_name))
        return;
    old_name = name;
    khint_t k;
    {
        char* lname = LowerCase(name);
        k = kh_get(params, params, lname);
        free(lname);
    }
    if(k == kh_end(params))
        return;
    my_params_t* param = &kh_value(params, k);
    #ifdef DYNAREC
    int olddynarec = box86_dynarec;
    #endif
    printf_log(LOG_INFO, "Apply RC params for %s\n", name);
    #define ENTRYBOOL(NAME, name) ENTRYINT(NAME, name, 0, 1, 1)
    #define CENTRYBOOL(NAME, name) if(param->is_##name##_present) {my_context->name = param->name; printf_log(LOG_INFO, "Applying %s=%d\n", #NAME, param->name);}
    #define ENTRYINT(NAME, name, minval, maxval, bits) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%d\n", #NAME, param->name);}
    #define ENTRYINTPOS(NAME, name) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%d\n", #NAME, param->name);}
    #define ENTRYSTRING(NAME, name) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%s\n", #NAME, param->name);}
    #define ENTRYSTRING_(NAME, name)  
    #define ENTRYDSTRING(NAME, name) if(param->is_##name##_present) {if(name) free(name); name = strdup(param->name); printf_log(LOG_INFO, "Applying %s=%s\n", #NAME, param->name);}
    #define ENTRYADDR(NAME, name) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%zd\n", #NAME, param->name);}
    #define ENTRYULONG(NAME, name) if(param->is_##name##_present) {name = param->name; printf_log(LOG_INFO, "Applying %s=%lld\n", #NAME, param->name);}
    SUPER()
    #undef ENTRYBOOL
    #undef CENTRYBOOL
    #undef ENTRYINT
    #undef ENTRYINTPOS
    #undef ENTRYSTRING
    #undef ENTRYSTRING_
    #undef ENTRYDSTRING
    #undef ENTRYADDR
    #undef ENTRYULONG
    // now handle the manuel entry (the one with ending underscore)
    if(new_cycle_log==1)
        new_cycle_log = 16;
    if(new_cycle_log!=cycle_log) {
        freeCycleLog(my_context);
        cycle_log = new_cycle_log;
        initCycleLog(my_context);
    }
    if(!box86_maxcpu_immutable) {
        if(new_maxcpu!=box86_maxcpu && box86_maxcpu && box86_maxcpu<new_maxcpu) {
            printf_log(LOG_INFO, "Not applying BOX86_MAXCPU=%d because a lesser value is already active: %d\n", new_maxcpu, box86_maxcpu);
        } else
            box86_maxcpu = new_maxcpu;
    } else if(new_maxcpu!=box86_maxcpu)
        printf_log(LOG_INFO, "Not applying BOX86_MAXCPU=%d because it's too late\n", new_maxcpu);
    if(param->is_ld_library_path_present) AppendList(&my_context->box86_ld_lib, param->ld_library_path, 1);
    if(param->is_box86_path_present) AppendList(&my_context->box86_path, param->box86_path, 1);
    if(param->is_trace_file_present) {
        if(ftrace_has_pid) {
            // open a new ftrace...
            fclose(ftrace);
        }
        openFTrace(param->trace_file);
    }
    if(param->is_emulated_libs_present) {
        AppendList(&my_context->box86_emulated_libs, param->emulated_libs, 0);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX86_EMULATED_LIBS", param->emulated_libs);
    }
    if(param->is_bash_present && (FileIsX86ELF(param->bash) || FileIsX64ELF(param->bash))) {
        if(my_context->bashpath)
            free(my_context->bashpath);
        my_context->bashpath = strdup(param->bash);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX86_BASH", param->bash);
    }
    if(param->is_box64_present && FileExist(param->box64, IS_FILE)) {
        if(my_context->box64path) {
            free(my_context->box64path);
        }
        my_context->box64path = strdup(param->box64);
        printf_log(LOG_INFO, "Applying %s=%s\n", "BOX86_BOX64", param->box64);
    }
    if(param->is_ld_preload_present) {
        if(preload) {
            printf_log(LOG_INFO, "Applying %s=%s\n", "BOX86_LD_PRELOAD", param->ld_preload);
            PrependList(preload, param->ld_preload, 1);
        } else {
            printf_log(LOG_INFO, "Cannot Apply %s=%s\n", "BOX86_LD_PRELOAD", param->ld_preload);
        }
    }
    #ifdef HAVE_TRACE
    int old_x86trace = my_context->x86trace;
    if(param->is_trace_present) {
        char*p = param->trace;
        if (strcmp(p, "0")) {
            my_context->x86trace = 1;
            box86_trace = p;
        }
        printf_log(LOG_INFO, "Applying %s=%s", "BOX86_TRACE", param->trace);
    }
    if(param->is_trace_init_present) {
        char* p = param->trace_init;
        if (strcmp(p, "0")) {
            my_context->x86trace = 1;
            trace_init = p;
        }
        printf_log(LOG_INFO, "Applying %s=%s", "BOX86_TRACE_INIT", param->trace_init);
    }
    if(my_context->x86trace && !old_x86trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if(InitX86Trace(my_context)) {
            printf_log(LOG_INFO, "Zydis init failed, no x86 trace activated\n");
            my_context->x86trace = 0;
        }
    }
    if(param->is_trace_init_present)
        setupTraceInit();
    if(param->is_trace_present)
        setupTrace();
    #endif
    #ifdef DYNAREC
    if(param->is_box86_nodynarec_present) {
        uintptr_t no_start = 0, no_end = 0;
        char* p;
        no_start = strtoul(param->box86_nodynarec, &p, 0);
        if(p!=param->box86_nodynarec && p[0]=='-') {
            char* p2;
            ++p;
            no_end = strtoul(p, &p2, 0);
            if(p2!=p && no_end>no_start) {
                box86_nodynarec_start = no_start;
                box86_nodynarec_end = no_end;
                printf_log(LOG_INFO, "Appling BOX86_NODYNAREC=%p-%p\n", (void*)box86_nodynarec_start, (void*)box86_nodynarec_end);
            }
        }
    }
    if(param->is_box86_dynarec_forward_present) {
        int forward = 0;
        if(sscanf(param->box86_dynarec_forward, "%d", &forward)==1) {
            box86_dynarec_forward = forward;
            printf_log(LOG_INFO, "Appling BOX86_DYNAREC_FORWARD=%d\n", box86_dynarec_forward);
        }
    }
    if(!olddynarec && box86_dynarec)
        GatherDynarecExtensions();
    if(param->is_box86_dynarec_test_present && box86_dynarec_test) {
        box86_dynarec_fastnan = 0;
        box86_dynarec_fastround = 0;
        box86_dynarec_callret = 0;
    }
    #endif
    if(box86_log==3) {
        box86_log = 2;
        box86_dump = 1;
    }
}
