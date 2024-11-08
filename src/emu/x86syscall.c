#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <asm/stat.h>
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#ifndef __NR_socketcall
#include <linux/net.h>
#include <sys/socket.h>
#endif
#include <sys/resource.h>
#include <poll.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "myalign.h"
#include "box86context.h"
#include "callback.h"
#include "signals.h"
#include "x86tls.h"

#ifndef __NR_socketcall
#ifndef SYS_RECVMMSG
#define SYS_RECVMMSG    19
#endif
#ifndef SYS_SENDMMSG
#define SYS_SENDMMSG    20
#endif
int32_t my_accept4(x86emu_t* emu, int32_t fd, void* a, void* l, int32_t flags); // not always present, so used wrapped version
#ifdef SYS_RECVMMSG
int32_t my_recvmmsg(x86emu_t* emu, int32_t fd, void* msgvec, uint32_t vlen, uint32_t flags, void* timeout);
int32_t my___sendmmsg(x86emu_t* emu, int32_t fd, void* msgvec, uint32_t vlen, uint32_t flags);
#endif
#endif
typedef struct x86_sigaction_s x86_sigaction_t;
typedef struct x86_sigaction_restorer_s x86_sigaction_restorer_t;


int32_t my_getrandom(x86emu_t* emu, void* buf, uint32_t buflen, uint32_t flags);
int of_convert(int flag);
int32_t my_open(x86emu_t* emu, void* pathname, int32_t flags, uint32_t mode);
#ifndef __NR_memfd_create
int my_memfd_create(x86emu_t* emu, void* name, uint32_t flags);
#endif

#ifndef NOALIGN
int my_epoll_create(x86emu_t* emu, int size);
int my_epoll_create1(x86emu_t* emu, int flags);
int32_t my_epoll_ctl(x86emu_t* emu, int32_t epfd, int32_t op, int32_t fd, void* event);
int32_t my_epoll_wait(x86emu_t* emu, int32_t epfd, void* events, int32_t maxevents, int32_t timeout);
#endif
int my_sigaction(x86emu_t* emu, int signum, const x86_sigaction_t *act, x86_sigaction_t *oldact);
int my_syscall_sigaction(x86emu_t* emu, int signum, const x86_sigaction_restorer_t *act, x86_sigaction_restorer_t *oldact, int sigsetsize);
int32_t my_execve(x86emu_t* emu, const char* path, char* const argv[], char* const envp[]);
void* my_mmap(x86emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, int offset);
void* my_mmap64(x86emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, int64_t offset);
int my_munmap(x86emu_t* emu, void* addr, unsigned long length);
int my_mprotect(x86emu_t* emu, void *addr, unsigned long len, int prot);
void* ElfSetBrk(void* newbrk);
int my_vfork(x86emu_t* emu);

#ifndef __NR_socketcall
int my_socketcall(x86emu_t* emu, int n, unsigned long *args) {
    int ret = 0;
    switch(n) {
        case SYS_SOCKET: ret = socket(args[0], args[1], args[2]); break;
        case SYS_BIND: ret = bind(args[0], (void*)args[1], args[2]); break;
        case SYS_CONNECT: ret = connect(args[0], (void*)args[1], args[2]); break;
        case SYS_LISTEN: ret = listen(args[0], args[1]); break;
        case SYS_ACCEPT: ret = accept(args[0], (void*)args[1], (void*)args[2]); break;
        case SYS_GETSOCKNAME: ret = getsockname(args[0], (void*)args[1], (void*)args[2]); break;
        case SYS_GETPEERNAME: ret = getpeername(args[0], (void*)args[1], (void*)args[2]); break;
        case SYS_SOCKETPAIR: ret = socketpair(args[0], args[1], args[2], (int*)args[3]); break;
        case SYS_SEND: ret = send(args[0], (void*)args[1], args[2], args[3]); break;
        case SYS_RECV: ret = recv(args[0], (void*)args[1], args[2], args[3]); break;
        case SYS_SENDTO: ret = sendto(args[0], (void*)args[1], args[2], args[3], (void*)args[4], args[5]); break;
        case SYS_RECVFROM: ret = recvfrom(args[0], (void*)args[1], args[2], args[3], (void*)args[4], (void*)args[5]); break;
        case SYS_SHUTDOWN: ret = shutdown(args[0], args[1]); break;
        case SYS_SETSOCKOPT: ret = setsockopt(args[0], args[1], args[2], (void*)args[3], args[4]); break;
        case SYS_GETSOCKOPT: ret = getsockopt(args[0], args[1], args[2], (void*)args[3], (void*)args[4]); break;
        case SYS_SENDMSG: ret = sendmsg(args[0], (void*)args[1], args[2]); break;
        case SYS_RECVMSG: ret = recvmsg(args[0], (void*)args[1], args[2]); break;
        case SYS_ACCEPT4: ret = my_accept4(emu, args[0], (void*)args[1], (void*)args[2], args[3]); break;
        #ifdef SYS_RECVMMSG
        case SYS_RECVMMSG: ret = my_recvmmsg(emu, args[0], (void*)args[1], args[2], args[3], (void*)args[4]); break;
        case SYS_SENDMMSG: ret = my___sendmmsg(emu, args[0], (void*)args[1], args[2], args[3]); break;
        #endif
        default:
            printf_log(LOG_DEBUG, "BOX86 Error on Syscall 102: Unknown Soket command %d\n", n);
            ret = -EINVAL;
    }
    return ret;
};
#endif

