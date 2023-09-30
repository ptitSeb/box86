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
    const char* libxcbName = "libxcb.so";
#else
    const char* libxcbName = "libxcb.so.1";
#endif

#define LIBNAME libxcb

typedef struct my_xcb_XXX_iterator_s {
    void*             data;
    int               rem;
    int               index;
} my_xcb_XXX_iterator_t;
// xcb_visualtype_iterator_t is like my_xcb_XXX_iterator_t
// xcb_depth_iterator_t is similar to my_xcb_XXX_iterator_t
// xcb_format_iterator_t is similar to my_xcb_XXX_iterator_t

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*XFp_t)(void*);
typedef my_xcb_cookie_t (*XFpC_t)(void*, uint8_t);
typedef my_xcb_cookie_t (*XFpp_t)(void*, void*);
typedef my_xcb_cookie_t (*XFpu_t)(void*, uint32_t);
typedef my_xcb_cookie_t (*XFppp_t)(void*, void*, void*);
typedef my_xcb_cookie_t (*XFpup_t)(void*, uint32_t, void*);
typedef my_xcb_cookie_t (*XFpCC_t)(void*, uint8_t, uint8_t);
typedef my_xcb_cookie_t (*XFpCWp_t)(void*, uint8_t, uint16_t, void*);
typedef my_xcb_cookie_t (*XFpuup_t)(void*, uint32_t, uint32_t, void*);
typedef my_xcb_cookie_t (*XFpuWp_t)(void*, uint32_t, uint16_t, void*);
typedef my_xcb_cookie_t (*XFpCuW_t)(void*, uint8_t, uint32_t, uint16_t);
typedef my_xcb_cookie_t (*XFpCuu_t)(void*, uint8_t, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuuu_t)(void*, uint32_t, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpCppp_t)(void*, uint8_t, void*, void*, void*);
typedef my_xcb_cookie_t (*XFpCuup_t)(void*, uint8_t, uint32_t, uint32_t, void*);
typedef my_xcb_cookie_t (*XFpuuup_t)(void*, uint32_t, uint32_t, uint32_t, void*);
typedef my_xcb_cookie_t (*XFpuuWW_t)(void*, uint32_t, uint32_t, uint16_t, uint16_t);
typedef my_xcb_cookie_t (*XFpuWWW_t)(void*, uint32_t, uint16_t, uint16_t, uint16_t);
typedef my_xcb_cookie_t (*XFpCuuup_t)(void*, uint8_t, uint32_t, uint32_t, uint32_t, void*);
typedef my_xcb_cookie_t (*XFpCuuCC_t)(void*, uint8_t, uint32_t, uint32_t, uint8_t, uint8_t);
typedef my_xcb_cookie_t (*XFpCuuWW_t)(void*, uint8_t, uint32_t, uint32_t, uint16_t, uint16_t);
typedef my_xcb_cookie_t (*XFpuuuuu_t)(void*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpCuuwwp_t)(void*, uint8_t, uint32_t, uint32_t, int16_t, int16_t, void*);
typedef my_xcb_cookie_t (*XFpCuWCCC_t)(void*, uint8_t, uint32_t, uint16_t, uint8_t, uint8_t, uint8_t);
typedef my_xcb_cookie_t (*XFpCuwwWW_t)(void*, uint8_t, uint32_t, int16_t, int16_t, uint16_t, uint16_t);
typedef my_xcb_cookie_t (*XFpCuuuuu_t)(void*, uint8_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpCuuuCup_t)(void*, uint8_t, uint32_t, uint32_t, uint32_t, uint8_t, uint32_t, void*);
typedef my_xcb_cookie_t (*XFpCuwwWWu_t)(void*, uint8_t, uint32_t, int16_t, int16_t, uint16_t, uint16_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuuwwWWww_t)(void*, uint32_t, uint32_t, int16_t, int16_t, uint16_t, uint16_t, int16_t, int16_t);
typedef my_xcb_cookie_t (*XFpCuWCCuuu_t)(void*, uint8_t, uint32_t, uint16_t, uint8_t, uint8_t, uint32_t, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuuuwwwwWW_t)(void*, uint32_t, uint32_t, uint32_t, int16_t, int16_t, int16_t, int16_t, uint16_t, uint16_t);
typedef my_xcb_cookie_t (*XFpCuWCCuuCW_t)(void*, uint8_t, uint32_t, uint16_t, uint8_t, uint8_t, uint32_t, uint32_t, uint8_t, uint16_t);
typedef my_xcb_cookie_t (*XFpCuuWWwwCCup_t)(void*, uint8_t, uint32_t, uint32_t, uint16_t, uint16_t, int16_t, int16_t, uint8_t, uint8_t, uint32_t, void*);
typedef my_xcb_cookie_t (*XFpuuuWWWWWWWW_t)(void*, uint32_t, uint32_t, uint32_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
typedef my_xcb_cookie_t (*XFpCuuwwWWWWuup_t)(void*, uint8_t, uint32_t, uint32_t, int16_t, int16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint32_t, void*);
typedef my_xcb_XXX_iterator_t (*S1Fp_t)(void*);
typedef int (*iFpppip_t)(void*, void*, void*, int, void*);
#define SUPER() \
    GO(xcb_alloc_color, XFpuWWW_t)                  \
    GO(xcb_change_gc, XFpuup_t)                     \
    GO(xcb_change_gc_checked, XFpuup_t)             \
    GO(xcb_change_keyboard_control, XFpup_t)        \
    GO(xcb_change_property, XFpCuuuCup_t)           \
    GO(xcb_change_property_checked, XFpCuuuCup_t)   \
    GO(xcb_change_window_attributes, XFpuup_t)      \
    GO(xcb_change_window_attributes_checked, XFpuup_t)\
    GO(xcb_clear_area, XFpCuwwWW_t)                 \
    GO(xcb_close_font, XFpu_t)                      \
    GO(xcb_close_font_checked, XFpu_t)              \
    GO(xcb_configure_window, XFpuWp_t)              \
    GO(xcb_convert_selection, XFpuuuuu_t)           \
    GO(xcb_copy_area, XFpuuuwwwwWW_t)               \
    GO(xcb_copy_area_checked, XFpuuuwwwwWW_t)       \
    GO(xcb_create_cursor, XFpuuuWWWWWWWW_t)         \
    GO(xcb_create_gc, XFpuuup_t)                    \
    GO(xcb_create_gc_checked, XFpuuup_t)            \
    GO(xcb_create_glyph_cursor, XFpuuuWWWWWWWW_t)   \
    GO(xcb_create_pixmap, XFpCuuWW_t)               \
    GO(xcb_create_pixmap_checked, XFpCuuWW_t)       \
    GO(xcb_create_window, XFpCuuwwWWWWuup_t)        \
    GO(xcb_create_window_checked, XFpCuuwwWWWWuup_t)\
    GO(xcb_delete_property, XFppp_t)                \
    GO(xcb_destroy_window, XFpu_t)                  \
    GO(xcb_free_colormap, XFpu_t)                   \
    GO(xcb_free_colormap_checked, XFpu_t)           \
    GO(xcb_free_gc, XFpu_t)                         \
    GO(xcb_free_pixmap, XFpu_t)                     \
    GO(xcb_get_atom_name, XFpu_t)                   \
    GO(xcb_get_geometry, XFpu_t)                    \
    GO(xcb_get_geometry_unchecked, XFpu_t)          \
    GO(xcb_get_image, XFpCuwwWWu_t)                 \
    GO(xcb_get_image_unchecked, XFpCuwwWWu_t)       \
    GO(xcb_get_input_focus, XFp_t)                  \
    GO(xcb_get_keyboard_mapping, XFpCC_t)           \
    GO(xcb_get_modifier_mapping, XFp_t)             \
    GO(xcb_get_property, XFpCuuuuu_t)               \
    GO(xcb_get_property_unchecked, XFpCuuuuu_t)     \
    GO(xcb_get_selection_owner, XFpu_t)             \
    GO(xcb_get_selection_owner_unchecked, XFpu_t)   \
    GO(xcb_get_window_attributes, XFpu_t)           \
    GO(xcb_get_window_attributes_unchecked, XFpu_t) \
    GO(xcb_grab_button, XFpCuWCCuuCW_t)             \
    GO(xcb_grab_button_checked, XFpCuWCCuuCW_t)     \
    GO(xcb_grab_key, XFpCuWCCC_t)                   \
    GO(xcb_grab_key_checked, XFpCuWCCC_t)           \
    GO(xcb_grab_keyboard, XFpCuuCC_t)               \
    GO(xcb_grab_pointer, XFpCuWCCuuu_t)             \
    GO(xcb_grab_server, XFp_t)                      \
    GO(xcb_image_text_8, XFpCuuwwp_t)               \
    GO(xcb_image_text_8_checked, XFpCuuwwp_t)       \
    GO(xcb_intern_atom, XFpCWp_t)                   \
    GO(xcb_intern_atom_unchecked, XFpCWp_t)         \
    GO(xcb_map_window, XFpu_t)                      \
    GO(xcb_map_window_checked, XFpu_t)              \
    GO(xcb_map_subwindows, XFpu_t)                  \
    GO(xcb_open_font, XFpuWp_t)                     \
    GO(xcb_open_font_checked, XFpuWp_t)             \
    GO(xcb_poly_arc, XFpuuup_t)                     \
    GO(xcb_poly_fill_rectangle, XFpuuup_t)          \
    GO(xcb_poly_line, XFpCuuup_t)                   \
    GO(xcb_poly_line_checked, XFpCuuup_t)           \
    GO(xcb_poly_point, XFpCuuup_t)                  \
    GO(xcb_poly_rectangle, XFpuuup_t)               \
    GO(xcb_poly_segment, XFpuuup_t)                 \
    GO(xcb_put_image, XFpCuuWWwwCCup_t)             \
    GO(xcb_query_pointer, XFpu_t)                   \
    GO(xcb_query_text_extents, XFpuup_t)            \
    GO(xcb_query_tree, XFpu_t)                      \
    GO(xcb_query_tree_unchecked, XFpu_t)            \
    GO(xcb_reparent_window, XFpuuWW_t)              \
    GO(xcb_send_event, XFpCuup_t)                   \
    GO(xcb_set_input_focus, XFpCuu_t)               \
    GO(xcb_set_selection_owner, XFpuuu_t)           \
    GO(xcb_translate_coordinates, XFpuuWW_t)        \
    GO(xcb_translate_coordinates_unchecked, XFpuuWW_t)\
    GO(xcb_ungrab_button, XFpCuW_t)                 \
    GO(xcb_ungrab_button_checked, XFpCuW_t)         \
    GO(xcb_ungrab_keyboard, XFpu_t)                 \
    GO(xcb_ungrab_keyboard_checked, XFpu_t)         \
    GO(xcb_ungrab_key, XFpCuW_t)                    \
    GO(xcb_ungrab_key_checked, XFpCuW_t)            \
    GO(xcb_ungrab_pointer, XFpu_t)                  \
    GO(xcb_ungrab_server, XFp_t)                    \
    GO(xcb_unmap_window, XFpu_t)                    \
    GO(xcb_warp_pointer, XFpuuwwWWww_t)             \
    GO(xcb_depth_visuals_iterator, S1Fp_t)          \
    GO(xcb_screen_allowed_depths_iterator, S1Fp_t)  \
    GO(xcb_setup_pixmap_formats_iterator, S1Fp_t)   \
    GO(xcb_setup_roots_iterator, S1Fp_t)            \
    GO(xcb_create_colormap, XFpCppp_t)              \
    GO(xcb_bell, XFpC_t)                            \
    GO(xcb_free_cursor, XFpp_t)                     \
    GO(xcb_no_operation, XFp_t)                     \
    GO(xcb_take_socket, iFpppip_t)                  \


#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \


// return_socket
#define GO(A)   \
static uintptr_t my_return_socket_fct_##A = 0;                      \
static void my_return_socket_##A(void* a)                           \
{                                                                   \
    RunFunctionFmt(my_return_socket_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* findreturn_socketFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_return_socket_fct_##A == (uintptr_t)fct) return my_return_socket_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_return_socket_fct_##A == 0) {my_return_socket_fct_##A = (uintptr_t)fct; return my_return_socket_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxcb return_socket callback (%p)\n", fct);
    return NULL;
}

#undef SUPER

#define SUPER(F, P, ...)           \
    EXPORT void* my_##F P          \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_alloc_color, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t cmap, uint16_t red, uint16_t green, uint16_t blue), c, cmap, red, green, blue)
SUPER(xcb_change_gc, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t gc, uint32_t mask, void* list), c, gc, mask, list)
SUPER(xcb_change_gc_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t gc, uint32_t mask, void* list), c, gc, mask, list)
SUPER(xcb_change_keyboard_control, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t mask, void* list), c, mask, list)
SUPER(xcb_change_property, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t mode, uint32_t w, uint32_t prop, uint32_t type, uint8_t f, uint32_t len, void* data), c, mode, w, prop, type, f, len, data)
SUPER(xcb_change_property_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t mode, uint32_t w, uint32_t prop, uint32_t type, uint8_t f, uint32_t len, void* data), c, mode, w, prop, type, f, len, data)
SUPER(xcb_change_window_attributes, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t mask, void* list), c, win, mask, list)
SUPER(xcb_change_window_attributes_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t mask, void* list), c, win, mask, list)
SUPER(xcb_clear_area, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t e, uint32_t win, int16_t x, int16_t y, uint16_t w, uint16_t h), c, e, win, x, y, w, h);
SUPER(xcb_close_font, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t font), c, font)
SUPER(xcb_close_font_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t font), c, font)
SUPER(xcb_configure_window, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint16_t mask, void* list), c, win, mask, list)
SUPER(xcb_convert_selection, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t r, uint32_t s, uint32_t t, uint32_t p, uint32_t time), c, r, s, t, p, time)
SUPER(xcb_copy_area, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t src, uint32_t dst, uint32_t gc, int16_t sx, int16_t sy, int16_t dx, int16_t dy, uint16_t w, uint16_t h), c, src, dst, gc, sx, sy, dx, dy, w, h)
SUPER(xcb_copy_area_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t src, uint32_t dst, uint32_t gc, int16_t sx, int16_t sy, int16_t dx, int16_t dy, uint16_t w, uint16_t h), c, src, dst, gc, sx, sy, dx, dy, w, h)
SUPER(xcb_create_cursor, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t cid, uint32_t s, uint32_t mask, uint16_t fr, uint16_t fg, uint16_t fb, uint16_t br, uint16_t bg, uint16_t bb, uint16_t x, uint16_t y), c, cid, s, mask, fr, fg, fb, br, bg, bb, x, y)
SUPER(xcb_create_gc, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t cid, uint32_t d, uint32_t mask, void* list), c, cid, d, mask, list)
SUPER(xcb_create_gc_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t cid, uint32_t d, uint32_t mask, void* list), c, cid, d, mask, list)
SUPER(xcb_create_glyph_cursor, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t cid, uint32_t sf, uint32_t mf, uint16_t sc, uint16_t mc, uint16_t fr, uint16_t fg, uint16_t fb, uint16_t br, uint16_t bg, uint16_t bb), c, cid, sf, mf, sc, mc, fr, fg, fb, br, bg, bb)
SUPER(xcb_create_pixmap, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t depth, uint32_t pid, uint32_t d, uint16_t w, uint16_t h), c, depth, pid, d, w, h)
SUPER(xcb_create_pixmap_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t depth, uint32_t pid, uint32_t d, uint16_t w, uint16_t h), c, depth, pid, d, w, h)
SUPER(xcb_create_window, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t depth, uint32_t  wid, uint32_t p, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bw, uint16_t _class, uint32_t v, uint32_t mask, void* list), c, depth, wid, p, x, y, w, h, bw, _class, v, mask, list)
SUPER(xcb_create_window_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t depth, uint32_t  wid, uint32_t p, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bw, uint16_t _class, uint32_t v, uint32_t mask, void* list), c, depth, wid, p, x, y, w, h, bw, _class, v, mask, list)
SUPER(xcb_delete_property, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, void* w, void* p), c, w, p)
SUPER(xcb_destroy_window, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t w), c, w)
SUPER(xcb_free_colormap, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t colormap), c, colormap)
SUPER(xcb_free_colormap_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t colormap), c, colormap)
SUPER(xcb_free_gc, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t gc), c, gc)
SUPER(xcb_free_pixmap, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t pixmap), c, pixmap)
SUPER(xcb_get_atom_name, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t atom), c, atom)
SUPER(xcb_get_geometry, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t d), c, d)
SUPER(xcb_get_geometry_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t d), c, d)
SUPER(xcb_get_image, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t f, uint32_t d, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t mask), c, f, d, x, y, w, h, mask)
SUPER(xcb_get_image_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t f, uint32_t d, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t mask), c, f, d, x, y, w, h, mask)
SUPER(xcb_get_input_focus, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c), c)
SUPER(xcb_get_keyboard_mapping, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t f, uint8_t count), c, f, count)
SUPER(xcb_get_modifier_mapping, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c), c)
SUPER(xcb_get_property, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t _del, uint32_t w, uint32_t p, uint32_t type, uint32_t off, uint32_t len), c, _del, w, p, type, off, len)
SUPER(xcb_get_property_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t _del, uint32_t w, uint32_t p, uint32_t type, uint32_t off, uint32_t len), c, _del, w, p, type, off, len)
SUPER(xcb_get_selection_owner, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t sel), c, sel)
SUPER(xcb_get_selection_owner_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t sel), c, sel)
SUPER(xcb_get_window_attributes, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_get_window_attributes_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_grab_button, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t owner, uint32_t win, uint16_t mask, uint8_t pmode, uint8_t kmode, uint32_t confine, uint32_t cursor, uint8_t button, uint16_t modif), c, owner, win, mask, pmode, kmode, confine, cursor, button, modif)
SUPER(xcb_grab_button_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t owner, uint32_t win, uint16_t mask, uint8_t pmode, uint8_t kmode, uint32_t confine, uint32_t cursor, uint8_t button, uint16_t modif), c, owner, win, mask, pmode, kmode, confine, cursor, button, modif)
SUPER(xcb_grab_key, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t owner, uint32_t win, uint16_t modif, uint8_t key, uint8_t pointer, uint8_t keymode), c, owner, win, modif, key, pointer, keymode)
SUPER(xcb_grab_key_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t owner, uint32_t win, uint16_t modif, uint8_t key, uint8_t pointer, uint8_t keymode), c, owner, win, modif, key, pointer, keymode)
SUPER(xcb_grab_keyboard, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t owner, uint32_t g, uint32_t time, uint8_t pointer, uint32_t keyboard), c, owner, g, time, pointer, keyboard)
SUPER(xcb_grab_pointer, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t owner, uint32_t g, uint16_t event, uint8_t pointer, uint8_t keyboard, uint32_t confine, uint32_t cursor, uint32_t time), c, owner, g, event, pointer, keyboard, confine, cursor, time)
SUPER(xcb_grab_server, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c), c)
SUPER(xcb_image_text_8, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t len, uint32_t d, uint32_t gc, int16_t x, int16_t y, void* string), c, len, d, gc, x, y, string)
SUPER(xcb_image_text_8_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t len, uint32_t d, uint32_t gc, int16_t x, int16_t y, void* string), c, len, d, gc, x, y, string)
SUPER(xcb_intern_atom, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t only, uint16_t len, void* name), c, only, len, name)
SUPER(xcb_intern_atom_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t only, uint16_t len, void* name), c, only, len, name)
SUPER(xcb_map_window, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_map_window_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_map_subwindows, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_open_font, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t fid, uint16_t len, void* name), c, fid, len, name)
SUPER(xcb_open_font_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t fid, uint16_t len, void* name), c, fid, len, name)
SUPER(xcb_poly_arc, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t d, uint32_t gc, uint32_t len, void* arcs), c, d, gc, len, arcs)
SUPER(xcb_poly_fill_rectangle, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t d, uint32_t gc, uint32_t len, void* rects), c, d, gc, len, rects)
SUPER(xcb_poly_line, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t mode, uint32_t d, uint32_t gc, uint32_t len, void* points), c, mode, d, gc, len, points)
SUPER(xcb_poly_line_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t mode, uint32_t d, uint32_t gc, uint32_t len, void* points), c, mode, d, gc, len, points)
SUPER(xcb_poly_point, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t mode, uint32_t d, uint32_t gc, uint32_t len, void* points), c, mode, d, gc, len, points)
SUPER(xcb_poly_rectangle, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t d, uint32_t gc, uint32_t len, void* rects), c, d, gc, len, rects)
SUPER(xcb_poly_segment, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t d, uint32_t gc, uint32_t len, void* segs), c, d, gc, len, segs)
SUPER(xcb_put_image, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t format, uint32_t d, uint32_t gc, uint16_t w, uint16_t h, int16_t dx, int16_t dy, uint8_t pad, uint8_t depth, uint32_t len, void* data), c, format, d, gc, w, h, dx, dy, pad, depth, len, data)
SUPER(xcb_query_pointer, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_query_text_extents, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t f, uint32_t len, void* string), c, f, len, string)
SUPER(xcb_query_tree, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_query_tree_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_reparent_window, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t parent, uint16_t x, uint16_t y), c, win, parent, x, y)
SUPER(xcb_send_event, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t p, uint32_t w, uint32_t mask, void* event), c, p, w, mask, event)
SUPER(xcb_set_input_focus, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t r, uint32_t f, uint32_t time), c, r, f, time)
SUPER(xcb_set_selection_owner, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t owner, uint32_t sel, uint32_t time), c, owner, sel, time)
SUPER(xcb_translate_coordinates, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t dest, uint16_t x, uint16_t y), c, win, dest, x, y)
SUPER(xcb_translate_coordinates_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t dest, uint16_t x, uint16_t y), c, win, dest, x, y)
SUPER(xcb_ungrab_keyboard, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t time), c, time)
SUPER(xcb_ungrab_keyboard_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t time), c, time)
SUPER(xcb_ungrab_key, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t k, uint32_t g, uint16_t m), c, k, g, m)
SUPER(xcb_ungrab_key_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t k, uint32_t g, uint16_t m), c, k, g, m)
SUPER(xcb_ungrab_pointer, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t time), c, time)
SUPER(xcb_ungrab_server, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c), c)
SUPER(xcb_unmap_window, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_warp_pointer, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t swin, uint32_t dwin, int16_t sx, int16_t sy, uint16_t sw, uint16_t sh, int16_t dx, int16_t dy), c, swin, dwin, sx, sy, sw, sh, dx, dy)
SUPER(xcb_create_colormap, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t alloc, void* mid, void* win, void* vis), c, alloc, mid, win, vis)
SUPER(xcb_bell, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t percent), c, percent)
SUPER(xcb_free_cursor, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, void* cursor), c, cursor)
SUPER(xcb_ungrab_button, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t k, uint32_t g, uint16_t m), c, k, g, m)
SUPER(xcb_ungrab_button_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t k, uint32_t g, uint16_t m), c, k, g, m)
SUPER(xcb_no_operation, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c), c)
#undef SUPER

