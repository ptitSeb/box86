#ifndef __DEBUG_H_
#define __DEBUG_H_

extern int box86_log;    // log level
#ifdef DYNAREC
extern int box86_dynarec_dump;
extern int box86_dynarec_log;
extern int box86_dynarec;
extern int box86_dynarec_linker;
extern int box86_dynarec_trace;
extern int box86_dynarec_forced;
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
#define LOG_NONE 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG_DUMP 3

extern FILE* ftrace;

#define printf_log(L, ...) do {if(L<=box86_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#ifdef DYNAREC
#define dynarec_log(L, ...) do {if(L<=box86_dynarec_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)
#endif

#define EXPORT __attribute__((visibility("default")))

#endif //__DEBUG_H_