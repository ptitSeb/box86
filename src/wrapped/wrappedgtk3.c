#define _GNU_SOURCE
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
#include "myalign.h"
#include "gtkclass.h"

#ifdef ANDROID
    const char* gtk3Name = "libgtk-3.so";
#else
    const char* gtk3Name = "libgtk-3.so.0";
#endif

static char* libname = NULL;
#define LIBNAME gtk3

typedef int           (*iFv_t)(void);
typedef void          (*vFp_t)(void*);
typedef double        (*dFp_t)(void*);
typedef void*         (*pFi_t)(int);
typedef int           (*iFpp_t)(void*, void*);
typedef void*         (*pFpp_t)(void*, void*);
typedef void*         (*pFpu_t)(void*, uint32_t);
typedef void*         (*pFppi_t)(void*, void*, int);
typedef void*         (*pFppp_t)(void*, void*, void*);
typedef int           (*iFppp_t)(void*, void*, void*);
typedef void          (*vFpipV_t)(void*, int, void*, ...);

#define ADDED_FUNCTIONS()                   \
GO(g_type_class_ref, pFi_t)                 \
GO(g_type_class_unref, vFp_t)               \
GO(g_initially_unowned_get_type, iFv_t)     \
GO(gtk_bin_get_type, iFv_t)                 \
GO(gtk_widget_get_type, iFv_t)              \
GO(gtk_button_get_type, iFv_t)              \
GO(gtk_container_get_type, iFv_t)           \
GO(gtk_misc_get_type, iFv_t)                \
GO(gtk_label_get_type, iFv_t)               \
GO(gtk_tree_view_get_type, iFv_t)           \
GO(gtk_window_get_type, iFv_t)              \
GO(gtk_table_get_type, iFv_t)               \
GO(gtk_fixed_get_type, iFv_t)               \
GO(gtk_combo_box_get_type, iFv_t)           \
GO(gtk_toggle_button_get_type, iFv_t)       \
GO(gtk_check_button_get_type, iFv_t)        \
GO(gtk_frame_get_type, iFv_t)               \
GO(gtk_entry_get_type, iFv_t)               \
GO(gtk_spin_button_get_type, iFv_t)         \
GO(gtk_progress_get_type, iFv_t)            \
GO(gtk_progress_bar_get_type, iFv_t)        \
GO(gtk_menu_shell_get_type, iFv_t)          \
GO(gtk_menu_bar_get_type, iFv_t)            \
GO(gtk_action_get_type, iFv_t)              \
GO(gtk_dialog_add_button, pFppi_t)          \
GO(gtk_spin_button_get_value, dFp_t)        \
GO(gtk_builder_lookup_callback_symbol, pFpp_t)  \
GO(g_module_symbol, iFppp_t)                \
GO(g_log, vFpipV_t)                         \
GO(g_module_open, pFpu_t)                   \
GO(g_module_close, vFp_t)                   \

#include "generated/wrappedgtk3types.h"


#include "wrappercallback.h"

