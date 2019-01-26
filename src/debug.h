#ifndef __DEBUG_H_
#define __DEBUG_H_

extern int box86_log;    // log level
extern int trace_xmm;    // include XMM reg in trace?
#define LOG_NONE 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG_DUMP 3

#define printf_log(L, ...) if(L<=box86_log) printf(__VA_ARGS__)

#define EXPORT __attribute__((visibility("default")))

#endif //__DEBUG_H_