// cannot include <fcntl.h>, it conflict with some asm includes...
#ifndef O_NONBLOCK
#define O_NONBLOCK 04000
#endif
int my_fcntl(x86emu_t* emu, int fd, int cmd, ... /* arg */ );
long my_ptrace(x86emu_t* emu, int request, pid_t pid, void* addr, uint32_t* data);

// Syscall table for x86 can be found here: http://shell-storm.org/shellcode/files/syscalls.html
typedef struct scwrap_s {
    int nats;
    int nbpars;
} scwrap_t;

scwrap_t syscallwrap[] = {
    [2] ={ __NR_fork, 1 },    // should wrap this one, because of the struct pt_regs (the only arg)?
    //[3] ={ __NR_read, 3 },  // wrapped so SA_RESTART can be handled by libc
    //[4] ={ __NR_write, 3 }, // same
    //[5] ={ __NR_open, 3 },  // flags need transformation
    //[6] ={ __NR_close, 1 },   // wrapped so SA_RESTART can be handled by libc
#ifdef __NR_waitpid
    [7] ={ __NR_waitpid, 3 },
#endif
    [10] { __NR_unlink, 1 },
    [12] { __NR_chdir, 1 },
#ifdef __NR_time
    [13] { __NR_time, 1 },
#endif
    [15] { __NR_chmod, 2 },
    [19] { __NR_lseek, 3 },
    [20] { __NR_getpid, 0 },
    [24] { __NR_getuid, 0 },
    //[26] ={__NR_ptrace, 4},
#ifdef __NR_alarm
    [27] { __NR_alarm, 1 },
#endif
    [33] { __NR_access, 2 },
    [37] { __NR_kill, 2 },
    [38] { __NR_rename, 2 },
    [39] { __NR_mkdir, 2 },
    [40] { __NR_rmdir, 1 },
    [41] { __NR_dup, 1 },
    [42] { __NR_pipe, 1 },
    //[45] { __NR_brk, 1 },
    [47] { __NR_getgid, 0 },
    [49] { __NR_geteuid, 0 },
    [50] { __NR_getegid, 0 },
    [54] { __NR_ioctl, 3 },    // should be wrapped to allow SA_RESTART handling by libc, but syscall is only 3 arguments, ioctl can be 5
    //[55] { __NR_fcntl, 3 },    // wrapped to allow filter of F_SETFD
    [60] { __NR_umask, 1 },
    [63] { __NR_dup2, 2 },
    [64] { __NR_getppid, 0 },
    [66] { __NR_setsid, 0 },
    //[67] { __NR_sigaction, 3 },
    [75] { __NR_setrlimit, 2 },
#ifdef __NR_getrlimit
    [76] { __NR_getrlimit, 2 },
#endif
    [77] { __NR_getrusage, 2 },
    [78] { __NR_gettimeofday, 2 },
    [80] { __NR_getgroups, 2 },
    [83] { __NR_symlink, 2 },
#ifdef __NR_select
    [82] { __NR_select, 5 },
#endif
    [85] { __NR_readlink, 3 },
    //[91] { __NR_munmap, 2 },
    [94] { __NR_fchmod, 2 },
    [99] { __NR_statfs, 2 },
#ifdef __NR_socketcall
    [102] { __NR_socketcall, 2 },
#endif
    [104] { __NR_setitimer, 3 },
    [105] { __NR_getitimer, 2 },
#ifdef __NR_newstat
    [106] { __NR_newstat, 2 },
#else
    [106] { __NR_stat, 2 },
#endif
#ifdef __NR_newlstat
    [107] { __NR_newlstat, 2 },
#else
    [107] { __NR_lstat, 2 },
#endif
#ifdef __NR_newfstat
    [108] { __NR_newfstat, 2 },
#else
    [108] { __NR_fstat, 2 },
#endif
#ifdef __NR_olduname
    [109] { __NR_olduname, 1 },
#endif
#ifdef __NR_iopl
    [110] { __NR_iopl, 1 },
#endif
    [114] { __NR_wait4, 4 }, //TODO: check struct rusage alignment
#ifdef __NR_ipc
    [117] { __NR_ipc, 6 },
#endif
    //[119] { __NR_sigreturn, 0},
    //[120] { __NR_clone, 5 },    // need works
    //[122] { __NR_uname, 1 },
    //[123] { __NR_modify_ldt },
    //[125] { __NR_mprotect, 3 },
    [126] { __NR_sigprocmask, 3 },
    [136] { __NR_personality, 1 },
    [140] { __NR__llseek, 5 },
    [141] { __NR_getdents, 3 },
    [142] { __NR__newselect, 5 },
    [143] { __NR_flock,  2 },
    [144] { __NR_msync, 3 },
    [145] { __NR_readv, 3 },
    [146] { __NR_writev, 3 },
    [148] { __NR_fdatasync, 1 },
    [149] { __NR__sysctl, 1 },    // need wrapping?
    [156] { __NR_sched_setscheduler, 3 },
    [157] { __NR_sched_getscheduler, 1 },
    [158] { __NR_sched_yield, 0 },
    [162] { __NR_nanosleep, 2 },
    [164] { __NR_setresuid, 3 },
    //[168] { __NR_poll, 3 },    // wrapped to allow SA_RESTART wrapping by libc
    [172] { __NR_prctl, 5 },
    //[173] { __NR_rt_sigreturn, 0 },
    [175] { __NR_rt_sigprocmask, 4 },
    [179] { __NR_rt_sigsuspend, 2 },
    [183] { __NR_getcwd, 2 },
    [184] { __NR_capget, 2},
    [185] { __NR_capset, 2},
    [186] { __NR_sigaltstack, 2 },    // neeed wrap or something?
    [191] { __NR_ugetrlimit, 2 },
//    [192] { __NR_mmap2, 6},
    //[195] { __NR_stat64, 2 },  // need proprer wrap because of structure size change
    //[196] { __NR_lstat64, 2 }, // need proprer wrap because of structure size change
    //[197] { __NR_fstat64, 2 },  // need proprer wrap because of structure size change
#ifndef POWERPCLE
    [199] { __NR_getuid32, 0 },
    [200] { __NR_getgid32, 0 },
    [201] { __NR_geteuid32, 0 },
    [202] { __NR_getegid32, 0 },
    [208] { __NR_setresuid32, 3 },
    [209] { __NR_getresuid32, 3 },
    [210] { __NR_setresgid32, 3 },
    [211] { __NR_getresgid32, 3 },
#else
    // PowerPC uses the same syscalls for 32/64 bit processes.
    [199] { __NR_getuid, 0 },
    [200] { __NR_getgid, 0 },
    [201] { __NR_geteuid, 0 },
    [202] { __NR_getegid, 0 },
    [208] { __NR_setresuid, 3 },
    [209] { __NR_getresuid, 3 },
    [210] { __NR_setresgid, 3 },
    [211] { __NR_getresgid, 3 },
#endif
    [220] { __NR_getdents64, 3 },
    //[221] { __NR_fcntl64, 3 },
    [224] { __NR_gettid, 0 },
    [240] { __NR_futex, 6 },
    [241] { __NR_sched_setaffinity, 3 },
    [242] { __NR_sched_getaffinity, 3 },
    [252] { __NR_exit_group, 1 },
#ifdef NOALIGN
    [254] { __NR_epoll_create, 1 },
    [255] { __NR_epoll_ctl, 4 },
    [256] { __NR_epoll_wait, 4 },
#endif
    [265] { __NR_clock_gettime, 2 },
    [266] { __NR_clock_getres, 2 },
    //[270] { __NR_tgkill, 3 },
    [271] { __NR_utimes, 2 },
    [291] { __NR_inotify_init, 0},
    [292] { __NR_inotify_add_watch, 3},
    [293] { __NR_inotify_rm_watch, 2},
    [311] { __NR_set_robust_list, 2 },
    [312] { __NR_get_robust_list, 4 },
    [318] { __NR_getcpu, 3},
    [328] { __NR_eventfd2, 2},
#ifdef NOALIGN
    [329] { __NR_epoll_create1, 1 },
#endif
    [331] { __NR_pipe2, 2},
    [332] { __NR_inotify_init1, 1},
#ifdef __NR_getrandom
    [355] { __NR_getrandom, 3 },
#endif
#ifdef __NR_memfd_create
    [356] { __NR_memfd_create, 2},
#endif
    //[449] { __NR_futex_waitv, 5},
};

