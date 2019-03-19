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
 *   2018      Evan Nemerson <evan@nemerson.com>
 */

#if !defined(SIMDE__AVX_H)
#  if !defined(SIMDE__AVX_H)
#    define SIMDE__AVX_H
#  endif
#  include "sse2.h"

#  if defined(SIMDE_AVX_NATIVE)
#    undef SIMDE_AVX_NATIVE
#  endif
#  if defined(SIMDE_AVX_FORCE_NATIVE)
#    define SIMDE_AVX_NATIVE
#  elif defined(__AVX__) && !defined(SIMDE_AVX_NO_NATIVE) && !defined(SIMDE_NO_NATIVE)
#    define SIMDE_AVX_NATIVE
#  elif defined(__ARM_NEON) && !defined(SIMDE_AVX_NO_NEON) && !defined(SIMDE_NO_NEON)
#    define SIMDE_AVX_NEON
#  endif

#  if defined(SIMDE_AVX_NATIVE)
#    include <immintrin.h>
#  endif

#  include <stdint.h>
#  include <limits.h>
#  include <string.h>

SIMDE__BEGIN_DECLS

typedef SIMDE__ALIGN(16) union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  int8_t          i8 __attribute__((__vector_size__(32), __may_alias__));
  int16_t        i16 __attribute__((__vector_size__(32), __may_alias__));
  int32_t        i32 __attribute__((__vector_size__(32), __may_alias__));
  int64_t        i64 __attribute__((__vector_size__(32), __may_alias__));
  uint8_t         u8 __attribute__((__vector_size__(32), __may_alias__));
  uint16_t       u16 __attribute__((__vector_size__(32), __may_alias__));
  uint32_t       u32 __attribute__((__vector_size__(32), __may_alias__));
  uint64_t       u64 __attribute__((__vector_size__(32), __may_alias__));
  #if defined(SIMDE__HAVE_INT128)
  simde_int128  i128 __attribute__((__vector_size__(32), __may_alias__));
  simde_uint128 u128 __attribute__((__vector_size__(32), __may_alias__));
  #endif
  simde_float32  f32 __attribute__((__vector_size__(32), __may_alias__));
  simde_float64  f64 __attribute__((__vector_size__(32), __may_alias__));
#else
  int8_t          i8[32];
  int16_t        i16[16];
  int32_t        i32[8];
  int64_t        i64[4];
  uint8_t         u8[32];
  uint16_t       u16[16];
  uint32_t       u32[8];
  uint64_t       u64[4];
  #if defined(SIMDE__HAVE_INT128)
  simde_int128  i128[2];
  simde_uint128 u128[2];
  #endif
  simde_float32  f32[8];
  simde_float64  f64[4];
#endif

#if defined(SIMDE_SSE_NATIVE)
  __m128         m128[2];
#endif
#if defined(SIMDE_AVX_NATIVE)
  __m256         n;
#endif
} simde__m256;

typedef SIMDE__ALIGN(16) union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  int8_t          i8 __attribute__((__vector_size__(32), __may_alias__));
  int16_t        i16 __attribute__((__vector_size__(32), __may_alias__));
  int32_t        i32 __attribute__((__vector_size__(32), __may_alias__));
  int64_t        i64 __attribute__((__vector_size__(32), __may_alias__));
  uint8_t         u8 __attribute__((__vector_size__(32), __may_alias__));
  uint16_t       u16 __attribute__((__vector_size__(32), __may_alias__));
  uint32_t       u32 __attribute__((__vector_size__(32), __may_alias__));
  uint64_t       u64 __attribute__((__vector_size__(32), __may_alias__));
  #if defined(SIMDE__HAVE_INT128)
  simde_int128  i128 __attribute__((__vector_size__(32), __may_alias__));
  simde_uint128 u128 __attribute__((__vector_size__(32), __may_alias__));
  #endif
  simde_float32  f32 __attribute__((__vector_size__(32), __may_alias__));
  simde_float64  f64 __attribute__((__vector_size__(32), __may_alias__));
