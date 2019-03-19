/* Permission is hereby granted, free of charge, to any person
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
 *
 * Copyright:
 *   2017      Evan Nemerson <evan@nemerson.com>
 */

#if !defined(SIMDE__EM_H_INSIDE)
#  error float32x4.h must not be included directly; use <simde/simde-js/simde-js.h> instead.
#endif

#if !defined(SIMDE__EM_FLOAT32X4_H)
#define SIMDE__EM_FLOAT32X4_H

#include <math.h>

SIMDE__BEGIN_DECLS

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_set (simde_float32 s0, simde_float32 s1, simde_float32 s2, simde_float32 s3) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_set(s0, s1, s2, s3));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_set_ps(s3, s2, s1, s0));
#elif defined(SIMDE_EM_NEON)
  SIMDE__ALIGN(16) simde_float32 data[4] = { s0, s1, s2, s3 };
  return SIMDE_EM_FLOAT32X4_NEON_C(vld1q_f32(data));
#else
  return (simde_em_float32x4) {
    .v = { s0, s1, s2, s3 }
  };
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_splat (simde_float32 s) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_splat(s));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_set1_ps(s));
#elif defined(SIMDE_SSE2_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vdupq_n_f32(a));
#else
  return (simde_em_float32x4) {
    .v = { s, s, s, s }
  };
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_add (simde_em_float32x4 a, simde_em_float32x4 b) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_add(a.n, b.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_add_ps(a.sse, b.sse));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vaddq_f32(a.neon, b.neon));
#elif defined(SIMDE__ENABLE_GCC_VEC_EXT)
  simde_em_float32x4 r;
  r.v = a.v + b.v;
  return r;
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = a.v[i] + b.v[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_sub (simde_em_float32x4 a, simde_em_float32x4 b) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_sub(a.n, b.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_sub_ps(a.sse, b.sse));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vsubq_f32(a.neon, b.neon));
#elif defined(SIMDE__ENABLE_GCC_VEC_EXT)
  simde_em_float32x4 r;
  r.v = a.v - b.v;
  return r;
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = a.v[i] - b.v[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_mul (simde_em_float32x4 a, simde_em_float32x4 b) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_mul(a.n, b.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_mul_ps(a.sse, b.sse));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vmulq_f32(a.neon, b.neon));
#elif defined(SIMDE__ENABLE_GCC_VEC_EXT)
  simde_em_float32x4 r;
  r.v = a.v * b.v;
  return r;
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = a.v[i] * b.v[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_div (simde_em_float32x4 a, simde_em_float32x4 b) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_div(a.n, b.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_div_ps(a.sse, b.sse));
#elif defined(SIMDE_EM_NEON) && defined(SIMDE_ARCH_AARCH64)
  return SIMDE_EM_FLOAT32X4_NEON_C(vdivq_f32(a.neon, b.neon));
#elif defined(SIMDE_EM_NEON)
  float32x4_t recip0 = vrecpeq_f32(b.neon);
  float32x4_t recip1 = vmulq_f32(recip0, vrecpsq_f32(recip0, b.neon));
  return SIMDE_EM_FLOAT32X4_NEON_C(vmulq_f32(a.neon, recip1));
#elif defined(SIMDE__ENABLE_GCC_VEC_EXT)
  simde_em_float32x4 r;
  r.v = a.v / b.v;
  return r;
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = a.v[i] / b.v[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_max (simde_em_float32x4 a, simde_em_float32x4 b) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_max(a.n, b.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_max_ps(a.sse, b.sse));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vmaxq_f32(a.neon, b.neon));
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = (a.v[i] > b.v[i]) ? a.v[i] : b.v[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_min (simde_em_float32x4 a, simde_em_float32x4 b) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_min(a.n, b.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_min_ps(a.sse, b.sse));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vminq_f32(a.neon, b.neon));
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = (a.v[i] < b.v[i]) ? a.v[i] : b.v[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_neg (simde_em_float32x4 a) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_neg(a.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_mul_ps(a.sse, _mm_set1_ps(-1)));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vnegq_f32(a.neon));
#elif defined(SIMDE__ENABLE_GCC_VEC_EXT)
  simde_em_float32x4 r;
  r.v = -(a.v);
  return r;
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = -(a.v[i]);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_sqrt (simde_em_float32x4 a) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_sqrt(a.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_sqrt_ps(a.sse));
#elif defined(SIMDE_EM_NEON) && defined(SIMDE_ARCH_AARCH64)
  return SIMDE_EM_FLOAT32X4_NEON_C(vsqrtq_f32(a.neon));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vrecpeq_f32(vrsqrteq_f32(a.neon)));
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = sqrtf(a.v[i]);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_reciprocalApproximation (simde_em_float32x4 a) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_reciprocalApproximation(a.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_rcp_ps(a.sse));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vrecpeq_f32(a.neon));
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = ((simde_float32) 1.0f) / a.v[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_reciprocalSqrtApproximation (simde_em_float32x4 a) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_reciprocalSqrtApproximation(a.n));
#elif defined(SIMDE_EM_SSE)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_rsqrt_ps(a.sse));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vrsqrteq_f32(a.neon));
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = ((simde_float32) 1.0f) / sqrtf(a.v[i]);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_abs (simde_em_float32x4 a) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_abs(a.n));
#elif defined(SIMDE_EM_SSE2)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_and_ps(a.sse, _mm_castsi128_ps(_mm_set1_epi32(~(1<<31)))));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vabsq_f32(a.neon));
#else
  simde_em_float32x4 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.v) / sizeof(r.v[0])) ; i++) {
    r.v[i] = fabsf(a.v[i]);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde_em_float32x4
simde_em_float32x4_and (simde_em_float32x4 a, simde_em_float32x4 b) {
#if defined(SIMDE_EM_NATIVE)
  return SIMDE_EM_FLOAT32X4_C(emscripten_float32x4_and(a.n, b.n));
#elif defined(SIMDE_EM_SSE2)
  return SIMDE_EM_FLOAT32X4_SSE_C(_mm_and_ps(a.sse, b.sse));
#elif defined(SIMDE_EM_NEON)
  return SIMDE_EM_FLOAT32X4_NEON_C(vreinterpretq_f32_s32(vandq_s32(vreinterpretq_s32_f32(a.neon), vreinterpretq_s32_f32(b.neon))));
#else
  union { simde_em_float32x4 f32; simde_em_int32x4 s32; } ap, bp, tmp;
  ap.f32 = a;
  bp.f32 = b;

  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ap.s32.v) / sizeof(ap.s32.v[0])) ; i++) {
    tmp.s32.v[i] = ap.s32.v[i] & bp.s32.v[i];
  }

  return tmp.f32;
#endif
}

SIMDE__END_DECLS

#endif /* !defined(SIMDE__EM_FLOAT32X4_H) */
