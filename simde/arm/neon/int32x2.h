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
#  error Do not include simde/arm/neon/int32x2.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_INT32X2_H)
#define SIMDE__NEON_INT32X2_H

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  int32_t         i32 __attribute__((__vector_size__(8)));
#else
  int32_t         i32[2];
#endif

#if defined(SIMDE_NEON_NATIVE)
  int32x2_t       n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx;
#endif
} simde_int32x2_t;

#if defined(SIMDE_NEON_NATIVE)
HEDLEY_STATIC_ASSERT(sizeof(int32x2_t) == sizeof(simde_int32x2_t), "int32x2_t size doesn't match simde_int32x2_t size");
#endif
HEDLEY_STATIC_ASSERT(8 == sizeof(simde_int32x2_t), "simde_int32x2_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vadd_s32(simde_int32x2_t a, simde_int32x2_t b) {
  simde_int32x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vadd_s32(a.n, b.n);
#elif defined(SIMDE_MMX_NATIVE)
  r.mmx = _mm_add_pi32(a.mmx, b.mmx);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.i32[i] + b.i32[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vld1_s32 (int32_t const ptr[2]) {
  simde_int32x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vld1_s32(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_x_vload_s32 (int32_t l0, int32_t l1) {
  int32_t v[] = { l0, l1 };
  return simde_vld1_s32(v);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vdup_n_s32 (int32_t value) {
  simde_int32x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vdup_n_s32(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vmul_s32(simde_int32x2_t a, simde_int32x2_t b) {
  simde_int32x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vmul_s32(a.n, b.n);
#elif defined(SIMDE_MMX_NATIVE)
  r.mmx = _mm_mul_pi32(a.mmx, b.mmx);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.i32[i] * b.i32[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vsub_s32(simde_int32x2_t a, simde_int32x2_t b) {
  simde_int32x2_t r;
#if defined(SIMDE_NEON_NATIVE)
  r.n = vsub_s32(a.n, b.n);
#elif defined(SIMDE_MMX_NATIVE)
  r.mmx = _mm_sub_pi32(a.mmx, b.mmx);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.i32[i] - b.i32[i];
  }
#endif
  return r;
}

#endif