EXPORT void* my_xcb_depth_visuals_iterator(x86emu_t* emu, void* ret, void* R)
{
    (void)emu;
    my_xcb_XXX_iterator_t tmp = my->xcb_depth_visuals_iterator(R);
    memcpy(ret, &tmp, sizeof(tmp));
    return ret;
}

EXPORT void* my_xcb_screen_allowed_depths_iterator(x86emu_t* emu, void* ret, void* R)
{
    (void)emu;
    my_xcb_XXX_iterator_t tmp = my->xcb_screen_allowed_depths_iterator(R);
    memcpy(ret, &tmp, sizeof(tmp));
    return ret;
}

EXPORT void* my_xcb_setup_pixmap_formats_iterator(x86emu_t* emu, void* ret, void* R)
{
    (void)emu;
    my_xcb_XXX_iterator_t tmp = my->xcb_setup_pixmap_formats_iterator(R);
    memcpy(ret, &tmp, sizeof(tmp));
    return ret;
}

EXPORT void* my_xcb_setup_roots_iterator(x86emu_t* emu, void* ret, void* R)
{
    (void)emu;
    my_xcb_XXX_iterator_t tmp = my->xcb_setup_roots_iterator(R);
    memcpy(ret, &tmp, sizeof(tmp));
    return ret;
}

EXPORT int my_xcb_take_socket(x86emu_t* emu, void* c, void* f, void* closure, int flags, void* sent)
{
    (void)emu;
    return my->xcb_take_socket(c, findreturn_socketFct(f), closure, flags,sent);
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