#else
  int8_t          i8[32];
  int16_t        i16[16];
  int32_t        i32[8];
  int64_t        i64[4];
  uint8_t         u8[32];
  uint16_t       u16[16];
  uint32_t       u32[8];
  uint64_t       u64[4];
  #if defined(SIMDE__HAVE_INT128)
  simde_int128  i128[2];
  simde_uint128 u128[2];
  #endif
  simde_float32  f32[8];
  simde_float64  f64[4];
#endif

#if defined(SIMDE_SSE2_NATIVE)
  __m128d        m128d[2];
#endif
#if defined(SIMDE_AVX_NATIVE)
  __m256d        n;
#endif
} simde__m256d;

typedef SIMDE__ALIGN(16) union {
#if defined(SIMDE__ENABLE_GCC_VEC_EXT)
  int8_t          i8 __attribute__((__vector_size__(32), __may_alias__));
  int16_t        i16 __attribute__((__vector_size__(32), __may_alias__));
  int32_t        i32 __attribute__((__vector_size__(32), __may_alias__));
  int64_t        i64 __attribute__((__vector_size__(32), __may_alias__));
  uint8_t         u8 __attribute__((__vector_size__(32), __may_alias__));
  uint16_t       u16 __attribute__((__vector_size__(32), __may_alias__));
  uint32_t       u32 __attribute__((__vector_size__(32), __may_alias__));
  uint64_t       u64 __attribute__((__vector_size__(32), __may_alias__));
  #if defined(SIMDE__HAVE_INT128)
  simde_int128  i128 __attribute__((__vector_size__(32), __may_alias__));
  simde_uint128 u128 __attribute__((__vector_size__(32), __may_alias__));
  #endif
  simde_float32  f32 __attribute__((__vector_size__(32), __may_alias__));
  simde_float64  f64 __attribute__((__vector_size__(32), __may_alias__));
#else
  int8_t          i8[32];
  int16_t        i16[16];
  int32_t        i32[8];
  int64_t        i64[4];
  uint8_t         u8[32];
  uint16_t       u16[16];
  uint32_t       u32[8];
  uint64_t       u64[4];
  #if defined(SIMDE__HAVE_INT128)
  simde_int128  i128[2];
  simde_uint128 u128[2];
  #endif
  simde_float32  f32[8];
  simde_float64  f64[4];
#endif

#if defined(SIMDE_SSE2_NATIVE)
  __m128i        m128i[2];
#endif
#if defined(SIMDE_AVX_NATIVE)
  __m256i        n;
#endif
} simde__m256i;

#if defined(SIMDE_AVX_NATIVE)
  HEDLEY_STATIC_ASSERT(sizeof(__m256i) == sizeof(simde__m256i), "__m256i size doesn't match simde__m256i size");
  HEDLEY_STATIC_ASSERT(sizeof(__m256d) == sizeof(simde__m256d), "__m256d size doesn't match simde__m256d size");
  HEDLEY_STATIC_ASSERT( sizeof(__m256) ==  sizeof(simde__m256), "__m256 size doesn't match simde__m256 size");
  SIMDE__FUNCTION_ATTRIBUTES simde__m256i SIMDE__M256I_C(__m256i v) { simde__m256i r; r.n = v; return r; }
  SIMDE__FUNCTION_ATTRIBUTES simde__m256d SIMDE__M256D_C(__m256d v) { simde__m256d r; r.n = v; return r; }
  SIMDE__FUNCTION_ATTRIBUTES simde__m256 SIMDE__M256_C(__m256 v) { simde__m256 r; r.n = v; return r; }
