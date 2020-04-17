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

#ifndef __NR_socketcall
//#ifndef SYS_RECVMMSG
//#define SYS_RECVMMSG    19
//#endif
//#ifndef SYS_SENDMMSG
//#define SYS_SENDMMSG    20
//#endif
int32_t my_accept4(x86emu_t* emu, int32_t fd, void* a, void* l, int32_t flags); // not always present, so used wrapped version
#endif


int32_t my_getrandom(x86emu_t* emu, void* buf, uint32_t buflen, uint32_t flags);
int of_convert(int flag);
int32_t my_open(x86emu_t* emu, void* pathname, int32_t flags, uint32_t mode);

// cannot include <fcntl.h>, it conflict with some asm includes...
#ifndef O_NONBLOCK
#define O_NONBLOCK 04000
#endif
#undef fcntl
int fcntl(int fd, int cmd, ... /* arg */ );

// Syscall table for x86 can be found here: http://shell-storm.org/shellcode/files/syscalls.html
typedef struct scwrap_s {
    int x86s;
    int nats;
    int nbpars;
} scwrap_t;

scwrap_t syscallwrap[] = {
    { 2, __NR_fork, 1 },    // should wrap this one, because of the struct pt_regs (the only arg)?
    //{ 3, __NR_read, 3 },  // wrapped so SA_RESTART can be handled by libc
    //{ 4, __NR_write, 3 }, // same
    //{ 5, __NR_open, 3 },  // flags need transformation
    //{ 6, __NR_close, 1 },   // wrapped so SA_RESTART can be handled by libc
#ifdef __NR_waitpid
    { 7, __NR_waitpid, 3 },
#endif
    { 10, __NR_unlink, 1 },
    { 12, __NR_chdir, 1 },
#ifdef __NR_time
    { 13, __NR_time, 1 },
#endif
    { 15, __NR_chmod, 2 },
    { 19, __NR_lseek, 3 },
    { 20, __NR_getpid, 0 },
    { 24, __NR_getuid, 0 },
    { 33, __NR_access, 2 },
    { 37, __NR_kill, 2 },
    { 38, __NR_rename, 2 },
    { 39, __NR_mkdir, 2 },
    { 40, __NR_rmdir, 1 },
    { 41, __NR_dup, 1 },
    { 42, __NR_pipe, 1 },
    { 45, __NR_brk, 1 },
    { 47, __NR_getgid, 0 },
    { 49, __NR_geteuid, 0 },
    { 50, __NR_getegid, 0 },
    //{ 54, __NR_ioctl, 5 },    // wrapped to allow SA_RESTART handling by libc
    //{ 55, __NR_fcntl, 3 },    // wrapped to allow filter of F_SETFD
    { 60, __NR_umask, 1 },
    { 63, __NR_dup2, 2 },
    { 64, __NR_getppid, 0 },
    { 75, __NR_setrlimit, 2 },
#ifdef __NR_getrlimit
    { 76, __NR_getrlimit, 2 },
#endif
    { 77, __NR_getrusage, 2 },
    { 78, __NR_gettimeofday, 2 },
    { 83, __NR_symlink, 2 },
#ifdef __NR_select
    { 82, __NR_select, 5 },
#endif
    { 85, __NR_readlink, 3 },
    { 91, __NR_munmap, 2 },
    { 94, __NR_fchmod, 2 },
    { 99, __NR_statfs, 2 },
#ifdef __NR_socketcall
    { 102, __NR_socketcall, 2 },
#endif
    { 104, __NR_setitimer, 3 },
    { 105, __NR_getitimer, 2 },
#ifdef __NR_newstat
    { 106, __NR_newstat, 2 },
#else
    { 106, __NR_stat, 2 },
#endif
#ifdef __NR_newlstat
    { 107, __NR_newlstat, 2 },
#else
    { 107, __NR_lstat, 2 },
#endif
#ifdef __NR_newfstat
    { 108, __NR_newfstat, 2 },
#else
    { 108, __NR_fstat, 2 },
#endif
#ifdef __NR_olduname
    { 109, __NR_olduname, 1 },
#endif
#ifdef __NR_iopl
    { 110, __NR_iopl, 1 },
#endif
    { 114, __NR_wait4, 4 }, //TODO: check struct rusage alignment
#ifdef __NR_ipc
    { 117, __NR_ipc, 6 },
#endif
    //{ 119, __NR_sigreturn, 0},
    //{ 120, __NR_clone, 5 },    // need works
    //{ 122, __NR_uname, 1 },
    { 125, __NR_mprotect, 3 },
    { 136, __NR_personality, 1 },
    { 140, __NR__llseek, 5 },
    { 141, __NR_getdents, 3 },
    { 142, __NR__newselect, 5 },
    { 143, __NR_flock,  2 },
    { 144, __NR_msync, 3 },
    { 145, __NR_readv, 3 },
    { 146, __NR_writev, 3 },
    { 148, __NR_fdatasync, 1 },
    { 149, __NR__sysctl, 1 },    // need wrapping?
    { 156, __NR_sched_setscheduler, 3 },
    { 157, __NR_sched_getscheduler, 1 },
    { 158, __NR_sched_yield, 0 },
    { 162, __NR_nanosleep, 2 },
    { 164, __NR_setresuid, 3 },
    //{ 168, __NR_poll, 3 },    // wrapped to allow SA_RESTART wrapping by libc
    { 172, __NR_prctl, 5 },
    //{ 173, __NR_rt_sigreturn, 0 },
    { 175, __NR_rt_sigprocmask, 4 },
    { 179, __NR_rt_sigsuspend, 2 },
    { 183, __NR_getcwd, 2 },
    { 186, __NR_sigaltstack, 2 },    // neeed wrap or something?
    { 191, __NR_ugetrlimit, 2 },
    { 192, __NR_mmap2, 6},
    //{ 195, __NR_stat64, 2 },  // need proprer wrap because of structure size change
    //{ 197, __NR_fstat64, 2 },  // need proprer wrap because of structure size change
    { 199, __NR_getuid32, 0 },
    { 200, __NR_getgid32, 0 },
    { 201, __NR_geteuid32, 0 },
    { 202, __NR_getegid32, 0 },
    { 208, __NR_setresuid32, 3 },
    { 220, __NR_getdents64, 3 },
    //{ 221, __NR_fcntl64, 3 },
    { 224, __NR_gettid, 0 },
    { 240, __NR_futex, 6 },
    { 252, __NR_exit_group, 1 },
    { 265, __NR_clock_gettime, 2 },
    { 266, __NR_clock_getres, 2 },
    //{ 270, __NR_tgkill, 3 },
    { 311, __NR_set_robust_list, 2 },
    { 312, __NR_get_robust_list, 4 },
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
    return ret;
}

