#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <asm/stat.h>
#include <wchar.h>
#include <sys/epoll.h>

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
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': *(mystack++) = *(st++); ++p; break; // fetch an int in the stack....
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
                    case '$': ++p; break; // should issue a warning, it's not handled...
                    case '*': *(mystack++) = *(st++); ++p; break; //fetch an int in the stack
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
	unsigned long long		st_blocks;
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
    
    memset(i386st->__pad0, 0, sizeof(i386st->__pad0));
	memset(i386st->__pad3, 0, sizeof(i386st->__pad3));
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


typedef struct __attribute__((packed)) {
  unsigned char   *body_data;
  long    body_storage;
  long    body_fill;
  long    body_returned;


  int     *lacing_vals;
  int64_t *granule_vals;
  long    lacing_storage;
  long    lacing_fill;
  long    lacing_packet;
  long    lacing_returned;

  unsigned char    header[282];
  unsigned char     dummy_align[2]; // not in the actual structure....
  int              header_fill;

  int     e_o_s;
  int     b_o_s;
  long    serialno;
  long    pageno;
  int64_t  packetno;
  int64_t   granulepos;

} ogg_stream_state_x86;

typedef struct __attribute__((packed)) vorbis_dsp_state_x86 {
  int analysisp;
  void *vi; //vorbis_info

  float **pcm;
  float **pcmret;
  int      pcm_storage;
  int      pcm_current;
  int      pcm_returned;

  int  preextrapolate;
  int  eofflag;

  long lW;
  long W;
  long nW;
  long centerW;

  int64_t granulepos;
  int64_t sequence;

  int64_t glue_bits;
  int64_t time_bits;
  int64_t floor_bits;
  int64_t res_bits;

  void       *backend_state;
} vorbis_dsp_state_x86;

typedef struct {
  long endbyte;
  int  endbit;

  unsigned char *buffer;
  unsigned char *ptr;
  long storage;
} oggpack_buffer_x86;

typedef struct __attribute__((packed)) vorbis_block_x86 {

  float  **pcm;
  oggpack_buffer_x86 opb;

  long  lW;
  long  W;
  long  nW;
  int   pcmend;
  int   mode;

  int         eofflag;
  int64_t granulepos;
  int64_t sequence;
  void *vd;
  
  void               *localstore;
  long                localtop;
  long                localalloc;
  long                totaluse;
  void *reap;

  long glue_bits;
  long time_bits;
  long floor_bits;
  long res_bits;

  void *internal;

} vorbis_block_x86;

typedef struct __attribute__((packed)) OggVorbis_x86  {
  void            *datasource; /* Pointer to a FILE *, etc. */
  int              seekable;
  int64_t      offset;
  int64_t      end;
  ogg_sync_state   oy;

  /* If the FILE handle isn't seekable (eg, a pipe), only the current
     stream appears */
  int              links;
  int64_t     *offsets;
  int64_t     *dataoffsets;
  long            *serialnos;
  int64_t     *pcmlengths; /* overloaded to maintain binary
                                  compatibility; x2 size, stores both
                                  beginning and end values */
  void     *vi; //vorbis_info
  void  *vc;    //vorbis_comment

  /* Decoding working state local storage */
  int64_t      pcm_offset;
  int              ready_state;
  long             current_serialno;
  int              current_link;

  double           bittrack;
  double           samptrack;

  ogg_stream_state_x86 os; /* take physical pages, weld into a logical
                          stream of packets */
  vorbis_dsp_state_x86 vd; /* central working state for the packet->PCM decoder */
  vorbis_block_x86     vb; /* local working space for packet->PCM decode */

  ov_callbacks callbacks;

} OggVorbis_x86;

