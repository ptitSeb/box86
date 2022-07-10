#ifndef __DEBUG_H_
#define __DEBUG_H_
#include <stdint.h>

typedef struct box86context_s box86context_t;
extern int box86_log;    // log level
extern int box86_dump;   // dump elf or not
extern int box86_dynarec_log;
extern int box86_dynarec;
extern int box86_pagesize;
extern uintptr_t box86_load_addr;
#ifdef DYNAREC
extern int box86_dynarec_dump;
extern int box86_dynarec_trace;
extern int box86_dynarec_forced;
extern int box86_dynarec_largest;
extern int box86_dynarec_bigblock;
extern int box86_dynarec_strongmem;
extern int box86_dynarec_x87double;
extern uintptr_t box86_nodynarec_start, box86_nodynarec_end;
extern int box86_dynarec_fastnan;
#ifdef ARM
extern int arm_vfp;     // vfp version (3 or 4), with 32 registers is mendatory
extern int arm_swap;
extern int arm_div;
#endif
#endif
extern int dlsym_error;  // log dlsym error
extern int trace_xmm;    // include XMM reg in trace?
extern int trace_emm;    // include EMM reg in trace?
extern int allow_missing_libs;
extern int box86_prefer_wrapped;
extern int box86_prefer_emulated;
extern int box86_steam;
extern int box86_wine;
extern int box86_nopulse;   // disabling the use of wrapped pulseaudio
extern int box86_nogtk; // disabling the use of wrapped gtk
extern int box86_novulkan;  // disabling the use of wrapped vulkan
extern int box86_mapclean;
extern int box86_showsegv;
extern int allow_missing_symbols;
extern uintptr_t   trace_start, trace_end;
extern char* trace_func;
extern uintptr_t fmod_smc_start, fmod_smc_end; // to handle libfmod (from Unreal) SMC (self modifying code)
extern uint32_t default_fs;
extern int jit_gdb; // launch gdb when a segfault is trapped
extern int box86_tcmalloc_minimal;  // when using tcmalloc_minimal
#define LOG_NONE 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG_NEVER 3
#define LOG_VERBOSE 3

extern FILE* ftrace;

#define printf_log(L, ...) do {if((L)<=box86_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define printf_dump(L, ...) do {if(box86_dump || ((L)<=box86_log)) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define printf_dlsym(L, ...) do {if(dlsym_error || ((L)<=box86_log)) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define dynarec_log(L, ...) do {if((L)<=box86_dynarec_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define EXPORT __attribute__((visibility("default")))
#ifdef BUILD_DYNAMIC
#define EXPORTDYN __attribute__((visibility("default")))
#else
#define EXPORTDYN 
#endif

#endif //__DEBUG_H_
