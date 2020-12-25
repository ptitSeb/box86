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

const char* gtk3Name = "libgtk-3.so.0";
static char* libname = NULL;
#define LIBNAME gtk3

typedef int           (*iFv_t)(void);
typedef void*         (*pFi_t)(int);
typedef void          (*vFp_t)(void*);
typedef void*         (*pFp_t)(void*);
typedef double        (*dFp_t)(void*);
typedef int           (*iFip_t)(int, void*);
typedef int           (*iFpp_t)(void*, void*);
typedef void*         (*pFpi_t)(void*, int);
typedef void          (*vFpp_t)(void*, void*);
typedef void*         (*pFppi_t)(void*, void*, int32_t);
typedef int32_t       (*iFppp_t)(void*, void*, void*);
typedef uint32_t      (*uFupp_t)(uint32_t, void*, void*);
typedef void          (*vFppp_t)(void*, void*, void*);
typedef int           (*iFpppp_t)(void*, void*, void*, void*);
typedef void          (*vFpppp_t)(void*, void*, void*, void*);
typedef void          (*vFpippp_t)(void*, int, void*, void*, void*);
typedef int           (*iFpppppp_t)(void*, void*, void*, void*, void*, void*);
typedef int           (*iFppuppp_t)(void*, void*, uint32_t, void*, void*, void*);
typedef void*         (*pFppppppi_t)(void*, void*, void*, void*, void*, void*, int);
typedef void*         (*pFppppppp_t)(void*, void*, void*, void*, void*, void*, void*);
typedef void*         (*pFpppppppi_t)(void*, void*, void*, void*, void*, void*, void*, int);
typedef void          (*vFpppppuu_t)(void*, void*, void*, void*, void*, uint32_t, uint32_t);
typedef unsigned long (*LFppppppii_t)(void*, void*, void*, void*, void*, void*, int32_t, int32_t);
typedef void*         (*pFpippppppp_t)(void*, int, void*, void*, void*, void*, void*, void*, void*);
typedef void*         (*pFpipppppppi_t)(void*, int, void*, void*, void*, void*, void*, void*, void*, int);

#define SUPER() \
    GO(gtk_object_get_type, iFv_t)              \
    GO(gtk_object_set_data_full, vFpppp_t)      \
    GO(g_type_check_instance_cast, pFpi_t)      \
    GO(gtk_bin_get_type, iFv_t)                 \
    GO(gtk_widget_get_type, iFv_t)              \
    GO(gtk_button_get_type, iFv_t)              \
    GO(gtk_container_get_type, iFv_t)           \
    GO(gtk_label_get_type, iFv_t)               \
    GO(gtk_tree_view_get_type, iFv_t)           \
    GO(gtk_type_class, pFi_t)                   \
    GO(gtk_button_get_label, pFp_t)             \
    GO(gtk_signal_connect_full, LFppppppii_t)   \
    GO(gtk_dialog_add_button, pFppi_t)          \
    GO(gtk_message_dialog_format_secondary_text, vFpp_t)    \
    GO(gtk_message_dialog_format_secondary_markup, vFpp_t)  \
    GO(gtk_init, vFpp_t)                        \
    GO(gtk_init_check, iFpp_t)                  \
    GO(gtk_init_with_args, iFpppppp_t)          \
    GO(gtk_menu_attach_to_widget, vFppp_t)      \
    GO(gtk_menu_popup, vFpppppuu_t)             \
    GO(gtk_timeout_add, uFupp_t)                \
    GO(gtk_clipboard_set_with_data, iFppuppp_t) \
    GO(gtk_clipboard_set_with_owner, iFppuppp_t)\
    GO(gtk_stock_set_translate_func, vFpppp_t)  \
    GO(gtk_container_forall, vFppp_t)           \
    GO(gtk_tree_view_set_search_equal_func, vFpppp_t)   \
    GO(gtk_text_iter_backward_find_char, iFpppp_t)      \
    GO(gtk_text_iter_forward_find_char, iFpppp_t)       \
    GO(gtk_toolbar_append_item, pFppppppp_t)    \
    GO(gtk_toolbar_prepend_item, pFppppppp_t)   \
    GO(gtk_toolbar_insert_item, pFpppppppi_t)   \
    GO(gtk_toolbar_append_element, pFpippppppp_t)       \
    GO(gtk_toolbar_prepend_element, pFpippppppp_t)      \
    GO(gtk_toolbar_insert_element, pFpipppppppi_t)      \
    GO(gtk_toolbar_insert_stock, pFppppppi_t)   \
    GO(gtk_tree_sortable_set_sort_func, vFpippp_t)      \
    GO(gtk_tree_sortable_set_default_sort_func, vFpppp_t)\
    GO(gtk_type_unique, iFip_t)                 \
    GO(gtk_spin_button_get_value, dFp_t)        \
    GO(gtk_builder_connect_signals_full, vFppp_t)       \
    GO(gtk_action_get_type, iFv_t)              \
    GO(g_type_class_ref, pFi_t)                 \
    GO(g_type_class_unref, vFp_t)               \


