#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "box86version.h"
#include "debug.h"

int box86_debug = DEBUG_NONE;

int main(int argc, const char **argv) {
    printf("Box86 v%d.%d.%d\n", BOX86_MAJOR, BOX86_MINOR, BOX86_REVISION);

    // check BOX86_LD_LIBRARY_PATH and load it
    #warning TODO: BOX86_LD_LIBRARY_PATH handling
    // check BOX86_PATH and load it
    #warning TODO: BOX86_PATH handling
    // check BOX86_DEBUG debug level
    char* p = getenv("BOX86_DEBUG");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>=DEBUG_NONE && p[1]<=DEBUG_DEBUG)
                box86_debug = p[0]-'0';
        } else {
            if(!strcmp(p, "NONE"))
                box86_debug = DEBUG_NONE;
            else if(!strcmp(p, "INFO"))
                box86_debug = DEBUG_INFO;
            else if(!strcmp(p, "DEBUG"))
                box86_debug = DEBUG_DEBUG;
        }
        printf_debug(DEBUG_INFO, "Debug level is %d\n", box86_debug);
    }

    // trying to open and load 1st arg
    if(argc>1) {
        printf_debug(DEBUG_INFO, "Openning %s\n", argv[1]);
    }

    return 0;
}