#endif
HEDLEY_STATIC_ASSERT(32 == sizeof(simde__m256i), "simde__m256i size incorrect");
HEDLEY_STATIC_ASSERT(32 == sizeof(simde__m256d), "simde__m256d size incorrect");
HEDLEY_STATIC_ASSERT(32 ==  sizeof(simde__m256),  "simde__m256 size incorrect");

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_set_epi8 (int8_t e31, int8_t e30, int8_t e29, int8_t e28,
                      int8_t e27, int8_t e26, int8_t e25, int8_t e24,
                      int8_t e23, int8_t e22, int8_t e21, int8_t e20,
                      int8_t e19, int8_t e18, int8_t e17, int8_t e16,
                      int8_t e15, int8_t e14, int8_t e13, int8_t e12,
                      int8_t e11, int8_t e10, int8_t  e9, int8_t  e8,
                      int8_t  e7, int8_t  e6, int8_t  e5, int8_t  e4,
                      int8_t  e3, int8_t  e2, int8_t  e1, int8_t  e0) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256I_C(_mm256_set_epi8(e31, e30, e29, e28, e27, e26, e25, e24,
                                        e23, e22, e21, e20, e19, e18, e17, e16,
                                        e15, e14, e13, e12, e11, e10,  e9,  e8,
                                         e7,  e6,  e5,  e4,  e3,  e2,  e1,  e0));
#elif defined(SIMDE_SSE2_NATIVE)
  simde__m256i res;
  res.m128i[0] = _mm_set_epi8(e15, e14, e13, e12, e11, e10,  e9,  e8,
                               e7,  e6,  e5,  e4,  e3,  e2,  e1,  e0);
  res.m128i[1] = _mm_set_epi8(e31, e30, e29, e28, e27, e26, e25, e24,
                              e23, e22, e21, e20, e19, e18, e17, e16);
  return res;
#else
  simde__m256i r;
  r.i8[ 0] =  e0;
  r.i8[ 1] =  e1;
  r.i8[ 2] =  e2;
  r.i8[ 3] =  e3;
  r.i8[ 4] =  e4;
  r.i8[ 5] =  e5;
  r.i8[ 6] =  e6;
  r.i8[ 7] =  e7;
  r.i8[ 8] =  e8;
  r.i8[ 9] =  e9;
  r.i8[10] = e10;
  r.i8[11] = e11;
  r.i8[12] = e12;
  r.i8[13] = e13;
  r.i8[14] = e14;
  r.i8[15] = e15;
  r.i8[16] = e16;
  r.i8[17] = e17;
  r.i8[18] = e18;
  r.i8[19] = e19;
  r.i8[20] = e20;
  r.i8[20] = e20;
  r.i8[21] = e21;
  r.i8[22] = e22;
  r.i8[23] = e23;
  r.i8[24] = e24;
  r.i8[25] = e25;
  r.i8[26] = e26;
  r.i8[27] = e27;
  r.i8[28] = e28;
  r.i8[29] = e29;
  r.i8[30] = e30;
  r.i8[31] = e31;
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_set_epi16 (int16_t e15, int16_t e14, int16_t e13, int16_t e12,
                       int16_t e11, int16_t e10, int16_t  e9, int16_t  e8,
                       int16_t  e7, int16_t  e6, int16_t  e5, int16_t  e4,
                       int16_t  e3, int16_t  e2, int16_t  e1, int16_t  e0) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256I_C(_mm256_set_epi16(e15, e14, e13, e12, e11, e10,  e9,  e8,
                                          e7,  e6,  e5,  e4,  e3,  e2,  e1,  e0));
#elif defined(SIMDE_SSE2_NATIVE)
  simde__m256i res;
  res.m128i[0] = _mm_set_epi16( e7,  e6,  e5,  e4,  e3,  e2,  e1,  e0);
  res.m128i[1] = _mm_set_epi16(e15, e14, e13, e12, e11, e10,  e9,  e8);
  return res;
