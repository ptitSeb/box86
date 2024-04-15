#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <complex.h>
#include <math.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "emu/x87emu_setround.h"
#include "debug.h"

const char* libmName =
#ifdef ANDROID
    "libm.so"
#else
    "libm.so.6"
#endif
    ;
#define LIBNAME libm

static library_t* my_lib = NULL;

EXPORT int my_fesetround (x86emu_t* emu, int __rounding_direction) {
    if ((~0xc00 & (unsigned int)__rounding_direction) != 0) // invalid rounding
        return 1;
    emu->cw.f.C87_RD = __rounding_direction >> 10;
    return 0;
}

EXPORT int my_fegetround (x86emu_t* emu) {
    return emu->cw.f.C87_RD << 10;
}


typedef float   (*fFff_t)   (float, float);
typedef double  (*dFdd_t)   (double, double);
typedef float   (*fFf_t)    (float);
typedef double  (*dFd_t)    (double);

typedef union my_float_complex_s {
    float complex   f;
    uint64_t        u64;
} my_float_complex_t;

#define set_round() \
    int oldround = fpu_setround(emu)
#define restore_round() \
    fesetround(oldround)

// complex <- FUNC(complex) wrapper
#define GO_cFc(N)                                   \
EXPORT void* my_##N(void* p, void* c)               \
{                                                   \
    *(double complex*)p = N(*(double complex*)c);   \
    return p;                                       \
}                                                   \
EXPORT uint64_t my_##N##f(void* c)                  \
{                                                   \
    my_float_complex_t ret;                         \
    ret.f = N##f(*(float complex*)c);               \
    return ret.u64;                                 \
}
// complex <- FUNC(complex, complex) wrapper
#define GO_cFcc(N)                                  \
EXPORT void* my_##N(void* p, void* c, void* d)      \
{                                                   \
    *(double complex*)p = N(*(double complex*)c, *(double complex*)d); \
    return p;                                       \
}                                                   \
EXPORT uint64_t my_##N##f(void* c, void* d)         \
{                                                   \
    my_float_complex_t ret;                         \
    ret.f = N##f(*(float complex*)c, *(float complex*)d); \
    return ret.u64;                                 \
}

#ifdef TERMUX
double complex clog(double complex);
float complex clogf(float complex);
double complex cpow(double complex, double complex);
float complex cpowf(float complex, float complex);
#endif

GO_cFc(clog)
GO_cFcc(cpow)
GO_cFc(csqrt)
GO_cFc(cproj)
GO_cFc(cexp)
GO_cFc(ccos)
GO_cFc(csin)
GO_cFc(ccosh)
GO_cFc(csinh)
GO_cFc(ctan)
GO_cFc(ctanh)
GO_cFc(cacos)
GO_cFc(casin)
GO_cFc(cacosh)
GO_cFc(casinh)
GO_cFc(catan)
GO_cFc(catanh)

#undef GO_cFc
#undef GO_cFcc

#define FINITE(N, T, R, P, ...)     \
EXPORT R my___##N##_finite P        \
{                                   \
    static int check = 0;           \
    static T f = NULL;              \
    if(!check) {                    \
        f = (T)dlsym(my_lib->w.lib, "__" #N "_finite");  \
        ++check;                    \
    }                               \
    if(f)                           \
        return f(__VA_ARGS__);      \
    else                            \
        return N(__VA_ARGS__);      \
}

#define F1F(N) FINITE(N, fFf_t, float, (float a), a)
#define F1D(N) FINITE(N, dFd_t, double, (double a), a)
#define F2F(N) FINITE(N, fFff_t, float, (float a, float b), a, b)
#define F2D(N) FINITE(N, dFdd_t, double, (double a, double b), a, b)

F2F(powf)
F2D(pow)
F1F(sinhf)
F1D(sinh)
F1F(sqrtf)
F1D(sqrt)
F1F(acosf)
F1D(acos)
F1F(acoshf)
F1D(acosh)
F1F(asinf)
F1D(asin)
F2F(atan2f)
F2D(atan2)
F1F(coshf)
F1D(cosh)
F1F(exp2f)
F1D(exp2)
F1F(expf)
F1D(exp)
F2F(hypotf)
F2D(hypot)
F1F(log10f)
F1D(log10)
F1F(log2f)
F1D(log2)
F1F(logf)
F1D(log)

#undef F2D
#undef F2F
#undef F1D
#undef F1F
#undef FINITE

#define WITH_ROUND(N, R, P, ...)    \
EXPORT R my_##N P                   \
{                                   \
    set_round();                    \
    R ret = N(__VA_ARGS__);         \
    restore_round();                \
    return ret;                     \
}

#ifdef HAVE_LD80BITS
#define RFr(N, r) \
    WITH_ROUND(N ## f, r, (x86emu_t* emu, float a), a) \
    WITH_ROUND(N, r, (x86emu_t* emu, double a), a) \
    WITH_ROUND(N ## l, r, (x86emu_t* emu, long double a), a)
#define RrFr(N) \
    WITH_ROUND(N ## f, float, (x86emu_t* emu, float a), a) \
    WITH_ROUND(N, double, (x86emu_t* emu, double a), a) \
    WITH_ROUND(N ## l, double, (x86emu_t* emu, long double a), a)
#define RrFrr(N) \
    WITH_ROUND(N ## f, float, (x86emu_t* emu, float a, float b), a, b) \
    WITH_ROUND(N, double, (x86emu_t* emu, double a, double b), a, b) \
    WITH_ROUND(N ## l, double, (x86emu_t* emu, long double a, long double b), a, b)
#else
#define RFr(N, r) \
    WITH_ROUND(N ## f, r, (x86emu_t* emu, float a), a) \
    WITH_ROUND(N, r, (x86emu_t* emu, double a), a)
#define RrFr(N) \
    WITH_ROUND(N ## f, float, (x86emu_t* emu, float a), a) \
    WITH_ROUND(N, double, (x86emu_t* emu, double a), a)
#define RrFrr(N) \
    WITH_ROUND(N ## f, float, (x86emu_t* emu, float a, float b), a, b) \
    WITH_ROUND(N, double, (x86emu_t* emu, double a, double b), a, b)
#endif

RrFr(nearbyint)
RrFr(rint)
RFr(llrint, long long)
RFr(lrint, int)

#undef RFr
#undef RrFr
#undef RrFrr
#undef WITH_ROUND

#define PRE_INIT\
    if(1)                                                           \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else

#define CUSTOM_INIT     \
    my_lib = lib;

#define CUSTOM_FINI     \
    my_lib = NULL;

#include "wrappedlib_init.h"
