#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>
#include <limits.h>
#include <stdint.h>

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

#include "debug.h"
#include "fileutils.h"

int FileExist(const char* filename)
{
    struct stat sb;
    if (stat(filename, &sb) == -1)
        return 0;
    // check type of file? should be executable, or folder
    return 1;
}

char* ResolveFile(const char* filename, path_collection_t* paths)
{
    char p[MAX_PATH];
    for (int i=0; i<paths->size; ++i) {
        strcpy(p, paths->paths[i]);
        strcat(p, filename);
        if(FileExist(p))
            return strdup(p);
    }

    return NULL;
}