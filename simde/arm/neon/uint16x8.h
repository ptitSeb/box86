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
#  error Do not include simde/arm/neon/uint16x8.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_UINT16X8_H)
#define SIMDE__NEON_UINT16X8_H

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  uint16_t         u16 __attribute__((__vector_size__(16)));
#else
  uint16_t         u16[8];
#endif

#if defined(SIMDE_NEON_NATIVE)
  uint16x8_t       n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx[2];
#endif
#if defined(SIMDE_NEON_SSE2)
  __m128i         sse;
#endif
} simde_uint16x8_t;

#if defined(SIMDE_NEON_NATIVE)
HEDLEY_STATIC_ASSERT(sizeof(uint16x8_t) == sizeof(simde_uint16x8_t), "uint16x8_t size doesn't match simde_uint16x8_t size");
#endif
HEDLEY_STATIC_ASSERT(16 == sizeof(simde_uint16x8_t), "simde_uint16x8_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vaddq_u16(simde_uint16x8_t a, simde_uint16x8_t b) {
  simde_uint16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vaddq_u16(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    r.u16[i] = a.u16[i] + b.u16[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vld1q_u16 (uint16_t const ptr[8]) {
  simde_uint16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vld1q_u16(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    r.u16[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_x_vloadq_u16 (uint16_t l0, uint16_t l1, uint16_t l2, uint16_t l3,
		    uint16_t l4, uint16_t l5, uint16_t l6, uint16_t l7) {
  uint16_t v[] = { l0, l1, l2, l3, l4, l5, l6, l7 };
  return simde_vld1q_u16(v);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vdupq_n_u16 (uint16_t value) {
  simde_uint16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vdupq_n_u16(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    r.u16[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vmulq_u16(simde_uint16x8_t a, simde_uint16x8_t b) {
  simde_uint16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vmulq_u16(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    r.u16[i] = a.u16[i] * b.u16[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vsubq_u16(simde_uint16x8_t a, simde_uint16x8_t b) {
  simde_uint16x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vsubq_u16(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    r.u16[i] = a.u16[i] - b.u16[i];
  }
#endif
  return r;
}

#endif
