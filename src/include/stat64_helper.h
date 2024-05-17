#ifndef __STAT64_HELPER_H_
#define __STAT64_HELPER_H_

int stat64_time64_helper(void *path, void *buf);
int fstatat64_time64_helper(int dirfd, void *path, void *buf, int flags);
int fstat64_time64_helper(int fd, void *buf);
int lstat64_time64_helper(void *path, void *buf);

#endif