#else
  simde__m256i r;
  r.i16[ 0] =  e0;
  r.i16[ 1] =  e1;
  r.i16[ 2] =  e2;
  r.i16[ 3] =  e3;
  r.i16[ 4] =  e4;
  r.i16[ 5] =  e5;
  r.i16[ 6] =  e6;
  r.i16[ 7] =  e7;
  r.i16[ 8] =  e8;
  r.i16[ 9] =  e9;
  r.i16[10] = e10;
  r.i16[11] = e11;
  r.i16[12] = e12;
  r.i16[13] = e13;
  r.i16[14] = e14;
  r.i16[15] = e15;
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_set_epi32 (int32_t e7, int32_t e6, int32_t e5, int32_t e4,
                       int32_t e3, int32_t e2, int32_t e1, int32_t e0) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256I_C(_mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0));
#elif defined(SIMDE_SSE2_NATIVE)
  simde__m256i res;
  res.m128i[0] = _mm_set_epi32(e3, e2, e1, e0);
  res.m128i[1] = _mm_set_epi32(e7, e6, e5, e4);
  return res;
#else
  simde__m256i r;
  r.i32[0] = e0;
  r.i32[1] = e1;
  r.i32[2] = e2;
  r.i32[3] = e3;
  r.i32[4] = e4;
  r.i32[5] = e5;
  r.i32[6] = e6;
  r.i32[7] = e7;
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_set_epi64x (int64_t  e3, int64_t  e2, int64_t  e1, int64_t  e0) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256I_C(_mm256_set_epi64x(e3, e2, e1, e0));
#elif defined(SIMDE_SSE2_NATIVE)
  simde__m256i res;
  res.m128i[0] = _mm_set_epi64x(e1, e0);
  res.m128i[1] = _mm_set_epi64x(e3, e2);
  return res;
