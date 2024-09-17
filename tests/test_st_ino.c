#define _LARGEFILE_SOURCE 1
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <stddef.h>
int main(int argc, char** argv)
{
    struct stat64 st;
    st.__st_ino = 0;
    return 0;
}