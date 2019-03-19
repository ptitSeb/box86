/* Copyright (c) 2018-2019 Evan Nemerson <evan@nemerson.com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#if !defined(SIMDE__INSIDE_NEON_H)
#  error Do not include simde/arm/neon/int8x16.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_INT8X16_H)
#define SIMDE__NEON_INT8X16_H

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  int8_t          i8 __attribute__((__vector_size__(16)));
#else
  int8_t          i8[16];
#endif

#if defined(SIMDE_NEON_NATIVE)
  int8x16_t        n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx[2];
#endif
#if defined(SIMDE_NEON_SSE2)
  __m128i         sse;
#endif
} simde_int8x16_t;

#if defined(SIMDE_NEON_NATIVE)
HEDLEY_STATIC_ASSERT(sizeof(int8x16_t) == sizeof(simde_int8x16_t), "int8x16_t size doesn't match simde_int8x16_t size");
#endif
HEDLEY_STATIC_ASSERT(16 == sizeof(simde_int8x16_t), "simde_int8x16_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vaddq_s8(simde_int8x16_t a, simde_int8x16_t b) {
  simde_int8x16_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vaddq_s8(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.sse = _mm_add_epi8(a.sse, b.sse);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a.i8[i] + b.i8[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vld1q_s8 (int8_t const ptr[8]) {
  simde_int8x16_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vld1q_s8(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_x_vloadq_s8 (int8_t  l0, int8_t  l1, int8_t  l2, int8_t  l3,
		   int8_t  l4, int8_t  l5, int8_t  l6, int8_t  l7,
		   int8_t  l8, int8_t  l9, int8_t l10, int8_t l11,
		   int8_t l12, int8_t l13, int8_t l14, int8_t l15) {
  int8_t v[] = { l0, l1,  l2,  l3,  l4,  l5,  l6,  l7,
		 l8, l9, l10, l11, l12, l13, l14, l15};
  return simde_vld1q_s8(v);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vdupq_n_s8 (int8_t value) {
  simde_int8x16_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vdupq_n_s8(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vmulq_s8(simde_int8x16_t a, simde_int8x16_t b) {
  simde_int8x16_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vmulq_s8(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.sse = _mm_mul_epi8(a.sse, b.sse);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a.i8[i] * b.i8[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vsubq_s8(simde_int8x16_t a, simde_int8x16_t b) {
  simde_int8x16_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vsubq_s8(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.sse = _mm_sub_epi8(a.sse, b.sse);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a.i8[i] - b.i8[i];
  }
#endif
  return r;
}

#endif
