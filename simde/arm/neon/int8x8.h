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
#  error Do not include simde/arm/neon/int8x8.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_INT8X8_H)
#define SIMDE__NEON_INT8X8_H

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  int8_t          i8 __attribute__((__vector_size__(8)));
#else
  int8_t          i8[8];
#endif

#if defined(SIMDE_NEON_NATIVE)
  int8x8_t        n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx;
#endif
} simde_int8x8_t;

#if defined(SIMDE_NEON_NATIVE)
HEDLEY_STATIC_ASSERT(sizeof(int8x8_t) == sizeof(simde_int8x8_t), "int8x8_t size doesn't match simde_int8x8_t size");
#endif
HEDLEY_STATIC_ASSERT(8 == sizeof(simde_int8x8_t), "simde_int8x8_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vadd_s8(simde_int8x8_t a, simde_int8x8_t b) {
  simde_int8x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vadd_s8(a.n, b.n);
#elif defined(SIMDE_MMX_NATIVE)
  r.mmx = _mm_add_pi8(a.mmx, b.mmx);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a.i8[i] + b.i8[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vld1_s8 (int8_t const ptr[8]) {
  simde_int8x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vld1_s8(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_x_vload_s8 (int8_t l0, int8_t l1, int8_t l2, int8_t l3,
		  int8_t l4, int8_t l5, int8_t l6, int8_t l7) {
  int8_t v[] = { l0, l1, l2, l3, l4, l5, l6, l7 };
  return simde_vld1_s8(v);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vdup_n_s8 (int8_t value) {
  simde_int8x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vdup_n_s8(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vmul_s8(simde_int8x8_t a, simde_int8x8_t b) {
  simde_int8x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vmul_s8(a.n, b.n);
#elif defined(SIMDE_MMX_NATIVE)
  r.mmx = _mm_mul_pi8(a.mmx, b.mmx);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a.i8[i] * b.i8[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vsub_s8(simde_int8x8_t a, simde_int8x8_t b) {
  simde_int8x8_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vsub_s8(a.n, b.n);
#elif defined(SIMDE_MMX_NATIVE)
  r.mmx = _mm_sub_pi8(a.mmx, b.mmx);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a.i8[i] - b.i8[i];
  }
#endif
  return r;
}

#endif
