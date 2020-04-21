#ifndef __DEBUG_H_
#define __DEBUG_H_

typedef struct box86context_s box86context_t;
int box86_log;    // log level
int box86_dynarec_log;
#ifdef DYNAREC
int box86_dynarec_dump;
int box86_dynarec;
int box86_dynarec_linker;
int box86_dynarec_trace;
int box86_dynarec_forced;
#ifdef ARM
int arm_vfp;     // vfp version (3 or 4), with 32 registers is mendatory
int arm_swap;
int arm_div;
#endif
#endif
int dlsym_error;  // log dlsym error
int trace_xmm;    // include XMM reg in trace?
int trace_emm;    // include EMM reg in trace?
int allow_missing_libs;
int box86_steam;
box86context_t *my_context; // global context
#define LOG_NONE 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG_DUMP 3

extern FILE* ftrace;

#define printf_log(L, ...) do {if(L<=box86_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define dynarec_log(L, ...) do {if(L<=box86_dynarec_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define EXPORT __attribute__((visibility("default")))

#endif //__DEBUG_H_