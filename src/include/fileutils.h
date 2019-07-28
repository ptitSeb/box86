#ifndef __FILEUTILS_H_
#define __FILEUTILS_H_

#include "pathcoll.h"

#define IS_EXECUTABLE   (1<<0)
#define IS_FILE         (1<<1)
       

int FileExist(const char* filename, int flags);    // 0 : doesn't exist, 1: Does exist
char* ResolveFile(const char* filename, path_collection_t* paths); // find a file, using Path if needed

#endif //__FILEUTILS_H_