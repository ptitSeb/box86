/* Copyright (c) 2017-2018 Evan Nemerson <evan@nemerson.com>
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

#if !defined(SIMDE__SSE4_1_H)
#  if !defined(SIMDE__SSE4_1_H)
#    define SIMDE__SSE4_1_H
#  endif
#  include "ssse3.h"

#  if defined(SIMDE_SSE4_1_NATIVE)
#    undef SIMDE_SSE4_1_NATIVE
#  endif
#  if defined(SIMDE_SSE4_1_FORCE_NATIVE)
#    define SIMDE_SSE4_1_NATIVE
#  elif defined(__SSE4_1__) && !defined(SIMDE_SSE4_1_NO_NATIVE) && !defined(SIMDE_NO_NATIVE)
#    define SIMDE_SSE4_1_NATIVE
#  elif defined(__ARM_NEON) && !defined(SIMDE_SSE4_1_NO_NEON) && !defined(SIMDE_NO_NEON)
#    define SIMDE_SSE4_1_NEON
#  endif

#  if defined(SIMDE_SSE4_1_NATIVE) && !defined(SIMDE_SSE3_NATIVE)
#    if defined(SIMDE_SSE4_1_FORCE_NATIVE)
#      error Native SSE4.1 support requires native SSE3 support
#    else
#      warning Native SSE4.1 support requires native SSE3 support, disabling
#      undef SIMDE_SSE4_1_NATIVE
#    endif
#  elif defined(SIMDE_SSE4_1_NEON) && !defined(SIMDE_SSE3_NEON)
#    warning SSE4.1 NEON support requires SSE3 NEON support, disabling
#    undef SIMDE_SSE4_1_NEON
#  endif

#  if defined(SIMDE_SSE4_1_NATIVE)
#    include <smmintrin.h>
#  else
#    if defined(SIMDE_SSE4_1_NEON)
#      include <arm_neon.h>
#    endif
#  endif

SIMDE__BEGIN_DECLS

#if defined(SIMDE_SSE4_1_NATIVE)
#  define SIMDE_MM_FROUND_TO_NEAREST_INT _MM_FROUND_TO_NEAREST_INT
#  define SIMDE_MM_FROUND_TO_NEG_INF     _MM_FROUND_TO_NEG_INF
#  define SIMDE_MM_FROUND_TO_POS_INF		 _MM_FROUND_TO_POS_INF
#  define SIMDE_MM_FROUND_TO_ZERO        _MM_FROUND_TO_ZERO
#  define SIMDE_MM_FROUND_CUR_DIRECTION  _MM_FROUND_CUR_DIRECTION

#  define SIMDE_MM_FROUND_RAISE_EXC      _MM_FROUND_RAISE_EXC
#  define SIMDE_MM_FROUND_NO_EXC         _MM_FROUND_NO_EXC
#else
#  define SIMDE_MM_FROUND_TO_NEAREST_INT 0x00
#  define SIMDE_MM_FROUND_TO_NEG_INF     0x01
#  define SIMDE_MM_FROUND_TO_POS_INF		 0x02
#  define SIMDE_MM_FROUND_TO_ZERO        0x03
#  define SIMDE_MM_FROUND_CUR_DIRECTION  0x04

#  define SIMDE_MM_FROUND_RAISE_EXC      0x00
#  define SIMDE_MM_FROUND_NO_EXC         0x08
#endif

#define SIMDE_MM_FROUND_NINT		\
  (SIMDE_MM_FROUND_TO_NEAREST_INT | SIMDE_MM_FROUND_RAISE_EXC)
#define SIMDE_MM_FROUND_FLOOR	\
  (SIMDE_MM_FROUND_TO_NEG_INF | SIMDE_MM_FROUND_RAISE_EXC)
#define SIMDE_MM_FROUND_CEIL		\
  (SIMDE_MM_FROUND_TO_POS_INF | SIMDE_MM_FROUND_RAISE_EXC)
#define SIMDE_MM_FROUND_TRUNC	\
  (SIMDE_MM_FROUND_TO_ZERO | SIMDE_MM_FROUND_RAISE_EXC)
#define SIMDE_MM_FROUND_RINT		\
  (SIMDE_MM_FROUND_CUR_DIRECTION | SIMDE_MM_FROUND_RAISE_EXC)
#define SIMDE_MM_FROUND_NEARBYINT	\
  (SIMDE_MM_FROUND_CUR_DIRECTION | SIMDE_MM_FROUND_NO_EXC)

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_blend_epi16 (simde__m128i a, simde__m128i b, const int imm8) {
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    r.u16[i] = ((imm8 >> i) & 1) ? b.u16[i] : a.u16[i];
  }
  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_blend_epi16(a, b, imm8) SIMDE__M128I_C(_mm_blend_epi16(a.n, b.n, imm8))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_blend_pd (simde__m128d a, simde__m128d b, const int imm8) {
  simde__m128d r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = ((imm8 >> i) & 1) ? b.f64[i] : a.f64[i];
  }
  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_blend_pd(a, b, imm8) SIMDE__M128D_C(_mm_blend_pd(a.n, b.n, imm8))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_blend_ps (simde__m128 a, simde__m128 b, const int imm8) {
  simde__m128 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = ((imm8 >> i) & 1) ? b.f32[i] : a.f32[i];
  }
  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_blend_ps(a, b, imm8) SIMDE__M128_C(_mm_blend_ps(a.n, b.n, imm8))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_blendv_epi8 (simde__m128i a, simde__m128i b, simde__m128i mask) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_blendv_epi8(a.n, b.n, mask.n));
#elif defined(SIMDE_SSE4_1_NEON)
  simde__m128i mask_ = simde_mm_cmplt_epi8(mask, simde_mm_set1_epi8(0));
  return SIMDE__M128I_NEON_C(i8, vbslq_s8(mask_.neon_u8, b.neon_i8, a.neon_i8));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u8) / sizeof(r.u8[0])) ; i++) {
    if (mask.u8[i] & 0x80) {
      r.u8[i] = b.u8[i];
    } else {
      r.u8[i] = a.u8[i];
    }
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_blendv_pd (simde__m128d a, simde__m128d b, simde__m128d mask) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128D_C(_mm_blendv_pd(a.n, b.n, mask.n));
#else
  simde__m128d r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    if (mask.u64[i] & (UINT64_C(1) << 63)) {
      r.f64[i] = b.f64[i];
    } else {
      r.f64[i] = a.f64[i];
    }
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_blendv_ps (simde__m128 a, simde__m128 b, simde__m128 mask) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128_C(_mm_blendv_ps(a.n, b.n, mask.n));
#else
  simde__m128 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    if (mask.u32[i] & (UINT32_C(1) << 31)) {
      r.f32[i] = b.f32[i];
    } else {
      r.f32[i] = a.f32[i];
    }
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_ceil_pd (simde__m128d a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128D_C(_mm_ceil_pd(a.n));
#else
  simde__m128d r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = ceil(a.f64[i]);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_ceil_ps (simde__m128 a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128_C(_mm_ceil_ps(a.n));
#else
  simde__m128 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = ceilf(a.f32[i]);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_ceil_sd (simde__m128d a, simde__m128d b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128D_C(_mm_ceil_sd(a.n, b.n));
#else
  return simde_mm_set_pd(a.f64[1], ceil(b.f64[0]));
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_ceil_ss (simde__m128 a, simde__m128 b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128_C(_mm_ceil_ss(a.n, b.n));
#else
  return simde_mm_set_ps(a.f32[3], a.f32[2], a.f32[1], ceil(b.f32[0]));
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cmpeq_epi64 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cmpeq_epi64(a.n, b.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.u64[i] = (a.u64[i] == b.u64[i]) ? ~UINT64_C(0) : UINT64_C(0);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepi8_epi16 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepi8_epi16(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = a.i8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepi8_epi32 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepi8_epi32(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.i8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepi8_epi64 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepi8_epi64(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepu8_epi16 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepu8_epi16(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = a.u8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepu8_epi32 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepu8_epi32(a.n));
#elif defined(SIMDE_SSE4_1_NEON)
  uint8x16_t u8x16 = a.neon_u8;                      /* xxxx xxxx xxxx DCBA */
  uint16x8_t u16x8 = vmovl_u8(vget_low_u8(u8x16));   /* 0x0x 0x0x 0D0C 0B0A */
  uint32x4_t u32x4 = vmovl_u16(vget_low_u16(u16x8)); /* 000D 000C 000B 000A */
  return SIMDE__M128I_NEON_C(u32, u32x4);
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.u8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepu8_epi64 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepu8_epi64(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.u8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepi16_epi32 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepi16_epi32(a.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(i32, vmovl_s16(vget_low_s16(a.neon_i16)));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.i16[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepu16_epi32 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepu16_epi32(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.u16[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepu16_epi64 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepu16_epi64(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.u16[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepi16_epi64 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepi16_epi64(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i16[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepi32_epi64 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepi32_epi64(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i32[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_cvtepu32_epi64 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_cvtepu32_epi64(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.u32[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_dp_pd (simde__m128d a, simde__m128d b, const int imm8) {
  simde__m128d r;
  simde_float64 sum = SIMDE_FLOAT64_C(0.0);

  SIMDE__VECTORIZE_REDUCTION(+:sum)
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    sum += ((imm8 >> (i + 4)) & 1) ? (a.f64[i] * b.f64[i]) : 0.0;
  }

  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = ((imm8 >> i) & 1) ? sum : 0.0;
  }

  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_dp_pd(a, b, imm8) SIMDE__M128D_C(_mm_dp_pd(a.n, b.n, imm8))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_dp_ps (simde__m128 a, simde__m128 b, const int imm8) {
  simde__m128 r;
  simde_float32 sum = SIMDE_FLOAT32_C(0.0);

  SIMDE__VECTORIZE_REDUCTION(+:sum)
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    sum += ((imm8 >> (i + 4)) & 1) ? (a.f32[i] * b.f32[i]) : 0.0;
  }

  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = ((imm8 >> i) & 1) ? sum : 0.0;
  }

  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_dp_ps(a, b, imm8) SIMDE__M128_C(_mm_dp_ps(a.n, b.n, imm8))
#endif

#if defined(simde_mm_extract_epi8)
#  undef simde_mm_extract_epi8
#endif
SIMDE__FUNCTION_ATTRIBUTES
int32_t
simde_mm_extract_epi8 (simde__m128i a, const int imm8) {
  return a.u8[imm8&15];
}
#if defined(SIMDE_SSE4_1_NATIVE) && !defined(SIMDE_BUG_GCC_BAD_MM_EXTRACT_EPI8)
#  define simde_mm_extract_epi8(a, imm8) _mm_extract_epi8(a.n, imm8)
#elif defined(SIMDE_SSE4_1_NEON)
#  define simde_mm_extract_epi8(a, imm8) (int32_t)((uint8_t)vgetq_lane_s8(a.neon_i8, imm8))
#endif

#if defined(simde_mm_extract_epi32)
#  undef simde_mm_extract_epi32
#endif
SIMDE__FUNCTION_ATTRIBUTES
int32_t
simde_mm_extract_epi32 (simde__m128i a, const int imm8) {
  return a.i32[imm8&3];
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_extract_epi32(a, imm8) _mm_extract_epi32(a.n, imm8)
#elif defined(SIMDE_SSE4_1_NEON)
#  define simde_mm_extract_epi32(a, imm8) vgetq_lane_s32(a.neon_i32, imm8)
#endif

#if defined(simde_mm_extract_epi64)
#  undef simde_mm_extract_epi64
#endif
SIMDE__FUNCTION_ATTRIBUTES
int64_t
simde_mm_extract_epi64 (simde__m128i a, const int imm8) {
  return a.i64[imm8&1];
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_extract_epi64(a, imm8) _mm_extract_epi64(a.n, imm8)
#elif defined(SIMDE_SSE4_1_NEON)
#  define simde_mm_extract_epi64(a, imm8) vgetq_lane_s64(a.neon_i64, imm8)
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_floor_pd (simde__m128d a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128D_C(_mm_floor_pd(a.n));
#else
  simde__m128d r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = floor(a.f64[i]);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_floor_ps (simde__m128 a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128_C(_mm_floor_ps(a.n));
#else
  simde__m128 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = floorf(a.f32[i]);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_floor_sd (simde__m128d a, simde__m128d b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128D_C(_mm_floor_sd(a.n, b.n));
#else
  simde__m128d r;
  r.f64[0] = floor(b.f64[0]);
  r.f64[1] = a.f64[1];
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_floor_ss (simde__m128 a, simde__m128 b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128_C(_mm_floor_ss(a.n, b.n));
#else
  simde__m128 r;
  r.f32[0] = floor(b.f32[0]);
  for (size_t i = 1 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = a.f32[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_insert_epi8 (simde__m128i a, int i, const int imm8) {
  a.i8[imm8] = (int8_t) i;
  return a;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_insert_epi8(a, i, imm8) SIMDE__M128I_C(_mm_insert_epi8(a.n, i, imm8));
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_insert_epi32 (simde__m128i a, int i, const int imm8) {
  a.i32[imm8] = (int32_t) i;
  return a;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_insert_epi32(a, i, imm8) SIMDE__M128I_C(_mm_insert_epi32(a.n, i, imm8));
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_insert_epi64 (simde__m128i a, int64_t i, const int imm8) {
  a.i64[imm8] = i;
  return a;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_insert_epi64(a, i, imm8) SIMDE__M128I_C(_mm_insert_epi64(a.n, i, imm8));
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_insert_ps (simde__m128 a, simde__m128 b, const int imm8) {
  simde__m128 r;

  a.f32[0] = b.f32[(imm8 >> 6) & 3];
  a.f32[(imm8 >> 4) & 3] = a.f32[0];

  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = (imm8 >> i) ? SIMDE_FLOAT32_C(0.0) : a.f32[i];
  }

  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_insert_ps(a, b, imm8) SIMDE__M128_C(_mm_insert_ps((a).n, (b).n, imm8));
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_max_epi8 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE) && !defined(__PGI)
  return SIMDE__M128I_C(_mm_max_epi8(a.n, b.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(i8, vmaxq_s8(a.neon_i8, b.neon_i8));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a.i8[i] > b.i8[i] ? a.i8[i] : b.i8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_max_epi32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE) && !defined(__PGI)
  return SIMDE__M128I_C(_mm_max_epi32(a.n, b.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(i32, vmaxq_s32(a.neon_i32, b.neon_i32));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.i32[i] > b.i32[i] ? a.i32[i] : b.i32[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_max_epu16 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_max_epu16(a.n, b.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(u16, vmaxq_u16(a.neon_u16, b.neon_u16));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    r.u16[i] = a.u16[i] > b.u16[i] ? a.u16[i] : b.u16[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_max_epu32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_max_epu32(a.n, b.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(u32, vmaxq_u32(a.neon_u32, b.neon_u32));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.u32[i] = a.u32[i] > b.u32[i] ? a.u32[i] : b.u32[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_min_epi8 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE) && !defined(__PGI)
  return SIMDE__M128I_C(_mm_min_epi8(a.n, b.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(i8, vminq_s8(a.neon_i8, b.neon_i8));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a.i8[i] < b.i8[i] ? a.i8[i] : b.i8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_min_epi32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE) && !defined(__PGI)
  return SIMDE__M128I_C(_mm_min_epi32(a.n, b.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(i32, vminq_s32(a.neon_i32, b.neon_i32));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.i32[i] < b.i32[i] ? a.i32[i] : b.i32[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_min_epu16 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_min_epu16(a.n, b.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(u16, vminq_u16(a.neon_u16, b.neon_u16));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    r.u16[i] = a.u16[i] < b.u16[i] ? a.u16[i] : b.u16[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_min_epu32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_min_epu32(a.n, b.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(u32, vminq_u32(a.neon_u32, b.neon_u32));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.u32[i] = a.u32[i] < b.u32[i] ? a.u32[i] : b.u32[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_minpos_epu16 (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_minpos_epu16(a.n));
#else
  simde__m128i r = simde_x_mm_set_epu16(0, 0, 0, 0, 0, 0, 0, UINT16_MAX);

  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    if (a.u16[i] < r.u16[0]) {
      r.u16[0] = a.u16[i];
      r.u16[1] = (uint16_t) i;
    }
  }

  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_mpsadbw_epu8 (simde__m128i a, simde__m128i b, const int imm8) {
  simde__m128i r;
  const int a_offset = imm8 & 4;
  const int b_offset = (imm8 & 3) << 2;

  for (size_t i = 0 ; i < (sizeof(r.u16) / sizeof(r.u16[0])) ; i++) {
    r.u16[i] =
      ((uint16_t) abs(a.u8[a_offset + i + 0] - b.u8[b_offset + 0])) +
      ((uint16_t) abs(a.u8[a_offset + i + 1] - b.u8[b_offset + 1])) +
      ((uint16_t) abs(a.u8[a_offset + i + 2] - b.u8[b_offset + 2])) +
      ((uint16_t) abs(a.u8[a_offset + i + 3] - b.u8[b_offset + 3]));
  }

  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_mpsadbw_epu8(a, b, imm8) SIMDE__M128I_C(_mm_mpsadbw_epu8(a.n, b.n, imm8));
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_mul_epi32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_mul_epi32(a.n, b.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] =
      ((int64_t) a.i32[i * 2]) *
      ((int64_t) b.i32[i * 2]);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_mullo_epi32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_mullo_epi32(a.n, b.n));
#elif defined(SIMDE_SSE4_1_NEON)
  return SIMDE__M128I_NEON_C(i32, vmulq_s32(a.neon_i32, b.neon_i32));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.u32[i] = (uint32_t) (((uint64_t) (((int64_t) a.i32[i]) * ((int64_t) b.i32[i]))) & 0xffffffff);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_packus_epi32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_packus_epi32(a.n, b.n));
#else
  simde__m128i r;
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.u16[i + 0] = (a.i32[i] < 0) ? UINT16_C(0) : ((a.i32[i] > UINT16_MAX) ? (UINT16_MAX) : ((uint32_t) a.i32[i]));
    r.u16[i + 4] = (b.i32[i] < 0) ? UINT16_C(0) : ((b.i32[i] > UINT16_MAX) ? (UINT16_MAX) : ((uint32_t) b.i32[i]));
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_round_pd (simde__m128d a, int rounding) {
  simde__m128d r;
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    switch (rounding & ~SIMDE_MM_FROUND_NO_EXC) {
      case SIMDE_MM_FROUND_TO_NEAREST_INT:
        r.f64[i] = nearbyint(a.f64[i]);
        break;
      case SIMDE_MM_FROUND_TO_NEG_INF:
        r.f64[i] = floor(a.f64[i]);
        break;
      case SIMDE_MM_FROUND_TO_POS_INF:
        r.f64[i] = ceil(a.f64[i]);
        break;
      case SIMDE_MM_FROUND_TO_ZERO:
        r.f64[i] = trunc(a.f64[i]);
        break;
      case SIMDE_MM_FROUND_CUR_DIRECTION:
        r.f64[i] = nearbyint(a.f64[i]);
        break;
      default:
        HEDLEY_UNREACHABLE();
        break;
    }
  }
  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_round_pd(a, rounding) SIMDE__M128D_C(_mm_round_pd((a).n, rounding))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_round_ps (simde__m128 a, int rounding) {
  simde__m128 r;
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    switch (rounding & ~SIMDE_MM_FROUND_NO_EXC) {
      case SIMDE_MM_FROUND_TO_NEAREST_INT:
        r.f32[i] = nearbyintf(a.f32[i]);
        break;
      case SIMDE_MM_FROUND_TO_NEG_INF:
        r.f32[i] = floorf(a.f32[i]);
        break;
      case SIMDE_MM_FROUND_TO_POS_INF:
        r.f32[i] = ceilf(a.f32[i]);
        break;
      case SIMDE_MM_FROUND_TO_ZERO:
        r.f32[i] = truncf(a.f32[i]);
        break;
      case SIMDE_MM_FROUND_CUR_DIRECTION:
        r.f32[i] = nearbyintf (a.f32[i]);
        break;
      default:
        HEDLEY_UNREACHABLE();
        break;
    }
  }
  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_round_ps(a, rounding) SIMDE__M128_C(_mm_round_ps((a).n, rounding))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm_round_sd (simde__m128d a, simde__m128d b, int rounding) {
  simde__m128d r = a;
  switch (rounding & ~SIMDE_MM_FROUND_NO_EXC) {
    case SIMDE_MM_FROUND_TO_NEAREST_INT:
      r.f64[0] = nearbyint(b.f64[0]);
      break;
    case SIMDE_MM_FROUND_TO_NEG_INF:
      r.f64[0] = floor(b.f64[0]);
      break;
    case SIMDE_MM_FROUND_TO_POS_INF:
      r.f64[0] = ceil(b.f64[0]);
      break;
    case SIMDE_MM_FROUND_TO_ZERO:
      r.f64[0] = trunc(b.f64[0]);
      break;
    case SIMDE_MM_FROUND_CUR_DIRECTION:
      r.f64[0] = nearbyint(b.f64[0]);
      break;
    default:
      HEDLEY_UNREACHABLE();
      break;
  }
  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_round_sd(a, b, rounding) SIMDE__M128D_C(_mm_round_sd((a).n, (b).n, rounding))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_round_ss (simde__m128 a, simde__m128 b, int rounding) {
  simde__m128 r = a;
  switch (rounding & ~SIMDE_MM_FROUND_NO_EXC) {
    case SIMDE_MM_FROUND_TO_NEAREST_INT:
      r.f32[0] = nearbyintf(b.f32[0]);
      break;
    case SIMDE_MM_FROUND_TO_NEG_INF:
      r.f32[0] = floorf(b.f32[0]);
      break;
    case SIMDE_MM_FROUND_TO_POS_INF:
      r.f32[0] = ceilf(b.f32[0]);
      break;
    case SIMDE_MM_FROUND_TO_ZERO:
      r.f32[0] = truncf(b.f32[0]);
      break;
    case SIMDE_MM_FROUND_CUR_DIRECTION:
      r.f32[0] = nearbyintf (b.f32[0]);
      break;
    default:
      HEDLEY_UNREACHABLE();
      break;
  }
  return r;
}
#if defined(SIMDE_SSE4_1_NATIVE)
#  define simde_mm_round_ss(a, b, rounding) SIMDE__M128_C(_mm_round_ss((a).n, (b).n, rounding))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_stream_load_si128 (const simde__m128i* mem_addr) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return SIMDE__M128I_C(_mm_stream_load_si128((__m128i*)(void*) &(mem_addr->n)));
#else
  return *mem_addr;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
int
simde_mm_test_all_ones (simde__m128i a) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return _mm_test_all_ones(a.n);
#else
  for (size_t i = 0 ; i < (sizeof(a.u64) / sizeof(a.u64[0])) ; i++) {
    if (a.u64[i] != ~UINT64_C(0))
      return 0;
  }
  return 1;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
int
simde_mm_test_all_zeros (simde__m128i a, simde__m128i mask) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return _mm_test_all_zeros(a.n, mask.n);
#else
  for (size_t i = 0 ; i < (sizeof(a.u64) / sizeof(a.u64[0])) ; i++) {
    if ((a.u64[i] & mask.u64[i]) != 0)
      return 0;
  }
  return 1;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
int
simde_mm_test_mix_ones_zeros (simde__m128i a, simde__m128i mask) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return _mm_test_mix_ones_zeros(a.n, mask.n);
#else
  for (size_t i = 0 ; i < (sizeof(a.u64) / sizeof(a.u64[0])) ; i++)
    if (((a.u64[i] & mask.u64[i]) != 0) && ((~a.u64[i] & mask.u64[i]) != 0))
      return 1;
  return 0;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
int
simde_mm_testc_si128 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return _mm_testc_si128(a.n, b.n);
#else
  for (size_t i = 0 ; i < (sizeof(a.u64) / sizeof(a.u64[0])) ; i++) {
    if ((~a.u64[i] & b.u64[i]) == 0)
      return 1;
  }
  return 0;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
int
simde_mm_testnzc_si128 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return _mm_testnzc_si128(a.n, b.n);
#else
  for (size_t i = 0 ; i < (sizeof(a.u64) / sizeof(a.u64[0])) ; i++) {
    if (((a.u64[i] & b.u64[i]) != 0) && ((~a.u64[i] & b.u64[i]) != 0))
      return 1;
  }
  return 0;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
int
simde_mm_testz_si128 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSE4_1_NATIVE)
  return _mm_testz_si128(a.n, b.n);
#else
  for (size_t i = 0 ; i < (sizeof(a.u64) / sizeof(a.u64[0])) ; i++) {
    if ((a.u64[i] & b.u64[i]) == 0)
      return 1;
  }
  return 0;
#endif
}

SIMDE__END_DECLS

#endif /* !defined(SIMDE__SSE4_1_H) */
