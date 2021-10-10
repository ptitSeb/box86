#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#if defined(__x86_64__)
uint64_t _ucomiss_(float a, float b)
{
    uint64_t ret;
    asm volatile (
    "ucomiss %%xmm0, %%xmm1\n"
    "pushf\n"
    "pop %%rax"
    :"=a" (ret)::"xmm0","xmm1","cc");
    return ret;
}
uint64_t _minss_(float a, float b)
{
    uint64_t ret;
    asm volatile (
    "minss %%xmm1, %%xmm0\n"
    "movd %%xmm0, %%eax"
    :"=a" (ret)::"xmm0","xmm1","cc");
    return ret;
}
uint64_t _maxss_(float a, float b)
{
    uint64_t ret;
    asm volatile (
    "maxss %%xmm1, %%xmm0\n"
    "movd %%xmm0, %%eax"
    :"=a" (ret)::"xmm0","xmm1","cc");
    return ret;
}
#else
uint64_t _ucomiss_(float a, float b)
{
    uint32_t ret;
    asm volatile (
    "movss %1, %%xmm0\n"
    "movss %2, %%xmm1\n"
    "ucomiss %%xmm0, %%xmm1\n"
    "pushf\n"
    "pop %%eax"
    :"=a" (ret):"m"(a), "m"(b):"xmm0", "xmm1", "cc");
    return ret;
}
uint64_t _minss_(float a, float b)
{
    uint32_t ret;
    asm volatile (
    "movss %1, %%xmm0\n"
    "movss %2, %%xmm1\n"
    "minss %%xmm1, %%xmm0\n"
    "movd %%xmm0, %%eax"
    :"=a" (ret):"m"(a), "m"(b):"xmm0", "xmm1", "cc");
    return ret;
}
uint64_t _maxss_(float a, float b)
{
    uint32_t ret;
    asm volatile (
    "movss %1, %%xmm0\n"
    "movss %2, %%xmm1\n"
    "maxss %%xmm1, %%xmm0\n"
    "movd %%xmm0, %%eax"
    :"=a" (ret):"m"(a), "m"(b):"xmm0", "xmm1", "cc");
    return ret;
}
#endif

int main(int argc, const char** argv)
{
 float a, b;
 uint64_t flags;
 uint32_t maxf = 0x7f7fffff;
 uint32_t minf = 0xff7fffff;
 uint32_t r;
 a = 1.0f; b = 2.0f;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 flags = _ucomiss_(b, a);
 printf("ucomiss %f, %f => 0x%lx\n", b, a, flags);
 b = INFINITY;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 flags = _ucomiss_(b, a);
 printf("ucomiss %f, %f => 0x%lx\n", b, a, flags);
 b = -INFINITY;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 flags = _ucomiss_(b, a);
 printf("ucomiss %f, %f => 0x%lx\n", b, a, flags);
 b = NAN;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 flags = _ucomiss_(b, a);
 printf("ucomiss %f, %f => 0x%lx\n", b, a, flags);
 b = a;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 a = b = INFINITY;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 a = b = NAN;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);

 a = 1.0f; b = 2.0f;
 r = _minss_(a, b);
 printf("minss %g, %g => %g\n", a, b, *(float*)&r);
 r = _minss_(b, a);
 printf("minss %g, %g => %g\n", b, a, *(float*)&r);
 a = -INFINITY;
 r = _minss_(a, b);
 printf("minss %g, %g => %g\n", a, b, *(float*)&r);
 r = _minss_(b, a);
 printf("minss %g, %g => %g\n", b, a, *(float*)&r);
 a = +INFINITY;
 r = _minss_(a, b);
 printf("minss %g, %g => %g\n", a, b, *(float*)&r);
 r = _minss_(b, a);
 printf("minss %g, %g => %g\n", b, a, *(float*)&r);
 a = NAN;
 r = _minss_(a, b);
 printf("minss %g, %g => %g\n", a, b, *(float*)&r);
 r = _minss_(b, a);
 printf("minss %g, %g => %g\n", b, a, *(float*)&r);
 b = *(float*)&maxf;
 r = _minss_(a, b);
 printf("minss %g, %g => %g\n", a, b, *(float*)&r);
 r = _minss_(b, a);
 printf("minss %g, %g => %g\n", b, a, *(float*)&r);
 a = -INFINITY;
 r = _minss_(a, b);
 printf("minss %g, %g => %g\n", a, b, *(float*)&r);
 r = _minss_(b, a);
 printf("minss %g, %g => %g\n", b, a, *(float*)&r);
 a = +INFINITY;
 r = _minss_(a, b);
 printf("minss %g, %g => %g\n", a, b, *(float*)&r);
 r = _minss_(b, a);
 printf("minss %g, %g => %g\n", b, a, *(float*)&r);

 a = 1.0f; b = 2.0f;
 r = _maxss_(a, b);
 printf("maxss %g, %g => %g\n", a, b, *(float*)&r);
 r = _maxss_(b, a);
 printf("maxss %g, %g => %g\n", b, a, *(float*)&r);
 a = -INFINITY;
 r = _maxss_(a, b);
 printf("maxss %g, %g => %g\n", a, b, *(float*)&r);
 r = _maxss_(b, a);
 printf("maxss %g, %g => %g\n", b, a, *(float*)&r);
 a = +INFINITY;
 r = _maxss_(a, b);
 printf("maxss %g, %g => %g\n", a, b, *(float*)&r);
 r = _maxss_(b, a);
 printf("maxss %g, %g => %g\n", b, a, *(float*)&r);
 a = NAN;
 r = _maxss_(a, b);
 printf("maxss %g, %g => %g\n", a, b, *(float*)&r);
 r = _maxss_(b, a);
 printf("maxss %g, %g => %g\n", b, a, *(float*)&r);
 b = *(float*)&minf;
 r = _maxss_(a, b);
 printf("maxss %g, %g => %g\n", a, b, *(float*)&r);
 r = _maxss_(b, a);
 printf("maxss %g, %g => %g\n", b, a, *(float*)&r);
 a = -INFINITY;
 r = _maxss_(a, b);
 printf("maxss %g, %g => %g\n", a, b, *(float*)&r);
 r = _maxss_(b, a);
 printf("maxss %g, %g => %g\n", b, a, *(float*)&r);
 a = +INFINITY;
 r = _maxss_(a, b);
 printf("maxss %g, %g => %g\n", a, b, *(float*)&r);
 r = _maxss_(b, a);
 printf("maxss %g, %g => %g\n", b, a, *(float*)&r);
 
 return 0;
}