void EXPORT x86Syscall(x86emu_t *emu)
{
    RESET_FLAGS(emu);
    uint32_t s = R_EAX;
    printf_log(LOG_DEBUG, "%p: Calling syscall 0x%02X (%d) %p %p %p %p %p", (void*)R_EIP, s, s, (void*)R_EBX, (void*)R_ECX, (void*)R_EDX, (void*)R_ESI, (void*)R_EDI); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x86s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: *(int32_t*)&R_EAX = syscall(sc); break;
                case 1: *(int32_t*)&R_EAX = syscall(sc, R_EBX); break;
                case 2: if(s==33) {printf_log(LOG_DUMP, " => sys_access(\"%s\", %d)\n", (char*)R_EBX, R_ECX);}; *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX); break;
                case 3: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX); break;
                case 4: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI); break;
                case 5: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI); break;
                case 6: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI, R_EBP); break;
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return;
            }
            printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
            return;
        }
    }
    switch (s) {
        case 1: // sys_exit
            emu->quit = 1;
            emu->exit = 1;
            R_EAX = R_EBX; // faking the syscall here, we don't want to really terminate the program now
            break;
        case 3:  // sys_read
            R_EAX = (uint32_t)read((int)R_EBX, (void*)R_ECX, (size_t)R_EDX);
            break;
        case 4:  // sys_write
            R_EAX = (uint32_t)write((int)R_EBX, (void*)R_ECX, (size_t)R_EDX);
            break;
        case 5: // sys_open
            if(s==5) {printf_log(LOG_DUMP, " => sys_open(\"%s\", %d, %d)", (char*)R_EBX, of_convert(R_ECX), R_EDX);}; 
            //R_EAX = (uint32_t)open((void*)R_EBX, of_convert(R_ECX), R_EDX);
            R_EAX = (uint32_t)my_open(emu, (void*)R_EBX, of_convert(R_ECX), R_EDX);
            break;
        case 6:  // sys_close
            R_EAX = (uint32_t)close((int)R_EBX);
            break;
#ifndef __NR_waitpid
        case 7: //sys_waitpid
            R_EAX = waitpid((pid_t)R_EBX, (int*)R_ECX, (int)R_EDX);
            break;
#endif
        case 11: // sys_execve
            {
                char* prog = (char*)R_EBX;
                char** argv = (char**)R_ECX;
                char** envv = (char**)R_EDX;
                printf_log(LOG_DUMP, " => sys_execve(\"%s\", %p(\"%s\", \"%s\", \"%s\"...), %p)\n", prog, argv, (argv && argv[0])?argv[0]:"nil", (argv && argv[0] && argv[1])?argv[1]:"nil", (argv && argv[0] && argv[1] && argv[2])?argv[2]:"nil", envv);
                R_EAX = execve((const char*)R_EBX, (void*)R_ECX, (void*)R_EDX);
            }
            break;
#ifndef __NR_time
        case 13:    // sys_time (it's deprecated and remove on ARM EABI it seems)
            R_EAX = time(NULL);
            break;
#endif
        case 54: // sys_ioctl
            R_EAX = (uint32_t)ioctl((int)R_EBX, R_ECX, R_EDX, R_ESI, R_EDI);
            break;
        case 55: // sys_fcntl
            if(R_ECX==4) {
                // filter out O_NONBLOCK so old stacally linked games that access X11 don't get EAGAIN error sometimes
                int tmp = of_convert((int)R_EDX)&(~O_NONBLOCK);
                if(R_EDX==0xFFFFF7FF) {
                    // special case for ~O_NONBLOCK...
                    int flags = fcntl(R_EBX, 3);
                    tmp = flags&~O_NONBLOCK;
                }
                R_EAX = (uint32_t)fcntl((int)R_EBX, (int)R_ECX, tmp);
            } else
                R_EAX = (uint32_t)fcntl((int)R_EBX, (int)R_ECX, R_EDX);
            break;
#ifndef __NR_getrlimit
        case 76:    // sys_getrlimit... this is the old version, using the new one. Maybe some tranform is needed?
            R_EAX = getrlimit(R_EBX, (void*)R_ECX);
            break;
#endif
#ifndef __NR_select
        case 82:   // select
            R_EAX = select(R_EBX, (fd_set*)R_ECX, (fd_set*)R_EDX, (fd_set*)R_ESI, (struct timeval*)R_EDI);
            break;
#endif
        case 90:    // old_mmap
            {
                struct mmap_arg_struct *st = (struct mmap_arg_struct*)R_EBX;
                R_EAX = (uintptr_t)mmap((void*)st->addr, st->len, st->prot, st->flags, st->fd, st->offset);
            }
            break;
#ifndef __NR_socketcall
        case 102: {
                unsigned long *args = (unsigned long *)R_ECX;
                // need to do all call "by hand"
                switch(R_EBX) {
                    case SYS_SOCKET: R_EAX = socket(args[0], args[1], args[2]); break;
                    case SYS_BIND: R_EAX = bind(args[0], (void*)args[1], args[2]); break;
                    case SYS_CONNECT: R_EAX = connect(args[0], (void*)args[1], args[2]); break;
                    case SYS_LISTEN: R_EAX = listen(args[0], args[1]); break;
                    case SYS_ACCEPT: R_EAX = accept(args[0], (void*)args[1], (void*)args[2]); break;
                    case SYS_GETSOCKNAME: R_EAX = getsockname(args[0], (void*)args[1], (void*)args[2]); break;
                    case SYS_GETPEERNAME: R_EAX = getpeername(args[0], (void*)args[1], (void*)args[2]); break;
                    case SYS_SOCKETPAIR: R_EAX = socketpair(args[0], args[1], args[2], (int*)args[3]); break;
                    case SYS_SEND: R_EAX = send(args[0], (void*)args[1], args[2], args[3]); break;
                    case SYS_RECV: R_EAX = recv(args[0], (void*)args[1], args[2], args[3]); break;
                    case SYS_SENDTO: R_EAX = sendto(args[0], (void*)args[1], args[2], args[3], (void*)args[4], args[5]); break;
                    case SYS_RECVFROM: R_EAX = recvfrom(args[0], (void*)args[1], args[2], args[3], (void*)args[4], (void*)args[5]); break;
                    case SYS_SHUTDOWN: R_EAX = shutdown(args[0], args[1]); break;
                    case SYS_SETSOCKOPT: R_EAX = setsockopt(args[0], args[1], args[2], (void*)args[3], args[4]); break;
                    case SYS_GETSOCKOPT: R_EAX = getsockopt(args[0], args[1], args[2], (void*)args[3], (void*)args[4]); break;
                    case SYS_SENDMSG: R_EAX = sendmsg(args[0], (void*)args[1], args[2]); break;
                    case SYS_RECVMSG: R_EAX = recvmsg(args[0], (void*)args[1], args[2]); break;
                    case SYS_ACCEPT4: R_EAX = my_accept4(emu, args[0], (void*)args[1], (void*)args[2], args[3]); break;
                    #ifdef SYS_RECVMMSG
                    // TODO: Create my_ version of recvmmsg and sendmmsg
                    case SYS_RECVMMSG: R_EAX = recvmmsg(args[0], (void*)args[1], args[2], args[3], (void*)args[4]); break;
                    case SYS_SENDMMSG: R_EAX = sendmmsg(args[0], (void*)args[1], args[2], args[3]); break;
                    #endif
                    default:
                        printf_log(LOG_DEBUG, "BOX86 Error on Syscall 102: Unknown Soket command %d\n", R_EBX);
                        R_EAX = -1;
                }
            }
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
            }
            break;
