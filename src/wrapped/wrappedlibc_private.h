#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(GOS))
#error Meh...
#endif

// a64l
GO(abort, vFv)      // Should be GOM once signal are handled properly
GO(abs, iFi)
GOW(accept, iFipp)
GOM(accept4, iFEippi)   // glibc 2.10+
GOW(access, iFpi)
// acct
GOW(addmntent, iFpp)
// addseverity
// adjtime  // Weak
// adjtimex // Weak
// __adjtimex
// advance  // Weak
// __aeabi_assert
// __aeabi_atexit
// __aeabi_errno_addr
// __aeabi_localeconv
// __aeabi_MB_CUR_MAX
// __aeabi_memclr
// __aeabi_memclr4
// __aeabi_memclr8
// __aeabi_memcpy
// __aeabi_memcpy4
// __aeabi_memcpy8
// __aeabi_memmove
// __aeabi_memmove4
// __aeabi_memmove8
// __aeabi_memset
// __aeabi_memset4
// __aeabi_memset8
DATAV(__after_morecore_hook, 4)
GO(alarm, iFu)
GO2(aligned_alloc, pFuu, memalign)
GO(alphasort, iFpp)
GO(alphasort64, iFpp)
DATA(argp_err_exit_status, 4)
// argp_error   // Weak
// argp_failure // Weak
// argp_help    // Weak
// argp_parse   // Weak
// argp_program_bug_address // type B
// argp_program_version // type B
// argp_program_version_hook    // type B
// argp_state_help  // Weak
// argp_usage   // Weak
// argz_add // Weak
// argz_add_sep // Weak
// argz_append  // Weak
// __argz_count
// argz_count   // Weak
// argz_create  // Weak
GOW(argz_create_sep, iFpipp)
// argz_delete
// argz_extract // Weak
GOW(argz_insert, iFpppp)
// __argz_next
GOW(argz_next, pFpLp)
// argz_replace // Weak
// __argz_stringify
GOW(argz_stringify, vFpLi)
GO(asctime, pFp)
GOW(asctime_r, pFpp)
GOM(asprintf, iFEppVV) // Weak
GOM(__asprintf, iFEppVV)
GOM(__asprintf_chk, iFEpipVV)
// __assert
GO(__assert_fail, vFppup)
GO(__assert_perror_fail, vFipup)
GO(atof, dFp)
GO(atoi, iFp)
GO(atol, iFp)
GO(atoll, IFp)
// authdes_create
// authdes_getucred
// authdes_pk_create
// _authenticate
// authnone_create
// authunix_create
// authunix_create_default
GOW(backtrace, iFpi)    //TODO: probably a my_backtrace version, that use emulated stack instead
GO(__backtrace, iFpi)
GO(__backtrace_symbols, pFpi)
GOW(backtrace_symbols, pFpi)
GO(__backtrace_symbols_fd, vFpii)
GOW(backtrace_symbols_fd, vFpii)
GO(basename, pFp)
GOW(bcmp, iFppu)
GO(bcopy, vFppu)
// bdflush
GOW(bind, iFipu)
// bindresvport
GOW(bindtextdomain, pFpp)
GOW(bind_textdomain_codeset, pFpp)
GOW(brk, iFp)
// __bsd_getpgrp
// bsd_signal   // Weak
GOM(bsearch, pFEppuup)
GOW(btowc, iFi)
GOW(bzero, vFpu)
GO(__bzero, vFpu)
GOW(calloc, pFLL)
// callrpc
GOW(canonicalize_file_name, pFp)
// capget
// capset
GO(catclose, iFp)
GO(catgets, pFpiip)
GO(catopen, pFpi)
// cbc_crypt
GO(cfgetispeed, uFp)
GO(cfgetospeed, uFp)
GO(cfmakeraw, vFp)
GOW(cfree, vFp)
GO(cfsetispeed, iFpu)
GO(cfsetospeed, iFpu)
GO(cfsetspeed, iFpi)
GOW(chdir, iFp)
DATA(__check_rhosts_file, 4)
// chflags
// __chk_fail
GOW(chmod, iFpu)
GOW(chown, iFpuu)
GO(chroot, iFp)
GOW(clearenv, iFv)
GO(clearerr, vFp)
GO(clearerr_unlocked, vFp)
// clnt_broadcast
// clnt_create
// clnt_pcreateerror
// clnt_perrno
// clnt_perror
// clntraw_create
// clnt_spcreateerror
// clnt_sperrno
// clnt_sperror
// clnttcp_create
// clntudp_bufcreate
// clntudp_create
// clntunix_create
GO(clock, uFv)
// clone    // Weak
// __clone
GOW(close, iFi)
// __close  // Weak
GOW(closedir, iFp)
GO(closelog, vFv)
GO(__cmsg_nxthdr, pFpp)
GO(confstr, uFipu)
// __confstr_chk
GOW(connect, iFipu)
GOW(__connect, iFipu)
// copysign // Weak
// copysignf    // Weak
// copysignl    // Weak
GOW(creat, iFpu)
GO(creat64, iFpu)
// create_module    // Weak
GO(ctermid, pFp)
GO(ctime, pFp)
GO(ctime_r, pFpp)
DATAM(__ctype_b, 4)
GO(__ctype_b_loc, pFv)
GOW(__ctype_get_mb_cur_max, uFv)
DATAM(__ctype_tolower, 4)
GO(__ctype_tolower_loc, pFv)
DATAM(__ctype_toupper, 4)
GO(__ctype_toupper_loc, pFv)
// __curbrk // type B
GO(cuserid, pFp)
GOM(__cxa_atexit, iFEppp)
GOM(atexit, iFEp)           // just in case
GOM(__cxa_finalize, vFEp)
DATAM(__cpu_model, 16)
GOM(__cxa_thread_atexit_impl, iFEppp)
// __cyg_profile_func_enter
// __cyg_profile_func_exit
// daemon
DATAV(daylight, 4)
// __daylight   // type B
GOW(dcgettext, pFppi)
GO(__dcgettext, pFppi)
GOW(dcngettext, pFpppui)
// __default_morecore
// __default_rt_sa_restorer_v1
// __default_rt_sa_restorer_v2
// __default_sa_restorer_v1
// __default_sa_restorer_v2
// delete_module
// des_setparity
GOW(dgettext, pFpp)
GO(__dgettext, pFpp)
GO(difftime, dFuu)
GO(dirfd, iFp)
GO(dirname, pFp)
GOS(div, pFpii)
// _dl_addr
GOM(dl_iterate_phdr, iFEpp)
// _dl_mcount_wrapper
// _dl_mcount_wrapper_check
// _dl_open_hook    // type B
// _dl_starting_up // Weak
// _dl_sym
// _dl_vsym
GOW(dngettext, pFpppu)
// dprintf
// __dprintf_chk
GO(drand48, dFv)
// drand48_r
GOW(dup, iFi)
GOW(dup2, iFii)
GO(__dup2, iFii)
GO(dup3, iFiiO)
GOW(duplocale, pFp)
GO(__duplocale, pFp)
// dysize
GOW(eaccess, iFpi)
// ecb_crypt
// ecvt
GO(ecvt_r, iFdipppL)
// endaliasent
// endfsent
GO(endgrent, vFv)
GO(endhostent, vFv)
GOW(endmntent, iFp)
// __endmntent
// endnetent
// endnetgrent
GO(endprotoent, vFv)
GO(endpwent, vFv)
// endrpcent
// endservent
GO(endspent, vFv)
// endttyent
// endusershell
GOW(endutent, vFv)
// endutxent
DATAM(environ, 4)
DATAM(_environ, 4)
DATAM(__environ, 4)    // type B
// envz_add
// envz_entry
// envz_get
// envz_merge
// envz_remove
// envz_strip
#ifdef NOALIGN
GO(epoll_create, iFi)
GO(epoll_create1, iFi)
GO(epoll_ctl, iFiiip)
// epoll_pwait
GO(epoll_wait, iFipii)
#else
GOM(epoll_create, iFEi)      // not needed, but used in syscall
GOM(epoll_create1, iFEO)
GOM(epoll_ctl, iFEiiip)     // align epool_event structure
// epoll_pwait
GOM(epoll_wait, iFEipii)    // need realign of epoll_event structure
#endif
// erand48
// erand48_r    // Weak
GO(err, vFippppppppp)
// errno    // type B
GO(__errno_location, pFv)
GOW(error, vFiippppppppp)  // Simple attempt: there is a vararg, but the alignment will/may be off if it tries some Double in the "printf" part
// error_at_line    // Weak
// error_message_count  // type B
// error_one_per_line   // type B
// error_print_progname // type B
GO(errx, vFippppppppp)
GO(ether_aton, pFp)
GO(ether_aton_r, pFpp)
GO(ether_hostton, iFpp)
GO(ether_line, iFppp)
GO(ether_ntoa, pFp)
GO(ether_ntoa_r, pFpp)
GO(ether_ntohost, iFpp)
GOW(euidaccess, iFpi)
GO(eventfd, iFui)
GO(eventfd_read, iFip)
GO(eventfd_write, iFiU)
GO2(execl, iFEpV, my_execv)
GO2(execle, iFEpV, my_execve)  //Nope! This one needs wrapping, because if char*, char*, ..., char*[]
GO2(execlp, iFEpV, execvp)
GOM(execv, iFEpp)    // is Weak
GOM(execve, iFEppp)   // and this one too...
GOW(execvp, iFpp)
GO(exit, vFi)
GO(_exit, vFi)
GOW(_Exit, vFi)
GOM(__explicit_bzero_chk, vFEpuu)    // not always defined
GO(faccessat, iFipii)
// fattach
GO(__fbufsize, uFp)
GOW(fchdir, iFi)
// fchflags
GOW(fchmod, iFiu)
GO(fchmodat, iFipui)
GOW(fchown, iFiuu)
GO(fchownat, iFipuii)
GO(fclose, iFp)
GOW(fcloseall, iFv)
GOM(fcntl, iFEiiN)  // this also use a vararg for 3rd argument
GOM(__fcntl, iFEiiN)
GOM(fcntl64, iFEiiN)
GO(fcvt, pFdipp)
GO(fcvt_r, iFdipppL)
GO(fdatasync, iFi)
// fdetach
GO(fdopen, pFip)
GOW(fdopendir, pFi)
GOW(feof, iFp)
GO(feof_unlocked, iFp)
GOW(ferror, iFp)
GO(ferror_unlocked, iFp)
GO(fexecve, iFipp)  //TODO: Check if needed to be wrapped, and target checked for x86 / native?
GOW(fflush, iFp)
GO(fflush_unlocked, iFp)
GO(ffs, iFi)
// __ffs
GOW(ffsl, iFi)
GO(ffsll, iFI)
GOW(fgetc, iFp)
GOW(fgetc_unlocked, iFp)
// fgetgrent
// fgetgrent_r  // Weak
GO(fgetpos, iFpp)
GO(fgetpos64, iFpp)
// fgetpwent
// fgetpwent_r  // Weak
GOW(fgets, pFpip)
GO(__fgets_chk, pFpuip)
// fgetspent
// fgetspent_r  // Weak
GO(fgets_unlocked, pFpip)
// __fgets_unlocked_chk
GOW(fgetwc, iFp)
GOW(fgetwc_unlocked, iFp)
GO(fgetws, pFpip)
// __fgetws_chk
GO(fgetws_unlocked, pFpip)
// __fgetws_unlocked_chk
GO(fgetxattr, iFippu)
GO(fileno, iFp)
GOW(fileno_unlocked, iFp)
GOW(finite, iFd)
GO(__finite, iFd)
GOW(finitef, iFf)
// __finitef
// finitel  // Weak
// __finitel
// __flbf
GO(flistxattr, iFipu)
GOW(flock, iFii)
GOW(flockfile, vFp)
GOW(_flushlbf, vFv)
GO(fmemopen, pFpup)
// fmtmsg
GO(fnmatch, iFppi)
GOM(fopen, pFEpp)
GOM(fopen64, pFEpp)  // Weak
GOM(fopencookie, pFEpppppp)   // last 4p are a struct with 4 callbacks...
GOM(fork, iFEv) // Weak
GOM(__fork, iFEv)
// __fortify_fail
GOW(fpathconf, iFii)
GO(__fpending, uFp)
GOM(fprintf, iFEppVV)
GOM(__fprintf_chk, iFEpvpVV)
//GO2(fprintf, iFppV, vfprintf)
//GO2(__fprintf_chk, iFpvpV, vfprintf)
// __fpu_control    // type B
GO(__fpurge, vFp)
GOW(fputc, iFip)
GO(fputc_unlocked, iFip)
GOW(fputs, iFpp)    // Weak
GO(fputs_unlocked, iFpp)
GO(fputwc, iFip)
GO(fputwc_unlocked, iFip)
GO(fputws, iFpp)
GO(fputws_unlocked, iFpp)
GOW(fread, LFpLLp)
GO(__freadable, iFp)
GO(__fread_chk, uFpuuup)
GO(__freading, iFp)
GO(fread_unlocked, uFpuup)
GO(__fread_unlocked_chk, uFpuuup)
GO(free, vFp)
GO(freeaddrinfo, vFp)
DATAV(__free_hook, 4)
GO(freeifaddrs, vFp)
GOW(freelocale, vFp)
GO(__freelocale, vFp)
GO(fremovexattr, iFip)
GO(freopen, pFppp)
GO(freopen64, pFppp)
// frexp    // Weak
// frexpf   // Weak
// frexpl   // Weak
GO2(fscanf, iFppV, vfscanf)
GO(fseek, iFpli)
GO(fseeko, iFpli)
GO(fseeko64, iFpIi)
GO(__fsetlocking, iFpi)
GO(fsetpos, iFpp)
GO(fsetpos64, iFpp)
GO(fsetxattr, iFippui)
GOW(fstatfs, iFip)
GOM(fstatfs64, iFip)    // weak
GO(fstatvfs, iFip)
GOW(fstatvfs64, iFip)   // alignment?
GOW(fsync, iFi)
GOW(ftell, lFp)
GO(ftello, lFp)
GO(ftello64, IFp)
GO(ftime, iFp)
GO(ftok, iFpi)
GOW(ftruncate, iFiu)
GOW(ftruncate64, iFiI)
GOW(ftrylockfile, iFp)
GOM(fts_children, pFEpi)
GOM(fts_close, iFEp)
GOM(fts_open, pFEpip)
GOM(fts_read, pFEp)
// fts_set
GOM(ftw, iFEppi)
GOM(ftw64, iFEppi)
GOW(funlockfile, vFp)
GO(futimens, iFip)
GOW(futimes, iFip) //int futimes(int fd, const struct timeval tv[2])
GO(futimesat, iFippp)
// fwide
GOM(fwprintf, iFEppVV) // Weak
GOM(__fwprintf_chk, iFEpvpVV)
GO(__fwritable, iFp)
GOW(fwrite, LFpLLp)
GO(fwrite_unlocked, uFpuup)
GO(__fwriting, iFp)
// fwscanf
GOM(__fxstat, iFEiip)
GOM(__fxstat64, iFEiip) // need reaalign of struct stat64
GOM(__fxstatat, iFEiippi)
GOM(__fxstatat64, iFEiippi) // struct stat64 again
// __gai_sigqueue
GO(gai_strerror, pFi)
// __gconv_get_alias_db
// __gconv_get_cache
// __gconv_get_modules_db
// gcvt
GO(getaddrinfo, iFpppp)
// getaliasbyname
// getaliasbyname_r
// getaliasent
// getaliasent_r
// get_avphys_pages // Weak
GOW(getc, iFp)
GOW(getchar, iFv)
GO(getchar_unlocked, iFv)
GOM(getcontext, iFEp)
GOW(getc_unlocked, iFp)
GO(get_current_dir_name, pFv)
GOW(getcwd, pFpL)
GO(__getcwd_chk, pFpLL)
GO(getdate, pFp)
// getdate_err  // type B
// getdate_r    // Weak
GOW(getdelim, iFppip)
GOW(__getdelim, iFppip)
// getdirentries
// getdirentries64
GO(getdomainname, iFpu)
// __getdomainname_chk
GOW(getdtablesize, iFv)
GOW(getegid, iFv)
GO(getenv, pFp)
GOW(geteuid, pFv)
// getfsent
// getfsfile
// getfsspec
GOW(getgid, iFv)
GO(getgrent, pFv)
// getgrent_r
GO(getgrgid, pFu)
GO(getgrgid_r, iFuppup)
GO(getgrnam, pFp)
GO(getgrnam_r, iFpppup)
GO(getgrouplist, iFpipp)
GOW(getgroups, iFiu)
// __getgroups_chk
GO(gethostbyaddr, pFpui)
GO(gethostbyaddr_r, iFpuippupp)
GO(gethostbyname, pFp)
GO(gethostbyname2, pFpi)
GO(gethostbyname2_r, iFpippupp)
GO(gethostbyname_r, iFpppupp)
GO(gethostent, pFv)
GO(gethostent_r, iFppupp)
// gethostid
GOW(gethostname, iFpu)
// __gethostname_chk
GO(getifaddrs, iFp)
// getipv4sourcefilter
GOW(getitimer, iFip)
// get_kernel_syms  // Weak
GOW(getline, iFppp)
GO(getloadavg, iFpi)
GO(getlogin, pFv)
GO(getlogin_r, iFpu)
// __getlogin_r_chk
GO(getmntent, pFp)
// __getmntent_r
GOW(getmntent_r, pFpppi)
// getmsg
// get_myaddress
GO(getnameinfo, iFpupupui)
// getnetbyaddr
// getnetbyaddr_r
// getnetbyname
// getnetbyname_r
// getnetent
// getnetent_r
// getnetgrent
// getnetgrent_r    // Weak
// getnetname
GOW(get_nprocs, iFv)
GOW(get_nprocs_conf, iFv)
GO(getopt, iFipp)
GO(getopt_long, iFipppp)
GO(getopt_long_only, iFipppp)
GOW(getpagesize, iFv)
GO(__getpagesize, iFv)
GO(getpass, pFp)
GOW(getpeername, iFipp)
GOW(getpgid, uFu)
// __getpgid
GO(getpgrp, iFv)
// get_phys_pages   // Weak
GO(getpid, uFv)
GO(__getpid, uFv)
// getpmsg
GOW(getppid, uFv)
GO(getpriority, iFii)
GOM(getrandom, iFEpuu)
GO(getprotobyname, pFp)
GO(getprotobyname_r, iFpppup)
GO(getprotobynumber, pFi)
GO(getprotobynumber_r, iFippup)
GO(getprotoent, pFv)
GO(getprotoent_r, iFppup)
GOW(getpt, iFv)
// getpublickey
// getpw    // Weak
GO(getpwent, pFv)
// getpwent_r
GO(getpwnam, pFp)
GO(getpwnam_r, iFpppup)
GO(getpwuid, pFu)
GO(getpwuid_r, iFuppup)
GOW(getresgid, iFppp)
GOW(getresuid, iFppp)
GO(getrlimit, iFip)
GO(getrlimit64, iFip)
// getrpcbyname
// getrpcbyname_r
// getrpcbynumber
// getrpcbynumber_r
// getrpcent
// getrpcent_r
// getrpcport
GOW(getrusage, iFip)
GOW(gets, pFp)
// __gets_chk
// getsecretkey
GO(getservbyname, pFpp)
GO(getservbyname_r, iFppppup)
GO(getservbyport, pFip)
GO(getservbyport_r, iFipppup)
// getservent
GO(getservent_r, iFppup)
GO(getsid, uFu)
GOW(getsockname, iFipp)
GOW(getsockopt, iFiiipp)
// getsourcefilter
GO(getspent, pFv)
// getspent_r
GO(getspnam, pFp)
// getspnam_r
// getsubopt
GOW(gettext, pFp)
GOW(gettimeofday, iFpp)
GO(__gettimeofday, iFpp)
// getttyent
// getttynam
GOW(getuid, uFv)
// getusershell
GOW(getutent, pFv)
GOW(getutent_r, iFpp)
GOW(getutid, pFp)
GOW(getutid_r, iFppp)
GOW(getutline, pFp)
GOW(getutline_r, iFppp)
// getutmp
// getutmpx
// getutxent
// getutxid
// getutxline
// getw
GO2(getwc, iFp, fgetwc)
// getwchar
GO(getwchar_unlocked, iFv)
GOW(getwc_unlocked, iFp)
GO(getwd, pFp)
// __getwd_chk
GO(getxattr, iFpppu)
GOM(glob, iFEpipp)
GOM(glob64, iFEpipp)
GO(globfree, vFp)
GO(globfree64, vFp)
// glob_pattern_p   // Weak
GO(gmtime, pFp)
GO(__gmtime_r, pFpp)
GOW(gmtime_r, pFpp)
GO(gnu_dev_major, uFU)
GO(gnu_dev_makedev, UFii)       // dev_t seems to be a u64
GO(gnu_dev_minor, uFU)
GOW(gnu_get_libc_release, pFv)
GOW(gnu_get_libc_version, pFv)
// __gnu_mcount_nc
// __gnu_Unwind_Find_exidx
GO(grantpt, iFi)
// group_member // Weak
// gsignal  // Weak
// gtty
GOW(hasmntopt, pFpp)
// hcreate
// hcreate_r
// hdestroy // Weak
// hdestroy_r
DATA(h_errlist, 4)
// h_errno  // type B
GO(__h_errno_location, pFv)
GO(herror, vFp)
// h_nerr   // type R
// host2netname
// hsearch
// hsearch_r
GO(hstrerror, pFi)
GO(htonl, uFu)
GO(htons, uFu)
GO(iconv, uFupppp)
GO(iconv_close, iFu)
GO(iconv_open, uFpp)
GO(iconv_canonicalize, pFp)
GO(if_freenameindex, vFp)
GO(if_indextoname, pFup)
GO(if_nameindex, pFv)
GO(if_nametoindex, uFp)
// imaxabs  // Weak
GOW(imaxdiv, IFII)
DATA(in6addr_any, 16)  // type R
DATA(in6addr_loopback, 16) // type R
// inb  // Weak
GOW(index, pFpi)
// inet6_opt_append
// inet6_opt_find
// inet6_opt_finish
// inet6_opt_get_val
// inet6_opt_init
// inet6_option_alloc
// inet6_option_append
// inet6_option_find
// inet6_option_init
// inet6_option_next
// inet6_option_space
// inet6_opt_next
// inet6_opt_set_val
// inet6_rth_add
// inet6_rth_getaddr
// inet6_rth_init
// inet6_rth_reverse
// inet6_rth_segments
// inet6_rth_space
GO(inet_addr, uFp)
GOW(inet_aton, iFpp)
// inet_lnaof
// inet_makeaddr
// inet_netof
GO(inet_network, iFp)
// inet_nsap_addr
GO(inet_nsap_ntoa, pFipp)
GO(inet_ntoa, pFu)
GO(inet_ntop, pFippu)
GO(inet_pton, iFipp)
GO(initgroups, iFpi)
// init_module
// initstate    // Weak
GOW(initstate_r, iFupup)
// inl  // Weak
// innetgr
GO(inotify_add_watch, iFipu)
GO(inotify_init, iFv)
GO(inotify_init1, iFi)
GO(inotify_rm_watch, iFii)
// insque
// __internal_endnetgrent
// __internal_getnetgrent_r
// __internal_setnetgrent
// inw  // Weak
DATA(_IO_2_1_stderr_, 152)  //sizeof(struct _IO_FILE_plus)
DATA(_IO_2_1_stdin_, 152)
DATA(_IO_2_1_stdout_, 152)
GO(_IO_adjust_column, uFupi)
// _IO_adjust_wcolumn
GO(ioctl, iFiLN)   //the vararg is just to have optional arg of various type, but only 1 arg
GO(_IO_default_doallocate, iFS)
GO(_IO_default_finish, vFSi)
GO(_IO_default_pbackfail, iFSi)
GO(_IO_default_uflow, iFS)
GO(_IO_default_xsgetn, uFSpu)
GO(_IO_default_xsputn, uFSpu)
GO(_IO_doallocbuf, vFS)
GO(_IO_do_write, iFSpu)
// _IO_fclose
// _IO_fdopen
// _IO_feof
// _IO_ferror
// _IO_fflush
// _IO_fgetpos
// _IO_fgetpos64
// _IO_fgets
GO(_IO_file_attach, pFSi)
GO(_IO_file_close, iFS)
GO(_IO_file_close_it, iFS)
GO(_IO_file_doallocate, iFS)
// _IO_file_finish
GO(_IO_file_fopen, pFSppi)
GO(_IO_file_init, vFS)
DATA(_IO_file_jumps, 4)
GO(_IO_file_open, pFSpiiii)
GO(_IO_file_overflow, iFSi)
GO(_IO_file_read, iFSpi)
GO(_IO_file_seek, IFSIi)
GO(_IO_file_seekoff, IFSIii)
GO(_IO_file_setbuf, pFSpi)
GOM(_IO_file_stat, iFESp)
GO(_IO_file_sync, iFS)
GO(_IO_file_underflow, iFS)
GO(_IO_file_write, iFSpi)
GO(_IO_file_xsputn, uFSpu)
GO(_IO_flockfile, vFS)
GO(_IO_flush_all, iFv)
GO(_IO_flush_all_linebuffered, vFv)
// _IO_fopen
// _IO_fprintf  // Weak
// _IO_fputs
// _IO_fread
GO(_IO_free_backup_area, vFS)
// _IO_free_wbackup_area
// _IO_fsetpos
// _IO_fsetpos64
// _IO_ftell
// _IO_ftrylockfile
GO(_IO_funlockfile, vFS)
// _IO_fwrite
GO(_IO_getc, iFS)
// _IO_getline
GO(_IO_getline_info, uFSpuiip)
// _IO_gets
GO(_IO_init, vFSi)
GO(_IO_init_marker, vFpS)
// _IO_init_wmarker
// _IO_iter_begin
// _IO_iter_end
// _IO_iter_file
// _IO_iter_next
// _IO_least_wmarker
GO(_IO_link_in, vFp)
DATA(_IO_list_all, 4)
// _IO_list_lock
// _IO_list_resetlock
// _IO_list_unlock
GO(_IO_marker_delta, iFp)
GO(_IO_marker_difference, iFpp)
GO(_IO_padn, iFpii)
GO(_IO_peekc_locked, iFp)
GOW(ioperm, iFuui)
GOW(iopl, iFi)
// _IO_popen
// _IO_printf
GO(_IO_proc_close, iFS)
GO(_IO_proc_open, pFSpp)
GO(_IO_putc, iFip)
// _IO_puts
GO(_IO_remove_marker, vFp)
GO(_IO_seekmark, iFSpi)
GO(_IO_seekoff, IFSIii)
GO(_IO_seekpos, IFSIi)
// _IO_seekwmark
GO(_IO_setb, vFSppi)
// _IO_setbuffer
// _IO_setvbuf
GO(_IO_sgetn, uFppu)
// _IO_sprintf
GO(_IO_sputbackc, iFSi)
// _IO_sputbackwc
// _IO_sscanf
GO(_IO_str_init_readonly, vFppi)
GO(_IO_str_init_static, vFppup)
GO(_IO_str_overflow, iFSi)
GO(_IO_str_pbackfail, iFSi)
GO(_IO_str_seekoff, UFSUii)
GO(_IO_str_underflow, iFS)
GO(_IO_sungetc, iFp)
// _IO_sungetwc
GO(_IO_switch_to_get_mode, iFp)
// _IO_switch_to_main_wget_area
// _IO_switch_to_wbackup_area
// _IO_switch_to_wget_mode
// _IO_ungetc
GO(_IO_un_link, vFp)
GO(_IO_unsave_markers, vFp)
// _IO_unsave_wmarkers
GOM(_IO_vfprintf, iFEpppp)
GOM(_IO_vfscanf, iFEppp)
// _IO_vsprintf
// _IO_wdefault_doallocate
// _IO_wdefault_finish
// _IO_wdefault_pbackfail
// _IO_wdefault_uflow
// _IO_wdefault_xsgetn
// _IO_wdefault_xsputn
// _IO_wdoallocbuf
// _IO_wdo_write
DATA(_IO_wfile_jumps, 4)
// _IO_wfile_overflow
// _IO_wfile_seekoff
// _IO_wfile_sync
// _IO_wfile_underflow
// _IO_wfile_xsputn
// _IO_wmarker_delta
// _IO_wsetb
// iruserok
// iruserok_af
GO(isalnum, iFi)
// __isalnum_l
// isalnum_l    // Weak
GO(isalpha, iFi)
// __isalpha_l
// isalpha_l    // Weak
GO(isascii, iFi)
// __isascii_l  // Weak
// isastream
GOW(isatty, iFi)
GO(isblank, iFi)
// __isblank_l
// isblank_l    // Weak
GO(iscntrl, iFi)
// __iscntrl_l
// iscntrl_l    // Weak
// isctype  // Weak
// __isctype
GO(isdigit, iFi)
// __isdigit_l
// isdigit_l    // Weak
// isfdtype
GO(isgraph, iFi)
// __isgraph_l
// isgraph_l    // Weak
GOW(isinf, iFd)
GO(__isinf, iFd)
GOW(isinff, iFf)
GO(__isinff, iFf)
// isinfl   // Weak
// __isinfl
GO(islower, iFi)
// __islower_l
// islower_l    // Weak
GOW(isnan, iFd)
GO(__isnan, iFd)
GOW(isnanf, iFf)
GO(__isnanf, iFf)
// isnanl   // Weak
// __isnanl
#ifdef POWERPCLE
GOM(__isoc99_fscanf, iFppV)
// __isoc99_fwscanf
// __isoc99_scanf
GOM(__isoc99_sscanf, iFppV)
// __isoc99_swscanf
GOM(__isoc99_vfscanf, iFppp)
// __isoc99_vfwscanf
// __isoc99_vscanf
GOM(__isoc99_vsscanf, iFppp) // TODO: check if ok
// __isoc99_vswscanf
// __isoc99_vwscanf
// __isoc99_wscanf
#else
GO2(__isoc99_fscanf, iFppV, __isoc99_vfscanf)
// __isoc99_fwscanf
// __isoc99_scanf
GO2(__isoc99_sscanf, iFppV, __isoc99_vsscanf)
// __isoc99_swscanf
GOM(__isoc99_vfscanf, iFppp)
// __isoc99_vfwscanf
// __isoc99_vscanf
GO(__isoc99_vsscanf, iFppp)
// __isoc99_vswscanf
// __isoc99_vwscanf
// __isoc99_wscanf
#endif
GO(isprint, iFi)
// __isprint_l
// isprint_l    // Weak
GO(ispunct, iFi)
// __ispunct_l
// ispunct_l    // Weak
GO(isspace, iFi)
// __isspace_l
// isspace_l    // Weak
GO(isupper, iFi)
// __isupper_l
// isupper_l    // Weak
GOW(iswalnum, iFi)
// __iswalnum_l
GOW(iswalnum_l, iFip)
GOW(iswalpha, iFi)
// __iswalpha_l
GOW(iswalpha_l, iFip)
GOW(iswblank, iFi)
// __iswblank_l
GOW(iswblank_l, iFip)
GOW(iswcntrl, iFi)
// __iswcntrl_l
GOW(iswcntrl_l, iFip)
GOW(iswctype, iFiu)
// __iswctype
GO(__iswctype_l, iFiup)
// iswctype_l   // Weak
GOW(iswdigit, iFi)
// __iswdigit_l
GOW(iswdigit_l, iFip)
GOW(iswgraph, iFi)
// __iswgraph_l
GOW(iswgraph_l, iFip)
GOW(iswlower, iFi)
// __iswlower_l
GOW(iswlower_l, iFip)
GOW(iswprint, iFi)
// __iswprint_l
GOW(iswprint_l, iFip)
GOW(iswpunct, iFi)
// __iswpunct_l
GOW(iswpunct_l, iFip)
GOW(iswspace, iFi)
// __iswspace_l
GOW(iswspace_l, iFip)
GOW(iswupper, iFi)
// __iswupper_l
GOW(iswupper_l, iFip)
GOW(iswxdigit, iFi)
// __iswxdigit_l
GOW(iswxdigit_l, iFip)
GO(isxdigit, iFi)
// __isxdigit_l
// isxdigit_l   // Weak
// _itoa_lower_digits   // type R
// __ivaliduser
GO(jrand48, iFp)
// jrand48_r    // Weak
// key_decryptsession
// key_decryptsession_pk
// __key_decryptsession_pk_LOCAL    // type B
// key_encryptsession
// key_encryptsession_pk
// __key_encryptsession_pk_LOCAL    // type B
// key_gendes
// __key_gendes_LOCAL   // type B
// key_get_conv
// key_secretkey_is_set
// key_setnet
// key_setsecret
GOW(kill, iFii)
GO(killpg, iFii)
// klogctl
// l64a
GO(labs, iFi)
// lchmod
GOW(lchown, iFpuu)
// lckpwdf  // Weak
// lcong48
// lcong48_r    // Weak
// ldexp    // Weak
// ldexpf   // Weak
// ldexpl   // Weak
GOS(ldiv, pFEpii)     // return a struct, so address of stuct is on the stack, as a shadow 1st element
GOM(lfind, pFEpppup)
GO(lgetxattr, iFpppu)
GOM(__libc_alloca_cutoff, iFEL)
// __libc_allocate_rtsig
// __libc_allocate_rtsig_private
GO(__libc_calloc, pFLL)
// __libc_clntudp_bufcreate
GO(__libc_current_sigrtmax, iFv)
// __libc_current_sigrtmax_private
GO(__libc_current_sigrtmin, iFv)
// __libc_current_sigrtmin_private
GOM(__libc_dlclose, iFEp)
// __libc_dl_error_tsd
GOM(__libc_dlopen_mode, pFEpi)
GOM(__libc_dlsym, pFEpp)
// __libc_fatal
// __libc_fork
GO(__libc_free, vFp)
// __libc_freeres
GOM(__libc_init_first, vFEipV)
// _libc_intl_domainname    // type R
// __libc_longjmp
// __libc_mallinfo
GO(__libc_malloc, pFL)
// __libc_mallopt
GO(__libc_memalign, pFLL)
// __libc_pthread_init
GO(__libc_pvalloc, pFL)
// __libc_pwrite
GO(__libc_realloc, pFpL)
// __libc_sa_len
// __libc_siglongjmp
GOM(__libc_start_main, iFEpippppp)
// __libc_system
// __libc_thread_freeres
GO(__libc_valloc, pFL)
GOW(link, iFpp)
GO(linkat, iFipipi)
GOW(listen, iFii)
GO(listxattr, iFppu)
// llabs
// lldiv
GO(llistxattr, iFppu)
// llseek   // Weak
// loc1 // type B
// loc2 // type B
GOW(localeconv, pFv)
GO(localtime, pFp)
GOW(localtime_r, pFpp)
GO(lockf, iFiiu)
GO(lockf64, iFiiI)
// locs // type B
GOM(longjmp, vFEpi)
GOM(_longjmp, vFEpi)
GOM(__longjmp_chk, vFEpi)
GO(lrand48, iFv)
// lrand48_r
GO(lremovexattr, iFpp)
GOM(lsearch, pFEpppup)
GOW(lseek, iFiii)
// __lseek  // Weak
GOW(lseek64, IFiIi)
GO(lsetxattr, iFpppui)
GO(lutimes, iFpp)
GOM(__lxstat, iFEipp)
GOM(__lxstat64, iFEipp)
GO(madvise, iFpLi)
GOM(makecontext, iFEppiV)
GOW(mallinfo, pFv)
#ifdef NOALIGN
GO(malloc, pFL)
#else
GOM(malloc, pFL)
#endif
// malloc_get_state // Weak
DATAV(__malloc_hook, 4)
DATAV(__malloc_initialize_hook, 4)
// malloc_set_state // Weak
// malloc_stats // Weak
GOW(malloc_trim, iFu)
GOW(malloc_usable_size, LFp)
GOW(mallopt, iFii)  // Weak
// mallwatch    // type B
GO(mblen, iFpu)
GOW(mbrlen, uFpup)
GO(__mbrlen, uFpup)
GOW(mbrtowc, uFppup)
GO(__mbrtowc, uFppup)
GOW(mbsinit, iFp)
GOW(mbsnrtowcs, LFppLLp)
// __mbsnrtowcs_chk
GOW(mbsrtowcs, uFppup)
// __mbsrtowcs_chk
GO(mbstowcs, uFppu)
// __mbstowcs_chk
GO(mbtowc, iFppu)
// mcheck
// mcheck_check_all
// mcheck_pedantic
// _mcleanup
GOM(mcount, vFpp)   // Weak
// _mcount
GOW(memalign, pFuu)
DATAV(__memalign_hook, 4)
GOW(memccpy, pFppiL)
GO(memchr, pFpiu)
GO(memcmp, iFppu)
GO(memcpy, pFppu)
GO(__memcpy_chk, pFppuu)
// memfrob
GO(memmem, pFpupu)
GO(memmove, pFppL)
GO(__memmove_chk, pFppuu)
GO(mempcpy, pFppL)
GO(__mempcpy, pFppu)
// __mempcpy_chk
// __mempcpy_small
GOW(memrchr, pFpiu)
GO(memset, pFpiL)
GO(__memset_chk, pFpiuu)
GO(mincore, iFpup)
GOW(mkdir, iFpu)
GO(mkdirat, iFipu)
GO(mkdtemp, pFp)
GO(mkfifo, iFpu)
GO(mkfifoat, iFipu)
GO(mkostemp, iFpi)
GO(mkostemp64, iFpi)
GO(mkstemp, iFp)
GO(mkstemp64, iFp)
GO(mktemp, pFp)
GO(mktime, uFp)
GO(mlock, iFpL)
GO(mlockall, iFi)
GOM(mmap, pFEpLiiii)
GOM(mmap64, pFEpLiiiI)
// modf // Weak
// modff    // Weak
// modfl    // Weak
// moncontrol   // Weak
// monstartup   // Weak
// __monstartup
DATA(__morecore, 4)
GOW(mount, iFpppup)
// mprobe
GOM(mprotect, iFEpLi)
// mrand48
// mrand48_r
GOW(mremap, pFpuuip)	// 5th hidden paramerer "void* new_addr" if flags is MREMAP_FIXED // does this need some GOM for protection handling?
GO(msgctl, iFiip)
GOW(msgget, iFpi)
GOW(msgrcv, lFipLli)
GOW(msgsnd, iFipLi)
GOW(msync, iFpLi)
// mtrace
GO(munlock, iFpL)
GO(munlockall, iFv)
GOM(munmap, iFEpL)
// muntrace
GOM(nanosleep, iFpp)	// weak
// __nanosleep  // Weak
// netname2host
// netname2user
GOW(newlocale, pFipp)
GO(__newlocale, pFipp)
// nfsservctl
GOM(nftw, iFEppii)
GOM(nftw64, iFEppii)
GOW(ngettext, pFppu)
GO(nice, iFi)
// _nl_default_dirname  // type R
// _nl_domain_bindings  // type B
GO(nl_langinfo, pFu)
GO(__nl_langinfo_l, pFup)
GOW(nl_langinfo_l, pFup)
DATAB(_nl_msg_cat_cntr, 4) // type B
// nrand48
// nrand48_r    // Weak
// __nss_configure_lookup
// __nss_database_lookup
// __nss_disable_nscd
// _nss_files_parse_grent
// _nss_files_parse_pwent
// _nss_files_parse_spent
// __nss_group_lookup
// __nss_group_lookup2
// __nss_hostname_digits_dots
// __nss_hosts_lookup
// __nss_hosts_lookup2
// __nss_lookup_function
// __nss_next
// __nss_next2
// __nss_passwd_lookup
// __nss_passwd_lookup2
// __nss_services_lookup2
GOW(ntohl, uFu)
GOW(ntohs, uFu)
// ntp_adjtime  // Weak
// ntp_gettime
// _null_auth   // type B
// _obstack_allocated_p
DATAM(obstack_alloc_failed_handler, 4)
GOM(_obstack_begin, iFpLLpp)
// _obstack_begin_1
DATA(obstack_exit_failure, 4)
GOM(_obstack_free, vFpp)
GOM(obstack_free, vFpp)
// _obstack_memory_used
GOM(_obstack_newchunk, vFpi)
// obstack_printf   // Weak
// __obstack_printf_chk
GOM(obstack_vprintf, iFEpppp)  // Weak
// __obstack_vprintf_chk
// on_exit  // Weak
GOM(open, iFEpOu)    //Weak
GOM(__open, iFEpOu) //Weak
GO(__open_2, iFpO)
GOM(open64, iFEpOu) //Weak
// __open64 // Weak
GO(__open64_2, iFpO)
GOW(openat, iFipOu)
// __openat_2
GOW(openat64, iFipOuuuuu)   // variable arg...
GO(__openat64_2, iFipOuuuuu)
// __open_catalog
GOW(opendir, pFp)
GO(openlog, vFpii)
// open_memstream
// open_wmemstream
DATAB(optarg, 4)
DATA(opterr, 4)
DATA(optind, 4)
DATA(optopt, 4)
// outb // Weak
// outl // Weak
// outw // Weak
GO(__overflow, iFpi)
GO(parse_printf_format, uFpup)
// passwd2des
GOW(pathconf, iFpi)
GOW(pause, iFv)
GO(pclose, iFp)
GO(perror, vFp)
// personality  // Weak
GOW(pipe, iFp)  // the array of 2 int seems to converted as a pointer, on both x86 and arm (and x86_64 too)
// __pipe
GOW(pipe2, iFpO) // assuming this works the same as pipe, so pointer for array of 2 int
// pivot_root
// pmap_getmaps
// pmap_getport
// pmap_rmtcall
// pmap_set
// pmap_unset
GOW(poll, iFpui)    // poll have an array of struct as 1st argument
GO(__poll, iFpui)
GO(popen, pFpp)
GO(posix_fadvise, iFiuui)
GO(posix_fadvise64, iFiuui)
GO(posix_fallocate, iFiii)
GO(posix_fallocate64, iFiII)
// posix_madvise
GOW(posix_memalign, iFpuu)
// posix_openpt // Weak
GO(posix_spawn, iFpppppp)
// posix_spawnattr_destroy
// posix_spawnattr_getflags
// posix_spawnattr_getpgroup
// posix_spawnattr_getschedparam
// posix_spawnattr_getschedpolicy
// posix_spawnattr_getsigdefault
// posix_spawnattr_getsigmask
// posix_spawnattr_init
// posix_spawnattr_setflags
// posix_spawnattr_setpgroup
// posix_spawnattr_setschedparam
// posix_spawnattr_setschedpolicy
// posix_spawnattr_setsigdefault
// posix_spawnattr_setsigmask
// posix_spawn_file_actions_addclose
GO(posix_spawn_file_actions_adddup2, iFpii)
GO(posix_spawn_file_actions_addopen, iFpipii)
GO(posix_spawn_file_actions_destroy, iFp)
GO(posix_spawn_file_actions_init, iFp)
GOM(posix_spawnp, iFEpppppp)
GO(ppoll, iFpupp)
GOW(prctl, iFiLLLL)
GOW(pread, lFipLl)
GOW(pread64, lFipLI)
// __pread64    // Weak
// __pread64_chk
GOM(preadv64, lFEipiI)  // not always present
// __pread_chk
GOM(printf, iFEpVV)
GOM(__printf_chk, iFEvpVV)
GO(__printf_fp, iFppp)  // does this needs aligment?
// printf_size
// printf_size_info
// profil   // Weak
// __profile_frequency
DATAM(__progname, 4)
DATAM(__progname_full, 4)
DATAM(program_invocation_name, 4)
DATAM(program_invocation_short_name, 4)
GOW(pselect, iFippppp)
// psignal
GO(ptrace, iFiupp)  // will that work???
GO(ptsname, pFi)
GOW(ptsname_r, iFipu)
// __ptsname_r_chk
GOW(putc, iFip)
GO(putchar, iFi)
GO(putchar_unlocked, iFi)
GO(putc_unlocked, iFip)
GO(putenv, iFp)
// putgrent
// putmsg
// putpmsg
// putpwent
GOW(puts, iFp)
// putspent
GOW(pututline, pFp)
// pututxline
// putw
GO(putwc, iFip)
// putwchar
GO(putwchar_unlocked, iFi)
GO(putwc_unlocked, iFip)
// pvalloc  // Weak
// pwrite   // Weak
GOW(pwrite64, lFipLI)
// __pwrite64   // Weak
GOM(pwritev64, lFEipiI)  // not always present
// qecvt
#ifdef HAVE_LD80BITS
GO(qecvt_r, iFDipppL)
#else
GO(qecvt_r, iFKipppL)
#endif
// qfcvt
#ifdef HAVE_LD80BITS
GO(qfcvt_r, iFDipppL)
#else
GO(qfcvt_r, iFKipppL)
#endif
// qgcvt
GOM(qsort, vFEpLLp)
GOM(qsort_r, vFEpLLpp)
// query_module // Weak
GO(quotactl, iFipip)
GO(raise, iFi)  // will need a GOM version once signal are implemented probably
GO(rand, iFv)
GOW(random, iFv)
GOW(random_r, iFpp)
GO(rand_r, iFp)
GOW(rawmemchr, pFpi)
GO(__rawmemchr, pFpi)
// rcmd
// rcmd_af
// __rcmd_errstr    // type B
GOM(read, lFipL)
GOW(__read, lFipL)
// readahead    // Weak
GO(__read_chk, lFipLL)
GOM(readdir, pFEp)  // should also be weak
GO(readdir64, pFp)  // check if alignement is correct
// readdir64_r
GOM(readdir_r, iFEppp)  // should also be weak
GOM(readlink, iFEppu)
GO(readlinkat, iFippu)
// __readlinkat_chk
// __readlink_chk
GO(readv, iFipi)
GO(realloc, pFpL)
DATAV(__realloc_hook, 4)
GOM(realpath, pFEpp)
GO(__realpath_chk, pFppu)
// reboot
// re_comp  // Weak
// re_compile_fastmap   // Weak
GOW(re_compile_pattern, pFpup)
GO(recv, lFipLi)
GO(__recv_chk, iFipuui)
GOW(recvfrom, lFipLipp)
// __recvfrom_chk
GOM(recvmmsg, iFEipuup)    // actual recvmmsg is glibc 2.12+. The syscall is Linux 2.6.33+, so use syscall...
GOW(recvmsg, lFipi)
// re_exec  // Weak
GOW(regcomp, iFppi)
GOW(regerror, uFippu)
GO(regexec, iFppupi)
GOW(regfree, vFp)
GOM(__register_atfork, iFEpppp)
// register_printf_function // Weak
// registerrpc
// remap_file_pages // Weak
GOW(re_match, iFppiip)
// re_match_2   // Weak
GO(remove, iFp)
GO(removexattr, iFpp)
// remque
GO(rename, iFpp)
GO(renameat, iFipip)
#ifdef PANDORA
GOM(renameat2, iFipipu)
#else
GO(renameat2, iFipipu)
#endif
// _res // type B
GOW(re_search, iFppiiip)
GOW(re_search_2, iFppipiiipi)
// re_set_registers // Weak
GOW(re_set_syntax, uFu)
// _res_hconf   // type B
GO(__res_iclose, vFpi)
GO(__res_init, iFv)
GO(__res_maybe_init, iFpi)
GO(__res_nclose, vFp)
GO(__res_ninit, iFp)
DATA(__resp, 4)
// __res_randomid
GO(__res_state, pFv)
DATA(re_syntax_options, 4)    // type B
// revoke
GO(rewind, vFp)
GO(rewinddir, vFp)
// rexec
// rexec_af
// rexecoptions // type B
GOW(rindex, pFpi)
GOW(rmdir, iFp)
GO(readdir64_r, iFppp)  // is this present?
// rpc_createerr    // type B
// _rpc_dtablesize
// __rpc_thread_createerr
// __rpc_thread_svc_fdset
// __rpc_thread_svc_max_pollfd
// __rpc_thread_svc_pollfd
GO(rpmatch, iFp)
// rresvport
// rresvport_af
// rtime
// ruserok
// ruserok_af
// ruserpass
GOW(sbrk, pFl)
GO(__sbrk, pFl)
// scalbn   // Weak
// scalbnf  // Weak
// scalbnl  // Weak
GOM(scandir, iFEpppp)
GOM(scandir64, iFEpppp)
GO2(scanf, iFpp, vscanf)
GO(__sched_cpualloc, pFu)   //TODO: check, return cpu_set_t* : should this be aligned/changed?
GO(__sched_cpucount, iFup)
GO(__sched_cpufree, vFp)
GO(sched_getaffinity, iFiup)
// sched_getcpu
GO(__sched_getparam, iFip)
GOW(sched_getparam, iFip)
GO(__sched_get_priority_max, iFi)
GOW(sched_get_priority_max, iFi)
GO(__sched_get_priority_min, iFi)
GOW(sched_get_priority_min, iFi)
GO(__sched_getscheduler, iFi)
GOW(sched_getscheduler, iFi)
GOW(sched_rr_get_interval, iFip)
GO(sched_setaffinity, iFiup)
GOW(sched_setparam, iFip)
GO(__sched_setscheduler, iFiip)
GOW(sched_setscheduler, iFiip)
GO(__sched_yield, iFv)
GOW(sched_yield, iFv)
GO2(__secure_getenv, pFp, getenv)   //__secure_getenv not always defined
GO2(secure_getenv, pFp, getenv)     // secure_getenv either
// seed48
// seed48_r // Weak
GO(seekdir, vFpi)
GOW(select, iFipppp)
GO(__select, iFipppp)
#ifdef POWERPCLE
GOM(semctl, iFEiiiN)  // use vararg after the 3 i
#else
GO(semctl, iFiiiN)
#endif
GOW(semget, iFuii)
GOW(semop, iFipL)
GO(semtimedop, iFipup)
GOW(send, lFipLi)
// __send   // Weak
GO(sendfile, lFiipL)
GO(sendfile64, lFiipL)
GOW(sendmsg, lFipi)
GOM(__sendmmsg, iFEipuu)    // actual __sendmmsg is glibc 2.14+. The syscall is Linux 3.0+, so use syscall...
GOW(sendto, iFipuipu)
// setaliasent
GOW(setbuf, vFpp)
GOW(setbuffer, vFppL)
GOM(setcontext, iFEp)
// setdomainname
GO(setegid, iFu)
GOW(setenv, iFppi)
// _seterr_reply
GO(seteuid, iFu)
// setfsent
// setfsgid
// setfsuid
GOW(setgid, iFu)
GO(setgrent, vFv)
GO(setgroups, iFup)
GO(sethostent, vFi)
// sethostid
GO(sethostname, iFpu)
// setipv4sourcefilter
GOW(setitimer, iFipp)
GOM(setjmp, iFEp)
GOM(_setjmp, iFEp)
GO(setlinebuf, vFp)
GO(setlocale, pFip)
// setlogin
GO(setlogmask, iFi)
GOW(setmntent, pFpp)
// __setmntent
// setnetent
// setnetgrent
GOW(setpgid, iFuu)
// __setpgid
GO(setpgrp, iFv)
GO(setpriority, iFiii)
GO(setprotoent, vFi)
GO(setpwent, vFv)
GOW(setregid, iFuu)
GOW(setresgid, iFuuu)
GOW(setresuid, iFuuu)
GOW(setreuid, iFuu)
GO(setrlimit, iFip)
GO(setrlimit64, iFip)
// setrpcent
// setservent
GOW(setsid, iFv)
GOW(setsockopt, iFiiipu)
// setsourcefilter
GO(setspent, vFv)
// setstate // Weak
GOW(setstate_r, iFpp)
GOW(settimeofday, iFpp)
// setttyent
GOW(setuid, iFu)
// setusershell
GOW(setutent, vFv)
// setutxent
GOW(setvbuf, iFppiu)
GO(setxattr, iFpppui)
// sgetspent
// sgetspent_r  // Weak
GOW(shmat, pFipi)
GOW(shmctl, iFiip)
GOW(shmdt, iFp)
GOW(shmget, iFuui)
GOW(shutdown, iFii)
GOM(sigaction, iFEipp)    // Weak
GOM(__sigaction, iFEipp)  // Weak
GO(sigaddset, iFpi)
// __sigaddset
GOM(sigaltstack, iFEpp) // Weak
// sigandset
GOW(sigblock, iFi)
GO(sigdelset, iFpi)
// __sigdelset
GO(sigemptyset, iFp)
GO(sigfillset, iFp)
GO(siggetmask, iFv)
// sighold
// sigignore
GO(siginterrupt, iFii)  // no need to wrap this one?
// sigisemptyset
GO(sigismember, iFpi)
// __sigismember
GOM(siglongjmp, pFEip)
GOM(signal, pFEip)   // Weak
// signalfd
GO(__signbit, iFd)
GO(__signbitf, iFf)
// sigorset
// sigpause // Weak
// __sigpause
GO(sigpending, iFp)
GOW(sigprocmask, iFipp)
// sigqueue // Weak
// sigrelse
// sigreturn    // Weak
GOM(sigset, pFEip)
GOM(__sigsetjmp, iFEp)
GOW(sigsetmask, iFi)
// sigstack
GOW(sigsuspend, iFp)
// __sigsuspend
GOW(sigtimedwait, iFppp)
GOW(sigvec, iFipp)
GOW(sigwait, iFpp)
GOW(sigwaitinfo, iFpp)
GOW(sleep, uFu)
GOM(snprintf, iFEpupVV)
GOM(__snprintf_chk, iFEpuvvpVV)
GOM(__snprintf, iFEpupVV)
// sockatmark
GOW(socket, iFiii)
GOW(socketpair, iFiiip)
GO(splice, iFipipuu)
GOM(sprintf, iFEppVV)
GOM(__sprintf_chk, iFEpvvpVV)
// sprofil  // Weak
GOW(srand, vFu)
GO(srand48, vFi)
// srand48_r    // Weak
GOW(srandom, vFu)
GOW(srandom_r, iFup)
#ifdef POWERPCLE
GOM(sscanf, iFppV)
#else
GO2(sscanf, iFppV, vsscanf)     // sscanf va_list is only pointer, no realign to do
#endif
// ssignal  // Weak
// sstk
GOM(__stack_chk_fail, vFE)
GOW(statfs, iFpp)
// __statfs
GOM(statfs64, iFpp)     //is weak
GO(statvfs, iFpp)
GOW(statvfs64, iFpp)    // is alignment ok?
DATA(stderr, 4)
DATA(stdin, 4)
DATA(stdout, 4)
// step // Weak
// stime
GO(stpcpy, pFpp)
// __stpcpy
GO(__stpcpy_chk, pFppu)
// __stpcpy_small
GOW(stpncpy, pFppu)
GO(__stpncpy, pFppu)
GO(__stpncpy_chk, pFppuu)
GOW(strcasecmp, iFpp)
GO(__strcasecmp, iFpp)
// __strcasecmp_l
// strcasecmp_l // Weak
GOW(strcasestr, pFpp)
GO(__strcasestr, pFpp)
GO(strcat, pFpp)
GO(__strcat_chk, pFppu)
GO(strchr, pFpi)
GOW(strchrnul, pFpi)
GO(strcmp, iFpp)
GO(strcoll, iFpp)
GO(__strcoll_l, iFppp)
GOW(strcoll_l, iFppp)
GO(strcpy, pFpp)
GO(__strcpy_chk, pFppu)
// __strcpy_small
GO(strcspn, LFpp)
// __strcspn_c1
// __strcspn_c2
// __strcspn_c3
GOW(strdup, pFp)
GO(__strdup, pFp)
GO(strerror, pFi)
GO(strerror_l, pFip)
GO(__strerror_r, pFipu)
GOW(strerror_r, pFipu)
GO(strfmon, lFpLpppppppppp) //vaarg, probably needs align, there are just double...
// __strfmon_l
// strfmon_l    // Weak
// strfry
GO(strftime, LFpLpp)
GO(__strftime_l, LFpLpp)
GOW(strftime_l, uFpupp)
GO(strlen, LFp)
GOW(strncasecmp, iFppu)
// __strncasecmp_l
// strncasecmp_l    // Weak
GO(strncat, pFppu)
GO(__strncat_chk, pFppuu)
GO(strncmp, iFppu)
GO(strncpy, pFppu)
GO(__strncpy_chk, pFppuu)
GOW(strndup, pFpu)
GO(__strndup, pFpu)
GO(strnlen, uFpu)
GO(strpbrk, pFpp)
// __strpbrk_c2
// __strpbrk_c3
GO(strptime, pFppp)
// strptime_l   // Weak
GO(strrchr, pFpi)
GOW(strsep, pFpp)
// __strsep_1c
// __strsep_2c
// __strsep_3c
// __strsep_g
GO(strsignal, pFi)
GO(strspn, uFpp)
// __strspn_c1
// __strspn_c2
// __strspn_c3
GO(strstr, pFpp)
GO(strtod, dFpp)
GO(__strtod_internal, dFppi)
GO(__strtod_l, dFppp)
GOW(strtod_l, dFppu)
GO(strtof, fFpp)
GO(__strtof_internal, fFppp)
GO(__strtof_l, fFppp)
GOW(strtof_l, fFppu)
GO(strtoimax, IFppi)
GO(strtok, pFpp)
GO(__strtok_r, pFppp)
GOW(strtok_r, pFppp)
// __strtok_r_1c
GO(strtol, lFppi)
GO(strtold, DFpp)
#ifdef HAVE_LD80BITS
GO(__strtold_internal, DFppi)
GO(__strtold_l, DFppip)
GOW(strtold_l, DFppu)
#else
GO2(__strtold_internal, KFppi, __strtod_internal)
GO2(__strtold_l, KFppip, __strtod_l)
GO2(strtold_l, KFppu, strtod_l)
#endif
GO(__strtol_internal, lFppi)
GO(strtoll, IFppi)
GO(__strtol_l, lFppiip)
GOW(strtol_l, lFppiip)
GO(__strtoll_internal, IFppii)
GO(__strtoll_l, IFppip)
GOW(strtoll_l, IFppip)
GOW(strtoq, IFppi)  // is that ok?
GO(strtoul, LFppi)
GO(__strtoul_internal, LFppii)
GO(strtoull, UFppi)
GO(__strtoul_l, uFppip)
GOW(strtoul_l, LFppip)
GO(__strtoull_internal, UFppii)
GO(__strtoull_l, UFppip)
GOW(strtoull_l, UFppip)
GO(strtoumax, UFppi)
GOW(strtouq, UFppi) // ok?
GOW(strverscmp, iFpp)
// __strverscmp
GO(strxfrm, uFppu)
GO(__strxfrm_l, uFppup)
GO(strxfrm_l, uFppup)
// stty
// svcauthdes_stats // type B
// svcerr_auth
// svcerr_decode
// svcerr_noproc
// svcerr_noprog
// svcerr_progvers
// svcerr_systemerr
// svcerr_weakauth
// svc_exit
// svcfd_create
// svc_fdset    // type B
// svc_getreq
// svc_getreq_common
// svc_getreq_poll
// svc_getreqset
// svc_max_pollfd   // type B
// svc_pollfd   // type B
// svcraw_create
// svc_register
// svc_run
// svc_sendreply
// svctcp_create
// svcudp_bufcreate
// svcudp_create
// svcudp_enablecache
// svcunix_create
// svcunixfd_create
// svc_unregister
GO(swab, vFppi)
GOM(swapcontext, iFEpp)
// swapoff  // Weak
// swapon   // Weak
GOM(swprintf, iFEpupV)
GOM(__swprintf_chk, iFEpuiupV)
GO2(swscanf, iFppV, vswscanf)     // swscanf va_list is only pointer, no realign to do
GOW(symlink, iFpp)
GO(symlinkat, iFpip)
GO(sync, vFv)
GO(syncfs, iFi)
// sync_file_range
GOM(syscall, uFE)
GOW(sysconf, lFi)
GO(__sysconf, lFi)
// sysctl   // Weak
// __sysctl
DATA(_sys_errlist, 4)
DATA(sys_errlist, 4)
GO(sysinfo, iFp)
GO2(syslog, vFiV, vsyslog)
GO2(__syslog_chk, vFiipV, __vsyslog_chk)
DATA(_sys_nerr, 4)    // type R
DATA(sys_nerr, 4) // type R
DATA(sys_sigabbrev, 4)
DATA(_sys_siglist, 4)
DATA(sys_siglist, 4)
GOW(system, iFp)    // Need to wrap to use box86 if needed?
GOM(__sysv_signal, pFip)
GOM(sysv_signal, pFip)  // Weak
GOW(tcdrain, iFi)
GO(tcflow, iFii)
GO(tcflush, iFii)
GOW(tcgetattr, iFip)
GO(tcgetpgrp, iFi)
// tcgetsid
GO(tcsendbreak, iFii)
GO(tcsetattr, iFiip)
GO(tcsetpgrp, iFii)
// tdelete  // Weak
// tdestroy // Weak
// tee
GO(telldir, iFp)
GO(tempnam, pFpp)
GOW(textdomain, pFp)
// tfind    // Weak
GO(time, uFp)
GO(timegm, uFp)
// timelocal    // Weak
GO(timerfd_create, iFii)
GO(timerfd_gettime, iFip)
GO(timerfd_settime, iFiipp)
GOW(times, iFp)
DATAV(timezone, 4)
DATAB(__timezone, 4)   // type B
GO(tmpfile, pFv)
GO(tmpfile64, pFv)
GO(tmpnam, pFp)
GO(tmpnam_r, pFp)
GO(toascii, iFi)
// __toascii_l  // Weak
GO(tolower, iFi)
// _tolower
// __tolower_l
GOW(tolower_l, iFip)
GO(toupper, iFi)
// _toupper
// __toupper_l
GOW(toupper_l, iFip)
// towctrans    // Weak
// __towctrans
// __towctrans_l
// towctrans_l  // Weak
GO(towlower, iFi)
GO(__towlower_l, iFip)
GOW(towlower_l, iFip)
GO(towupper, iFi)
GO(__towupper_l, iFip)
GOW(towupper_l, iFip)
// tr_break
GOW(truncate, iFpu)
GO(truncate64, iFpU)
// tsearch  // Weak
GO(ttyname, pFi)
GOW(ttyname_r, iFipu)
// __ttyname_r_chk
// ttyslot
// twalk    // Weak
DATAV(tzname, 4)
DATA(__tzname, 4)
GOW(tzset, vFv)
// ualarm
GO(__uflow, iFp)
// ulckpwdf // Weak
// ulimit   // Weak
GOW(umask, uFu)
GOW(umount, iFp)
GOW(umount2, iFpi)
GOM(uname, iFp) //Weak
GO(__underflow, iFp)
GOW(ungetc, iFip)
GO(ungetwc, iFip)
GOW(unlink, iFp)
GO(unlinkat, iFipi)
GO(unlockpt, iFi)
GOW(unsetenv, iFp)
// unshare
GOW(updwtmp, vFpp)
// updwtmpx
// uselib
GOW(uselocale, pFp)
GO(__uselocale, pFp)
// user2netname
GO(usleep, iFu)
// ustat
GO(utime, iFpp)
GO(utimensat, iFippi)
GOW(utimes, iFpp)   //TODO: check, signature is int utimes(const char *filename, const struct timeval times[2]);
GOW(utmpname, iFp)
// utmpxname
GOW(valloc, pFu)
GOM(vasprintf, iFEpppp)
GOM(__vasprintf_chk, iFEpippp)
// vdprintf // Weak
// __vdprintf_chk
GOM(verr, vFEpV)
// verrx
GO(versionsort, iFpp)
// versionsort64
GOM(vfork, iFEv) // Weak
// __vfork
GOM(vfprintf, iFEppp)
GOM(__vfprintf_chk, iFEpvpp)
GOM(vfscanf, iFEppp)  // Weak
// __vfscanf
GOM(vfwprintf, iFEppp)    // Weak
GO2(__vfwprintf_chk, iFEpvpp, my_vfwprintf)
GOW(vfwscanf, iFppp)
// vhangup
// vlimit
// vmsplice
GOM(vprintf, iFEppp)
GOM(__vprintf_chk, iFEvppp)
// vscanf   // Weak
GOM(vsnprintf, iFEpLppp)    // Weak
GOM(__vsnprintf, iFEpuppp)  // Weak
GOM(__vsnprintf_chk, iFEpuvvppp)
GOM(vsprintf, iFEpppp) // Weak
GOM(__vsprintf_chk, iFEpvvppp)   // ignoring flag and slen, just use vsprintf in fact
#ifdef POWERPCLE
GOM(vsscanf, iFppp)
#else
GO(vsscanf, iFppp)
#endif
// __vsscanf    // Weak
GOM(vswprintf, iFEpuppp)    // Weak
GOM(__vswprintf_chk, iFEpuvvppp)    // Weak
GO(vswscanf, iFppp)
GO(vsyslog, vFipp)
GO(__vsyslog_chk, vFiipp)
// vtimes
GOM(vwarn, vFEppp)
// vwarnx
GOM(vwprintf, iFEpp)
GO2(__vwprintf_chk, iFEvpp, my_vwprintf)
GO(vwscanf, iFpp)
GOW(wait, iFp)
GOW(__wait, iFp)
GOW(wait3, iFpip)
GOW(wait4, iFipip)
GOW(waitid, iFiipi)
GOW(waitpid, iFipi)
GOW(__waitpid, iFipi)
GO(warn, vFppppppppp)
GO(warnx, vFppppppppp)
GOW(wcpcpy, pFpp)
// __wcpcpy_chk
GOW(wcpncpy, pFpp)
// __wcpncpy_chk
GOW(wcrtomb, uFpip)
// __wcrtomb_chk
GOW(wcscasecmp, iFpp)
// __wcscasecmp_l
GOW(wcscasecmp_l, iFppp)
GOW(wcscat, pFpp)
GO(__wcscat_chk, pFppu)
GO(wcschr, pFpi)
// wcschrnul    // Weak
GO(wcscmp, iFpp)
GOW(wcscoll, iFpp)
GO(__wcscoll_l, iFppp)
GOW(wcscoll_l, iFppp)
GO(wcscpy, pFpp)
GO(__wcscpy_chk, pFppu)
GO(wcscspn, uFpp)
GO(wcsdup, pFp)
GO(wcsftime, uFpupp)
GO(__wcsftime_l, LFpLppp)
GOW(wcsftime_l, LFpLppp)
GOW(wcslen, uFp)
GOW(wcsncasecmp, iFppu)
// __wcsncasecmp_l
GOW(wcsncasecmp_l, iFppup)
GO(wcsncat, pFppu)
// __wcsncat_chk
GO(wcsncmp, iFppu)
GOW(wcsncpy, pFppu)
GO(__wcsncpy_chk, pFppuu)
GOW(wcsnlen, LFpL)
GOW(wcsnrtombs, uFppuup)
// __wcsnrtombs_chk
GO(wcspbrk, pFpp)
GO(wcsrchr, pFpi)
GOW(wcsrtombs, uFppup)
// __wcsrtombs_chk
GO(wcsspn, uFpp)
GO(wcsstr, pFpp)
GO(wcstod, dFpp)
GO(__wcstod_internal, dFppi)
// __wcstod_l
// wcstod_l // Weak
GO(wcstof, fFpp)
// __wcstof_internal
// __wcstof_l
// wcstof_l // Weak
// wcstoimax
GO(wcstok, pFppp)
GO(wcstol, iFppi)
GO(wcstold, DFpp)
// __wcstold_internal
// __wcstold_l
// wcstold_l    // Weak
GO(__wcstol_internal, iFppii)
GO(wcstoll, IFppi)
// __wcstol_l
// wcstol_l // Weak
// __wcstoll_internal
// __wcstoll_l
// wcstoll_l    // Weak
GO(wcstombs, uFppu)
// __wcstombs_chk
// wcstoq   // Weak
GO(wcstoul, iFppi)
GO(__wcstoul_internal, LFppii)
GO(wcstoull, UFppi)
// __wcstoul_l
// wcstoul_l    // Weak
// __wcstoull_internal
// __wcstoull_l
// wcstoull_l   // Weak
// wcstoumax
// wcstouq  // Weak
// wcswcs   // Weak
GO(wcswidth, iFpu)
GO(wcsxfrm, uFppu)
GOW(wcsxfrm_l, uFppup)
GO(__wcsxfrm_l, uFppup)
GO(wctob, iFi)
GO(wctomb, iFpi)
GO(__wctomb_chk, iFpuL)
// wctrans  // Weak
// __wctrans_l
// wctrans_l    // Weak
GOW(wctype, uFp)
GO(__wctype_l, uFpp)
GOW(wctype_l, uFpp)
GO(wcwidth, iFu)
GOW(wmemchr, pFpiu)
GO(wmemcmp, iFppL)
GOW(wmemcpy, pFppL)
GO(__wmemcpy_chk, pFppLL)
GOW(wmemmove, pFppL)
// __wmemmove_chk
// wmempcpy // Weak
// __wmempcpy_chk
GO(wmemset, pFpiu)
// __wmemset_chk
GO(wordexp, iFppi)
GO(wordfree, vFp)
// __woverflow
GOM(wprintf, iFEpVV)
GOM(__wprintf_chk, iFEipVV)
GOW(write, lFipL)
GOW(__write, lFipL)
GOW(writev, lFipi)
// wscanf
// __wuflow
// __wunderflow
// xdecrypt
// xdr_accepted_reply
// xdr_array
// xdr_authdes_cred
// xdr_authdes_verf
// xdr_authunix_parms
// xdr_bool
// xdr_bytes
// xdr_callhdr
// xdr_callmsg
// xdr_char
// xdr_cryptkeyarg
// xdr_cryptkeyarg2
// xdr_cryptkeyres
// xdr_des_block
// xdr_double
// xdr_enum
// xdr_float
// xdr_free
// xdr_getcredres
// xdr_hyper
GO(xdr_int, iFpp)
// xdr_int16_t
// xdr_int32_t
// xdr_int64_t
// xdr_int8_t
// xdr_keybuf
// xdr_key_netstarg
// xdr_key_netstres
// xdr_keystatus
// xdr_long
// xdr_longlong_t
// xdrmem_create
// xdr_netnamestr
// xdr_netobj
// xdr_opaque
// xdr_opaque_auth
// xdr_pmap
// xdr_pmaplist
// xdr_pointer
// xdr_quad_t
// xdrrec_create
// xdrrec_endofrecord
// xdrrec_eof
// xdrrec_skiprecord
// xdr_reference
// xdr_rejected_reply
// xdr_replymsg
// xdr_rmtcall_args
// xdr_rmtcallres
// xdr_short
// xdr_sizeof
// xdrstdio_create
// xdr_string
// xdr_u_char
// xdr_u_hyper
GO(xdr_u_int, iFpp)
// xdr_uint16_t
// xdr_uint32_t
// xdr_uint64_t
// xdr_uint8_t
// xdr_u_long
// xdr_u_longlong_t
// xdr_union
// xdr_unixcred
// xdr_u_quad_t
// xdr_u_short
// xdr_vector
// xdr_void
// xdr_wrapstring
// xencrypt
GO(__xmknod, iFipup)
GO(__xmknodat, iFiipip)
GO(__xpg_basename, pFp)
// __xpg_sigpause   // Weak
GO(__xpg_strerror_r, pFipu)
// xprt_register
// xprt_unregister
GOM(__xstat, iFEipp)
GOM(__xstat64, iFEipp)

