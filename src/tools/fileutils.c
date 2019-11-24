#define _FILE_OFFSET_BITS 64
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

static const char* x86sign = "\x7f" "ELF" "\x01" "\x01" "\x01" "\x00" "\x00" "\x00" "\x00" "\x00" "\x00" "\x00" "\x00" "\x00" "\x02" "\x00" "\x03" "\x00";

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

int FileIsX86ELF(const char* filename)
{
    FILE *f = fopen(filename, "rb");
    if(!f)
        return 0;
    char head[sizeof(x86sign)] = {0};
    int sz = fread(head, sizeof(x86sign), 1, f);
    if(sz!=1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    if(memcmp(head, x86sign, sizeof(x86sign))==0)
        return 1;
    return 0;
}