#endif
#ifndef __NR_iopl
        case 110:   // iopl
            R_EAX = 0;  // only on x86, so return 0...
            break;
#endif
        case 119: // sys_sigreturn
            emu->quit = 1;  // we should be inside a DynaCall in a sigaction callback....
            break;
        case 120:   // clone
            {
                //struct x86_pt_regs *regs = (struct x86_pt_regs *)R_EDI;
                // lets duplicate the emu
                void* stack_base = (void*)R_ECX;
                int stack_size = 0;
                if(!R_ECX) {
                    // allocate a new stack...
                    stack_size = 1024*1024;
                    stack_base = malloc(stack_size); // why not 1M... (normal operation do copy on write, simpler to just copy)
                    // copy value from old stack to new stack
                    int size_to_copy = (uintptr_t)emu->init_stack + emu->size_stack - (R_ESP);
                    memcpy(stack_base-size_to_copy, (void*)R_ESP, size_to_copy);
                }
                x86emu_t * newemu = NewX86Emu(emu->context, R_EIP, (uintptr_t)stack_base, stack_size, (R_ECX)?0:1);
                SetupX86Emu(newemu);
                CloneEmu(newemu, emu);
                SetESP(newemu, (uintptr_t)stack_base);
                // setup registers
                /*if(regs) {
                    SetEAX(newemu, regs->eax);
                    SetEBX(newemu, regs->ebx);
                    SetECX(newemu, regs->ecx);
                    SetEDX(newemu, regs->edx);
                    SetEDI(newemu, regs->edi);
                    SetESI(newemu, regs->esi);
                    SetEBP(newemu, regs->ebp);
                    SetESP(newemu, (uintptr_t)stack_base - size_to_copy);
                }
                SetESP(newemu, (uintptr_t)stack_base - size_to_copy);*/
                void* mystack = (R_EBX&CLONE_VM)?malloc(1024*1024):NULL;  // stack for own process... memory leak, but no practical way to remove it
                int ret = clone(clone_fn, (void*)((R_EBX&CLONE_VM)?((uintptr_t)mystack+1024*1024):0), R_EBX, newemu, R_ESI, R_EDI, R_EBP);
                //int r = syscall(__NR_clone, R_EBX, (R_EBX&CLONE_VM)?((uintptr_t)mystack):0, R_EDX, R_ESI, NULL);  // cannot use that syscall in C: how to setup the stack?!
                R_EAX = ret;
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
            break;
        case 168: // sys_poll
            R_EAX = (uint32_t)poll((void*)R_EBX, R_ECX, (int)R_EDX);
            break;
        case 173: // sys_rt_sigreturn
            emu->quit = 1;  // we should be inside a DynaCall in a sigaction callback....
            break;
        case 174: // sys_rt_sigaction
            //printf_log(LOG_NONE, "Warning, Ignoring sys_rt_sigaction(0x%02X, %p, %p)\n", R_EBX, (void*)R_ECX, (void*)R_EDX);
            R_EAX = my_syscall_sigaction(emu, R_EBX, (void*)R_ECX, (void*)R_EDX, R_ESI);
            break;
        case 190:   // vfork
            {
                int r = vfork();
                R_EAX = r;
            }
            break;
        case 195:   // stat64
            {   
                struct stat64 st;
                unsigned int r = syscall(__NR_stat64, R_EBX, &st);
                UnalignStat64(&st, (void*)R_ECX);
                
                R_EAX = r;
            }
            break;
        case 197:   // fstat64
            {   
                struct stat64 st;
                unsigned int r = syscall(__NR_fstat64, R_EBX, &st);
                UnalignStat64(&st, (void*)R_ECX);
                
                R_EAX = r;
            }
            break;
        case 221: // sys_fcntl64
            if(R_ECX==4) {
                int tmp = of_convert((int)R_EDX);
                R_EAX = (uint32_t)fcntl((int)R_EBX, R_ECX, tmp);
            } else
                R_EAX = (uint32_t)fcntl((int)R_EBX, (int)R_ECX, R_EDX);
            break;
        case 243: // set_thread_area
            printf_log(LOG_INFO, "Warning, set_thread_area syscall unsupported yet\n");
            errno = ENOSYS;
            R_EAX = (uint32_t)-1;
            break;
        case 270:   // tgkill
            R_EAX = syscall(__NR_tgkill, R_EBX, R_ECX, R_EDX);
            break;
        case 355:  // get_random
            R_EAX = my_getrandom(emu, (void*)R_EBX, R_ECX, R_EDX);
            break;
        default:
            printf_log(LOG_INFO, "Error: Unsupported Syscall 0x%02Xh (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
            return;
    }
    printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
}

