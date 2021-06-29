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
#endif

int main(int argc, const char** argv)
{
 float a, b;
 uint64_t flags;
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

 return 0;
}
