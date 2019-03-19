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
#  error Do not include simde/arm/neon/int64x2.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_INT64X2_H)
#define SIMDE__NEON_INT64X2_H

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  int64_t         i64 __attribute__((__vector_size__(16)));
#else
  int64_t         i64[2];
#endif

#if defined(SIMDE_NEON_NATIVE)
  int64x2_t       n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx[2];
#endif
#if defined(SIMDE_NEON_SSE2)
  __m128i         sse;
#endif
} simde_int64x2_t;

#if defined(SIMDE_NEON_NATIVE)
HEDLEY_STATIC_ASSERT(sizeof(int64x2_t) == sizeof(simde_int64x2_t), "int64x2_t size doesn't match simde_int64x2_t size");
#endif
HEDLEY_STATIC_ASSERT(16 == sizeof(simde_int64x2_t), "simde_int64x2_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vaddq_s64(simde_int64x2_t a, simde_int64x2_t b) {
  simde_int64x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vaddq_s64(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.sse = _mm_add_epi64(a.sse, b.sse);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i64[i] + b.i64[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vld1q_s64 (int64_t const ptr[2]) {
  simde_int64x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vld1q_s64(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_x_vloadq_s64 (int64_t l0, int64_t l1) {
  int64_t v[] = { l0, l1 };
  return simde_vld1q_s64(v);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vdupq_n_s64 (int64_t value) {
  simde_int64x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vdupq_n_s64(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_x_vmulq_s64(simde_int64x2_t a, simde_int64x2_t b) {
  simde_int64x2_t r;
#if defined(SIMDE_SSE2_NATIVE)
  r.sse = _mm_mul_epi64(a.sse, b.sse);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i64[i] * b.i64[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vsubq_s64(simde_int64x2_t a, simde_int64x2_t b) {
  simde_int64x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vsubq_s64(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.sse = _mm_sub_epi64(a.sse, b.sse);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i64[i] - b.i64[i];
  }
#endif
  return r;
}

#endif