#define TRANSFERT \
GO(datasource) \
GO(seekable) \
GO(offset) \
GO(end) \
GO(oy) \
GO(links) \
GO(offsets) \
GO(dataoffsets) \
GO(serialnos) \
GO(pcmlengths) \
GO(vi) \
GO(vc) \
GO(pcm_offset) \
GO(ready_state) \
GO(current_serialno) \
GO(current_link) \
GOM(bittrack, 16) \
GO(os.body_data) \
GO(os.body_storage) \
GO(os.body_fill) \
GO(os.body_returned) \
GO(os.lacing_vals) \
GO(os.granule_vals) \
GO(os.lacing_storage) \
GO(os.lacing_fill) \
GO(os.lacing_packet) \
GO(os.lacing_returned) \
GOM(os.header, 282) \
GO(os.header_fill) \
GO(os.e_o_s) \
GO(os.b_o_s) \
GO(os.serialno) \
GO(os.pageno) \
GO(os.packetno) \
GO(os.granulepos) \
GO(vd.analysisp) \
GO(vd.vi) \
GO(vd.pcm) \
GO(vd.pcmret) \
GO(vd.pcm_storage) \
GO(vd.pcm_current) \
GO(vd.pcm_returned) \
GO(vd.preextrapolate) \
GO(vd.eofflag) \
GO(vd.lW) \
GO(vd.W) \
GO(vd.nW) \
GO(vd.centerW) \
GO(vd.granulepos) \
GO(vd.sequence) \
GO(vd.glue_bits) \
GO(vd.time_bits) \
GO(vd.floor_bits) \
GO(vd.res_bits) \
GO(vd.backend_state) \
GO(vb.pcm) \
GO(vb.opb.endbyte) \
GO(vb.opb.endbit) \
GO(vb.opb.buffer) \
GO(vb.opb.ptr) \
GO(vb.opb.storage) \
GO(vb.lW) \
GO(vb.W) \
GO(vb.nW) \
GO(vb.pcmend) \
GO(vb.mode) \
GO(vb.eofflag) \
GO(vb.granulepos) \
GO(vb.sequence) \
GO(vb.vd) \
GO(vb.localstore) \
GO(vb.localtop) \
GO(vb.localalloc) \
GO(vb.totaluse) \
GO(vb.reap) \
GO(vb.glue_bits) \
GO(vb.time_bits) \
GO(vb.floor_bits) \
GO(vb.res_bits) \
GO(vb.internal) \
GO(callbacks)

void AlignOggVorbis(void* dest, void* source)
{
     // Arm -> x86
     #define GO(A) ((OggVorbis*)dest)->A = ((OggVorbis_x86*)source)->A;
     #define GOM(A, S) memcpy(&((OggVorbis*)dest)->A, &((OggVorbis_x86*)source)->A, S);
     TRANSFERT
     #undef GO
     #undef GOM
}
void UnalignOggVorbis(void* dest, void* source)
{
    // x86 -> Arm
     #define GO(A) ((OggVorbis_x86*)dest)->A = ((OggVorbis*)source)->A;
     #define GOM(A, S) memcpy(&((OggVorbis_x86*)dest)->A, &((OggVorbis*)source)->A, S);
     TRANSFERT
     #undef GO
     #undef GOM
}
#undef TRANSFERT

typedef union __attribute__((packed)) x86_epoll_data {
    void    *ptr;
    int      fd;
    uint32_t u32;
    uint64_t u64;
} x86_epoll_data_t;

struct __attribute__((packed)) x86_epoll_event {
    uint32_t            events;
    x86_epoll_data_t    data;
};
// Arm -> x86
void UnalignEpollEvent(void* dest, void* source, int nbr)
{
    struct x86_epoll_event *x86_struct = (struct x86_epoll_event*)dest;
    struct epoll_event *arm_struct = (struct epoll_event*)source;
    while(nbr) {
        x86_struct->events = arm_struct->events;
        x86_struct->data.u64 = arm_struct->data.u64;
        ++x86_struct;
        ++arm_struct;
        --nbr;
    }
}

// x86 -> Arm
void AlignEpollEvent(void* dest, void* source, int nbr)
{
    struct x86_epoll_event *x86_struct = (struct x86_epoll_event*)source;
    struct epoll_event *arm_struct = (struct epoll_event*)dest;
    while(nbr) {
        arm_struct->events = x86_struct->events;
        arm_struct->data.u64 = x86_struct->data.u64;
        ++x86_struct;
        ++arm_struct;
        --nbr;
    }
}