struct mmap_arg_struct {
    unsigned long addr;
    unsigned long len;
    unsigned long prot;
    unsigned long flags;
    unsigned long fd;
    unsigned long offset;
};

#undef st_atime
#undef st_ctime
#undef st_mtime

struct x86_pt_regs {
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long eax;
	int  xds;
	int  xes;
	int  xfs;
	int  xgs;
	long orig_eax;
	long eip;
	int  xcs;
	long eflags;
	long esp;
	int  xss;
};

#ifndef __NR_olduname
struct oldold_utsname {
        char sysname[9];
        char nodename[9];
        char release[9];
        char version[9];
        char machine[9];
};
#endif
struct old_utsname {
        char sysname[65];
        char nodename[65];
        char release[65];
        char version[65];
        char machine[65];
};

struct i386_user_desc {
    unsigned int  entry_number;
    unsigned long base_addr;
    unsigned int  limit;
    unsigned int  seg_32bit:1;
    unsigned int  contents:2;
    unsigned int  read_exec_only:1;
    unsigned int  limit_in_pages:1;
    unsigned int  seg_not_present:1;
    unsigned int  useable:1;
};

int clone_fn(void* arg)
{
    x86emu_t *emu = (x86emu_t*)arg;
    R_EAX = 0;
    DynaRun(emu);
    int ret = R_EAX;
    FreeX86Emu(&emu);
    my_context->stack_clone_used = 0;
    return ret;
}

