#define _LARGEFILE_SOURCE 1
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <asm/stat.h>
#include <sys/vfs.h>
#include <signal.h>
#include <stddef.h>

#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "myalign.h"
#include "debug.h"

void UnalignSysStat64(const void* source, void* dest)
{
    struct i386_stat64 *i386st = (struct i386_stat64*)dest;
    struct stat64 *st = (struct stat64*) source;
    
    i386st->__pad0 = 0;
	i386st->__pad3 = 0;
    i386st->st_dev      = st->st_dev;
#ifndef POWERPCLE
    i386st->__st_ino    = st->__st_ino;
#else
    i386st->__st_ino    = st->st_ino; // Separate __st_ino doesn't 
                                      // exist on powerpc
#endif
    i386st->st_mode     = st->st_mode;
    i386st->st_nlink    = st->st_nlink;
    i386st->st_uid      = st->st_uid;
    i386st->st_gid      = st->st_gid;
    i386st->st_rdev     = st->st_rdev;
    i386st->st_size     = st->st_size;
    i386st->st_blksize  = st->st_blksize;
    i386st->st_blocks   = st->st_blocks;
    i386st->st_atime    = st->st_atime;
    i386st->st_atime_nsec   = st->st_atime_nsec;
    i386st->st_mtime    = st->st_mtime;
    i386st->st_mtime_nsec   = st->st_mtime_nsec;
    i386st->st_ctime    = st->st_ctime;
    i386st->st_ctime_nsec   = st->st_ctime_nsec;
    i386st->st_ino      = st->st_ino;
}
