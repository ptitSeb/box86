#include <stdio.h>
#include <stdint.h>
#include <math.h>

typedef uint32_t    uint32;
typedef uint32      angle_t;

int main(int argc, char **argv)
{
    int64_t i64 = 1000000000000;
    double d = i64;
    printf("%lli => %f\n", i64, d);
    i64 = -i64;
    d = i64;
    printf("%lli => %f\n", i64, d);

    d = M_PI/4.0;
    d = d*(1<<30)/M_PI;
    angle_t u32 = (angle_t)d;
    printf("(angle_t)%f = %u == 0x%08X\n", d, u32, u32);
    /*d = -d;
    u32 = angle_t(d);
    printf("(angle_t)%f = %u == 0x%08X\n", d, u32, u32);*/

    return 0;
}