typedef struct gtk3_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gtk3_my_t;

static void* getGtk3My(library_t* lib)
{
    gtk3_my_t* my = (gtk3_my_t*)calloc(1, sizeof(gtk3_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

static void freeGtk3My(void* lib)
{
    //gtk3_my_t *my = (gtk3_my_t *)lib;
}

static box86context_t* context = NULL;

EXPORT uintptr_t my3_gtk_signal_connect_full(x86emu_t* emu, void* object, void* name, void* c_handler, void* unsupported, void* data, void* closure, uint32_t signal, int after)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    if(!context)
        context = emu->context;

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
static uintptr_t my_menudetach_fct_##A = 0;   \
static void my_menudetach_##A(void* widget, void* menu)     \
{                                       \
    RunFunction(my_context, my_menudetach_fct_##A, 2, widget, menu);\
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
static uintptr_t my_menuposition_fct_##A = 0;   \
static void my_menuposition_##A(void* menu, void* x, void* y, void* push_in, void* data)     \
{                                       \
    RunFunction(my_context, my_menuposition_fct_##A, 5, menu, x, y, push_in, data);\
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
static uintptr_t my3_gtkfunction_fct_##A = 0;   \
static int my3_gtkfunction_##A(void* data)     \
{                                       \
    return RunFunction(my_context, my3_gtkfunction_fct_##A, 1, data);\
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
static uintptr_t my_clipboardget_fct_##A = 0;   \
static void my_clipboardget_##A(void* clipboard, void* selection, uint32_t info, void* data)     \
{                                       \
    RunFunction(my_context, my_clipboardget_fct_##A, 4, clipboard, selection, info, data);\
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
static uintptr_t my_clipboardclear_fct_##A = 0;   \
static void my_clipboardclear_##A(void* clipboard, void* data)     \
{                                       \
    RunFunction(my_context, my_clipboardclear_fct_##A, 2, clipboard, data);\
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
static uintptr_t my3_gtkcallback_fct_##A = 0;   \
static void my3_gtkcallback_##A(void* widget, void* data)     \
{                                       \
    RunFunction(my_context, my3_gtkcallback_fct_##A, 2, widget, data);\
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
static uintptr_t my_textcharpredicate_fct_##A = 0;   \
static int my_textcharpredicate_##A(uint32_t ch, void* data)     \
{                                       \
    return (int)RunFunction(my_context, my_textcharpredicate_fct_##A, 2, ch, data);\
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
static uintptr_t my_toolbar_fct_##A = 0;   \
static void my_toolbar_##A(void* widget, void* data)     \
{                                       \
    RunFunction(my_context, my_toolbar_fct_##A, 2, widget, data);\
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
static uintptr_t my_builderconnect_fct_##A = 0;   \
static void my_builderconnect_##A(void* builder, void* object, void* signal, void* handler, void* connect, int flags, void* data)     \
{                                       \
    RunFunction(my_context, my_builderconnect_fct_##A, 7, builder, object, signal, handler, connect, flags, data);\
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
static uintptr_t my_GtkTreeViewSearchEqualFunc_fct_##A = 0;                                                     \
static int my_GtkTreeViewSearchEqualFunc_##A(void* model, int column, void* key, void* iter, void* data)        \
{                                                                                                               \
    return RunFunction(my_context, my_GtkTreeViewSearchEqualFunc_fct_##A, 5, model, column, key, iter, data);   \
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

// GDestroyNotify
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;                     \
static void my_GDestroyNotify_##A(void* data)                       \
{                                                                   \
    RunFunction(my_context, my_GDestroyNotify_fct_##A, 1, data);    \
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
static uintptr_t my_GtkTreeIterCompareFunc_fct_##A = 0;                                         \
static int my_GtkTreeIterCompareFunc_##A(void* model, void* a, void* b, void* data)             \
{                                                                                               \
    return RunFunction(my_context, my_GtkTreeIterCompareFunc_fct_##A, 4, model, a, b, data);    \
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
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    void* btn = first;
    while(btn) {
        int id = (int)*(b++);
        my->gtk_dialog_add_button(dialog, btn, id);
        btn = (void*)*(b++);
    }
}

EXPORT void my3_gtk_message_dialog_format_secondary_text(x86emu_t* emu, void* dialog, void* fmt, void* b)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    my->gtk_message_dialog_format_secondary_text(dialog, buf);
    free(buf);
}

EXPORT void my3_gtk_message_dialog_format_secondary_markup(x86emu_t* emu, void* dialog, void* fmt, void* b)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    my->gtk_message_dialog_format_secondary_markup(dialog, buf);
    free(buf);
}
EXPORT void* my3_gtk_type_class(x86emu_t* emu, int type)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    void* class = my->gtk_type_class(type);
    return wrapCopyGTKClass(class, type);
}

EXPORT void my3_gtk_init(x86emu_t* emu, void* argc, void* argv)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my->gtk_init(argc, argv);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
}

EXPORT int my3_gtk_init_check(x86emu_t* emu, void* argc, void* argv)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    int ret = my->gtk_init_check(argc, argv);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
    return ret;
}

EXPORT int my3_gtk_init_with_args(x86emu_t* emu, void* argc, void* argv, void* param, void* entries, void* trans, void* error)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    int ret = my->gtk_init_with_args(argc, argv, param, entries, trans, error);
    my_checkGlobalGdkDisplay();
    AutoBridgeGtk(my->g_type_class_ref, my->g_type_class_unref);
    return ret;
}

EXPORT void my3_gtk_menu_attach_to_widget(x86emu_t* emu, void* menu, void* widget, void* f)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my->gtk_menu_attach_to_widget(menu, widget, findMenuDetachFct(f));
}

EXPORT void my3_gtk_menu_popup(x86emu_t* emu, void* menu, void* shell, void* item, void* f, void* data, uint32_t button, uint32_t time_)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my->gtk_menu_popup(menu, shell, item, findMenuPositionFct(f), data, button, time_);
}

EXPORT uint32_t my3_gtk_timeout_add(x86emu_t* emu, uint32_t interval, void* f, void* data)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_timeout_add(interval, findGtkFunctionFct(f), data);
}

EXPORT int my3_gtk_clipboard_set_with_data(x86emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_clipboard_set_with_data(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

EXPORT int my3_gtk_clipboard_set_with_owner(x86emu_t* emu, void* clipboard, void* target, uint32_t n, void* f_get, void* f_clear, void* data)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_clipboard_set_with_owner(clipboard, target, n, findClipboadGetFct(f_get), findClipboadClearFct(f_clear), data);
}

static void* my_translate_func(void* path, my_signal_t* sig)
{
    return (void*)RunFunction(my_context, sig->c_handler, 2, path, sig->data);
}

EXPORT void my3_gtk_stock_set_translate_func(x86emu_t* emu, void* domain, void* f, void* data, void* notify)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my_signal_t *sig = new_mysignal(f, data, notify);
    my->gtk_stock_set_translate_func(domain, my_translate_func, sig, my_signal_delete);
}

EXPORT void my3_gtk_container_forall(x86emu_t* emu, void* container, void* f, void* data)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my->gtk_container_forall(container, findGtkCallbackFct(f), data);
}

EXPORT void my3_gtk_tree_view_set_search_equal_func(x86emu_t* emu, void* tree_view, void* f, void* data, void* notify)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my->gtk_tree_view_set_search_equal_func(tree_view, findGtkTreeViewSearchEqualFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT int my3_gtk_text_iter_backward_find_char(x86emu_t* emu, void* iter, void* f, void* data, void* limit)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_text_iter_backward_find_char(iter, findGtkTextCharPredicateFct(f), data, limit);
}

EXPORT int my3_gtk_text_iter_forward_find_char(x86emu_t* emu, void* iter, void* f, void* data, void* limit)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_text_iter_forward_find_char(iter, findGtkTextCharPredicateFct(f), data, limit);
}

EXPORT void* my3_gtk_toolbar_append_item(x86emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_toolbar_append_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_prepend_item(x86emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_toolbar_prepend_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_insert_item(x86emu_t* emu, void* toolbar, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data, int position)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_toolbar_insert_item(toolbar, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data, position);
}

EXPORT void* my3_gtk_toolbar_append_element(x86emu_t* emu, void* toolbar, int type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_toolbar_append_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_prepend_element(x86emu_t* emu, void* toolbar, int type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_toolbar_prepend_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data);
}

EXPORT void* my3_gtk_toolbar_insert_element(x86emu_t* emu, void* toolbar, int type, void* widget, void* text, void* tooltip_text, void* tooltip_private, void* icon, void* f, void* data, int position)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_toolbar_insert_element(toolbar, type, widget, text, tooltip_text, tooltip_private, icon, findToolbarFct(f), data, position);
}

EXPORT void* my3_gtk_toolbar_insert_stock(x86emu_t* emu, void* toolbar, void* stock_id, void* tooltip_text, void* tooltip_private, void* f, void* data, int position)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_toolbar_insert_stock(toolbar, stock_id, tooltip_text, tooltip_private, findToolbarFct(f), data, position);
}

EXPORT void my3_gtk_tree_sortable_set_sort_func(x86emu_t* emu, void* sortable, int id, void* f, void* data, void* notify)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my->gtk_tree_sortable_set_sort_func(sortable, id, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void my3_gtk_tree_sortable_set_default_sort_func(x86emu_t* emu, void* sortable, void* f, void* data, void* notify)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my->gtk_tree_sortable_set_default_sort_func(sortable, findGtkTreeIterCompareFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT int my3_gtk_type_unique(x86emu_t* emu, int parent, my_GtkTypeInfo_t* gtkinfo)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_type_unique(parent, findFreeGtkTypeInfo(gtkinfo, parent));
}

EXPORT unsigned long my3_gtk_signal_connect(x86emu_t* emu, void* object, void* name, void* func, void* data)
{
    return my3_gtk_signal_connect_full(emu, object, name, func, NULL, data, NULL, 0, 0);
}

EXPORT void my3_gtk_object_set_data_full(x86emu_t* emu, void* object, void* key, void* data, void* notify)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my->gtk_object_set_data_full(object, key, data, findGDestroyNotifyFct(notify));
}

EXPORT float my3_gtk_spin_button_get_value_as_float(x86emu_t* emu, void* spinner)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    return my->gtk_spin_button_get_value(spinner);
}

EXPORT void my3_gtk_builder_connect_signals_full(x86emu_t* emu, void* builder, void* f, void* data)
{
    library_t * lib = GetLibInternal(libname);
    gtk3_my_t *my = (gtk3_my_t*)lib->priv.w.p2;

    my->gtk_builder_connect_signals_full(builder, findBuilderConnectFct(f), data);
}
//GtkMisc is deprecated in GTK3, so use widget directly...

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    libname = lib->name;                \
    lib->priv.w.p2 = getGtk3My(lib);    \
    lib->altmy = strdup("my3_");        \
    SetGtkWidgetID(((gtk3_my_t*)lib->priv.w.p2)->gtk_widget_get_type());    \
    SetGtkContainerID(((gtk3_my_t*)lib->priv.w.p2)->gtk_container_get_type());     \
    SetGtkActionID(((gtk3_my_t*)lib->priv.w.p2)->gtk_action_get_type());    \
    SetGtkMiscID(((gtk3_my_t*)lib->priv.w.p2)->gtk_widget_get_type());      \
    SetGtkLabelID(((gtk3_my_t*)lib->priv.w.p2)->gtk_label_get_type());      \
    SetGtkTreeViewID(((gtk3_my_t*)lib->priv.w.p2)->gtk_tree_view_get_type());       \
    lib->priv.w.needed = 2; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libgdk-3.so.0");                  \
    lib->priv.w.neededlibs[1] = strdup("libpangocairo-1.0.so.0");

#define CUSTOM_FINI \
    freeGtk3My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