#ifndef __NR_ipc
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

static int my_ipc (uint call, int first, int second, int third, void *ptr, long fifth)
{
#define IPC_SEMOP		 1
#define IPC_SEMGET		 2
#define IPC_SEMCTL		 3
#define IPC_SEMTIMEDOP	 4
#define IPC_MSGSND		11
#define IPC_MSGRCV		12
#define IPC_MSGGET		13
#define IPC_MSGCTL		14
#define IPC_SHMAT		21
#define IPC_SHMDT		22
#define IPC_SHMGET		23
#define IPC_SHMCTL		24
    int version;
    version = call>>16;
    call &= 0xffff;
    switch (call) {
        case IPC_SHMGET: 
            return semget(first, second, third);
        default:
            printf_log(LOG_INFO, "Unsuported IPC call %d (version %d)\n", call, version);
            return -ENOSYS;
    }
}
#endif

void EXPORT x86Syscall(x86emu_t *emu)
{
    RESET_FLAGS(emu);
    uint32_t s = R_EAX;
    printf_log(LOG_DEBUG, "%p: Calling syscall 0x%02X (%d) %p %p %p %p %p", (void*)R_EIP, s, s, (void*)R_EBX, (void*)R_ECX, (void*)R_EDX, (void*)R_ESI, (void*)R_EDI); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    if(s<cnt && syscallwrap[s].nats) {
        int sc = syscallwrap[s].nats;
        switch(syscallwrap[s].nbpars) {
            case 0: S_EAX = syscall(sc); break;
            case 1: S_EAX = syscall(sc, R_EBX); break;
            case 2: if(s==33) {printf_log(LOG_DEBUG, " => sys_access(\"%s\", %d)\n", (char*)R_EBX, R_ECX);}; S_EAX = syscall(sc, R_EBX, R_ECX); break;
            case 3: S_EAX = syscall(sc, R_EBX, R_ECX, R_EDX); break;
            case 4: S_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI); break;
            case 5: S_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI); break;
            case 6: S_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI, R_EBP); break;
            default:
                printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[s].nbpars); 
                emu->quit = 1;
                return;
        }
        if(S_EAX==-1 && errno>0)
            S_EAX = -errno;
        printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
        return;
    }
    switch (s) {
        case 1: // sys_exit
            emu->quit = 1;
            emu->exit = 1;
            //R_EAX = syscall(__NR_exit, R_EBX);  // the syscall should exit only current thread
            R_EAX = R_EBX; // faking the syscall here, we don't want to really terminate the thread now
            break;
        case 3:  // sys_read
            S_EAX = read(S_EBX, (void*)R_ECX, (size_t)R_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 4:  // sys_write
            S_EAX = write(S_EBX, (void*)R_ECX, (size_t)R_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 5: // sys_open
            if(s==5) {printf_log(LOG_DEBUG, " => sys_open(\"%s\", %d, %d)", (char*)R_EBX, of_convert(R_ECX), R_EDX);}; 
            //S_EAX = open((void*)R_EBX, of_convert(R_ECX), R_EDX);
            S_EAX = my_open(emu, (void*)R_EBX, of_convert(R_ECX), R_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 6:  // sys_close
            S_EAX = close(S_EBX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#ifndef __NR_waitpid
        case 7: //sys_waitpid
            R_EAX = waitpid((pid_t)R_EBX, (int*)R_ECX, S_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
        case 11: // sys_execve
            {
                char* prog = (char*)R_EBX;
                char** argv = (char**)R_ECX;
                char** envv = (char**)R_EDX;
                printf_log(LOG_DEBUG, " => sys_execve(\"%s\", %p(\"%s\", \"%s\", \"%s\"...), %p)\n", prog, argv, (argv && argv[0])?argv[0]:"nil", (argv && argv[0] && argv[1])?argv[1]:"nil", (argv && argv[0] && argv[1] && argv[2])?argv[2]:"nil", envv);
                R_EAX = my_execve(emu, (const char*)R_EBX, (void*)R_ECX, (void*)R_EDX);
            }
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#ifndef __NR_time
        case 13:    // sys_time (it's deprecated and remove on ARM EABI it seems)
            R_EAX = time(NULL);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
        case 26:    // sys_ptrace
            S_EAX = my_ptrace(emu, R_EBX, R_ECX, (void*)R_EDX, (void*)R_ESI);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#ifndef __NR_alarm
        case 27:
            R_EAX = alarm(R_EBX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
        case 45:    // sys_brk
            R_EAX = (uintptr_t)ElfSetBrk((void*)R_EBX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 54: // sys_ioctl
            S_EAX = ioctl(S_EBX, R_ECX, R_EDX, R_ESI, R_EDI);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 55: // sys_fcntl
            S_EAX = my_fcntl(emu, S_EBX, S_ECX, R_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 67: // sys_sigaction
            S_EAX = my_syscall_sigaction(emu, R_EBX, (x86_sigaction_restorer_t*)R_ECX, (x86_sigaction_restorer_t*)R_EDX, R_ESI);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#ifndef __NR_getrlimit
        case 76:    // sys_getrlimit... this is the old version, using the new one. Maybe some tranform is needed?
            R_EAX = getrlimit(R_EBX, (void*)R_ECX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
#ifndef __NR_select
        case 82:   // select
            R_EAX = select(R_EBX, (fd_set*)R_ECX, (fd_set*)R_EDX, (fd_set*)R_ESI, (struct timeval*)R_EDI);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
        case 90:    // old_mmap
            {
                struct mmap_arg_struct *st = (struct mmap_arg_struct*)R_EBX;
                R_EAX = (uintptr_t)my_mmap(emu, (void*)st->addr, st->len, st->prot, st->flags, st->fd, st->offset);
            }
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 91:   // munmap
            R_EAX = my_munmap(emu, (void*)R_EBX, (unsigned long)R_ECX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#ifndef __NR_socketcall
        case 102:
            R_EAX = my_socketcall(emu, R_EBX, (unsigned long *)R_ECX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
#ifndef __NR_olduname
        case 109:   // olduname
            {
                struct utsname un;
                R_EAX = uname(&un);
                if(!R_EAX) {
                    struct oldold_utsname *old = (struct oldold_utsname*)R_EBX;
                    memcpy(old->sysname, un.sysname, 9);
                    memcpy(old->nodename, un.nodename, 9);
                    memcpy(old->release, un.release, 9);
                    memcpy(old->version, un.version, 9);
                    strcpy(old->machine, "i686");
                }
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            }
            break;
#endif
#ifndef __NR_iopl
        case 110:   // iopl
            R_EAX = 0;  // only on x86, so return 0...
            break;
#endif
#ifndef __NR_ipc
        case 117:   // ipc
            S_EAX = my_ipc(R_EBX, S_ECX, S_EDX, S_ESI, (void*)R_EDI, (long)R_EBP);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
        case 119: // sys_sigreturn
            emu->quit = 1;  // we should be inside a DynaCall in a sigaction callback....
            break;
        case 120:   // clone
            // x86 raw syscall is long clone(unsigned long flags, void *stack, int *parent_tid, unsigned long tls, int *child_tid);
            printf_log(LOG_DEBUG, "clone syscall flags=0x%x, stack=%p, parent_tid=%d,tls=%p, child_tid=%d\n", R_EBX, (void*)R_ECX, R_EDX, (void*)R_ESI, R_EDI);
            // so flags=R_EBX, stack=R_ECX, parent_tid=R_EDX, tls=R_ESI, child_tid=R_EDI
            if((R_EBX&~0xff)==0x4100) {
                // this is a case of vfork...
                R_EAX = my_vfork(emu);
                if(S_EAX==-1)
                    S_EAX = -errno;
            } else {
                if(R_ECX)
                {
                    void* stack_base = (void*)R_ECX;
                    int stack_size = 0;
                    uintptr_t sp = R_ECX;
                    if(!R_ECX) {
                        // allocate a new stack...
                        int currstack = 0;
                        if((R_ESP>=(uintptr_t)emu->init_stack) && (R_ESP<=((uintptr_t)emu->init_stack+emu->size_stack)))
                            currstack = 1;
                        stack_size = (currstack)?emu->size_stack:(1024*1024);
                        stack_base = mmap(NULL, stack_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
                        // copy value from old stack to new stack
                        if(currstack) {
                            memcpy(stack_base, emu->init_stack, stack_size);
                            sp = (uintptr_t)emu->init_stack + R_ESP - (uintptr_t)stack_base;
                        } else {
                            int size_to_copy = (uintptr_t)emu->init_stack + emu->size_stack - (R_ESP);
                            memcpy(stack_base+stack_size-size_to_copy, (void*)R_ESP, size_to_copy);
                            sp = (uintptr_t)stack_base+stack_size-size_to_copy;
                        }
                    }
                    x86emu_t * newemu = NewX86Emu(emu->context, R_EIP, (uintptr_t)stack_base, stack_size, (R_ECX)?0:1);
                    SetupX86Emu(newemu);
                    CloneEmu(newemu, emu);
                    newemu->regs[_SP].dword[0] = sp;  // setupnew stack pointer
                    void* mystack = NULL;
                    if(my_context->stack_clone_used) {
                        mystack = box_malloc(1024*1024);  // stack for own process... memory leak, but no practical way to remove it
                    } else {
                        if(!my_context->stack_clone)
                            my_context->stack_clone = box_malloc(1024*1024);
                        mystack = my_context->stack_clone;
                        my_context->stack_clone_used = 1;
                    }
                    long ret = clone(clone_fn, (void*)((uintptr_t)mystack+1024*1024), R_EBX, newemu, R_EDX, R_ESI, R_EDI);
                    S_EAX = ret;
                }
                else
                    S_EAX = syscall(__NR_clone, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI);
                if(S_EAX==-1 && errno>0)
                    S_EAX = -errno;
            }
            break;
        case 122:   // uname
            {
                struct utsname un;
                R_EAX = uname(&un);
                if(!R_EAX) {
                    struct old_utsname *old = (struct old_utsname*)R_EBX;
                    memcpy(old->sysname, un.sysname, 65);
                    memcpy(old->nodename, un.nodename, 65);
                    memcpy(old->release, un.release, 65);
                    memcpy(old->version, un.version, 65);
                    strcpy(old->machine, "i686");
                }
            }
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 123:   // SYS_modify_ldt
            R_EAX = my_modify_ldt(emu, R_EBX, (thread_area_t*)R_ECX, R_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 125:   // mprotect
            R_EAX = my_mprotect(emu, (void*)R_EBX, (unsigned long)R_ECX, R_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 168: // sys_poll
            S_EAX = poll((void*)R_EBX, R_ECX, S_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 173: // sys_rt_sigreturn
            emu->quit = 1;  // we should be inside a DynaCall in a sigaction callback....
            break;
        case 174: // sys_rt_sigaction
            //printf_log(LOG_NONE, "Warning, Ignoring sys_rt_sigaction(0x%02X, %p, %p)\n", R_EBX, (void*)R_ECX, (void*)R_EDX);
            R_EAX = my_syscall_sigaction(emu, R_EBX, (void*)R_ECX, (void*)R_EDX, R_ESI);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 190:   // vfork
            {
                int r = my_vfork(emu);
                R_EAX = r;
            }
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 192:   // mmap2
            S_EAX = (intptr_t)my_mmap64(emu, (void*)R_EBX, (unsigned long)R_ECX, R_EDX, R_ESI, R_EDI, R_EBP);
            break;
        case 195:   // stat64
            {   
                struct stat64 st;
                unsigned int r = syscall(__NR_stat64, R_EBX, &st);
                UnalignSysStat64(&st, (void*)R_ECX);
                
                R_EAX = r;
            }
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 196:   // lstat64
            {   
                struct stat64 st;
                unsigned int r = syscall(__NR_lstat64, R_EBX, &st);
                UnalignSysStat64(&st, (void*)R_ECX);
                
                R_EAX = r;
            }
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 197:   // fstat64
            {   
                struct stat64 st;
                unsigned int r = syscall(__NR_fstat64, R_EBX, &st);
                UnalignSysStat64(&st, (void*)R_ECX);
                
                R_EAX = r;
            }
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 221: // sys_fcntl64
            S_EAX = my_fcntl(emu, S_EBX, S_ECX, R_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 243: // set_thread_area
            R_EAX = my_set_thread_area((thread_area_t*)R_EBX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#ifndef NOALIGN
        case 254: // epoll_create
            R_EAX = my_epoll_create(emu, S_EBX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 255: // epoll_ctl
            R_EAX = my_epoll_ctl(emu, S_EBX, S_ECX, S_EDX, (void*)R_ESI);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
        case 256: // epoll_wait
            R_EAX = my_epoll_wait(emu, S_EBX, (void*)R_ECX, S_EDX, S_ESI);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
        case 270:   // tgkill
            R_EAX = syscall(__NR_tgkill, R_EBX, R_ECX, R_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#ifndef NOALIGN
        case 329:   // epoll_create1
            R_EAX = my_epoll_create1(emu, of_convert(R_EBX));
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
#ifndef __NR_getrandom
        case 355:  // getrandom
            R_EAX = my_getrandom(emu, (void*)R_EBX, R_ECX, R_EDX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
#ifndef __NR_memfd_create
        case 356:  // memfd_create
            R_EAX = my_memfd_create(emu, (void*)R_EBX, R_ECX);
            if(S_EAX==-1 && errno>0)
                S_EAX = -errno;
            break;
#endif
        case 449:
            #ifdef __NR_futex_waitv
            if(box86_futex_waitv)
                S_EAX = syscall(__NR_futex_waitv, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI);
            else
            #endif
                S_EAX = -ENOSYS;
            break;
        default:
            printf_log(LOG_INFO, "Warning: Unsupported Syscall 0x%02Xh (%d)\n", s, s);
            S_EAX = -ENOSYS;
            return;
    }
    printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
}

#define stack(n) (R_ESP+4+n)
#define i32(n)  *(int32_t*)stack(n)
#define u32(n)  *(uint32_t*)stack(n)
#define p(n)    *(void**)stack(n)

long EXPORT my_syscall(x86emu_t *emu)
{
    static uint32_t warned = 0;
    uint32_t s = u32(0);
    printf_log(LOG_DEBUG, "%p: Calling libc syscall 0x%02X (%d) %p %p %p %p %p\n", (void*)R_EIP, s, s, (void*)u32(4), (void*)u32(8), (void*)u32(12), (void*)u32(16), (void*)u32(20)); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    if(s<cnt && syscallwrap[s].nats) {
        int sc = syscallwrap[s].nats;
        switch(syscallwrap[s].nbpars) {
            case 0: return syscall(sc);
            case 1: return syscall(sc, u32(4));
            case 2: return syscall(sc, u32(4), u32(8));
            case 3: return syscall(sc, u32(4), u32(8), u32(12));
            case 4: return syscall(sc, u32(4), u32(8), u32(12), u32(16));
            case 5: return syscall(sc, u32(4), u32(8), u32(12), u32(16), u32(20));
            case 6: return syscall(sc, u32(4), u32(8), u32(12), u32(16), u32(20), u32(24));
            default:
                printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[s].nbpars); 
                emu->quit = 1;
                return 0;
        }
    }
    switch (s) {
        case 1: // __NR_exit
            emu->quit = 1;
            return i32(4); // faking the syscall here, we don't want to really terminate the program now
        case 3:  // sys_read
            return read(i32(4), p(8), u32(12));
        case 4:  // sys_write
            return write(i32(4), p(8), u32(12));
        case 5: // sys_open
            return my_open(emu, p(4), of_convert(u32(8)), u32(12));
        case 6:  // sys_close
            return close(i32(4));
        case 11: // execve
            return my_execve(emu, p(4), p(8), p(12));
#ifndef __NR_alarm
        case 27:
            return alarm(R_EBX);
#endif
        case 45: // brk
            return (intptr_t)ElfSetBrk(p(4));
        case 67: // sys_sigaction
            return my_syscall_sigaction(emu, u32(4), (x86_sigaction_restorer_t*)p(8), (x86_sigaction_restorer_t*)p(12), u32(16));
        case 91:   // munmap
            return my_munmap(emu, p(4), u32(8));
#ifndef __NR_socketcall
        case 102:
            return my_socketcall(emu, i32(4), (unsigned long *)p(8));
#endif
#ifndef __NR_ipc
        case 117:   // ipc
            return my_ipc(u32(4), i32(8), i32(12), i32(16), p(20), i32(24));
#endif
        case 120:   // clone
            // x86 raw syscall is long clone(unsigned long flags, void *stack, int *parent_tid, unsigned long tls, int *child_tid);
            // so flags=u(4), stack=p(8), parent_tid=p(12), tls=p(16), child_tid=p(20)
            if(p(8))
            {
                void* stack_base = p(8);
                int stack_size = 0;
                uintptr_t sp = u32(8);
                if(!stack_base) {
                    // allocate a new stack...
                    int currstack = 0;
                    if((R_ESP>=(uintptr_t)emu->init_stack) && (R_ESP<=((uintptr_t)emu->init_stack+emu->size_stack)))
                        currstack = 1;
                    stack_size = (currstack)?emu->size_stack:(1024*1024);
                    stack_base = mmap(NULL, stack_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
                    // copy value from old stack to new stack
                    if(currstack) {
                        memcpy(stack_base, emu->init_stack, stack_size);
                        sp = (uintptr_t)emu->init_stack + R_ESP - (uintptr_t)stack_base;
                    } else {
                        int size_to_copy = (uintptr_t)emu->init_stack + emu->size_stack - (R_ESP);
                        memcpy(stack_base+stack_size-size_to_copy, (void*)R_ESP, size_to_copy);
                        sp = (uintptr_t)stack_base+stack_size-size_to_copy;
                    }
                }
                x86emu_t * newemu = NewX86Emu(emu->context, R_EIP, (uintptr_t)stack_base, stack_size, (p(8))?0:1);
                SetupX86Emu(newemu);
                CloneEmu(newemu, emu);
                newemu->regs[_SP].dword[0] = sp;
                void* mystack = NULL;
                if(my_context->stack_clone_used) {
                    mystack = box_malloc(1024*1024);  // stack for own process... memory leak, but no practical way to remove it
                } else {
                    if(!my_context->stack_clone)
                        my_context->stack_clone = box_malloc(1024*1024);
                    mystack = my_context->stack_clone;
                    my_context->stack_clone_used = 1;
                }
                // x86_64 raw clone is long clone(unsigned long flags, void *stack, int *parent_tid, int *child_tid, unsigned long tls);
                long ret = clone(clone_fn, (void*)((uintptr_t)mystack+1024*1024), u32(4), newemu, p(12), p(16), p(20));
                return ret;
            }
            else
                return syscall(__NR_clone, u32(4), p(8), p(12), p(16), p(20));
            break;
        case 123:   // SYS_modify_ldt
            return my_modify_ldt(emu, i32(4), (thread_area_t*)p(8), i32(12));
        case 125:   // mprotect
            return my_mprotect(emu, p(4), u32(8), i32(12));
        case 174:   // sys_rt_sigaction
            return my_syscall_sigaction(emu, i32(4), p(8), p(12), i32(16));
        case 192:   // mmap2
            return (intptr_t)my_mmap64(emu, p(4), u32(8), i32(12), i32(16), i32(20), u32(24));
        case 243: // set_thread_area
            return my_set_thread_area((thread_area_t*)p(4));
#ifndef NOALIGN
        case 254: // epoll_create
            return my_epoll_create(emu, i32(4));
        case 255: // epoll_ctl
            return my_epoll_ctl(emu, i32(4), i32(8), i32(12), p(16));
        case 256: // epoll_wait
            return my_epoll_wait(emu, i32(4), p(8), i32(12), i32(16));
#endif
        case 270: //_NR_tgkill
            /*if(!u32(12))*/ {
                //printf("tgkill(%u, %u, %u) => ", u32(4), u32(8), u32(12));
                uint32_t ret = (uint32_t)syscall(__NR_tgkill, u32(4), u32(8), u32(12));
                //printf("%u (errno=%d)\n", ret, (ret==(uint32_t)-1)?errno:0);
                return ret;
            }/* else {
                printf_log(LOG_INFO, "Warning: ignoring libc Syscall tgkill (%u, %u, %u)\n", u32(4), u32(8), u32(12));
            }*/
            return 0;
#ifndef NOALIGN
        case 329:   // epoll_create1
            return my_epoll_create1(emu, of_convert(i32(4)));
#endif
#ifndef __NR_getrandom
        case 355:  // getrandom
            return my_getrandom(emu, p(4), u32(8), u32(12));
#endif
#ifndef __NR_memfd_create
        case 356:  // memfd_create
            return my_memfd_create(emu, p(4), u32(8));
#endif
        case 449:
            #ifdef __NR_futex_waitv
            if(box86_futex_waitv)
                return syscall(__NR_futex_waitv, u32(4), u32(8), u32(12), u32(16), u32(20));
            else
            #endif
                {
                    errno = ENOSYS;
                    return -1;
                }
        default:
            if(!(warned&(1<<s))) {
                printf_log(LOG_INFO, "Warning: Unsupported libc Syscall 0x%02X (%d)\n", s, s);
                warned|=(1<<s);
            }
            errno = ENOSYS;
            return -1;
    }
}
