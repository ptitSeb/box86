#ifndef __AUXVAL_H__
#define __AUXVAL_H__

typedef struct x86emu_s x86emu_t;

#ifndef BUILD_LIB
int init_auxval(int argc, const char **argv, char **env);
#endif

unsigned long real_getauxval(unsigned long type);
unsigned long my_getauxval(x86emu_t* emu, unsigned long type);

#endif //__AUXVAL_H__