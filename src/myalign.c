#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <asm/stat.h>
#include <wchar.h>

#include "x86emu.h"
#include "x86emu_private.h"
#include "myalign.h"


void myStackAlign(const char* fmt, uint32_t* st, uint32_t* mystack)
{
    // loop...
    const char* p = fmt;
    int state = 0;
    double d;
    while(*p)
    {
        switch(state) {
            case 0:
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$':
                    case '*': ++p; break; // should issue a warning, it's not handled...
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 23:    // 64bits int
                if((((uint32_t)mystack)&0x7)!=0)
                    mystack++;
                *(uint64_t*)mystack = *(uint64_t*)st;
                st+=2; mystack+=2;
                state = 0;
                ++p;
                break;
            case 14:    //%LG long double
                #ifdef HAVE_LD80BITS
                if((((uint32_t)mystack)&0x7)!=0)
                    mystack++;
                memcpy(mystack, st, 10);
                st+=3; mystack+=3;
                #else
                // there is no long double on ARM, so tranform that in a regular double
                LD2D((void*)st, &d);
                if((((uint32_t)mystack)&0x7)!=0)
                    mystack++;
                *(uint64_t*)mystack = *(uint64_t*)&d;
                st+=3; mystack+=2;
                #endif
                state = 0;
                ++p;
                break;
            case 20:    // fallback
            case 21:
            case 22:
            case 24:    // normal int / pointer
            case 30:
                *mystack = *st;
                ++mystack;
                ++st;
                state = 0;
                ++p;
                break;
            default:
                // whattt?
                state = 0;
        }
    }
}

void myStackAlignW(const char* fmt, uint32_t* st, uint32_t* mystack)
{
    // loop...
    const wchar_t* p = (const wchar_t*)fmt;
    int state = 0;
    double d;
    while(*p)
    {
        switch(state) {
            case 0:
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 'S':
                    case 's': state = 30; break; // pointers
                    case '$':
                    case '*': ++p; break; // should issue a warning, it's not handled...
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 23:    // 64bits int
                if((((uint32_t)mystack)&0x7)!=0)
                    mystack++;
                *(uint64_t*)mystack = *(uint64_t*)st;
                st+=2; mystack+=2;
                state = 0;
                ++p;
                break;
            case 14:    //%LG long double
                #ifdef HAVE_LD80BITS
                if((((uint32_t)mystack)&0x7)!=0)
                    mystack++;
                memcpy(mystack, st, 10);
                st+=3; mystack+=3;
                #else
                // there is no long double on ARM, so tranform that in a regular double
                LD2D((void*)st, &d);
                if((((uint32_t)mystack)&0x7)!=0)
                    mystack++;
                *(uint64_t*)mystack = *(uint64_t*)&d;
                st+=3; mystack+=2;
                #endif
                state = 0;
                ++p;
                break;
            case 20:    // fallback
            case 21:
            case 22:
            case 24:    // normal int / pointer
            case 30:
                *mystack = *st;
                ++mystack;
                ++st;
                state = 0;
                ++p;
                break;
            default:
                // whattt?
                state = 0;
        }
    }
}

// stat64 is packed on i386, not on arm (and possibly other structures)
struct i386_stat64 {
	unsigned long long	st_dev;
	unsigned char		__pad0[4];
	unsigned int		__st_ino;
	unsigned int		st_mode;
	unsigned int		st_nlink;
	unsigned int		st_uid;
	unsigned int		st_gid;
	unsigned long long	st_rdev;
	unsigned char		__pad3[4];
	long long		st_size;
	unsigned int		st_blksize;
	long long		st_blocks;
	unsigned int	st_atime;
	unsigned int	st_atime_nsec;
	unsigned int	st_mtime;
	unsigned int	st_mtime_nsec;
	unsigned int	st_ctime;
	unsigned int	st_ctime_nsec;
	unsigned long long	st_ino;
} __attribute__((packed));

void UnalignStat64(void* source, void* dest)
{
    struct i386_stat64 *i386st = (struct i386_stat64*)dest;
    struct stat64 *st = (struct stat64*) source;
    
    i386st->st_dev      = st->st_dev;
    i386st->__st_ino    = st->__st_ino;
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