// forcing a custom __gmon_start__ that does nothing
GOM(__gmon_start__, vFv)

GOM(_Jv_RegisterClasses, vFv)   // dummy

GOM(__fdelt_chk, iFi)

GOM(getauxval, uFEu)  // implemented since glibc 2.16

GOM(prlimit64, lFpupp)
GOM(reallocarray, pFpLL)
GOM(__open_nocancel, iFEpOV)
GO2(__read_nocancel, lFipL, read)
GO2(__close_nocancel, iFi, close)

GOM(mkstemps64, iFEpi)   // not always implemented
GOM(getentropy, iFEpL)  // starting from glibc 2.25

// not found (libitm???), but it seems OK to declare dummies:

GOM(_ITM_RU1, uFp)
GOM(_ITM_RU4, uFp)
//GOM(_ITM_RU8, UFp)
GOM(_ITM_memcpyRtWn, vFppu) // register(2)
GOM(_ITM_memcpyRnWt, vFppu) // register(2)
GOM(_ITM_addUserCommitAction, vFEpup)
GOM(_ITM_registerTMCloneTable, vFEpu)
GOM(_ITM_deregisterTMCloneTable, vFEp)

GOM(__umoddi3, UFUU)
GOM(__udivdi3, UFUU)
GOM(__divdi3, IFII)
GOM(__poll_chk, iFpuii)

GOM(fallocate64, iFiiII)

DATAM(__libc_stack_end, 4)

DATAM(___brk_addr, 4)
DATA(__libc_enable_secure, 4)

GOM(__register_frame_info, vFpp)    // faked function
GOM(__deregister_frame_info, pFp)

GO(name_to_handle_at, iFipppi) // only glibc 2.14+, so may not be present...
