#define _LARGEFILE_SOURCE 1
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <signal.h>
#include <stddef.h>

#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "myalign.h"
#include "debug.h"

void UnalignStat64(const void* source, void* dest)
{
    struct i386_stat64 *i386st = (struct i386_stat64*)dest;
    struct stat64 *st = (struct stat64*) source;
    
    i386st->__pad0 = 0;
	i386st->__pad3 = 0;
    i386st->st_dev      = st->st_dev;
#if defined(__USE_TIME64_REDIRECTS) || defined(POWERPCLE) || !defined(HAVE_ST_INO)
    i386st->__st_ino    = st->st_ino; 
#else
    i386st->__st_ino    = st->__st_ino;
#endif
    i386st->st_mode     = st->st_mode;
    i386st->st_nlink    = st->st_nlink;
    i386st->st_uid      = st->st_uid;
    i386st->st_gid      = st->st_gid;
    i386st->st_rdev     = st->st_rdev;
    i386st->st_size     = st->st_size;
    i386st->st_blksize  = st->st_blksize;
    i386st->st_blocks   = st->st_blocks;
#if defined(__USE_XOPEN2K8) || defined(ANDROID) || defined(PANDORA)
    i386st->st_atime    = st->st_atim.tv_sec;
    i386st->st_atime_nsec   = st->st_atim.tv_nsec;
    i386st->st_mtime    = st->st_mtim.tv_sec;
    i386st->st_mtime_nsec   = st->st_mtim.tv_nsec;
    i386st->st_ctime    = st->st_ctim.tv_sec;
    i386st->st_ctime_nsec   = st->st_ctim.tv_nsec;
#else
    i386st->st_atime    = st->st_atime;
    i386st->st_atime_nsec   = st->st_atime_nsec;
    i386st->st_mtime    = st->st_mtime;
    i386st->st_mtime_nsec   = st->st_mtime_nsec;
    i386st->st_ctime    = st->st_ctime;
    i386st->st_ctime_nsec   = st->st_ctime_nsec;
#endif
    i386st->st_ino      = st->st_ino;
}

void AlignStat64(const void* source, void* dest)
{
    struct stat64 *st = (struct stat64*) dest;
    struct i386_stat64 *i386st = (struct i386_stat64*)source;
    
    st->st_dev          = i386st->st_dev;
#if defined(__USE_TIME64_REDIRECTS) || defined(POWERPCLE)|| !defined(HAVE_ST_INO)
    // Separate __st_ino doesn't exist
#else
    st->__st_ino        = i386st->__st_ino;
#endif
    st->st_mode         = i386st->st_mode;
    st->st_nlink        = i386st->st_nlink;
    st->st_uid          = i386st->st_uid;
    st->st_gid          = i386st->st_gid;
    st->st_rdev         = i386st->st_rdev;
    st->st_size         = i386st->st_size;
    st->st_blksize      = i386st->st_blksize;
    st->st_blocks       = i386st->st_blocks;
#if defined(__USE_XOPEN2K8) || defined(ANDROID) || defined(PANDORA)
    st->st_atim.tv_sec  = i386st->st_atime;
    st->st_atim.tv_nsec = i386st->st_atime_nsec;
    st->st_mtim.tv_sec  = i386st->st_mtime;
    st->st_mtim.tv_nsec = i386st->st_mtime_nsec;
    st->st_ctim.tv_sec  = i386st->st_ctime;
    st->st_ctim.tv_nsec = i386st->st_ctime_nsec;
#else
    st->st_atime        = i386st->st_atime;
    st->st_atime_nsec   = i386st->st_atime_nsec;
    st->st_mtime        = i386st->st_mtime;
    st->st_mtime_nsec   = i386st->st_mtime_nsec;
    st->st_ctime        = i386st->st_ctime;
    st->st_ctime_nsec   = i386st->st_ctime_nsec;
#endif
    st->st_ino          = i386st->st_ino;
}

struct native_fsid {
  int     val[2];
};

struct native_statfs64 {
  uint32_t    f_type;
  uint32_t    f_bsize;
  uint64_t    f_blocks;
  uint64_t    f_bfree;
  uint64_t    f_bavail;
  uint64_t    f_files;
  uint64_t    f_ffree;
  struct native_fsid f_fsid;
  uint32_t    f_namelen;
  uint32_t    f_frsize;
  uint32_t    f_flags;
  uint32_t    f_spare[4];
};  // f_flags is not always defined, but then f_spare is [5] in that case


void UnalignStatFS64(const void* source, void* dest)
{
    struct i386_statfs64 *i386st = (struct i386_statfs64*)dest;
    struct native_statfs64 *st = (struct native_statfs64*) source;

    i386st->f_type      = st->f_type;
    i386st->f_bsize     = st->f_bsize;
    i386st->f_blocks    = st->f_blocks;
    i386st->f_bfree     = st->f_bfree;
    i386st->f_bavail    = st->f_bavail;
    i386st->f_files     = st->f_files;
    i386st->f_ffree     = st->f_ffree;
    memcpy(&i386st->f_fsid, &st->f_fsid, sizeof(i386st->f_fsid));
    i386st->f_namelen   = st->f_namelen;
    i386st->f_frsize    = st->f_frsize;
    i386st->f_flags     = st->f_flags;
    i386st->f_spare[0]  = st->f_spare[0];
    i386st->f_spare[1]  = st->f_spare[1];
    i386st->f_spare[2]  = st->f_spare[2];
    i386st->f_spare[3]  = st->f_spare[3];
}

#define TRANSFERT   \
GO(l_type)          \
GO(l_whence)        \
GO(l_start)         \
GO(l_len)           \
GO(l_pid)

// Arm -> x86
void UnalignFlock64(void* dest, void* source)
{
    #define GO(A) ((x86_flock64_t*)dest)->A = ((my_flock64_t*)source)->A;
    TRANSFERT
    #undef GO
}

// x86 -> Arm
void AlignFlock64(void* dest, void* source)
{
    #define GO(A) ((my_flock64_t*)dest)->A = ((x86_flock64_t*)source)->A;
    TRANSFERT
    #undef GO
}
#undef TRANSFERT
