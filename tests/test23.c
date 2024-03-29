#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
// Build with `gcc -march=core2 -O2 -m32 test23.c -o test23`


uint64_t a = 0x12345678abcdefed;
uint32_t b = 0x12345678;
uint16_t c = 0x1234;

int main()
{
    uint32_t ret2;
    uint16_t ret3;

    asm volatile(
        "movbe %1, %0\n"
        : "=r"(ret2)
        : "m"(b)
        : "memory");
    printf("ret = 0x%x\n", ret2);

    asm volatile(
        "movbe %1, %0\n"
        : "=r"(ret3)
        : "m"(c)
        : "memory");
    printf("ret = 0x%x\n", ret3);

    asm volatile(
        "movbe %1, %0\n"
        : "=m"(ret2)
        : "r"(b)
        : "memory");
    printf("ret = 0x%x\n", ret2);

    asm volatile(
        "movbe %1, %0\n"
        : "=m"(ret3)
        : "r"(c)
        : "memory");
    printf("ret = 0x%x\n", ret3);

    asm volatile(
        "bswap %0\n"
        : "+r"(ret2)
        :
        :);
    printf("ret = 0x%x\n", ret2);
    return 0;
}