#define stack(n) (R_ESP+4+n)
#define i32(n)  *(int32_t*)stack(n)
#define u32(n)  *(uint32_t*)stack(n)
#define p(n)    *(void**)stack(n)

uint32_t EXPORT my_syscall(x86emu_t *emu)
{
    uint32_t s = u32(0);
    printf_log(LOG_DUMP, "%p: Calling libc syscall 0x%02X (%d) %p %p %p %p %p\n", (void*)R_EIP, s, s, (void*)u32(4), (void*)u32(8), (void*)u32(12), (void*)u32(16), (void*)u32(20)); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x86s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: return syscall(sc);
                case 1: return syscall(sc, u32(4));
                case 2: return syscall(sc, u32(4), u32(8));
                case 3: return syscall(sc, u32(4), u32(8), u32(12));
                case 4: return syscall(sc, u32(4), u32(8), u32(12), u32(16));
                case 5: return syscall(sc, u32(4), u32(8), u32(12), u32(16), u32(20));
                case 6: return syscall(sc, u32(4), u32(8), u32(12), u32(16), u32(20), u32(24));
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return 0;
            }
        }
    }
    switch (s) {
        case 1: // __NR_exit
            emu->quit = 1;
            return u32(4); // faking the syscall here, we don't want to really terminate the program now
        case 3:  // sys_read
            return (uint32_t)read(i32(4), p(8), u32(12));
            break;
        case 4:  // sys_write
            return (uint32_t)write(i32(4), p(8), u32(12));
            break;
        case 5: // sys_open
            return my_open(emu, p(4), of_convert(u32(8)), u32(12));
        case 6:  // sys_close
            return (uint32_t)close(i32(4));
            break;
        case 270: //_NR_tgkill
            if(!u32(12)) {
                //printf("tgkill(%u, %u, %u) => ", u32(4), u32(8), u32(12));
                uint32_t ret = (uint32_t)syscall(__NR_tgkill, u32(4), u32(8), u32(12));
                //printf("%u (errno=%d)\n", ret, (ret==(uint32_t)-1)?errno:0);
                return ret;
            } else {
                printf_log(LOG_INFO, "Warning: ignoring libc Syscall tgkill (%u, %u, %u)\n", u32(4), u32(8), u32(12));
            }
            return 0;
        default:
            printf_log(LOG_INFO, "Error: Unsupported libc Syscall 0x%02X (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
    }
    return 0;
}
