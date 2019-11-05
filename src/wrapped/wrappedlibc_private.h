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
// alphasort
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
// argz_create_sep  // Weak
// argz_delete
// argz_extract // Weak
// argz_insert  // Weak
// __argz_next
// argz_next    // Weak
// argz_replace // Weak
// __argz_stringify
// argz_stringify   // Weak
GO(asctime, pFp)
GOW(asctime_r, pFpp)
// asprintf // Weak
// __asprintf
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
// brk  // Weak
// __bsd_getpgrp
// bsd_signal   // Weak
GOM(bsearch, pFEppuup)
GOW(btowc, iFi)
GOW(bzero, vFpu)
GO(__bzero, vFpu)
GOW(calloc, pFuu)
// callrpc
// canonicalize_file_name   // Weak
// capget
// capset
GO(catclose, iFp)
GO(catgets, pFpiip)
GO(catopen, pFpi)
// cbc_crypt
GO(cfgetispeed, iFp)
GO(cfgetospeed, iFp)
// cfmakeraw
GOW(cfree, vFp)
GO(cfsetispeed, iFpi)
GO(cfsetospeed, iFpi)
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
// __cmsg_nxthdr
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
GO(dup3, iFiii)
GOW(duplocale, pFp)
GO(__duplocale, pFp)
// dysize
GOW(eaccess, iFpi)
// ecb_crypt
// ecvt
// ecvt_r
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
// endspent
// endttyent
// endusershell
GOW(endutent, vFv)
// endutxent
DATAV(environ, 4)
DATAV(_environ, 4)
DATA(__environ, 4)    // type B
// envz_add
// envz_entry
// envz_get
// envz_merge
// envz_remove
// envz_strip
GO(epoll_create, iFi)
GO(epoll_create1, iFi)
#ifdef NOALIGN
GO(epoll_ctl, iFiiip)
// epoll_pwait
GO(epoll_wait, iFipii)
#else
GOM(epoll_ctl, iFEiiip)     // align epool_event structure
// epoll_pwait
GOM(epoll_wait, iFEipii)    // need realign of epoll_event structure
#endif
// erand48
// erand48_r    // Weak
// err
// errno    // type B
GO(__errno_location, pFv)
GOW(error, vFiippppppppp)  // Simple attempt: there is a vararg, but the alignment will/may be off if it tries some Double in the "printf" part
// error_at_line    // Weak
// error_message_count  // type B
// error_one_per_line   // type B
// error_print_progname // type B
// errx
// ether_aton
// ether_aton_r
// ether_hostton
// ether_line
// ether_ntoa
// ether_ntoa_r
// ether_ntohost
GOW(euidaccess, iFpi)
GO(eventfd, iFui)
GO(eventfd_read, iFip)
GO(eventfd_write, iFiU)
GOM(execl, iFEpVV)
GOM(execle, iFEpVV)
GOM(execlp, iFEpVV)
GO(execv, iFpp)     // maybe need to GOM this one, and check if path is an x86 file...
GOW(execve, iFppp)   // and this one too...
GOM(execvp, iFEpVV)
GO(exit, vFi)
GO(_exit, vFi)
GO(_Exit, vFi)    // Weak
GO(faccessat, iFipiA)
// fattach
GO(__fbufsize, uFp)
GOW(fchdir, iFi)
// fchflags
GOW(fchmod, iFiu)
GO(fchmodat, iFipuA)
GOW(fchown, iFiuu)
GO(fchownat, iFipuiA)
GO(fclose, iFp)
GOW(fcloseall, iFv)
GOW(fcntl, iFiiuuuuuu)  // this also use a vararg for 3rd argument
GOW(__fcntl, iFiiuuuuuu)
GOM(fcntl64, iFEiiuuuuuu)
GO(fcvt, pFdipp)
// fcvt_r
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
GO(fopen, pFpp)
GOW(fopen64, pFpp)
//GOM(fopencookie, pFppp)   // last p are 4 callbacks...
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
GOW(fread, uFpuup)
GO(__freadable, iFp)
GO(__fread_chk, uFpuuup)
GO(__freading, iFp)
GO(fread_unlocked, uFpuup)
// __fread_unlocked_chk
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
GO(fseek, iFpii)
GO(fseeko, iFpii)
GO(fseeko64, iFpIi)
GO(__fsetlocking, iFpi)
GO(fsetpos, iFpp)
GO(fsetpos64, iFpp)
GO(fsetxattr, iFippui)
GOW(fstatfs, iFip)
GOW(fstatfs64, iFip)
GO(fstatvfs, iFip)
GOW(fstatvfs64, iFip)   // allignment?
GOW(fsync, iFi)
GOW(ftell, iFp)
GO(ftello, uFp)
GO(ftello64, IFp)
GO(ftime, iFp)
GO(ftok, iFpi)
GOW(ftruncate, iFiu)
GOW(ftruncate64, iFiU)
GOW(ftrylockfile, iFp)
// fts_children
// fts_close
// fts_open
// fts_read
// fts_set
// ftw
GOM(ftw64, iFEppi)
GOW(funlockfile, vFp)
GO(futimens, iFip)
GOW(futimes, iFipp) //int futimes(int fd, const struct timeval tv[2]) TODO: check how it ends up
// futimesat
// fwide
GOM(fwprintf, iFEppVV) // Weak
GOM(__fwprintf_chk, iFEp0pVV)
GO(__fwritable, iFp)
GOW(fwrite, uFpuup)
GO(fwrite_unlocked, uFpuup)
GO(__fwriting, iFp)
// fwscanf
GO(__fxstat, iFiip)
GOM(__fxstat64, iFEiip) // need reaalign of struct stat64
GO(__fxstatat, iFiippA)
GOM(__fxstatat64, iFEiippA) // struct stat64 again
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
GOW(getcwd, pFpu)
// __getcwd_chk
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
// getprotobyname_r
GO(getprotobynumber, pFi)
// getprotobynumber_r
GO(getprotoent, pFv)
// getprotoent_r
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
// getspent
// getspent_r
// getspnam
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
// glob64
GO(globfree, vFp)
GO(globfree64, vFp)
// glob_pattern_p   // Weak
GO(gmtime, pFp)
GO(__gmtime_r, pFpp)
GOW(gmtime_r, pFpp)
// gnu_dev_major
GO(gnu_dev_makedev, UFuu)
// gnu_dev_minor
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
// imaxdiv  // Weak
DATA(in6addr_any, 16)  // type R
// in6addr_loopback // type R
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
GO(inet_addr, iFp)
GOW(inet_aton, iFpp)
// inet_lnaof
// inet_makeaddr
// inet_netof
GO(inet_network, iFp)
// inet_nsap_addr
GO(inet_nsap_ntoa, pFipp)
GO(inet_ntoa, pFi)
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
DATA(_IO_2_1_stderr_, 4)
DATA(_IO_2_1_stdin_, 4)
DATA(_IO_2_1_stdout_, 4)
GO(_IO_adjust_column, uFupi)
// _IO_adjust_wcolumn
GO(ioctl, iFiuppppp)   //the vararg is just to have args of various type, but only 1 arg
GO(_IO_default_doallocate, iFp)
GO(_IO_default_finish, vFpi)
GO(_IO_default_pbackfail, iFpi)
GO(_IO_default_uflow, iFp)
GO(_IO_default_xsgetn, uFppu)
GO(_IO_default_xsputn, uFppu)
GO(_IO_doallocbuf, vFp)
GO(_IO_do_write, iFppu)
// _IO_fclose
// _IO_fdopen
// _IO_feof
// _IO_ferror
// _IO_fflush
// _IO_fgetpos
// _IO_fgetpos64
// _IO_fgets
GO(_IO_file_attach, pFpi)
GO(_IO_file_close, iFp)
GO(_IO_file_close_it, iFp)
GO(_IO_file_doallocate, iFp)
// _IO_file_finish
GO(_IO_file_fopen, pFpppi)
GO(_IO_file_init, vFp)
DATA(_IO_file_jumps, 4)
GO(_IO_file_open, pFppiiii)
GO(_IO_file_overflow, iFpi)
GO(_IO_file_read, iFppi)
GO(_IO_file_seek, IFpIi)
GO(_IO_file_seekoff, IFpIii)
GO(_IO_file_setbuf, pFppi)
GOM(_IO_file_stat, iFEpp)
GO(_IO_file_sync, iFp)
GO(_IO_file_underflow, iFp)
GO(_IO_file_write, iFppi)
GO(_IO_file_xsputn, uFppu)
GO(_IO_flockfile, vFp)
GO(_IO_flush_all, iFv)
GO(_IO_flush_all_linebuffered, vFv)
// _IO_fopen
// _IO_fprintf  // Weak
// _IO_fputs
// _IO_fread
GO(_IO_free_backup_area, vFp)
// _IO_free_wbackup_area
// _IO_fsetpos
// _IO_fsetpos64
// _IO_ftell
// _IO_ftrylockfile
GO(_IO_funlockfile, vFp)
// _IO_fwrite
GO(_IO_getc, iFp)
// _IO_getline
GO(_IO_getline_info, uFppuiip)
// _IO_gets
GO(_IO_init, vFpi)
GO(_IO_init_marker, vFpp)
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
// _IO_proc_close
// _IO_proc_open
GO(_IO_putc, iFip)
// _IO_puts
GO(_IO_remove_marker, vFp)
GO(_IO_seekmark, iFppi)
GO(_IO_seekoff, IFpIii)
GO(_IO_seekpos, IFpIi)
// _IO_seekwmark
GO(_IO_setb, vFpppi)
// _IO_setbuffer
// _IO_setvbuf
GO(_IO_sgetn, uFppu)
// _IO_sprintf
GO(_IO_sputbackc, iFpi)
// _IO_sputbackwc
// _IO_sscanf
// _IO_str_init_readonly
// _IO_str_init_static
// _IO_str_overflow
// _IO_str_pbackfail
// _IO_str_seekoff
// _IO_str_underflow
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
// _IO_vfprintf
GOM(_IO_vfscanf, iFEppV)
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
// isascii
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
GO2(__isoc99_fscanf, iFppV, __isoc99_vfscanf)
// __isoc99_fwscanf
// __isoc99_scanf
GO2(__isoc99_sscanf, iFppV, __isoc99_vsscanf)
// __isoc99_swscanf
// __isoc99_vfscanf
// __isoc99_vfwscanf
// __isoc99_vscanf
GO(__isoc99_vsscanf, iFppp) // TODO: check if ok
// __isoc99_vswscanf
// __isoc99_vwscanf
// __isoc99_wscanf
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
GO(__libc_alloca_cutoff, iFu)
// __libc_allocate_rtsig
// __libc_allocate_rtsig_private
// __libc_calloc
// __libc_clntudp_bufcreate
GO(__libc_current_sigrtmax, iFv)
// __libc_current_sigrtmax_private
GO(__libc_current_sigrtmin, iFv)
// __libc_current_sigrtmin_private
// __libc_dlclose
// __libc_dl_error_tsd
// __libc_dlopen_mode
// __libc_dlsym
// __libc_fatal
// __libc_fork
// __libc_free
// __libc_freeres
// __libc_init_first
// _libc_intl_domainname    // type R
// __libc_longjmp
// __libc_mallinfo
// __libc_malloc
// __libc_mallopt
// __libc_memalign
// __libc_pthread_init
// __libc_pvalloc
// __libc_pwrite
// __libc_realloc
// __libc_sa_len
// __libc_siglongjmp
GOM(__libc_start_main, iFEpippppp)
// __libc_system
// __libc_thread_freeres
// __libc_valloc
GOW(link, iFpp)
GO(linkat, iFipipA)
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
// lrand48
// lrand48_r
GO(lremovexattr, iFpp)
GOM(lsearch, pFEpppup)
GOW(lseek, iFiii)
// __lseek  // Weak
GOW(lseek64, IFiIi)
GO(lsetxattr, iFpppui)
// lutimes
GO(__lxstat, iFipp)
GOM(__lxstat64, iFEipp)
GO(madvise, iFpui)
GOM(makecontext, iFEpppV)
GOW(mallinfo, pFv)
GO(malloc, pFu)
// malloc_get_state // Weak
DATAV(__malloc_hook, 4)
DATAV(__malloc_initialize_hook, 4)
// malloc_set_state // Weak
// malloc_stats // Weak
GOW(malloc_trim, iFu)
GOW(malloc_usable_size, uFp)
// mallopt  // Weak
// mallwatch    // type B
GO(mblen, iFpu)
GOW(mbrlen, uFpup)
GO(__mbrlen, uFpup)
GOW(mbrtowc, uFppup)
GO(__mbrtowc, uFppup)
GOW(mbsinit, iFp)
GOW(mbsnrtowcs, uFppuup)
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
// mcount   // Weak
// _mcount
GOW(memalign, pFuu)
DATAV(__memalign_hook, 4)
// memccpy  // Weak
GO(memchr, pFpiu)
GO(memcmp, iFppu)
GO(memcpy, pFppu)
GO(__memcpy_chk, pFppuu)
// memfrob
GO(memmem, pFpupu)
GO(memmove, pFppu)
GO(__memmove_chk, pFppuu)
// mempcpy
// __mempcpy
// __mempcpy_chk
// __mempcpy_small
GOW(memrchr, pFpiu)
GO(memset, pFpiu)
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
GO(mlock, iFpu)
GO(mlockall, iFi)
GOW(mmap, pFpuiiii)
GOW(mmap64, pFpuiiiI)
// modf // Weak
// modff    // Weak
// modfl    // Weak
// moncontrol   // Weak
// monstartup   // Weak
// __monstartup
DATA(__morecore, 4)
GOW(mount, iFpppup)
// mprobe
GOW(mprotect, iFpui)
// mrand48
// mrand48_r
GOW(mremap, pFpuui)
// msgctl
// msgget   // Weak
// msgrcv   // Weak
// msgsnd   // Weak
// msync    // Weak
// mtrace
GO(munlock, iFpu)
GO(munlockall, iFv)
GOW(munmap, iFpu)
// muntrace
GOW(nanosleep, iFpp)
// __nanosleep  // Weak
// netname2host
// netname2user
GOW(newlocale, pFipp)
GO(__newlocale, pFipp)
// nfsservctl
GOM(nftw, iFEppii)
// nftw64
GOW(ngettext, pFppu)
GO(nice, iFi)
// _nl_default_dirname  // type R
// _nl_domain_bindings  // type B
GO(nl_langinfo, pFu)
GO(__nl_langinfo_l, pFup)
// nl_langinfo_l    // Weak
// _nl_msg_cat_cntr // type B
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
DATA(obstack_alloc_failed_handler, 4)
// _obstack_begin
// _obstack_begin_1
DATA(obstack_exit_failure, 4)
// _obstack_free
// obstack_free
// _obstack_memory_used
GO(_obstack_newchunk, vFpi)
// obstack_printf   // Weak
// __obstack_printf_chk
GOM(obstack_vprintf, iFEppVV)  // Weak
// __obstack_vprintf_chk
// on_exit  // Weak
GOM(open, iFEpiu)    //Weak
GOM(__open, iFEpiu) //Weak
GO(__open_2, iFpi)
GOM(open64, iFEpiu) //Weak
// __open64 // Weak
GO(__open64_2, iFpi)
GOW(openat, iFipAu)
// __openat_2
GOW(openat64, iFipAuuuuu)   // variable arg...
// __openat64_2
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
// parse_printf_format
// passwd2des
GOW(pathconf, iFpi)
GOW(pause, iFv)
GO(pclose, iFp)
GO(perror, vFp)
// personality  // Weak
GOW(pipe, iFp)  // the array of 2 int seems to converted as a pointer, on both x86 and arm (and x86_64 too)
// __pipe
GOW(pipe2, iFpi) // assuming this works the same as pipe, so pointer for array of 2 int
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
// posix_spawn
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
// posix_spawn_file_actions_adddup2
// posix_spawn_file_actions_addopen
// posix_spawn_file_actions_destroy
// posix_spawn_file_actions_init
// posix_spawnp
GO(ppoll, iFpupp)
GOW(prctl, iFiuuuu)
GOW(pread, iFipui)
GOW(pread64, iFipuI)
// __pread64    // Weak
// __pread64_chk
GOM(preadv64, iFEipiI)  // not always present
// __pread_chk
GOM(printf, iFEpVV)
GOM(__printf_chk, iFEvpVV)
GO(__printf_fp, iFppp)  // does this needs aligment?
// printf_size
// printf_size_info
// profil   // Weak
// __profile_frequency
DATA(__progname, 4)
DATA(__progname_full, 4)
DATAV(program_invocation_name, 4)
DATAV(program_invocation_short_name, 4)
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
GOW(pwrite64, iFiuI)
// __pwrite64   // Weak
GOM(pwritev64, iFEipiI)  // not always present
// qecvt
// qecvt_r
// qfcvt
// qfcvt_r
// qgcvt
GOM(qsort, vFEpuup)
GOM(qsort_r, vFEpuupp)
// query_module // Weak
// quotactl
GO(raise, iFi)  // will need a GOM version once signal are implemented probably
GO(rand, iFv)
GOW(random, iFv)
GOW(random_r, iFpp)
GO(rand_r, iFp)
// rawmemchr    // Weak
GO(__rawmemchr, pFpi)
// rcmd
// rcmd_af
// __rcmd_errstr    // type B
GO(read, iFipu)
GOW(__read, iFipu)
// readahead    // Weak
GO(__read_chk, iFipuu)
GOW(readdir, pFp)
GO(readdir64, pFp)  // check if alignement is correct
// readdir64_r
GOW(readdir_r, iFppp)
GOM(readlink, iFEppu)
GO(readlinkat, iFippu)
// __readlinkat_chk
// __readlink_chk
GO(readv, iFipi)
GO(realloc, pFpu)
DATAV(__realloc_hook, 4)
GO(realpath, pFpp)
GO(__realpath_chk, pFppu)
// reboot
// re_comp  // Weak
// re_compile_fastmap   // Weak
// re_compile_pattern   // Weak
GO(recv, iFipui)
GO(__recv_chk, iFipuui)
GOW(recvfrom, iFipuipp)
// __recvfrom_chk
GOW(recvmsg, iFipi)
// re_exec  // Weak
GOW(regcomp, iFppi)
GOW(regerror, uFippu)
GO(regexec, iFppupi)
GOW(regfree, vFp)
GOM(__register_atfork, iFEppp)  // ignoring last pointer parameter (dso_handle)
// register_printf_function // Weak
// registerrpc
// remap_file_pages // Weak
// re_match // Weak
// re_match_2   // Weak
GO(remove, iFp)
GO(removexattr, iFpp)
// remque
GO(rename, iFpp)
GO(renameat, iFipip)
// _res // type B
GOW(re_search, iFppiiip)
GOW(re_search_2, iFppipiiipi)
// re_set_registers // Weak
// re_set_syntax    // Weak
// _res_hconf   // type B
GO(__res_iclose, vFpi)
GO(__res_init, iFv)
GO(__res_maybe_init, iFpi)
GO(__res_nclose, vFp)
GO(__res_ninit, iFp)
DATA(__resp, 4)
// __res_randomid
// __res_state
// re_syntax_options    // type B
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
// rpmatch
// rresvport
// rresvport_af
// rtime
// ruserok
// ruserok_af
// ruserpass
// sbrk // Weak
// __sbrk
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
GO(semctl, iFiiippppp)  // use vararg after the 3 i
GOW(semget, iFuii)
GOW(semop, iFipu)
GO(semtimedop, iFipup)
GOW(send, iFipui)
// __send   // Weak
// sendfile
GO(sendfile64, iFiipu)
GOW(sendmsg, iFipi)
GOM(__sendmmsg, iFEipuu)    // actual __sendmmsg is glibc 2.14+. The syscall is Linux 3.0+, so use syscall...
GOW(sendto, iFipuipu)
// setaliasent
GOW(setbuf, vFpp)
GOW(setbuffer, vFppu)
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
// setlogmask
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
// setspent
// setstate // Weak
GOW(setstate_r, iFpp)
// settimeofday // Weak
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
GOW(sigaltstack, iFpp)
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
// sigset
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
// sockatmark
GOW(socket, iFiii)
GOW(socketpair, iFiiip)
// splice
GOM(sprintf, iFEppVV)
GOM(__sprintf_chk, iFEpvvpVV)
// sprofil  // Weak
GOW(srand, vFu)
GO(srand48, vFi)
// srand48_r    // Weak
GOW(srandom, vFu)
GOW(srandom_r, iFup)
GO2(sscanf, iFppV, vsscanf)     // sscanf va_list is only pointer, no realign to do
// ssignal  // Weak
// sstk
GOM(__stack_chk_fail, vFE)
GOW(statfs, iFpp)
// __statfs
GOW(statfs64, iFpp)     // is alignment ok?
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
// stpncpy  // Weak
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
GO(strcspn, uFpp)
// __strcspn_c1
// __strcspn_c2
// __strcspn_c3
GOW(strdup, pFp)
GO(__strdup, pFp)
GO(strerror, pFi)
// strerror_l
GO(__strerror_r, pFipu)
GOW(strerror_r, pFipu)
// strfmon
// __strfmon_l
// strfmon_l    // Weak
// strfry
GO(strftime, uFpupp)
GO(__strftime_l, uFpupp)
GOW(strftime_l, uFpupp)
GO(strlen, uFp)
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
GO(__strtod_internal, dFppp)
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
GO(strtol, iFppi)
GO(strtold, DFpp)
// __strtold_internal
// __strtold_l
GOW(strtold_l, DFppu)
GO(__strtol_internal, iFppi)
GO(strtoll, IFppi)
// __strtol_l
// strtol_l // Weak
GO(__strtoll_internal, IFppii)
// __strtoll_l
GOW(strtoll_l, IFppip)
// strtoq   // Weak
GO(strtoul, uFppi)
GO(__strtoul_internal, uFppii)
GO(strtoull, UFppi)
// __strtoul_l
// strtoul_l    // Weak
GO(__strtoull_internal, UFppii)
// __strtoull_l
GOW(strtoull_l, UFppip)
GO(strtoumax, UFppi)
// strtouq  // Weak
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
GO(swapcontext, iFpp)
// swapoff  // Weak
// swapon   // Weak
GOM(swprintf, iFEpupV)
GOM(__swprintf_chk, iFEpuiupV)
GO2(swscanf, iFppV, vswscanf)     // swscanf va_list is only pointer, no realign to do
GOW(symlink, iFpp)
GO(symlinkat, iFpip)
GO(sync, vFv)
// sync_file_range
GOM(syscall, uFE)
GOW(sysconf, iFi)
GO(__sysconf, iFi)
// sysctl   // Weak
// __sysctl
DATA(_sys_errlist, 4)
DATA(sys_errlist, 4)
GO(sysinfo, iFp)
GO2(syslog, vFiV, vsyslog)
GO2(__syslog_chk, vFiipV, __vsyslog_chk)
// _sys_nerr    // type R
// sys_nerr // type R
DATA(sys_sigabbrev, 4)
DATA(_sys_siglist, 4)
DATA(sys_siglist, 4)
GOW(system, iFp)
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
// timerfd_create
// timerfd_gettime
// timerfd_settime
GOW(times, iFp)
DATAV(timezone, 4)
DATAB(__timezone, 4)   // type B
GO(tmpfile, pFv)
GO(tmpfile64, pFv)
GO(tmpnam, pFp)
// tmpnam_r
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
GO(unlinkat, iFipA)
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
GO(utimensat, iFippA)
GOW(utimes, iFpp)   //TODO: check, signature is int utimes(const char *filename, const struct timeval times[2]);
GOW(utmpname, iFp)
// utmpxname
GOW(valloc, pFu)
GOM(vasprintf, iFEppVV)
GOM(__vasprintf_chk, iFEpipVV)
// vdprintf // Weak
// __vdprintf_chk
GOM(verr, vFEpV)
// verrx
// versionsort
// versionsort64
GOM(vfork, iFEv) // Weak
// __vfork
GOM(vfprintf, iFEppVV)
GOM(__vfprintf_chk, iFEpvpVV)
GOM(vfscanf, iFEppV)  // Weak
// __vfscanf
// vfwprintf    // Weak
// __vfwprintf_chk
// vfwscanf // Weak
// vhangup
// vlimit
// vmsplice
GOM(vprintf, iFEpVV)
GOM(__vprintf_chk, iFEvpVV)
// vscanf   // Weak
GOM(vsnprintf, iFEpupVV)    // Weak
GOM(__vsnprintf, iFEpupVV)  // Weak
GOM(__vsnprintf_chk, iFEpuvvpVV)
GOM(vsprintf, iFEppVV) // Weak
GOM(__vsprintf_chk, iFEpvvpp)
GO(vsscanf, iFppp)
// __vsscanf    // Weak
GOM(vswprintf, iFEpupVV)    // Weak
GOM(__vswprintf_chk, iFEpuvvpVV)    // Weak
GO(vswscanf, iFppp)
GO(vsyslog, vFipV)
GO(__vsyslog_chk, vFiipV)
// vtimes
// vwarn
// vwarnx
// vwprintf
// __vwprintf_chk
// vwscanf
GOW(wait, iFp)
GOW(__wait, iFp)
GOW(wait3, iFpip)
GOW(wait4, iFipip)
GOW(waitid, iFiipi)
GOW(waitpid, iFipi)
GOW(__waitpid, iFip)
// warn
// warnx
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
GO(__wcsftime_l, uFpuppu)
// wcsftime_l   // Weak
GOW(wcslen, uFp)
GOW(wcsncasecmp, iFppu)
// __wcsncasecmp_l
GOW(wcsncasecmp_l, iFppup)
GO(wcsncat, pFppu)
// __wcsncat_chk
GO(wcsncmp, iFppu)
GOW(wcsncpy, pFppu)
GO(__wcsncpy_chk, pFppuu)
// wcsnlen  // Weak
GOW(wcsnrtombs, uFppuup)
// __wcsnrtombs_chk
GO(wcspbrk, pFpp)
GO(wcsrchr, pFpi)
GOW(wcsrtombs, uFppup)
// __wcsrtombs_chk
GO(wcsspn, uFpp)
GO(wcsstr, pFpp)
GO(wcstod, dFpp)
// __wcstod_internal
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
// __wcstoul_internal
GO(wcstoull, UFppi)
// __wcstoul_l
// wcstoul_l    // Weak
// __wcstoull_internal
// __wcstoull_l
// wcstoull_l   // Weak
// wcstoumax
// wcstouq  // Weak
// wcswcs   // Weak
// wcswidth
GO(wcsxfrm, uFppu)
GOW(wcsxfrm_l, uFppup)
GO(__wcsxfrm_l, uFppup)
GO(wctob, iFi)
GO(wctomb, iFpi)
// __wctomb_chk
// wctrans  // Weak
// __wctrans_l
// wctrans_l    // Weak
GOW(wctype, uFp)
GO(__wctype_l, uFpp)
GOW(wctype_l, uFpp)
GO(wcwidth, iFu)
GOW(wmemchr, pFpiu)
GO(wmemcmp, iFppu)
GOW(wmemcpy, pFppu)
GO(__wmemcpy_chk, pFppuu)
GOW(wmemmove, pFppu)
// __wmemmove_chk
// wmempcpy // Weak
// __wmempcpy_chk
GO(wmemset, pFpiu)
// __wmemset_chk
GO(wordexp, iFppi)
GO(wordfree, vFp)
// __woverflow
GOM(wprintf, iFEpVV)
// __wprintf_chk
GOW(write, iFipu)
GOW(__write, iFipu)
GOW(writev, iFipi)
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
// xdr_int
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
// xdr_u_int
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
GO(__xstat, iFipp)
GOM(__xstat64, iFEipp)

// forcing a custom __gmon_start__ that does nothing
GOM(__gmon_start__, vFv)

GOM(_Jv_RegisterClasses, vFv)   // dummy

GOM(__fdelt_chk, iFi)

//GOM(getauxval, uFEu)  // implemented since glibc 2.16
GO(getauxval, uFu)

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
GOM(__poll_chk, iFpuii)

GO2(fallocate64, iFiII, posix_fallocate64)

DATAM(__libc_stack_end, 4)

DATAM(___brk_addr, 4)

GOM(__register_frame_info, vFpp)    // faked function
GOM(__deregister_frame_info, pFp)

GO(name_to_handle_at, iFipppA) // only glibc 2.14+, so may not be present...
