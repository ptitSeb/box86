#ifndef __X86TRACE_H_
#define __X86TRACE_H_
#include <stdint.h>

typedef struct box86context_s box86context_t;
typedef struct zydis_dec_s zydis_dec_t;

int InitX86Trace(box86context_t *context);
void DeleteX86Trace(box86context_t *context);

zydis_dec_t* InitX86TraceDecoder(box86context_t *context);
void DeleteX86TraceDecoder(zydis_dec_t **dec);
const char* DecodeX86Trace(zydis_dec_t *dec, uint32_t p);

#define ZYDIS_RUNTIME_ADDRESS_NONE (uint64_t)(-1)

#endif //__X86TRACE_H_