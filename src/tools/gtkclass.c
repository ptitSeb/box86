#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "box86context.h"
#include "librarian.h"
#include "gtkclass.h"


static bridge_t*        my_bridge       = NULL;
static int              my_gobject      = -1;
static int              my_gtkobject    = -1;
static int              my_gtkwidget    = -1;
static int              my_gtkcontainer = -1;
static const char* (*g_type_name)(int)  = NULL;
// ---- Defining the multiple functions now -----
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)

#define WRAPPED(A, NAME, RET, DEF, N, ...)  \
static uintptr_t my_##NAME##_fct_##A = 0;   \
static RET my_##NAME##_##A DEF              \
{                                           \
    printf_log(LOG_DEBUG, "Calling " #NAME "_" #A " wrapper\n");             \
    return (RET)RunFunction(my_context, my_##NAME##_fct_##A, N, __VA_ARGS__);\
}

#define FIND(NAME) \
static void* find_##NAME(void* fct) \
{   \
    if(!fct) return fct;    \
    if(my_##NAME##_fct_0 == (uintptr_t)fct) return my_##NAME##_0;   \
    if(my_##NAME##_fct_1 == (uintptr_t)fct) return my_##NAME##_1;   \
    if(my_##NAME##_fct_2 == (uintptr_t)fct) return my_##NAME##_2;   \
    if(my_##NAME##_fct_3 == (uintptr_t)fct) return my_##NAME##_3;   \
    if(my_##NAME##_fct_4 == (uintptr_t)fct) return my_##NAME##_4;   \
    if(my_##NAME##_fct_5 == (uintptr_t)fct) return my_##NAME##_5;   \
    if(my_##NAME##_fct_6 == (uintptr_t)fct) return my_##NAME##_6;   \
    if(my_##NAME##_fct_7 == (uintptr_t)fct) return my_##NAME##_7;   \
    if(my_##NAME##_fct_0 == 0) {my_##NAME##_fct_0 = (uintptr_t)fct; return my_##NAME##_0; } \
    if(my_##NAME##_fct_1 == 0) {my_##NAME##_fct_1 = (uintptr_t)fct; return my_##NAME##_1; } \
    if(my_##NAME##_fct_2 == 0) {my_##NAME##_fct_2 = (uintptr_t)fct; return my_##NAME##_2; } \
    if(my_##NAME##_fct_3 == 0) {my_##NAME##_fct_3 = (uintptr_t)fct; return my_##NAME##_3; } \
    if(my_##NAME##_fct_4 == 0) {my_##NAME##_fct_4 = (uintptr_t)fct; return my_##NAME##_4; } \
    if(my_##NAME##_fct_5 == 0) {my_##NAME##_fct_5 = (uintptr_t)fct; return my_##NAME##_5; } \
    if(my_##NAME##_fct_6 == 0) {my_##NAME##_fct_6 = (uintptr_t)fct; return my_##NAME##_6; } \
    if(my_##NAME##_fct_7 == 0) {my_##NAME##_fct_7 = (uintptr_t)fct; return my_##NAME##_7; } \
    printf_log(LOG_NONE, "Warning, no more slot for " #NAME " gtkclass callback\n");    \
    return NULL;    \
}

#define WRAPPER(A, NAME, RET, DEF, N, ...)  \
WRAPPED(0, NAME, RET, DEF, N, __VA_ARGS__)  \
WRAPPED(1, NAME, RET, DEF, N, __VA_ARGS__)  \
WRAPPED(2, NAME, RET, DEF, N, __VA_ARGS__)  \
WRAPPED(3, NAME, RET, DEF, N, __VA_ARGS__)  \
WRAPPED(4, NAME, RET, DEF, N, __VA_ARGS__)  \
WRAPPED(5, NAME, RET, DEF, N, __VA_ARGS__)  \
WRAPPED(6, NAME, RET, DEF, N, __VA_ARGS__)  \
WRAPPED(7, NAME, RET, DEF, N, __VA_ARGS__)  \
FIND(NAME)

// ----- GObjectClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(A, constructor, void*, (int type, uint32_t n_construct_properties, void* construct_properties), 3, type, n_construct_properties, construct_properties);
WRAPPER(A, set_property, void, (void* object, uint32_t property_id, void* value, void* pspec), 4, object, property_id, value, pspec);
WRAPPER(A, get_property, void, (void* object, uint32_t property_id, void* value, void* pspec), 4, object, property_id, value, pspec);
WRAPPER(A, dispose, void, (void* object), 1, object);
WRAPPER(A, finalize, void, (void* object), 1, object);
WRAPPER(A, dispatch_properties_changed, void*, (int type, uint32_t n_pspecs, void* pspecs), 3, type, n_pspecs, pspecs);
WRAPPER(A, notify, void*, (int type, void* pspecs), 2, type, pspecs);
WRAPPER(A, constructed, void, (void* object), 1, object);

#define SUPERGO() \
    GO(constructor, pFiup);                 \
    GO(set_property, vFpupp);               \
    GO(get_property, vFpupp);               \
    GO(dispose, vFp);                       \
    GO(finalize, vFp);                      \
    GO(dispatch_properties_changed, vFpup); \
    GO(notify, vFpp);                       \
    GO(constructed, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGObjectClass(my_GObjectClass_t* class)
{
    #define GO(A, W) if(class->A) class->A = (void*)AddCheckBridge(my_bridge, W, class->A, 0)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGObjectClass(my_GObjectClass_t* class)
{   
    void* tmp;
    #define GO(A, W)   if(class->A) {tmp = GetNativeFnc((uintptr_t)class->A); if(tmp) class->A = tmp; else class->A = find_##A (class->A);}
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GtkObjectClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(A, set_arg, void, (void* object, void* arg, uint32_t arg_id), 3, object, arg, arg_id);
WRAPPER(A, get_arg, void, (void* object, void* arg, uint32_t arg_id), 3, object, arg, arg_id);
WRAPPER(A, destroy, void, (void* object), 1, object);

#define SUPERGO() \
    GO(set_arg, vFppu); \
    GO(get_arg, vFppu); \
    GO(destroy, vFp);
// wrap (so bridge all calls, just in case)
static void wrapGtkObjectClass(my_GtkObjectClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) if(class->A) class->A = (void*)AddCheckBridge(my_bridge, W, class->A, 0)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkObjectClass(my_GtkObjectClass_t* class)
{   
    unwrapGObjectClass(&class->parent_class);
    void* tmp;
    #define GO(A, W)   if(class->A) {tmp = GetNativeFnc((uintptr_t)class->A); if(tmp) class->A = tmp; else class->A = find_##A (class->A);}
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GtkWidgetClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(A, dispatch_child_properties_changed, void, (void* widget, uint32_t n_pspecs, void* pspecs), 3, widget, n_pspecs, pspecs);
WRAPPER(A, show,              void, (void* widget), 1, widget);
WRAPPER(A, show_all,          void, (void* widget), 1, widget);
WRAPPER(A, hide,              void, (void* widget), 1, widget);
WRAPPER(A, hide_all,          void, (void* widget), 1, widget);
WRAPPER(A, map,               void, (void* widget), 1, widget);
WRAPPER(A, unmap,             void, (void* widget), 1, widget);
WRAPPER(A, realize,           void, (void* widget), 1, widget);
WRAPPER(A, unrealize,         void, (void* widget), 1, widget);
WRAPPER(A, size_request,      void, (void* widget, void* requisition), 2, widget, requisition);
WRAPPER(A, size_allocate,     void, (void* widget, void* allocation), 2, widget, allocation);
WRAPPER(A, state_changed,     void, (void* widget, int previous_state), 2, widget, previous_state);
WRAPPER(A, parent_set,        void, (void* widget, void* previous_parent), 2, widget, previous_parent);
WRAPPER(A, hierarchy_changed, void, (void* widget, void* previous_toplevel), 2, widget, previous_toplevel);
WRAPPER(A, style_set,         void, (void* widget, void* previous_style), 2, widget, previous_style);
WRAPPER(A, direction_changed, void, (void* widget, int previous_direction), 2, widget, previous_direction);
WRAPPER(A, grab_notify,       void, (void* widget, int was_grabbed), 2, widget, was_grabbed);
WRAPPER(A, child_notify,      void, (void* widget, void* pspec), 2, widget, pspec);
WRAPPER(A, mnemonic_activate, int, (void* widget, int group_cycling), 2, widget, group_cycling);
WRAPPER(A, grab_focus,        void, (void* widget), 1, widget);
WRAPPER(A, focus,             int, (void* widget, int direction), 2, widget, direction);
WRAPPER(A, event,             int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, button_press_event,int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, button_release_event, int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, scroll_event,      int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, motion_notify_event, int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, delete_event,       int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, destroy_event,      int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, expose_event,       int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, key_press_event,    int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, key_release_event,  int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, enter_notify_event, int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, leave_notify_event, int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, configure_event,    int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, focus_in_event,     int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, focus_out_event,    int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, map_event,          int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, unmap_event,        int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, property_notify_event,  int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, selection_clear_event,  int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, selection_request_event,int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, selection_notify_event, int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, proximity_in_event,  int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, proximity_out_event, int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, visibility_notify_event, int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, client_event,        int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, no_expose_event,     int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, window_state_event,  int, (void* widget, void* event), 2, widget, event);
WRAPPER(A, selection_get,       void, (void* widget, void* selection_data, uint32_t info, uint32_t time_), 4, widget, selection_data, info, time_);
WRAPPER(A, selection_received,  void, (void* widget, void* selection_data, uint32_t time_), 3, widget, selection_data, time_);
WRAPPER(A, drag_begin,          void, (void* widget, void* context), 2, widget, context);
WRAPPER(A, drag_end,            void, (void* widget, void* context), 2, widget, context);
WRAPPER(A, drag_data_get,       void, (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_), 5, widget, context, selection_data, info, time_);
WRAPPER(A, drag_data_delete,    void, (void* widget, void* context), 2, widget, context);
WRAPPER(A, drag_leave,          void, (void* widget, void* context, uint32_t time_), 3, widget, context, time_);
WRAPPER(A, drag_motion,         int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), 5, widget, context, x, y, time_);
WRAPPER(A, drag_drop,           int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), 5, widget, context, x, y, time_);
WRAPPER(A, drag_data_received,  void, (void* widget, void* context, int32_t x, int32_t y, void* selection_data, uint32_t info, uint32_t time_), 7, widget, context, x, y, selection_data, info, time_);
WRAPPER(A,  popup_menu,         int  , (void* widget), 1, widget);
WRAPPER(A,  show_help,          int  , (void* widget, int help_type), 2, widget, help_type);
WRAPPER(A, get_accessible,      void*, (void* widget), 1, widget);
WRAPPER(A, screen_changed,      void , (void* widget, void* previous_screen), 2, widget, previous_screen);
WRAPPER(A, can_activate_accel,  int  , (void* widget, uint32_t signal_id), 2, widget, signal_id);
WRAPPER(A, grab_broken_event,   int  , (void* widget, void* event), 2, widget, event);
WRAPPER(A,  composited_changed, void , (void* widget), 1, widget);
WRAPPER(A,  query_tooltip,      int  , (void* widget, int32_t x, int32_t y, int keyboard_tooltip, void* tooltip), 5, widget, x, y, keyboard_tooltip, tooltip);

#define SUPERGO() \
    GO(dispatch_child_properties_changed, vFpup);   \
    GO(show, vFp);                                  \
    GO(show_all, vFp);                              \
    GO(hide, vFp);                                  \
    GO(hide_all, vFp);                              \
    GO(map, vFp);                                   \
    GO(unmap, vFp);                                 \
    GO(realize, vFp);                               \
    GO(unrealize, vFp);                             \
    GO(size_request, vFpp);                         \
    GO(size_allocate, vFpp);                        \
    GO(state_changed, vFpi);                        \
    GO(parent_set, vFpp);                           \
    GO(hierarchy_changed, vFpp);                    \
    GO(style_set, vFpp);                            \
    GO(direction_changed, vFpi);                    \
    GO(grab_notify, vFpi);                          \
    GO(child_notify, vFpp);                         \
    GO(mnemonic_activate, iFpi);                    \
    GO(grab_focus, vFp);                            \
    GO(focus, iFpi);                                \
    GO(event, iFpp);                                \
    GO(button_press_event, iFpp);                   \
    GO(button_release_event, iFpp);                 \
    GO(scroll_event, iFpp);                         \
    GO(motion_notify_event, iFpp);                  \
    GO(delete_event, iFpp);                         \
    GO(destroy_event, iFpp);                        \
    GO(expose_event, iFpp);                         \
    GO(key_press_event, iFpp);                      \
    GO(key_release_event, iFpp);                    \
    GO(enter_notify_event, iFpp);                   \
    GO(leave_notify_event, iFpp);                   \
    GO(configure_event, iFpp);                      \
    GO(focus_in_event, iFpp);                       \
    GO(focus_out_event, iFpp);                      \
    GO(map_event, iFpp);                            \
    GO(unmap_event, iFpp);                          \
    GO(property_notify_event, iFpp);                \
    GO(selection_clear_event, iFpp);                \
    GO(selection_request_event, iFpp);              \
    GO(selection_notify_event, iFpp);               \
    GO(proximity_in_event, iFpp);                   \
    GO(proximity_out_event, iFpp);                  \
    GO(visibility_notify_event, iFpp);              \
    GO(client_event, iFpp);                         \
    GO(no_expose_event, iFpp);                      \
    GO(window_state_event, iFpp);                   \
    GO(selection_get, vFppuu);                      \
    GO(selection_received, vFppu);                  \
    GO(drag_begin, vFpp);                           \
    GO(drag_end, vFpp);                             \
    GO(drag_data_get, vFpppuu);                     \
    GO(drag_data_delete, vFpp);                     \
    GO(drag_leave, vFppu);                          \
    GO(drag_motion, iFppiiu);                       \
    GO(drag_drop, iFppiiu);                         \
    GO(drag_data_received, vFppiipuu);              \
    GO(popup_menu, iFp);                            \
    GO(show_help, iFpi);                            \
    GO(get_accessible, pFp);                        \
    GO(screen_changed, vFpp);                       \
    GO(can_activate_accel, iFpu);                   \
    GO(grab_broken_event, iFpp);                    \
    GO(composited_changed, vFp);                    \
    GO(query_tooltip, iFpiiip);

// wrap (so bridge all calls, just in case)
static void wrapGtkWidgetClass(my_GtkWidgetClass_t* class)
{
    wrapGtkObjectClass(&class->parent_class);
    #define GO(A, W) if(class->A) class->A = (void*)AddCheckBridge(my_bridge, W, class->A, 0)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWidgetClass(my_GtkWidgetClass_t* class)
{   
    unwrapGtkObjectClass(&class->parent_class);
    void* tmp;
    #define GO(A, W)   if(class->A) {tmp = GetNativeFnc((uintptr_t)class->A); if(tmp) class->A = tmp; else class->A = find_##A (class->A);}
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// ----- GtkContainerClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(A, add, void, (void* container, void* widget), 2, container, widget);
WRAPPER(A, remove, void, (void* container, void* widget), 2, container, widget);
WRAPPER(A, check_resize, void, (void* container), 1, container);
WRAPPER(A, forall, void, (void* container, int include_internals, void* callback, void* callback_data), 4, container, include_internals, AddCheckBridge(my_bridge, vFpp, callback, 0), callback_data);
WRAPPER(A, set_focus_child, void, (void* container, void* widget), 2, container, widget);
WRAPPER(A, child_type, int, (void* container), 1, container);
WRAPPER(A, composite_name, void*, (void* container, void* child), 2, container, child);
WRAPPER(A, set_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), 5, container, child, property_id, value, pspec);
WRAPPER(A, get_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), 5, container, child, property_id, value, pspec);

#define SUPERGO() \
    GO(add, vFpp);                  \
    GO(remove, vFpp);               \
    GO(check_resize, vFp);          \
    GO(forall, vFpipp);             \
    GO(set_focus_child, vFpp);      \
    GO(child_type, iFp);            \
    GO(composite_name, pFpp);       \
    GO(set_child_property, vFppupp);\
    GO(get_child_property, vFppupp);

// wrap (so bridge all calls, just in case)
static void wrapGtkContainerClass(my_GtkContainerClass_t* class)
{
    wrapGtkWidgetClass(&class->parent_class);
    #define GO(A, W) if(class->A) class->A = (void*)AddCheckBridge(my_bridge, W, class->A, 0)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkContainerClass(my_GtkContainerClass_t* class)
{   
    unwrapGtkWidgetClass(&class->parent_class);
    void* tmp;
    #define GO(A, W)   if(class->A) {tmp = GetNativeFnc((uintptr_t)class->A); if(tmp) class->A = tmp; else class->A = find_##A (class->A);}
    SUPERGO()
    #undef GO
}
#undef SUPERGO

// No more wrap/unwrap
#undef WRAPPER
#undef FIND
#undef WRAPPED

static void wrapGTKClass(void* class, int type)
{
    if(type==my_gtkcontainer)
        wrapGtkContainerClass((my_GtkContainerClass_t*)class);
    else if(type==my_gtkwidget)
        wrapGtkWidgetClass((my_GtkWidgetClass_t*)class);
    else if(type==my_gtkobject)
        wrapGtkObjectClass((my_GtkObjectClass_t*)class);
    else if(type==my_gobject)
        wrapGObjectClass((my_GObjectClass_t*)class);
    else {
        printf_log(LOG_INFO, "Warning, Custom Class initializer with unknown class type %d (%s)\n", type, g_type_name(type));
    }
}

static void unwrapGTKClass(void* class, int type)
{
    if(type==my_gtkcontainer)
        unwrapGtkContainerClass((my_GtkContainerClass_t*)class);
    else if(type==my_gtkwidget)
        unwrapGtkWidgetClass((my_GtkWidgetClass_t*)class);
    else if(type==my_gtkobject)
        unwrapGtkObjectClass((my_GtkObjectClass_t*)class);
    else if(type==my_gobject)
        unwrapGObjectClass((my_GObjectClass_t*)class);
    // no warning, one is enough...
}

// ---- GTKClass -----
static void** table_ref = NULL;
static void** table_nat = NULL;
static int     table_sz = 0;
static int    table_cap = 0;
void* wrapCopyGTKClass(void* class, int type)
{
    // search for an existing ref...
    for (int i=0; i<table_sz; ++i)
        if(table_ref[i] == class)
            return table_nat[i];
    // add a new one...
    if(table_sz == table_cap) {
        table_cap += 4;
        table_ref = (void**)realloc(table_ref, sizeof(void*)*table_cap);
        table_nat = (void**)realloc(table_nat, sizeof(void*)*table_cap);
    }
    table_ref[table_sz] = class;
    if(type==my_gtkcontainer) {
        my_GtkContainerClass_t *tmp = (my_GtkContainerClass_t*)malloc(sizeof(my_GtkContainerClass_t));
        memcpy(tmp, class, sizeof(my_GtkContainerClass_t));
        wrapGtkContainerClass(tmp);
        table_nat[table_sz] = tmp;
    } else if(type==my_gtkwidget) {
        my_GtkWidgetClass_t *tmp = (my_GtkWidgetClass_t*)malloc(sizeof(my_GtkWidgetClass_t));
        memcpy(tmp, class, sizeof(my_GtkWidgetClass_t));
        wrapGtkWidgetClass(tmp);
        table_nat[table_sz] = tmp;
    } else if(type==my_gtkobject) {
        my_GtkObjectClass_t *tmp = (my_GtkObjectClass_t*)malloc(sizeof(my_GtkObjectClass_t));
        memcpy(tmp, class, sizeof(my_GtkObjectClass_t));
        wrapGtkObjectClass((my_GtkObjectClass_t*)class);
        table_nat[table_sz] = tmp;
    } else if(type==my_gobject) {
        my_GObjectClass_t *tmp = (my_GObjectClass_t*)malloc(sizeof(my_GObjectClass_t));
        memcpy(tmp, class, sizeof(my_GObjectClass_t));
        wrapGObjectClass((my_GObjectClass_t*)class);
        table_nat[table_sz] = tmp;
    } else {
        printf_log(LOG_INFO, "Warning, Custom Class function with unknown class type %d (%s)\n", type, g_type_name(type));
        return class;
    }
    return table_nat[table_sz++];
}

// ---- GTypeValueTable ----

// First the structure GTypeInfo statics, with paired x86 source pointer
#define GO(A) \
static my_GTypeValueTable_t     my_gtypevaluetable_##A = {0};   \
static my_GTypeValueTable_t   *ref_gtypevaluetable_##A = NULL;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
#define GO(A)   \
static uintptr_t fct_funcs_value_init_##A = 0;  \
static void my_funcs_value_init_##A(void* value) {   \
    printf_log(LOG_DEBUG, "Calling fct_funcs_value_init_" #A " wrapper\n");             \
    RunFunction(my_context, fct_funcs_value_init_##A, 1, value);    \
}   \
static uintptr_t fct_funcs_value_free_##A = 0;  \
static void my_funcs_value_free_##A(void* value) {   \
    printf_log(LOG_DEBUG, "Calling fct_funcs_value_free_" #A " wrapper\n");             \
    RunFunction(my_context, fct_funcs_value_free_##A, 1, value);    \
}   \
static uintptr_t fct_funcs_value_copy_##A = 0;  \
static void my_funcs_value_copy_##A(void* source, void* dest) {   \
    printf_log(LOG_DEBUG, "Calling fct_funcs_value_copy_" #A " wrapper\n");             \
    RunFunction(my_context, fct_funcs_value_copy_##A, 2, source, dest);    \
}   \
static uintptr_t fct_funcs_value_peek_pointer_##A = 0;  \
static void* my_funcs_value_peek_pointer_##A(void* value) {   \
    printf_log(LOG_DEBUG, "Calling fct_funcs_value_peek_pointer_" #A " wrapper\n");             \
    return (void*)RunFunction(my_context, fct_funcs_value_peek_pointer_##A, 1, value);    \
}   \
static uintptr_t fct_funcs_collect_value_##A = 0;  \
static void* my_funcs_collect_value_##A(void* value, uint32_t n, void* collect, uint32_t flags) {   \
    printf_log(LOG_DEBUG, "Calling fct_funcs_collect_value_" #A " wrapper\n");             \
    return (void*)RunFunction(my_context, fct_funcs_collect_value_##A, 4, value, n, collect, flags);    \
}   \
static uintptr_t fct_funcs_lcopy_value_##A = 0;  \
static void* my_funcs_lcopy_value_##A(void* value, uint32_t n, void* collect, uint32_t flags) {   \
    printf_log(LOG_DEBUG, "Calling fct_funcs_lcopy_value_" #A " wrapper\n");             \
    return (void*)RunFunction(my_context, fct_funcs_lcopy_value_##A, 4, value, n, collect, flags);    \
}
SUPER()
#undef GO
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GTypeValueTable_t* findFreeGTypeValueTable(my_GTypeValueTable_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_gtypevaluetable_##A == fcts) return &my_gtypevaluetable_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_gtypevaluetable_##A == 0) {    \
        ref_gtypevaluetable_##A = fcts;                 \
        my_gtypevaluetable_##A.value_init = (fcts->value_init)?((GetNativeFnc((uintptr_t)fcts->value_init))?GetNativeFnc((uintptr_t)fcts->value_init):my_funcs_value_init_##A):NULL;    \
        fct_funcs_value_init_##A = (uintptr_t)fcts->value_init;                             \
        my_gtypevaluetable_##A.value_free = (fcts->value_free)?((GetNativeFnc((uintptr_t)fcts->value_free))?GetNativeFnc((uintptr_t)fcts->value_free):my_funcs_value_free_##A):NULL;    \
        fct_funcs_value_free_##A = (uintptr_t)fcts->value_free;                             \
        my_gtypevaluetable_##A.value_copy = (fcts->value_copy)?((GetNativeFnc((uintptr_t)fcts->value_copy))?GetNativeFnc((uintptr_t)fcts->value_copy):my_funcs_value_copy_##A):NULL;    \
        fct_funcs_value_copy_##A = (uintptr_t)fcts->value_copy;                             \
        my_gtypevaluetable_##A.value_peek_pointer = (fcts->value_peek_pointer)?((GetNativeFnc((uintptr_t)fcts->value_peek_pointer))?GetNativeFnc((uintptr_t)fcts->value_peek_pointer):my_funcs_value_peek_pointer_##A):NULL;    \
        fct_funcs_value_peek_pointer_##A = (uintptr_t)fcts->value_peek_pointer;             \
        my_gtypevaluetable_##A.collect_format = fcts->collect_format;                       \
        my_gtypevaluetable_##A.collect_value = (fcts->collect_value)?((GetNativeFnc((uintptr_t)fcts->collect_value))?GetNativeFnc((uintptr_t)fcts->collect_value):my_funcs_collect_value_##A):NULL;    \
        fct_funcs_collect_value_##A = (uintptr_t)fcts->collect_value;                       \
        my_gtypevaluetable_##A.lcopy_format = fcts->lcopy_format;                           \
        my_gtypevaluetable_##A.lcopy_value = (fcts->lcopy_value)?((GetNativeFnc((uintptr_t)fcts->lcopy_value))?GetNativeFnc((uintptr_t)fcts->lcopy_value):my_funcs_lcopy_value_##A):NULL;    \
        fct_funcs_lcopy_value_##A = (uintptr_t)fcts->lcopy_value;                           \
        return &my_gtypevaluetable_##A;                 \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeValueTable callback\n");
    return NULL;
}


// ---- GTypeInfo ----

// First the structure my_GTypeInfo_t statics, with paired x86 source pointer
#define GO(A) \
static my_GTypeInfo_t     my_gtypeinfo_##A = {0};   \
static my_GTypeInfo_t    ref_gtypeinfo_##A = {0};   \
static int              used_gtypeinfo_##A = 0;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
#define GO(A)   \
static int fct_parent_##A = 0 ;                     \
static uintptr_t fct_funcs_base_init_##A = 0;       \
static int my_funcs_base_init_##A(void* g_class) {  \
    printf_log(LOG_DEBUG, "Calling fct_funcs_base_init_" #A " wrapper\n");             \
    return (int)RunFunction(my_context, fct_funcs_base_init_##A, 1, g_class);    \
}   \
static uintptr_t fct_funcs_base_finalize_##A = 0;   \
static int my_funcs_base_finalize_##A(void* g_class) {   \
    printf_log(LOG_DEBUG, "Calling fct_funcs_base_finalize_" #A " wrapper\n");             \
    return (int)RunFunction(my_context, fct_funcs_base_finalize_##A, 1, g_class);    \
}   \
static uintptr_t fct_funcs_class_init_##A = 0;                      \
static int my_funcs_class_init_##A(void* g_class, void* data) {     \
    printf_log(LOG_DEBUG, "Calling fct_funcs_class_init_" #A " wrapper\n");             \
    wrapGTKClass(g_class, fct_parent_##A);                          \
    int ret = (int)RunFunction(my_context, fct_funcs_class_init_##A, 2, g_class, data);    \
    unwrapGTKClass(g_class, fct_parent_##A);                        \
    return ret;                                                     \
}   \
static uintptr_t fct_funcs_class_finalize_##A = 0;  \
static int my_funcs_class_finalize_##A(void* g_class, void* data) { \
    printf_log(LOG_DEBUG, "Calling fct_funcs_class_finalize_" #A " wrapper\n");             \
    wrapGTKClass(g_class, fct_parent_##A);                          \
    int ret = (int)RunFunction(my_context, fct_funcs_class_finalize_##A, 2, g_class, data);    \
    unwrapGTKClass(g_class, fct_parent_##A);                        \
    return ret;                                                     \
}   \
static uintptr_t fct_funcs_instance_init_##A = 0;  \
static int my_funcs_instance_init_##A(void* instance, void* data) {   \
    printf_log(LOG_DEBUG, "Calling fct_funcs_instance_init_" #A " wrapper\n");             \
    return (int)RunFunction(my_context, fct_funcs_instance_init_##A, 2, instance, data);    \
}

SUPER()
#undef GO
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GTypeInfo_t* findFreeGTypeInfo(my_GTypeInfo_t* fcts, int parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtypeinfo_##A==0 && memcmp(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t))==0) return &my_gtypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtypeinfo_##A == 0) {                         \
        memcpy(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t));            \
        fct_parent_##A = parent;                                        \
        my_gtypeinfo_##A.class_size = fcts->class_size;                 \
        my_gtypeinfo_##A.base_init = (fcts->base_init)?((GetNativeFnc((uintptr_t)fcts->base_init))?GetNativeFnc((uintptr_t)fcts->base_init):my_funcs_base_init_##A):NULL;    \
        fct_funcs_base_init_##A = (uintptr_t)fcts->base_init;           \
        my_gtypeinfo_##A.base_finalize = (fcts->base_finalize)?((GetNativeFnc((uintptr_t)fcts->base_finalize))?GetNativeFnc((uintptr_t)fcts->base_finalize):my_funcs_base_finalize_##A):NULL;    \
        fct_funcs_base_finalize_##A = (uintptr_t)fcts->base_finalize;   \
        my_gtypeinfo_##A.class_init = (fcts->class_init)?((GetNativeFnc((uintptr_t)fcts->class_init))?GetNativeFnc((uintptr_t)fcts->class_init):my_funcs_class_init_##A):NULL;    \
        fct_funcs_class_init_##A = (uintptr_t)fcts->class_init;         \
        my_gtypeinfo_##A.class_finalize = (fcts->class_finalize)?((GetNativeFnc((uintptr_t)fcts->class_finalize))?GetNativeFnc((uintptr_t)fcts->class_finalize):my_funcs_class_finalize_##A):NULL;    \
        fct_funcs_class_finalize_##A = (uintptr_t)fcts->class_finalize; \
        my_gtypeinfo_##A.class_data = fcts->class_data;                 \
        my_gtypeinfo_##A.instance_size = fcts->instance_size;           \
        my_gtypeinfo_##A.n_preallocs = fcts->n_preallocs;               \
        my_gtypeinfo_##A.instance_init = (fcts->instance_init)?((GetNativeFnc((uintptr_t)fcts->instance_init))?GetNativeFnc((uintptr_t)fcts->instance_init):my_funcs_instance_init_##A):NULL;    \
        fct_funcs_instance_init_##A = (uintptr_t)fcts->instance_init;   \
        my_gtypeinfo_##A.value_table = findFreeGTypeValueTable(fcts->value_table);           \
        return &my_gtypeinfo_##A;                                       \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo callback\n");
    return NULL;
}

// ---- GtkTypeInfo ----

// First the structure my_GtkTypeInfo_t statics, with paired x86 source pointer
#define GO(A) \
static my_GtkTypeInfo_t     my_gtktypeinfo_##A = {0};   \
static my_GtkTypeInfo_t    ref_gtktypeinfo_##A = {0};  \
static int                used_gtktypeinfo_##A = 0;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
#define GO(A)   \
static int fct_gtk_parent_##A = 0 ;                     \
static uintptr_t fct_gtk_class_init_##A = 0;       \
static int my_gtk_class_init_##A(void* g_class) {  \
    printf_log(LOG_DEBUG, "Calling fct_gtk_class_init_" #A " wrapper\n");             \
    return (int)RunFunction(my_context, fct_gtk_class_init_##A, 1, g_class);    \
}   \
static uintptr_t fct_gtk_object_init_##A = 0;  \
static int my_gtk_object_init_##A(void* object, void* data) {   \
    printf_log(LOG_DEBUG, "Calling fct_gtk_object_init_" #A " wrapper\n");             \
    return (int)RunFunction(my_context, fct_gtk_object_init_##A, 2, object, data);    \
}   \
static uintptr_t fct_gtk_base_class_init_##A = 0;  \
static int my_gtk_base_class_init_##A(void* instance, void* data) {   \
    printf_log(LOG_DEBUG, "Calling fct_gtk_base_class_init_" #A " wrapper\n");             \
    return (int)RunFunction(my_context, fct_gtk_base_class_init_##A, 2, instance, data);    \
}

SUPER()
#undef GO
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GtkTypeInfo_t* findFreeGtkTypeInfo(my_GtkTypeInfo_t* fcts, int parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtktypeinfo_##A && memcmp(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t))==0) return &my_gtktypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtktypeinfo_##A == 0) {          \
        memcpy(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t));        \
        fct_gtk_parent_##A = parent;                        \
        my_gtktypeinfo_##A.type_name = fcts->type_name; \
        my_gtktypeinfo_##A.object_size = fcts->object_size; \
        my_gtktypeinfo_##A.class_size = fcts->class_size; \
        my_gtktypeinfo_##A.class_init_func = (fcts->class_init_func)?((GetNativeFnc((uintptr_t)fcts->class_init_func))?GetNativeFnc((uintptr_t)fcts->class_init_func):my_gtk_class_init_##A):NULL;    \
        fct_gtk_class_init_##A = (uintptr_t)fcts->class_init_func;           \
        my_gtktypeinfo_##A.object_init_func = (fcts->object_init_func)?((GetNativeFnc((uintptr_t)fcts->object_init_func))?GetNativeFnc((uintptr_t)fcts->object_init_func):my_gtk_object_init_##A):NULL;    \
        fct_gtk_object_init_##A = (uintptr_t)fcts->object_init_func;         \
        my_gtktypeinfo_##A.reserved_1 = fcts->reserved_1;                 \
        my_gtktypeinfo_##A.reserved_2 = fcts->reserved_2;                 \
        my_gtktypeinfo_##A.base_class_init_func = (fcts->base_class_init_func)?((GetNativeFnc((uintptr_t)fcts->base_class_init_func))?GetNativeFnc((uintptr_t)fcts->base_class_init_func):my_gtk_base_class_init_##A):NULL;    \
        fct_gtk_base_class_init_##A = (uintptr_t)fcts->base_class_init_func;   \
        return &my_gtktypeinfo_##A;                       \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GtkTypeInfo callback\n");
    return NULL;
}

#undef SUPER

void InitGTKClass(bridge_t *bridge)
{
    my_bridge  = bridge;
}

void FiniGTKClass()
{
    if(table_ref)
        free(table_ref);
    for (int i=0; i<table_sz; ++i)
        free(table_nat[i]);
    if(table_nat)
        free(table_nat);
    table_ref = table_nat = NULL;
    table_sz = table_cap = 0;
}

void SetGObjectID(int id)
{
    my_gobject = id;
}

void SetGTKObjectID(int id)
{
    my_gtkobject = id;
}

void SetGTKWidgetID(int id)
{
    my_gtkwidget = id;
}

void SetGTKContainerID(int id)
{
    my_gtkcontainer = id;
}

void SetGTypeName(void* f)
{
    g_type_name = f;
}

// workaround for Globals symbols

EXPORT void* gdk_display;   // in case it's used...
    
void my_checkGlobalGdkDisplay()
{
    // workaround, because gdk_display maybe declared as global in the calling program, but there is no way to send this info to the linker
    uintptr_t globoffs, globend;
    if (GetGlobalNoWeakSymbolStartEnd(my_context->maplib, "gdk_display", &globoffs, &globend)) {
        printf_log(LOG_DEBUG, "Global gdk_display workaround, @%p <= %p\n", (void*)globoffs, gdk_display);
        memcpy((void*)globoffs, &gdk_display, sizeof(gdk_display));
    }
}

void my_setGlobalGThreadsInit()
{
    // workaround, because gdk_display maybe declared as global in the calling program, but there is no way to send this info to the linker
    int val = 1;
    uintptr_t globoffs, globend;
    if (GetGlobalNoWeakSymbolStartEnd(my_context->maplib, "g_threads_got_initialized", &globoffs, &globend)) {
        printf_log(LOG_DEBUG, "Global g_threads_got_initialized workaround, @%p <= %p\n", (void*)globoffs, (void*)val);
        memcpy((void*)globoffs, &val, sizeof(gdk_display));
    }
}

my_signal_t* new_mysignal(void* f, void* data, void* destroy)
{
    my_signal_t* sig = (my_signal_t*)calloc(1, sizeof(my_signal_t));
    sig->sign = SIGN;
    sig->c_handler = (uintptr_t)f;
    sig->destroy = (uintptr_t)destroy;
    sig->data = data;
    return sig;
}
void my_signal_delete(my_signal_t* sig)
{
    uintptr_t d = sig->destroy;
    if(d) {
        RunFunction(my_context, d, 1, sig->data);
    }
    printf_log(LOG_DEBUG, "gtk Data deleted, sig=%p, data=%p, destroy=%p\n", sig, sig->data, (void*)d);
    free(sig);
}

int my_signal_cb(void* a, void* b, void* c, void* d)
{
    // signal can have many signature... so first job is to find the data!
    // hopefully, no callback have more than 4 arguments...
    my_signal_t* sig = NULL;
    int i = 0;
    if(a)
        if(((my_signal_t*)a)->sign == SIGN) {
            sig = (my_signal_t*)a;
            i = 1;
        }
    if(!sig && b)
        if(((my_signal_t*)b)->sign == SIGN) {
            sig = (my_signal_t*)b;
            i = 2;
        }
    if(!sig && c)
        if(((my_signal_t*)c)->sign == SIGN) {
            sig = (my_signal_t*)c;
            i = 3;
        }
    if(!sig && d)
        if(((my_signal_t*)d)->sign == SIGN) {
            sig = (my_signal_t*)d;
            i = 4;
        }
    printf_log(LOG_DEBUG, "gtk Signal called, sig=%p, NArgs=%d\n", sig, i);
    switch(i) {
        case 1: return (int)RunFunction(my_context, sig->c_handler, 1, sig->data);
        case 2: return (int)RunFunction(my_context, sig->c_handler, 2, a, sig->data);
        case 3: return (int)RunFunction(my_context, sig->c_handler, 3, a, b, sig->data);
        case 4: return (int)RunFunction(my_context, sig->c_handler, 4, a, b, c, sig->data);
    }
    printf_log(LOG_NONE, "Warning, Gtk signal callback but no data found!");
    return 0;
}
