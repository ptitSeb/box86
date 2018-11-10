#ifndef __DEBUG_H_
#define __DEBUG_H_

extern int box86_debug;    // debug level
#define DEBUG_NONE 0
#define DEBUG_INFO 1
#define DEBUG_DEBUG 2
#define DEBUG_DUMP 3

#define printf_debug(L, ...) if(L<=box86_debug) printf(__VA_ARGS__)

#endif //__DEBUG_H_