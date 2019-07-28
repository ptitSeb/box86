#include <stdint.h>

void myStackAlign(const char* fmt, uint32_t* st, uint32_t* mystack);
void myStackAlignW(const char* fmt, uint32_t* st, uint32_t* mystack);

void UnalignStat64(const void* source, void* dest);

void UnalignOggVorbis(void* dest, void* source); // Arm -> x86
void AlignOggVorbis(void* dest, void* source);   // x86 -> Arm

void UnalignVorbisDspState(void* dest, void* source); // Arm -> x86
void AlignVorbisDspState(void* dest, void* source);   // x86 -> Arm

void UnalignVorbisBlock(void* dest, void* source); // Arm -> x86
void AlignVorbisBlock(void* dest, void* source);   // x86 -> Arm

void UnalignEpollEvent(void* dest, void* source, int nbr); // Arm -> x86
void AlignEpollEvent(void* dest, void* source, int nbr); // x86 -> Arm

// stat64 is packed on i386, not on arm (and possibly other structures)
#undef st_atime
#undef st_mtime
#undef st_ctime
struct i386_stat64 {
	uint64_t	st_dev;
	uint8_t		__pad0[4];
	uint32_t		__st_ino;
	uint32_t		st_mode;
	uint32_t		st_nlink;
	uint32_t		st_uid;
	uint32_t		st_gid;
	uint64_t	st_rdev;
	uint8_t		__pad3[4];
	int64_t		st_size;
	uint32_t		st_blksize;
	uint64_t		st_blocks;
	uint32_t	st_atime;
	uint32_t	st_atime_nsec;
	uint32_t	st_mtime;
	uint32_t	st_mtime_nsec;
	uint32_t	st_ctime;
	uint32_t	st_ctime_nsec;
	uint64_t	st_ino;
} __attribute__((packed));

typedef struct {
  unsigned char *data;
  int storage;
  int fill;
  int returned;

  int unsynced;
  int headerbytes;
  int bodybytes;
} ogg_sync_state;

typedef struct {
  unsigned char   *body_data;    /* bytes from packet bodies */
  long    body_storage;          /* storage elements allocated */
  long    body_fill;             /* elements stored; fill mark */
  long    body_returned;         /* elements of fill returned */


  int     *lacing_vals;      /* The values that will go to the segment table */
  int64_t *granule_vals; /* granulepos values for headers. Not compact
                                this way, but it is simple coupled to the
                                lacing fifo */
  long    lacing_storage;
  long    lacing_fill;
  long    lacing_packet;
  long    lacing_returned;

  unsigned char    header[282];      /* working space for header encode */
  int              header_fill;

  int     e_o_s;          /* set when we have buffered the last packet in the
                             logical bitstream */
  int     b_o_s;          /* set after we've written the initial page
                             of a logical bitstream */
  long    serialno;
  long    pageno;
  int64_t  packetno;  /* sequence number for decode; the framing
                             knows where there's a hole in the data,
                             but we need coupling so that the codec
                             (which is in a separate abstraction
                             layer) also knows about the gap */
  int64_t   granulepos;

} ogg_stream_state;

typedef struct vorbis_dsp_state {
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
} vorbis_dsp_state;

typedef struct {
  long endbyte;
  int  endbit;

  unsigned char *buffer;
  unsigned char *ptr;
  long storage;
} oggpack_buffer;

typedef struct vorbis_block {
  /* necessary stream state for linking to the framing abstraction */
  float  **pcm;       /* this is a pointer into local storage */
  oggpack_buffer opb;

  long  lW;
  long  W;
  long  nW;
  int   pcmend;
  int   mode;

  int         eofflag;
  int64_t granulepos;
  int64_t sequence;
  vorbis_dsp_state *vd; /* For read-only access of configuration */

  /* local storage to avoid remallocing; it's up to the mapping to
     structure it */
  void               *localstore;
  long                localtop;
  long                localalloc;
  long                totaluse;
  struct alloc_chain *reap;

  /* bitmetrics for the frame */
  long glue_bits;
  long time_bits;
  long floor_bits;
  long res_bits;

  void *internal;

} vorbis_block;

typedef struct {
  size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource);
  int    (*seek_func)  (void *datasource, int64_t offset, int whence);
  int    (*close_func) (void *datasource);
  long   (*tell_func)  (void *datasource);
} ov_callbacks;

typedef struct OggVorbis  {
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

  ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block     vb; /* local working space for packet->PCM decode */

  ov_callbacks callbacks;

} OggVorbis;