EXPORT uintptr_t my3_gtk_signal_connect_full(x86emu_t* emu, void* object, void* name, void* c_handler, void* unsupported, void* data, void* closure, uint32_t signal, int after)
{
    (void)emu; (void)unsupported;
    my_signal_t *sig = new_mysignal(c_handler, data, closure);
    uintptr_t ret = my->gtk_signal_connect_full(object, name, my_signal_cb, NULL, sig, my_signal_delete, signal, after);
    printf_log(LOG_DEBUG, "Connecting gtk signal \"%s\" with cb=%p\n", (char*)name, sig);
    return ret;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GtkMenuDetachFunc
#define GO(A)   \
static uintptr_t my_menudetach_fct_##A = 0;                                 \
static void my_menudetach_##A(void* widget, void* menu)                     \
{                                                                           \
    RunFunctionFmt(my_menudetach_fct_##A, "pp", widget, menu);  \
}
SUPER()
#undef GO
static void* findMenuDetachFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_menudetach_fct_##A == (uintptr_t)fct) return my_menudetach_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_menudetach_fct_##A == 0) {my_menudetach_fct_##A = (uintptr_t)fct; return my_menudetach_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GMenuDetachFunc callback\n");
    return NULL;
}

// GtkMenuPositionFunc
#define GO(A)   \
static uintptr_t my_menuposition_fct_##A = 0;                                               \
static void my_menuposition_##A(void* menu, void* x, void* y, void* push_in, void* data)    \
{                                                                                           \
    RunFunctionFmt(my_menuposition_fct_##A, "ppppp", menu, x, y, push_in, data);\
}
SUPER()
#undef GO
static void* findMenuPositionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_menuposition_fct_##A == (uintptr_t)fct) return my_menuposition_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_menuposition_fct_##A == 0) {my_menuposition_fct_##A = (uintptr_t)fct; return my_menuposition_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkMenuPositionFunc callback\n");
    return NULL;
}

// GtkFunction
#define GO(A)   \
static uintptr_t my3_gtkfunction_fct_##A = 0;                               \
static int my3_gtkfunction_##A(void* data)                                  \
{                                                                           \
    return RunFunctionFmt(my3_gtkfunction_fct_##A, "p", data);  \
}
SUPER()
#undef GO
static void* findGtkFunctionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my3_gtkfunction_fct_##A == (uintptr_t)fct) return my3_gtkfunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_gtkfunction_fct_##A == 0) {my3_gtkfunction_fct_##A = (uintptr_t)fct; return my3_gtkfunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkFunction callback\n");
    return NULL;
}

// GtkClipboardGetFunc
#define GO(A)   \
static uintptr_t my_clipboardget_fct_##A = 0;                                                       \
static void my_clipboardget_##A(void* clipboard, void* selection, uint32_t info, void* data)        \
{                                                                                                   \
    RunFunctionFmt(my_clipboardget_fct_##A, "ppup", clipboard, selection, info, data);  \
}
SUPER()
#undef GO
static void* findClipboadGetFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_clipboardget_fct_##A == (uintptr_t)fct) return my_clipboardget_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_clipboardget_fct_##A == 0) {my_clipboardget_fct_##A = (uintptr_t)fct; return my_clipboardget_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkClipboardGetFunc callback\n");
    return NULL;
}

// GtkClipboardClearFunc
#define GO(A)   \
static uintptr_t my_clipboardclear_fct_##A = 0;                                     \
static void my_clipboardclear_##A(void* clipboard, void* data)                      \
{                                                                                   \
    RunFunctionFmt(my_clipboardclear_fct_##A, "pp", clipboard, data);   \
}
SUPER()
#undef GO
static void* findClipboadClearFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_clipboardclear_fct_##A == (uintptr_t)fct) return my_clipboardclear_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_clipboardclear_fct_##A == 0) {my_clipboardclear_fct_##A = (uintptr_t)fct; return my_clipboardclear_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkClipboardClearFunc callback\n");
    return NULL;
}

// GtkCallback
#define GO(A)   \
static uintptr_t my3_gtkcallback_fct_##A = 0;                               \
static void my3_gtkcallback_##A(void* widget, void* data)                   \
{                                                                           \
    RunFunctionFmt(my3_gtkcallback_fct_##A, "pp", widget, data);\
}
SUPER()
#undef GO
static void* findGtkCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my3_gtkcallback_fct_##A == (uintptr_t)fct) return my3_gtkcallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_gtkcallback_fct_##A == 0) {my3_gtkcallback_fct_##A = (uintptr_t)fct; return my3_gtkcallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkCallback callback\n");
    return NULL;
}

// GtkTextCharPredicate
#define GO(A)   \
static uintptr_t my_textcharpredicate_fct_##A = 0;                                          \
static int my_textcharpredicate_##A(uint32_t ch, void* data)                                \
{                                                                                           \
    return (int)RunFunctionFmt(my_textcharpredicate_fct_##A, "up", ch, data);   \
}
SUPER()
#undef GO
static void* findGtkTextCharPredicateFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_textcharpredicate_fct_##A == (uintptr_t)fct) return my_textcharpredicate_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_textcharpredicate_fct_##A == 0) {my_textcharpredicate_fct_##A = (uintptr_t)fct; return my_textcharpredicate_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkTextCharPredicate callback\n");
    return NULL;
}

// Toolbar
#define GO(A)   \
static uintptr_t my_toolbar_fct_##A = 0;                                \
static void my_toolbar_##A(void* widget, void* data)                    \
{                                                                       \
    RunFunctionFmt(my_toolbar_fct_##A, "pp", widget, data); \
}
SUPER()
#undef GO
static void* findToolbarFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_toolbar_fct_##A == (uintptr_t)fct) return my_toolbar_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_toolbar_fct_##A == 0) {my_toolbar_fct_##A = (uintptr_t)fct; return my_toolbar_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 Toolbar callback\n");
    return NULL;
}

// Builder
#define GO(A)   \
static uintptr_t my_builderconnect_fct_##A = 0;                                                                                     \
static void my_builderconnect_##A(void* builder, void* object, void* signal, void* handler, void* connect, int flags, void* data)   \
{                                                                                                                                   \
    RunFunctionFmt(my_builderconnect_fct_##A, "pppppip", builder, object, signal, handler, connect, flags, data);       \
}
SUPER()
#undef GO
static void* findBuilderConnectFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_builderconnect_fct_##A == (uintptr_t)fct) return my_builderconnect_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_builderconnect_fct_##A == 0) {my_builderconnect_fct_##A = (uintptr_t)fct; return my_builderconnect_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 BuilderConnect callback\n");
    return NULL;
}

// GtkTreeViewSearchEqualFunc
#define GO(A)   \
static uintptr_t my_GtkTreeViewSearchEqualFunc_fct_##A = 0;                                                             \
static int my_GtkTreeViewSearchEqualFunc_##A(void* model, int column, void* key, void* iter, void* data)                \
{                                                                                                                       \
    return RunFunctionFmt(my_GtkTreeViewSearchEqualFunc_fct_##A, "pippp", model, column, key, iter, data);  \
}
SUPER()
#undef GO
static void* findGtkTreeViewSearchEqualFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkTreeViewSearchEqualFunc_fct_##A == (uintptr_t)fct) return my_GtkTreeViewSearchEqualFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkTreeViewSearchEqualFunc_fct_##A == 0) {my_GtkTreeViewSearchEqualFunc_fct_##A = (uintptr_t)fct; return my_GtkTreeViewSearchEqualFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkTreeViewSearchEqualFunc callback\n");
    return NULL;
}
// GtkTreeCellDataFunc
#define GO(A)   \
static uintptr_t my_GtkTreeCellDataFunc_fct_##A = 0;                                                    \
static void my_GtkTreeCellDataFunc_##A(void* tree, void* cell, void* model, void* iter, void* data)     \
{                                                                                                       \
    RunFunctionFmt(my_GtkTreeCellDataFunc_fct_##A, "ppppp", tree, cell, model, iter, data); \
}
SUPER()
#undef GO
static void* findGtkTreeCellDataFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkTreeCellDataFunc_fct_##A == (uintptr_t)fct) return my_GtkTreeCellDataFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkTreeCellDataFunc_fct_##A == 0) {my_GtkTreeCellDataFunc_fct_##A = (uintptr_t)fct; return my_GtkTreeCellDataFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkTreeCellDataFunc callback\n");
    return NULL;
}

// GDestroyNotify
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;                         \
static void my_GDestroyNotify_##A(void* data)                           \
{                                                                       \
    RunFunctionFmt(my_GDestroyNotify_fct_##A, "p", data);   \
}
SUPER()
#undef GO
static void* findGDestroyNotifyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDestroyNotify_fct_##A == (uintptr_t)fct) return my_GDestroyNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDestroyNotify_fct_##A == 0) {my_GDestroyNotify_fct_##A = (uintptr_t)fct; return my_GDestroyNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GDestroyNotify callback\n");
    return NULL;
}

// GtkTreeIterCompareFunc
#define GO(A)   \
static uintptr_t my_GtkTreeIterCompareFunc_fct_##A = 0;                                             \
static int my_GtkTreeIterCompareFunc_##A(void* model, void* a, void* b, void* data)                 \
{                                                                                                   \
    return RunFunctionFmt(my_GtkTreeIterCompareFunc_fct_##A, "pppp", model, a, b, data);\
}
SUPER()
#undef GO
static void* findGtkTreeIterCompareFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GtkTreeIterCompareFunc_fct_##A == (uintptr_t)fct) return my_GtkTreeIterCompareFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GtkTreeIterCompareFunc_fct_##A == 0) {my_GtkTreeIterCompareFunc_fct_##A = (uintptr_t)fct; return my_GtkTreeIterCompareFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-3 GtkTreeIterCompareFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my3_gtk_dialog_add_buttons(x86emu_t* emu, void* dialog, void* first, uintptr_t* b)
{
    (void)emu;
    void* btn = first;
    while(btn) {
        int id = (int)*(b++);
        my->gtk_dialog_add_button(dialog, btn, id);
        btn = (void*)*(b++);
    }
}

EXPORT void my3_gtk_message_dialog_format_secondary_text(x86emu_t* emu, void* dialog, void* fmt, void* b)
{
    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    (void)emu;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    my->gtk_message_dialog_format_secondary_text(dialog, buf);
    free(buf);
}

EXPORT void my3_gtk_message_dialog_format_secondary_markup(x86emu_t* emu, void* dialog, void* fmt, void* b)
{
    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    (void)emu;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    my->gtk_message_dialog_format_secondary_markup(dialog, buf);
    free(buf);
}
EXPORT void* my3_gtk_type_class(x86emu_t* emu, int type)
{
    (void)emu;
    void* class = my->gtk_type_class(type);
    return wrapCopyGTKClass(class, type);
}

EXPORT void my3_gtk_init(x86emu_t* emu, void* argc, void* argv)
{
    (void)emu;
    my->gtk_init(argc, argv);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
}

EXPORT int my3_gtk_init_check(x86emu_t* emu, void* argc, void* argv)
{
    (void)emu;
    int ret = my->gtk_init_check(argc, argv);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
    return ret;
}

EXPORT int my3_gtk_init_with_args(x86emu_t* emu, void* argc, void* argv, void* param, void* entries, void* trans, void* error)
{
    (void)emu;
    int ret = my->gtk_init_with_args(argc, argv, param, entries, trans, error);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
    return ret;
}

EXPORT void my3_gtk_menu_attach_to_widget(x86emu_t* emu, void* menu, void* widget, void* f)
{
    (void)emu;
    my->gtk_menu_attach_to_widget(menu, widget, findMenuDetachFct(f));
}

EXPORT void my3_gtk_menu_popup(x86emu_t* emu, void* menu, void* shell, void* item, void* f, void* data, uint32_t button, uint32_t time_)
{
    (void)emu;
    my->gtk_menu_popup(menu, shell, item, findMenuPositionFct(f), data, button, time_);
}

EXPORT uint32_t my3_gtk_timeout_add(x86emu_t* emu, uint32_t interval, void* f, void* data)
{
    (void)emu;
    return my->gtk_timeout_add(interval, findGtkFunctionFct(f), data);
}

EXPORT int my3_gtk_clipboard_set_with_data(x86emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    (void)emu;
    return my->gtk_clipboard_set_with_data(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

EXPORT int my3_gtk_clipboard_set_with_owner(x86emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    (void)emu;
    return my->gtk_clipboard_set_with_owner(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

static void* my_translate_func(void* path, my_signal_t* sig)
{
    return (void*)RunFunctionFmt(sig->c_handler, "pp", path, sig->data);
}

EXPORT void my3_gtk_stock_set_translate_func(x86emu_t* emu, void* domain, void* f, void* data, void* notify)
{
    (void)emu;
    my_signal_t *sig = new_mysignal(f, data, notify);
    my->gtk_stock_set_translate_func(domain, my_translate_func, sig, my_signal_delete);
}

EXPORT void my3_gtk_container_forall(x86emu_t* emu, void* container, void* f, void* data)
{
    (void)emu;
    my->gtk_container_forall(container, findGtkCallbackFct(f), data);
}

EXPORT void my3_gtk_tree_view_set_search_equal_func(x86emu_t* emu, void* tree_view, void* f, void* data, void* notify)
{
    (void)emu;
    my->gtk_tree_view_set_search_equal_func(tree_view, findGtkTreeViewSearchEqualFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT int my3_gtk_text_iter_backward_find_char(x86emu_t* emu, void* iter, void* f, void* data, void* limit)
{
    (void)emu;
    return my->gtk_text_iter_backward_find_char(iter, findGtkTextCharPredicateFct(f), data, limit);
}

EXPORT int my3_gtk_text_iter_forward_find_char(x86emu_t* emu, void* iter, void* f, void* data, void* limit)
{
    (void)emu;
    return my->gtk_text_iter_forward_find_char(iter, findGtkTextCharPredicateFct(f), data, limit);
}

EXPORT void* my3_gtk_toolbar_append_item(x86emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    (void)emu;
    return my->gtk_toolbar_append_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_prepend_item(x86emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    (void)emu;
    return my->gtk_toolbar_prepend_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_insert_item(x86emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data, int position)
{
    (void)emu;
    return my->gtk_toolbar_insert_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data, position);
}

EXPORT void* my3_gtk_toolbar_append_element(x86emu_t* emu, void* toolbar, int type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    (void)emu;
    return my->gtk_toolbar_append_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_prepend_element(x86emu_t* emu, void* toolbar, int type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    (void)emu;
    return my->gtk_toolbar_prepend_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_insert_element(x86emu_t* emu, void* toolbar, int type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data, int position)
{
    (void)emu;
    return my->gtk_toolbar_insert_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data, position);
}

EXPORT void* my3_gtk_toolbar_insert_stock(x86emu_t* emu, void* toolbar, void* stock_id, void* tooltip_text, void* tooltip_private, void* f, void* data, int position)
{
    (void)emu;
    return my->gtk_toolbar_insert_stock(toolbar, stock_id, tooltip_text, tooltip_private, findToolbarFct(f), data, position);
}

EXPORT void my3_gtk_tree_sortable_set_sort_func(x86emu_t* emu, void* sortable, int id, void* f, void* data, void* notify)
{
    (void)emu;
    my->gtk_tree_sortable_set_sort_func(sortable, id, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void my3_gtk_tree_sortable_set_default_sort_func(x86emu_t* emu, void* sortable, void* f, void* data, void* notify)
{
    (void)emu;
    my->gtk_tree_sortable_set_default_sort_func(sortable, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT int my3_gtk_type_unique(x86emu_t* emu, size_t parent, my_GtkTypeInfo_t* gtkinfo)
{
    (void)emu;
    return my->gtk_type_unique(parent, findFreeGtkTypeInfo(gtkinfo, parent));
}

EXPORT unsigned long my3_gtk_signal_connect(x86emu_t* emu, void* object, void* name, void* func, void* data)
{
    return my3_gtk_signal_connect_full(emu, object, name, func, NULL, data, NULL, 0, 0);
}

EXPORT void my3_gtk_object_set_data_full(x86emu_t* emu, void* object, void* key, void* data, void* notify)
{
    (void)emu;
    my->gtk_object_set_data_full(object, key, data, findGDestroyNotifyFct(notify));
}

EXPORT float my3_gtk_spin_button_get_value_as_float(x86emu_t* emu, void* spinner)
{
    (void)emu;
    return my->gtk_spin_button_get_value(spinner);
}

EXPORT void my3_gtk_builder_connect_signals_full(x86emu_t* emu, void* builder, void* f, void* data)
{
    (void)emu;
    my->gtk_builder_connect_signals_full(builder, findBuilderConnectFct(f), data);
}

typedef struct my_connectargs_s {
  void* module;
  void* data;
} my_connectargs_t;
//defined in gobject2...
uintptr_t my_g_signal_connect_object(x86emu_t* emu, void* instance, void* detailed, void* c_handler, void* object, uint32_t flags);
uintptr_t my_g_signal_connect_data(x86emu_t* emu, void* instance, void* detailed, void* c_handler, void* data, void* closure, uint32_t flags);
static void my3_gtk_builder_connect_signals_default(void* builder, void* object, 
                                                    char* signal_name, char* handler_name,
                                                    void* connect_object, uint32_t flags, my_connectargs_t* args)
{
  void* func = my->gtk_builder_lookup_callback_symbol(builder, handler_name);
  if (!func && args && args->module) {
    my->g_module_symbol(args->module, handler_name, &func);
  }
  // Mixing Native and emulated code... the my_g_signal_* function will handle that (GetNativeFnc does)
  if(!func)
      func = (void*)FindGlobalSymbol(my_context->maplib, handler_name, 0, NULL);

  if(!func) {
      my->g_log("Gtk", 1<<4, "Could not find signal handler '%s'.", handler_name);
      return;
  }

  if (connect_object)
    my_g_signal_connect_object(thread_get_emu(), object, signal_name, func, connect_object, flags);
  else
    my_g_signal_connect_data(thread_get_emu(), object, signal_name, func, args->data, NULL, flags);
}

EXPORT void my3_gtk_builder_connect_signals(x86emu_t* emu, void* builder, void* data)
{
    (void)emu;
    my_connectargs_t args = {0};
    args.data = data;
    if(my->g_module_open && my->g_module_close)
        args.module = my->g_module_open(NULL, 1);

    my->gtk_builder_connect_signals_full(builder, my3_gtk_builder_connect_signals_default, &args);

    if(args.module)
        my->g_module_close(args.module);
}

EXPORT void my3_gtk_tree_view_column_set_cell_data_func(x86emu_t* emu, void* tree, void* cell, void* f, void* data, void* destroy)
{
    (void)emu;
    my->gtk_tree_view_column_set_cell_data_func(tree, cell, findGtkTreeCellDataFuncFct(f), data, findGDestroyNotifyFct(destroy));
}

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    libname = lib->name;                                        \
    getMy(lib);                                                 \
    SETALT(my3_);                                               \
    SetGInitiallyUnownedID(my->g_initially_unowned_get_type()); \
    SetGtkWidget3ID(my->gtk_widget_get_type());                 \
    SetGtkActionID(my->gtk_action_get_type());                  \
    setNeededLibs(lib, 2, "libgdk-3.so.0", "libpangocairo-1.0.so.0");

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
