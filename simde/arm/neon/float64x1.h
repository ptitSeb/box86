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
#  error Do not include simde/arm/neon/float64x1.h directly; use simde/arm/neon.h.
#endif

#if !defined(SIMDE__NEON_FLOAT64X1_H)
#define SIMDE__NEON_FLOAT64X1_H

#if defined(SIMDE_NEON64_NATIVE)
#  if defined(HEDLEY_GCC_VERSION) && !HEDLEY_GCC_VERSION_CHECK(4,9,0)
#  else
#    define SIMDE_NEON_HAVE_FLOAT64X1
#  endif
#endif

typedef union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  simde_float64     f64 __attribute__((__vector_size__(8)));
#else
  simde_float64     f64[1];
#endif

#if defined(SIMDE_NEON_HAVE_FLOAT64X1)
  float64x1_t       n;
#endif

#if defined(SIMDE_NEON_MMX)
  __m64           mmx;
#endif
} simde_float64x1_t;

#if defined(SIMDE_NEON_HAVE_FLOAT64X1)
HEDLEY_STATIC_ASSERT(sizeof(float64x1_t) == sizeof(simde_float64x1_t), "float64x1_t size doesn't match simde_float64x1_t size");
#endif
HEDLEY_STATIC_ASSERT(8 == sizeof(simde_float64x1_t), "simde_float64x1_t size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vadd_f64(simde_float64x1_t a, simde_float64x1_t b) {
  simde_float64x1_t r;
#if defined(SIMDE_NEON_HAVE_FLOAT64X1)
  r.n = vadd_f64(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = a.f64[i] + b.f64[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vld1_f64 (simde_float64 const ptr[1]) {
  simde_float64x1_t r;
#if defined(SIMDE_NEON_HAVE_FLOAT64X1)
  r.n = vld1_f64(ptr);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = ptr[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_x_vload_f64 (simde_float64 l0) {
  simde_float64 v[] = { l0 };
  return simde_vld1_f64(v);
}

SIMDE__FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vdup_n_f64 (simde_float64 value) {
  simde_float64x1_t r;
#if defined(SIMDE_NEON_HAVE_FLOAT64X1)
  r.n = vdup_n_f64(value);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = value;
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vmul_f64(simde_float64x1_t a, simde_float64x1_t b) {
  simde_float64x1_t r;
#if defined(SIMDE_NEON_HAVE_FLOAT64X1)
  r.n = vmul_f64(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = a.f64[i] * b.f64[i];
  }
#endif
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vsub_f64(simde_float64x1_t a, simde_float64x1_t b) {
  simde_float64x1_t r;
#if defined(SIMDE_NEON_HAVE_FLOAT64X1)
  r.n = vsub_f64(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = a.f64[i] - b.f64[i];
  }
#endif
  return r;
}

#endif
