#include <stdio.h>

int main(int argc, char **argv)
{
    printf("Hello, argc=%d, argv[0]=%s, argv[%d]=%s\n", argc, argv[0], argc-1, argv[argc-1]);
    return 0;
}
