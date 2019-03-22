#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "librarian.h"

int my_snd_async_add_handler(x86emu_t *emu, void *handler, int fd, void* callback, void *private_data) EXPORT;
int my_snd_async_add_pcm_handler(x86emu_t *emu, void *handler, void *pcm,  void* callback, void *private_data) EXPORT;
void* my_snd_async_handler_get_callback_private(x86emu_t *emu, void *handler) EXPORT;
int my_snd_lib_error_set_handler(x86emu_t *emu, void* handler) EXPORT;

#define LIBNAME libasound
const char* libasoundName = "libasound.so.2";

// define all standard library functions
#include "wrappedlib_init.h"

typedef int (*iFp_t)(void*);

// Implementation
//typedef void(* 	snd_async_callback_t )(snd_async_handler_t *handler)
/*
typedef void (* snd_lib_error_handler_t)(
    const char *file,
    int line,
    const char *function,
    int err,
    const char *fmt,
    ...
    )
*/
// error handler is a function pointer with a ... !
int my_snd_async_add_handler(x86emu_t *emu, void *handler, int fd, void* callback, void *private_data)
{
    //snd_async_add_handler(snd_async_handler_t **handler, int fd, snd_async_callback_t callback, void *private_data);
    printf_log(LOG_NONE, "Error: snd_async_add_handler not implemented\n");
    emu->quit = 1;
    return -1;
}

int my_snd_async_add_pcm_handler(x86emu_t *emu, void *handler, void* pcm,  void* callback, void *private_data)
{
    //snd_async_add_pcm_handler(snd_async_handler_t **handler, snd_pcm_t *pcm,  snd_async_callback_t callback, void *private_data)
    printf_log(LOG_NONE, "Error: snd_async_add_pcm_handler not implemented\n");
    emu->quit = 1;
    return -1;

}

void* my_snd_async_handler_get_callback_private(x86emu_t* emu, void* handler)
{
    //snd_async_handler_get_callback_private(snd_async_handler_t *handler)
    printf_log(LOG_NONE, "Error: snd_async_handler_get_callback_private not implemented\n");
    emu->quit = 1;
    return NULL;
}

static void dummy_error_handler(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
    va_list ap;

    fprintf(ftrace, "Warning: this is a dummy snd_lib error handler\n");
    fprintf(ftrace, "Error in file %s on line %i: ", file, line);
    va_start(ap, fmt);
    vfprintf(ftrace, fmt, ap);
    va_end(ap);
    fflush(ftrace);
}

static void empty_error_handler(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
    // do nothing
}

int my_snd_lib_error_set_handler(x86emu_t* emu, void* handler)
{
    library_t* lib = GetLib(emu->context->maplib, libasoundName);
    if(!lib) return -1;
    void* f = dlsym(lib->priv.w.lib, "snd_lib_error_set_handler");
    if(f) {
        void *error_handler;
        uint8_t *code = (uint8_t *)handler;
        if (code) {
            if (code && ((code[0] == 0xC3) || ((code[0] == 0xF3) && (code[1] == 0xC3)))) {
                error_handler = &empty_error_handler;
            } else {
                error_handler = &dummy_error_handler;
                printf_log(LOG_NONE, "Warning: snd_lib_error_set_handler: using dummy error handler\n");
            }
        } else error_handler = NULL;

        return ((iFp_t)f)(error_handler);
    }
    return -1;
}
