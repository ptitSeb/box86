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
#  error Do not include simde/arm/neon/int16x8.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_INT16X8_H)
#define SIMDE__NEON_INT16X8_H

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  int16_t         i16 __attribute__((__vector_size__(16)));
#else
  int16_t         i16[8];
#endif

#if defined(SIMDE_NEON_NATIVE)
  int16x8_t       n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx[2];
#endif
#if defined(SIMDE_NEON_SSE2)
  __m128i         sse;
#endif
} simde_int16x8_t;

#if defined(SIMDE_NEON_NATIVE)
HEDLEY_STATIC_ASSERT(sizeof(int16x8_t) == sizeof(simde_int16x8_t), "int16x8_t size doesn't match simde_int16x8_t size");
#endif
HEDLEY_STATIC_ASSERT(16 == sizeof(simde_int16x8_t), "simde_int16x8_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vaddq_s16(simde_int16x8_t a, simde_int16x8_t b) {
  simde_int16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vaddq_s16(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.sse = _mm_add_epi16(a.sse, b.sse);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = a.i16[i] + b.i16[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vld1q_s16 (int16_t const ptr[8]) {
  simde_int16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vld1q_s16(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_x_vloadq_s16 (int16_t l0, int16_t l1, int16_t l2, int16_t l3,
		    int16_t l4, int16_t l5, int16_t l6, int16_t l7) {
  int16_t v[] = { l0, l1, l2, l3,
		  l4, l5, l6, l7 };
  return simde_vld1q_s16(v);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vdupq_n_s16 (int16_t value) {
  simde_int16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vdupq_n_s16(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vmulq_s16(simde_int16x8_t a, simde_int16x8_t b) {
  simde_int16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vmulq_s16(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.sse = _mm_mul_epi16(a.sse, b.sse);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = a.i16[i] * b.i16[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vsubq_s16(simde_int16x8_t a, simde_int16x8_t b) {
  simde_int16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vsubq_s16(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.sse = _mm_sub_epi16(a.sse, b.sse);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = a.i16[i] - b.i16[i];
  }
#endif
  return r;
}

#endif