#else
  simde__m256i r;
  r.i64[0] = e0;
  r.i64[1] = e1;
  r.i64[2] = e2;
  r.i64[3] = e3;
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_x_mm256_set_epu8 (uint8_t e31, uint8_t e30, uint8_t e29, uint8_t e28,
                        uint8_t e27, uint8_t e26, uint8_t e25, uint8_t e24,
                        uint8_t e23, uint8_t e22, uint8_t e21, uint8_t e20,
                        uint8_t e19, uint8_t e18, uint8_t e17, uint8_t e16,
                        uint8_t e15, uint8_t e14, uint8_t e13, uint8_t e12,
                        uint8_t e11, uint8_t e10, uint8_t  e9, uint8_t  e8,
                        uint8_t  e7, uint8_t  e6, uint8_t  e5, uint8_t  e4,
                        uint8_t  e3, uint8_t  e2, uint8_t  e1, uint8_t  e0) {
  simde__m256i r;
  r.u8[ 0] =  e0;
  r.u8[ 1] =  e1;
  r.u8[ 2] =  e2;
  r.u8[ 3] =  e3;
  r.u8[ 4] =  e4;
  r.u8[ 5] =  e5;
  r.u8[ 6] =  e6;
  r.u8[ 7] =  e7;
  r.u8[ 8] =  e8;
  r.u8[ 9] =  e9;
  r.u8[10] = e10;
  r.u8[11] = e11;
  r.u8[12] = e12;
  r.u8[13] = e13;
  r.u8[14] = e14;
  r.u8[15] = e15;
  r.u8[16] = e16;
  r.u8[17] = e17;
  r.u8[18] = e18;
  r.u8[19] = e19;
  r.u8[20] = e20;
  r.u8[20] = e20;
  r.u8[21] = e21;
  r.u8[22] = e22;
  r.u8[23] = e23;
  r.u8[24] = e24;
  r.u8[25] = e25;
  r.u8[26] = e26;
  r.u8[27] = e27;
  r.u8[28] = e28;
  r.u8[29] = e29;
  r.u8[30] = e30;
  r.u8[31] = e31;
  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_set_ps (simde_float32 e7, simde_float32 e6, simde_float32 e5, simde_float32 e4,
                    simde_float32 e3, simde_float32 e2, simde_float32 e1, simde_float32 e0) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256_C(_mm256_set_ps(e7, e6, e5, e4, e3, e2, e1, e0));
#elif defined(SIMDE_SSE_NATIVE)
  simde__m256 res;
  res.m128[0] = _mm_set_ps(e3, e2, e1, e0);
  res.m128[1] = _mm_set_ps(e7, e6, e5, e4);
  return res;
#else
  simde__m256 r;
  r.f32[0] = e0;
  r.f32[1] = e1;
  r.f32[2] = e2;
  r.f32[3] = e3;
  r.f32[4] = e4;
  r.f32[5] = e5;
  r.f32[6] = e6;
  r.f32[7] = e7;
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_set_pd (simde_float64 e3, simde_float64 e2, simde_float64 e1, simde_float64 e0) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256D_C(_mm256_set_pd(e3, e2, e1, e0));
#elif defined(SIMDE_SSE2_NATIVE)
  simde__m256d res;
  res.m128d[0] = _mm_set_pd(e1, e0);
  res.m128d[1] = _mm_set_pd(e3, e2);
  return res;
#else
  simde__m256d r;
  r.f64[0] = e0;
  r.f64[1] = e1;
  r.f64[2] = e2;
  r.f64[3] = e3;
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_set_m128 (simde__m128 e1, simde__m128 e0) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_insertf128_ps(_mm256_castps128_ps256(e0.n), e1.n, 1);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128[0] = e0.n;
  r.m128[1] = e1.n;
#elif defined(SIMDE__HAVE_INT128)
  r.i128[0] = e0.i128[0];
  r.i128[1] = e1.i128[0];
#else
  r.i64[0] = e0.i64[0];
  r.i64[1] = e0.i64[1];
  r.i64[2] = e1.i64[0];
  r.i64[3] = e1.i64[1];
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_set_m128d (simde__m128d e1, simde__m128d e0) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_insertf128_pd(_mm256_castpd128_pd256(e0.n), e1.n, 1);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128d[0] = e0.n;
  r.m128d[1] = e1.n;
#else
  r.i64[0] = e0.i64[0];
  r.i64[1] = e0.i64[1];
  r.i64[2] = e1.i64[0];
  r.i64[3] = e1.i64[1];
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_set_m128i (simde__m128i e1, simde__m128i e0) {
  simde__m256i r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_insertf128_si256(_mm256_castsi128_si256(e0.n), e1.n, 1);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = e0.n;
  r.m128i[1] = e1.n;
#else
  r.i64[0] = e0.i64[0];
  r.i64[1] = e0.i64[1];
  r.i64[2] = e1.i64[0];
  r.i64[3] = e1.i64[1];
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_set1_epi8 (int8_t a) {
  simde__m256i r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_set1_epi8(a);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_set1_epi8(a);
  r.m128i[1] = _mm_set1_epi8(a);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a;
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_set1_epi16 (int16_t a) {
  simde__m256i r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_set1_epi16(a);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_set1_epi16(a);
  r.m128i[1] = _mm_set1_epi16(a);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = a;
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_set1_epi32 (int32_t a) {
  simde__m256i r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_set1_epi32(a);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_set1_epi32(a);
  r.m128i[1] = _mm_set1_epi32(a);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a;
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_set1_epi64x (int64_t a) {
  simde__m256i r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_set1_epi64x(a);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_set1_epi64x(a);
  r.m128i[1] = _mm_set1_epi64x(a);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a;
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_set1_ps (simde_float32 a) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_set1_ps(a);
#elif defined(SIMDE_SSE_NATIVE)
  r.m128[0] = _mm_set1_ps(a);
  r.m128[1] = _mm_set1_ps(a);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = a;
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_set1_pd (simde_float64 a) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_set1_pd(a);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128d[0] = _mm_set1_pd(a);
  r.m128d[1] = _mm_set1_pd(a);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = a;
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_add_ps (simde__m256 a, simde__m256 b) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_add_ps(a.n, b.n);
#elif defined(SIMDE_SSE_NATIVE)
  r.m128[0] = _mm_add_ps(a.m128[0], b.m128[0]);
  r.m128[1] = _mm_add_ps(a.m128[1], b.m128[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = a.f32[i] + b.f32[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_add_pd (simde__m256d a, simde__m256d b) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_add_pd(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128d[0] = _mm_add_pd(a.m128d[0], b.m128d[0]);
  r.m128d[1] = _mm_add_pd(a.m128d[1], b.m128d[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = a.f64[i] + b.f64[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_addsub_ps (simde__m256 a, simde__m256 b) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_addsub_ps(a.n, b.n);
#elif defined(SIMDE_SSE3_NATIVE)
  r.m128[0] = _mm_addsub_ps(a.m128[0], b.m128[0]);
  r.m128[1] = _mm_addsub_ps(a.m128[1], b.m128[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i += 2) {
    r.f32[  i  ] = a.f32[  i  ] - b.f32[  i  ];
    r.f32[i + 1] = a.f32[i + 1] + b.f32[i + 1];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_addsub_pd (simde__m256d a, simde__m256d b) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_addsub_pd(a.n, b.n);
#elif defined(SIMDE_SSE3_NATIVE)
  r.m128d[0] = _mm_addsub_pd(a.m128d[0], b.m128d[0]);
  r.m128d[1] = _mm_addsub_pd(a.m128d[1], b.m128d[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i += 2) {
    r.f64[  i  ] = a.f64[  i  ] - b.f64[  i  ];
    r.f64[i + 1] = a.f64[i + 1] + b.f64[i + 1];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_and_ps (simde__m256 a, simde__m256 b) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_and_ps(a.n, b.n);
#elif defined(SIMDE_SSE_NATIVE)
  r.m128[0] = _mm_and_ps(a.m128[0], b.m128[0]);
  r.m128[1] = _mm_and_ps(a.m128[1], b.m128[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.u32[i] = a.u32[i] & b.u32[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_and_pd (simde__m256d a, simde__m256d b) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_and_pd(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128d[0] = _mm_and_pd(a.m128d[0], b.m128d[0]);
  r.m128d[1] = _mm_and_pd(a.m128d[1], b.m128d[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.u64[i] = a.u64[i] & b.u64[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_andnot_ps (simde__m256 a, simde__m256 b) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_andnot_ps(a.n, b.n);
#elif defined(SIMDE_SSE_NATIVE)
  r.m128[0] = _mm_andnot_ps(a.m128[0], b.m128[0]);
  r.m128[1] = _mm_andnot_ps(a.m128[1], b.m128[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.u32[i] = a.u32[i] & b.u32[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_andnot_pd (simde__m256d a, simde__m256d b) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_andnot_pd(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128d[0] = _mm_andnot_pd(a.m128d[0], b.m128d[0]);
  r.m128d[1] = _mm_andnot_pd(a.m128d[1], b.m128d[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.u64[i] = a.u64[i] & b.u64[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_blend_ps (simde__m256 a, simde__m256 b, const int imm8) {
  simde__m256 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = ((imm8 >> i) & 1) ? b.f32[i] : a.f32[i];
  }
  return r;
}
#if defined(SIMDE_AVX_NATIVE)
#  define simde_mm256_blend_ps(a, b, imm8) SIMDE__M256_C(_mm256_blend_ps(a.n, b.n, imm8))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_blend_pd (simde__m256d a, simde__m256d b, const int imm8) {
  simde__m256d r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = ((imm8 >> i) & 1) ? b.f64[i] : a.f64[i];
  }
  return r;
}
#if defined(SIMDE_AVX_NATIVE)
#  define simde_mm256_blend_pd(a, b, imm8) SIMDE__M256D_C(_mm256_blend_pd(a.n, b.n, imm8))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_blendv_ps (simde__m256 a, simde__m256 b, simde__m256 mask) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_blendv_ps(a.n, b.n, mask.n);
#elif defined(SIMDE_SSE4_1_NATIVE)
  r.m128[0] = _mm_blendv_ps(a.m128[0], b.m128[0], mask.m128[0]);
  r.m128[1] = _mm_blendv_ps(a.m128[1], b.m128[1], mask.m128[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u32) / sizeof(r.u32[0])) ; i++) {
    r.f32[i] = (mask.u32[i] & (UINT32_C(1) << 31)) ? b.f32[i] : a.f32[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_blendv_pd (simde__m256d a, simde__m256d b, simde__m256d mask) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_blendv_pd(a.n, b.n, mask.n);
#elif defined(SIMDE_SSE4_1_NATIVE)
  r.m128d[0] = _mm_blendv_pd(a.m128d[0], b.m128d[0], mask.m128d[0]);
  r.m128d[1] = _mm_blendv_pd(a.m128d[1], b.m128d[1], mask.m128d[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.f64[i] = (mask.u64[i] & (UINT64_C(1) << 63)) ? b.f64[i] : a.f64[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_broadcast_pd (simde__m128d const * mem_addr) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_broadcast_pd(&(mem_addr->n));
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i += 2) {
    r.f64[  i  ] = mem_addr->f64[0];
    r.f64[i + 1] = mem_addr->f64[1];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_broadcast_ps (simde__m128 const * mem_addr) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_broadcast_ps(&(mem_addr->n));
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = mem_addr->f32[i & 3];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_broadcast_sd (simde_float64 const * a) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_broadcast_sd(a);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = *a;
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm_broadcast_ss (simde_float32 const * a) {
  simde__m128 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm_broadcast_ss(a);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = *a;
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_broadcast_ss (simde_float32 const * a) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_broadcast_ss(a);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = *a;
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_castpd_ps (simde__m256d a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256_C(_mm256_castpd_ps(a.n));
#else
  return *((simde__m256*) &a);
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_castpd_si256 (simde__m256d a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256I_C(_mm256_castpd_si256(a.n));
#else
  return *((simde__m256i*) &a);
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_castpd128_pd256 (simde__m128d a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256D_C(_mm256_castpd128_pd256(a.n));
#else
  simde__m256d r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < 2 ; i++) {
    r.i64[i] = a.i64[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128d
simde_mm256_castpd256_pd128 (simde__m256d a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M128D_C(_mm256_castpd256_pd128(a.n));
#else
  simde__m128d r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < 2 ; i++) {
    r.i64[i] = a.i64[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_castps_pd (simde__m256 a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256D_C(_mm256_castps_pd(a.n));
#else
  return *((simde__m256d*) &a);
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_castps_si256 (simde__m256 a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256I_C(_mm256_castps_si256(a.n));
#else
  return *((simde__m256i*) &a);
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_castps128_ps256 (simde__m128 a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256_C(_mm256_castps128_ps256(a.n));
#else
  simde__m256 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < 2 ; i++) {
    r.i64[i] = a.i64[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128
simde_mm256_castps256_ps128 (simde__m256 a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M128_C(_mm256_castps256_ps128(a.n));
#else
  simde__m128 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < 2 ; i++) {
    r.i64[i] = a.i64[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_castsi128_si256 (simde__m128i a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256I_C(_mm256_castsi128_si256(a.n));
#else
  simde__m256i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < 2 ; i++) {
    r.i64[i] = a.i64[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm256_castsi256_si128 (simde__m256i a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M128I_C(_mm256_castsi256_si128(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < 2 ; i++) {
    r.i64[i] = a.i64[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_castsi256_pd (simde__m256i a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256D_C(_mm256_castsi256_pd(a.n));
#else
  return *((simde__m256d*) &a);
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_castsi256_ps (simde__m256i a) {
#if defined(SIMDE_AVX_NATIVE)
  return SIMDE__M256_C(_mm256_castsi256_ps(a.n));
#else
  return *((simde__m256*) &a);
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_ceil_pd (simde__m256d a) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_ceil_pd(a.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = ceil(a.f64[i]);
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_ceil_ps (simde__m256 a) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_ceil_ps(a.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = ceilf(a.f32[i]);
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_cvtepi32_pd (simde__m128i a) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_cvtepi32_pd(a.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = (simde_float64) a.i32[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_cvtepi32_ps (simde__m256i a) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_cvtepi32_ps(a.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = (simde_float32) a.i32[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_div_ps (simde__m256 a, simde__m256 b) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_div_ps(a.n, b.n);
#elif defined(SIMDE_SSE_NATIVE)
  r.m128[0] = _mm_div_ps(a.m128[0], b.m128[0]);
  r.m128[1] = _mm_div_ps(a.m128[1], b.m128[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = a.f32[i] / b.f32[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_div_pd (simde__m256d a, simde__m256d b) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_div_pd(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128d[0] = _mm_div_pd(a.m128d[0], b.m128d[0]);
  r.m128d[1] = _mm_div_pd(a.m128d[1], b.m128d[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = a.f64[i] / b.f64[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_floor_pd (simde__m256d a) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_floor_pd(a.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f64) / sizeof(r.f64[0])) ; i++) {
    r.f64[i] = floor(a.f64[i]);
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_floor_ps (simde__m256 a) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_floor_ps(a.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.f32) / sizeof(r.f32[0])) ; i++) {
    r.f32[i] = floorf(a.f32[i]);
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_hadd_ps (simde__m256 a, simde__m256 b) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_hadd_ps(a.n, b.n);
#else
  r.f32[0] = a.f32[0] + a.f32[1];
  r.f32[1] = a.f32[2] + a.f32[3];
  r.f32[2] = b.f32[0] + b.f32[1];
  r.f32[3] = b.f32[2] + b.f32[3];
  r.f32[4] = a.f32[4] + a.f32[5];
  r.f32[5] = a.f32[6] + a.f32[7];
  r.f32[6] = b.f32[4] + b.f32[5];
  r.f32[7] = b.f32[6] + b.f32[7];
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_hadd_pd (simde__m256d a, simde__m256d b) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_hadd_pd(a.n, b.n);
#else
  r.f64[0] = a.f64[0] + a.f64[1];
  r.f64[1] = b.f64[0] + b.f64[1];
  r.f64[2] = a.f64[2] + a.f64[3];
  r.f64[3] = b.f64[2] + b.f64[3];
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256
simde_mm256_hsub_ps (simde__m256 a, simde__m256 b) {
  simde__m256 r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_hsub_ps(a.n, b.n);
#else
  r.f32[0] = a.f32[0] - a.f32[1];
  r.f32[1] = a.f32[2] - a.f32[3];
  r.f32[2] = b.f32[0] - b.f32[1];
  r.f32[3] = b.f32[2] - b.f32[3];
  r.f32[4] = a.f32[4] - a.f32[5];
  r.f32[5] = a.f32[6] - a.f32[7];
  r.f32[6] = b.f32[4] - b.f32[5];
  r.f32[7] = b.f32[6] - b.f32[7];
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256d
simde_mm256_hsub_pd (simde__m256d a, simde__m256d b) {
  simde__m256d r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_hsub_pd(a.n, b.n);
#else
  r.f64[0] = a.f64[0] - a.f64[1];
  r.f64[1] = b.f64[0] - b.f64[1];
  r.f64[2] = a.f64[2] - a.f64[3];
  r.f64[3] = b.f64[2] - b.f64[3];
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_loadu_si256 (simde__m256i const * a) {
  simde__m256i r;

#if defined(SIMDE_AVX_NATIVE)
  r.n = _mm256_loadu_si256(&(a->n));
#else
  memcpy(&(r.i64), &(a->i64), sizeof(r.i64));
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
void
simde_mm256_storeu_si256(simde__m256i * mem_addr, simde__m256i a) {
  memcpy(mem_addr, &a, sizeof(a));
}

SIMDE__END_DECLS

#endif /* !defined(SIMDE__AVX_H) */
