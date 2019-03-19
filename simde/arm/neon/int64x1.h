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
#  error Do not include simde/arm/neon/int64x1.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_INT64X1_H)
#define SIMDE__NEON_INT64X1_H

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  int64_t         i64 __attribute__((__vector_size__(8)));
#else
  int64_t         i64[1];
#endif

#if defined(SIMDE_NEON_NATIVE)
  int64x1_t       n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx;
#endif
} simde_int64x1_t;

#if defined(SIMDE_NEON_NATIVE)
HEDLEY_STATIC_ASSERT(sizeof(int64x1_t) == sizeof(simde_int64x1_t), "int64x1_t size doesn't match simde_int64x1_t size");
#endif
HEDLEY_STATIC_ASSERT(8 == sizeof(simde_int64x1_t), "simde_int64x1_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vadd_s64(simde_int64x1_t a, simde_int64x1_t b) {
  simde_int64x1_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vadd_s64(a.n, b.n);
#elif defined(SIMDE_MMX_NATIVE)
  r.mmx = _mm_add_pi64(a.mmx, b.mmx);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i64[i] + b.i64[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vld1_s64 (int64_t const ptr[1]) {
  simde_int64x1_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vld1_s64(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_x_vload_s64 (int64_t l0) {
  return simde_vld1_s64(&l0);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vdup_n_s64 (int64_t value) {
  simde_int64x1_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vdup_n_s64(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_x_vmul_s64(simde_int64x1_t a, simde_int64x1_t b) {
  simde_int64x1_t r;
#if defined(SIMDE_MMX_NATIVE)
  r.mmx = _mm_mul_pi64(a.mmx, b.mmx);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i64[i] * b.i64[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vsub_s64(simde_int64x1_t a, simde_int64x1_t b) {
  simde_int64x1_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vsub_s64(a.n, b.n);
#elif defined(SIMDE_MMX_NATIVE)
  r.mmx = _mm_sub_pi64(a.mmx, b.mmx);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i64[i] - b.i64[i];
  }
#endif
  return r;
}

#endif
