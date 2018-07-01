#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "box86version.h"
#include "debug.h"
#include "pathcoll.h"

int box86_debug = DEBUG_NONE;

path_collection_t box86_path = {0};
path_collection_t box86_ld_lib = {0};

void LoadEnvPath(path_collection_t *col, const char* defpath, const char* env)
{
    const char* p = getenv(env);
    if(p) {
        printf_debug(DEBUG_INFO, "%s: ", env);
        ParseList(p, col);
    } else {
        printf_debug(DEBUG_INFO, "Using default %s: ", env);
        ParseList(defpath, col);
    }
    if(DEBUG_INFO<=box86_debug) {
        for(int i=0; i<col->size; i++)
            printf("%s%s", col->paths[i], (i==col->size-1)?"\n":":");
    }
}

int main(int argc, const char **argv) {
    printf("Box86 v%d.%d.%d\n", BOX86_MAJOR, BOX86_MINOR, BOX86_REVISION);

    char *p;
    // check BOX86_DEBUG debug level
    p = getenv("BOX86_DEBUG");
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
    // check BOX86_LD_LIBRARY_PATH and load it
    LoadEnvPath(&box86_ld_lib, ".:lib", "BOX86_LD_LIBRARY_PATH");
    // check BOX86_PATH and load it
    LoadEnvPath(&box86_path, ".:bin", "BOX86_PATH");

    // trying to open and load 1st arg
    if(argc>1) {
        printf_debug(DEBUG_INFO, "Openning %s\n", argv[1]);
    }

    return 0;
}