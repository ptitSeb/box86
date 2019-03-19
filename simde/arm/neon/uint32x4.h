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
#  error Do not include simde/arm/neon/uint32x4.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_UINT32X4_H)
#define SIMDE__NEON_UINT32X4_H

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  uint32_t         u32 __attribute__((__vector_size__(16)));
#else
  uint32_t         u32[4];
#endif

#if defined(SIMDE_NEON_NATIVE)
  uint32x4_t       n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx[2];
#endif
#if defined(SIMDE_NEON_SSE2)
  __m128i         sse;
#endif
} simde_uint32x4_t;

#if defined(SIMDE_NEON_NATIVE)
HEDLEY_STATIC_ASSERT(sizeof(uint32x4_t) == sizeof(simde_uint32x4_t), "uint32x4_t size doesn't match simde_uint32x4_t size");
#endif
HEDLEY_STATIC_ASSERT(16 == sizeof(simde_uint32x4_t), "simde_uint32x4_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vaddq_u32(simde_uint32x4_t a, simde_uint32x4_t b) {
  simde_uint32x4_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vaddq_u32(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.u32[i] = a.u32[i] + b.u32[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vld1q_u32 (uint32_t const ptr[4]) {
  simde_uint32x4_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vld1q_u32(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.u32[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_x_vloadq_u32 (uint32_t l0, uint32_t l1, uint32_t l2, uint32_t l3) {
  uint32_t v[] = { l0, l1, l2, l3 };
  return simde_vld1q_u32(v);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vdupq_n_u32 (uint32_t value) {
  simde_uint32x4_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vdupq_n_u32(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.u32[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vmulq_u32(simde_uint32x4_t a, simde_uint32x4_t b) {
  simde_uint32x4_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vmulq_u32(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.u32[i] = a.u32[i] * b.u32[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsubq_u32(simde_uint32x4_t a, simde_uint32x4_t b) {
  simde_uint32x4_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vsubq_u32(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.u32[i] = a.u32[i] - b.u32[i];
  }
#endif
  return r;
}

#endif
