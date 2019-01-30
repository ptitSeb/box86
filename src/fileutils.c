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

int FileExist(const char* filename, int flags)
{
    struct stat sb;
    if (stat(filename, &sb) == -1)
        return 0;
    // check type of file? should be executable, or folder
    if(flags&IS_FILE) {
        if(!S_ISREG(sb.st_mode))
            return 0;
    } else {
        if(!S_ISDIR(sb.st_mode))
            return 0;
    }
    if(flags&IS_EXECUTABLE) {
        if((sb.st_mode&S_IXUSR)!=S_IXUSR)
            return 0;   // nope
    }
    return 1;
}

char* ResolveFile(const char* filename, path_collection_t* paths)
{
    char p[MAX_PATH];
    for (int i=0; i<paths->size; ++i) {
        strcpy(p, paths->paths[i]);
        strcat(p, filename);
        if(FileExist(p, IS_FILE))
            return strdup(p);
    }

    return NULL;
}