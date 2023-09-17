#ifndef __GL_TOOLS_H__
#define __GL_TOOLS_H__

typedef struct box86context_s box86context_t;

typedef void* (*glprocaddress_t)(const char* name);

void freeGLProcWrapper(box86context_t* context);

void* getGLProcAddress(x86emu_t* emu, glprocaddress_t procaddr, const char* rname);

#endif //__GL_TOOLS_H__