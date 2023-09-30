#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"

#ifdef ANDROID
    const char* libxcbshmName = "libxcb-shm.so";
#else
    const char* libxcbshmName = "libxcb-shm.so.0";
#endif

#define LIBNAME libxcbshm

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef struct my_xcb_iterator_s {
    void*   data;
    int     rem;
    int     index;
} my_xcb_iterator_t;

typedef my_xcb_cookie_t (*XFp_t)                (void*);
typedef my_xcb_cookie_t (*XFpu_t)               (void*, uint32_t);
typedef my_xcb_cookie_t (*XFpuuC_t)             (void*, uint32_t, uint32_t, uint8_t);
typedef my_xcb_cookie_t (*XFpuuWWCuu_t)         (void*, uint32_t, uint32_t, uint16_t, uint16_t, uint8_t, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuwwWWuCuu_t)       (void*, uint32_t, int16_t, int16_t, uint16_t, uint16_t, uint32_t, uint8_t, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuuWWWWWWwwCCCuu_t) (void*, uint32_t, uint32_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, int16_t, int16_t, uint8_t, uint8_t, uint8_t, uint32_t, uint32_t);
typedef my_xcb_iterator_t   (*YFY_t)            (my_xcb_iterator_t);

#define SUPER() \
    GO(xcb_shm_attach, XFpuuC_t)                        \
    GO(xcb_shm_attach_checked, XFpuuC_t)                \
    GO(xcb_shm_attach_fd, XFpuuC_t)                     \
    GO(xcb_shm_attach_fd_checked, XFpuuC_t)             \
    GO(xcb_shm_create_pixmap, XFpuuWWCuu_t)             \
    GO(xcb_shm_create_pixmap_checked, XFpuuWWCuu_t)     \
    GO(xcb_shm_create_segment, XFpuuC_t)                \
    GO(xcb_shm_create_segment_unchecked, XFpuuC_t)      \
    GO(xcb_shm_detach, XFpu_t)                          \
    GO(xcb_shm_detach_checked, XFpu_t)                  \
    GO(xcb_shm_get_image, XFpuwwWWuCuu_t)               \
    GO(xcb_shm_get_image_unchecked, XFpuwwWWuCuu_t)     \
    GO(xcb_shm_put_image, XFpuuWWWWWWwwCCCuu_t)         \
    GO(xcb_shm_put_image_checked, XFpuuWWWWWWwwCCCuu_t) \
    GO(xcb_shm_query_version, XFp_t)                    \
    GO(xcb_shm_query_version_unchecked, XFp_t)          \
    GO(xcb_shm_seg_end, YFY_t)                          \


#include "wrappercallback.h"

#define SUPER(F, P, ...)           \
    EXPORT void* my_##F P          \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_shm_attach, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t seg, uint32_t shmid, uint8_t read_only), c, seg, shmid, read_only)
SUPER(xcb_shm_attach_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t seg, uint32_t shmid, uint8_t read_only), c, seg, shmid, read_only)
SUPER(xcb_shm_attach_fd, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t seg, uint32_t shm_fd, uint8_t read_only), c, seg, shm_fd, read_only)
SUPER(xcb_shm_attach_fd_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t seg, uint32_t shm_fd, uint8_t read_only), c, seg, shm_fd, read_only)
SUPER(xcb_shm_create_pixmap, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t pid, uint32_t draw, uint16_t w, uint16_t h, uint8_t d, uint32_t seg, uint32_t offs), c, pid, draw, w, h, d, seg, offs)
SUPER(xcb_shm_create_pixmap_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t pid, uint32_t draw, uint16_t w, uint16_t h, uint8_t d, uint32_t seg, uint32_t offs), c, pid, draw, w, h, d, seg, offs)
SUPER(xcb_shm_create_segment, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t seg, uint32_t size, uint8_t read_only), c, seg, size, read_only)
SUPER(xcb_shm_create_segment_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t seg, uint32_t size, uint8_t read_only), c, seg, size, read_only)
SUPER(xcb_shm_detach, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t seg), c, seg)
SUPER(xcb_shm_detach_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t seg), c, seg)
SUPER(xcb_shm_get_image, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t draw, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t plane, uint8_t f, uint32_t seg, uint32_t offs), c, draw, x, y, w, h, plane, f, seg, offs)
SUPER(xcb_shm_get_image_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t draw, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t plane, uint8_t f, uint32_t seg, uint32_t offs), c, draw, x, y, w, h, plane, f, seg, offs)
SUPER(xcb_shm_put_image, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t draw, uint32_t gc, uint16_t tw, uint16_t th, uint16_t sx, uint16_t sy, uint16_t w, uint16_t h, int16_t x, int16_t y, uint8_t d, uint8_t f, uint8_t ev, uint32_t seg, uint32_t offs), c, draw, gc, tw, th, sx, sy, w, h, x, y, d, f, ev, seg, offs)
SUPER(xcb_shm_put_image_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t draw, uint32_t gc, uint16_t tw, uint16_t th, uint16_t sx, uint16_t sy, uint16_t w, uint16_t h, int16_t x, int16_t y, uint8_t d, uint8_t f, uint8_t ev, uint32_t seg, uint32_t offs), c, draw, gc, tw, th, sx, sy, w, h, x, y, d, f, ev, seg, offs)
SUPER(xcb_shm_query_version, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c), c)
SUPER(xcb_shm_query_version_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c), c)
SUPER(xcb_shm_seg_end, (x86emu_t* emu, my_xcb_iterator_t* ret, my_xcb_iterator_t i), i)

#undef SUPER


#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
