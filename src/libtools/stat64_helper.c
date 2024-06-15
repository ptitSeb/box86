#define _TIME_BITS 64
#define _FILE_OFFSET_BITS 64

#include <string.h>
#include <sys/stat.h>

void stat64_time64_armhf_to_i386(struct stat *src, char *dest) {
  /*
  i386 t64
  size of stat struct 108
  Offset of st_dev 0 size 8
  Offset of st_ino 8 size 8
  Offset of st_mode 16 size 4
  Offset of st_nlink 20 size 4
  Offset of st_uid 24 size 4
  Offset of st_gid 28 size 4
  Offset of st_rdev 32 size 8
  Offset of st_size 40 size 8
  Offset of st_blksize 48 size 4
  Offset of st_blocks 52 size 8
  Offset of st_atim 60 size 16
  Offset of st_mtim 76 size 16
  Offset of st_ctim 92 size 16

  armhf t64
  size of stat struct 112
  Offset of st_dev 0 size 8
  Offset of st_ino 8 size 8
  Offset of st_mode 16 size 4
  Offset of st_nlink 20 size 4
  Offset of st_uid 24 size 4
  Offset of st_gid 28 size 4
  Offset of st_rdev 32 size 8
  Offset of st_size 40 size 8
  Offset of st_blksize 48 size 4
  Offset of st_blocks 56 size 8
  Offset of st_atim 64 size 16
  Offset of st_mtim 80 size 16
  Offset of st_ctim 96 size 16
  */

  char *st = (char *)src;
  memset(dest, 0, 108);
  memcpy(dest, st, 8);
  memcpy(dest + 8, st + 8, 8);
  memcpy(dest + 16, st + 16, 4);
  memcpy(dest + 20, st + 20, 4);
  memcpy(dest + 24, st + 24, 4);
  memcpy(dest + 28, st + 28, 4);
  memcpy(dest + 32, st + 32, 8);
  memcpy(dest + 40, st + 40, 8);
  memcpy(dest + 48, st + 48, 4);
  memcpy(dest + 52, st + 56, 8);
  memcpy(dest + 60, st + 64, 16);
  memcpy(dest + 76, st + 80, 16);
  memcpy(dest + 92, st + 96, 16);
}

int stat64_time64_helper(void *path, void *buf) {
  struct stat st;
  int r = stat(path, &st);
  stat64_time64_armhf_to_i386(&st, buf);
  return r;
}

int fstatat64_time64_helper(int dirfd, void *path, void *buf, int flags) {
  struct stat st;
  int r = fstatat(dirfd, path, &st, flags);
  stat64_time64_armhf_to_i386(&st, buf);
  return r;
}

int fstat64_time64_helper(int fd, void *buf) {
  struct stat st;
  int r = fstat(fd, &st);
  stat64_time64_armhf_to_i386(&st, buf);
  return r;
}

int lstat64_time64_helper(void *path, void *buf) {
  struct stat st;
  int r = lstat(path, &st);
  stat64_time64_armhf_to_i386(&st, buf);
  return r;
}
