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
#  error Do not include simde/arm/neon/uint64x2.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_UINT64X2_H)
#define SIMDE__NEON_UINT64X2_H

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  uint64_t         u64 __attribute__((__vector_size__(16)));
#else
  uint64_t         u64[2];
#endif

#if defined(SIMDE_NEON_NATIVE)
  uint64x2_t       n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx[2];
#endif
#if defined(SIMDE_NEON_SSE2)
  __m128i         sse;
#endif
} simde_uint64x2_t;

#if defined(SIMDE_NEON_NATIVE)
HEDLEY_STATIC_ASSERT(sizeof(uint64x2_t) == sizeof(simde_uint64x2_t), "uint64x2_t size doesn't match simde_uint64x2_t size");
#endif
HEDLEY_STATIC_ASSERT(16 == sizeof(simde_uint64x2_t), "simde_uint64x2_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vaddq_u64(simde_uint64x2_t a, simde_uint64x2_t b) {
  simde_uint64x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vaddq_u64(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.u64[i] = a.u64[i] + b.u64[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vld1q_u64 (uint64_t const ptr[2]) {
  simde_uint64x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vld1q_u64(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.u64[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_x_vloadq_u64 (uint64_t l0, uint64_t l1) {
  uint64_t v[] = { l0, l1 };
  return simde_vld1q_u64(v);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vdupq_n_u64 (uint64_t value) {
  simde_uint64x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vdupq_n_u64(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.u64[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_x_vmulq_u64(simde_uint64x2_t a, simde_uint64x2_t b) {
  simde_uint64x2_t r;

  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.u64[i] = a.u64[i] * b.u64[i];
  }

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vsubq_u64(simde_uint64x2_t a, simde_uint64x2_t b) {
  simde_uint64x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vsubq_u64(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.u64[i] = a.u64[i] - b.u64[i];
  }
#endif
  return r;
}

#endif
