/* Copyright (c) 2017 Evan Nemerson <evan@nemerson.com>
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

#if !defined(SIMDE__SSE3_H)
#  if !defined(SIMDE__SSE3_H)
#    define SIMDE__SSE3_H
#  endif
#  include "sse2.h"

#  if defined(SIMDE_SSE3_NATIVE)
#    undef SIMDE_SSE3_NATIVE
#  endif
#  if defined(SIMDE_SSE3_FORCE_NATIVE)
#    define SIMDE_SSE3_NATIVE
#  elif defined(__SSE3__) && (!defined(SIMDE_SSE3_NO_NATIVE) && !defined(SIMDE_NO_NATIVE))
#    define SIMDE_SSE3_NATIVE
#  elif defined(__ARM_NEON) && !defined(SIMDE_SSE3_NO_NEON) && !defined(SIMDE_NO_NEON)
#    define SIMDE_SSE3_NEON
#  endif

#  if defined(SIMDE_SSE3_NATIVE) && !defined(SIMDE_SSE2_NATIVE)
#    if defined(SIMDE_SSE3_FORCE_NATIVE)
#      error Native SSE3 support requires native SSE2 support
#    else
#      warning Native SSE3 support requires native SSE2 support, disabling
#      undef SIMDE_SSE3_NATIVE
#    endif
#  elif defined(SIMDE_SSE3_NEON) && !defined(SIMDE_SSE2_NEON)
#    warning SSE3 NEON support requires SSE2 NEON support, disabling
#    undef SIMDE_SSE3_NEON
#  endif

#  if defined(SIMDE_SSE3_NATIVE)
#    include <pmmintrin.h>
#  endif

SIMDE__BEGIN_DECLS

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_addsub_pd (simde__m128d a, simde__m128d b) {
#if defined(SIMDE_SSE3_NATIVE)
  return SIMDE__M128D_C(_mm_addsub_pd(a.n, b.n));
#else
  simde__m128d r;
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i += 2) {
    r.f64[    i] = a.f64[    i] - b.f64[    i];
    r.f64[1 + i] = a.f64[1 + i] + b.f64[1 + i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_addsub_ps (simde__m128 a, simde__m128 b) {
#if defined(SIMDE_SSE3_NATIVE)
  return SIMDE__M128_C(_mm_addsub_ps(a.n, b.n));
#else
  return simde_mm_add_ps(a, simde_mm_mul_ps(simde_mm_set_ps( 1.0f, -1.0f,  1.0f, -1.0f), b));
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_hadd_pd (simde__m128d a, simde__m128d b) {
#if defined(SIMDE_SSE3_NATIVE)
  return SIMDE__M128D_C(_mm_hadd_pd(a.n, b.n));
#else
  simde__m128d r;
  r.f64[0] = a.f64[0] + a.f64[1];
  r.f64[1] = b.f64[0] + b.f64[1];
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_hadd_ps (simde__m128 a, simde__m128 b) {
#if defined(SIMDE_SSE3_NATIVE)
  return SIMDE__M128_C(_mm_hadd_ps(a.n, b.n));
#elif defined(SIMDE_SSE3_NEON)
  #if defined(SIMDE_ARCH_AARCH64)
    return SIMDE__M128_NEON_C(f32, vpaddq_f32(a.neon_f32, b.neon_f32));
  #else
    float32x2_t a10 = vget_low_f32(a.neon_f32);
    float32x2_t a32 = vget_high_f32(a.neon_f32);
    float32x2_t b10 = vget_low_f32(b.neon_f32);
    float32x2_t b32 = vget_high_f32(b.neon_f32);
    return SIMDE__M128_NEON_C(f32, vcombine_f32(vpadd_f32(a10, a32), vpadd_f32(b10, b32)));
  #endif
#else
  simde__m128 r;
  r.f32[0] = a.f32[0] + a.f32[1];
  r.f32[1] = a.f32[2] + a.f32[3];
  r.f32[2] = b.f32[0] + b.f32[1];
  r.f32[3] = b.f32[2] + b.f32[3];
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_hsub_pd (simde__m128d a, simde__m128d b) {
#if defined(SIMDE_SSE3_NATIVE)
  return SIMDE__M128D_C(_mm_hsub_pd(a.n, b.n));
#else
  simde__m128d r;
  r.f64[0] = a.f64[0] - a.f64[1];
  r.f64[1] = b.f64[0] - b.f64[1];
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_hsub_ps (simde__m128 a, simde__m128 b) {
#if defined(SIMDE_SSE3_NATIVE)
  return SIMDE__M128_C(_mm_hsub_ps(a.n, b.n));
#elif defined(SIMDE_SSE3_NEON)
  const float32_t mp[] = { 1.0f, -1.0f, 1.0f, -1.0f };
  const float32x4_t m = vld1q_f32(mp);

  float32x4_t ap = vmulq_f32(a.neon_f32, m);
  float32x4_t bp = vmulq_f32(b.neon_f32, m);
  float32x2_t ax = vpadd_f32(vget_low_f32(ap), vget_high_f32(ap));
  float32x2_t bx = vpadd_f32(vget_low_f32(bp), vget_high_f32(bp));

  return SIMDE__M128_NEON_C(f32, vcombine_f32(ax, bx));
#else
  simde__m128 r;
  r.f32[0] = a.f32[0] - a.f32[1];
  r.f32[1] = a.f32[2] - a.f32[3];
  r.f32[2] = b.f32[0] - b.f32[1];
  r.f32[3] = b.f32[2] - b.f32[3];
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_lddqu_si128 (simde__m128i const* mem_addr) {
#if defined(SIMDE_SSE3_NATIVE)
  return SIMDE__M128I_C(_mm_lddqu_si128(&mem_addr->n));
#elif defined(SIMDE_SSE3_NEON)
  return SIMDE__M128I_NEON_C(i32, vld1q_s32((int32_t const*) mem_addr));
#else
  simde__m128i r;
  memcpy(&r, mem_addr, sizeof(r));
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_movedup_pd (simde__m128d a) {
#if defined(SIMDE_SSE3_NATIVE)
  return SIMDE__M128D_C(_mm_movedup_pd(a.n));
#else
  simde__m128d r;
  r.f64[0] = a.f64[0];
  r.f64[1] = a.f64[0];
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_movehdup_ps (simde__m128 a) {
#if defined(SIMDE_SSE3_NATIVE)
  return SIMDE__M128_C(_mm_movehdup_ps(a.n));
#else
  simde__m128 r;
  r.f32[0] = a.f32[1];
  r.f32[1] = a.f32[1];
  r.f32[2] = a.f32[3];
  r.f32[3] = a.f32[3];
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_moveldup_ps (simde__m128 a) {
#if defined(SIMDE__SSE3_NATIVE)
  return SIMDE__M128_C(_mm_moveldup_ps(a.n));
#else
  simde__m128 r;
  r.f32[0] = a.f32[0];
  r.f32[1] = a.f32[0];
  r.f32[2] = a.f32[2];
  r.f32[3] = a.f32[2];
  return r;
#endif
}

SIMDE__END_DECLS

#endif /* !defined(SIMDE__SSE3_H) */
