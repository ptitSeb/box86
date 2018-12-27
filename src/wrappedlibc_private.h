#if defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(END)

GOM(__stack_chk_fail, vFE)
GOM(__libc_start_main, iFEpippppp)
GOM(syscall, uFE)
GO(puts, iFp)
GO2(printf, iFopV, vfprintf)
GO2(__printf_chk, iFvopV, vfprintf)
GO(calloc, pFuu)
GO(free, vFp)
GO(putchar, iFi)
GO(strtol, iFppi)
GO(strerror, pFv)
END()

#else
#error Mmmm...
#endif