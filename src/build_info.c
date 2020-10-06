#include <stdio.h>
#include "debug.h"
#include "box86version.h"
#ifdef NOGIT
#define GITREV "nogit"
#else
#include "git_head.h"
#endif

void PrintBox86Version()
{
    printf("Box86%s%s v%d.%d.%d %s built on %s %s\n", 
    #ifdef HAVE_TRACE
        " with trace",
    #else
        "",
    #endif
    #ifdef DYNAREC
        " with Dynarec",
    #else
        "",
    #endif
        BOX86_MAJOR, BOX86_MINOR, BOX86_REVISION,
        GITREV,
        __DATE__, __TIME__);
}

