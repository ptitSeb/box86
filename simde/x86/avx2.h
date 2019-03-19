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

#if !defined(SIMDE__AVX2_H)
#  if !defined(SIMDE__AVX2_H)
#    define SIMDE__AVX2_H
#  endif
#  include "avx.h"

#  if defined(SIMDE_AVX2_NATIVE)
#    undef SIMDE_AVX2_NATIVE
#  endif
#  if defined(SIMDE_AVX2_FORCE_NATIVE)
#    define SIMDE_AVX2_NATIVE
#  elif defined(__AVX2__) && !defined(SIMDE_AVX2_NO_NATIVE) && !defined(SIMDE_NO_NATIVE)
#    define SIMDE_AVX2_NATIVE
#  elif defined(__ARM_NEON) && !defined(SIMDE_AVX2_NO_NEON) && !defined(SIMDE_NO_NEON)
#    define SIMDE_AVX2_NEON
#  endif

#  if defined(SIMDE_AVX2_NATIVE) && !defined(SIMDE_AVX_NATIVE)
#    if defined(SIMDE_AVX2_FORCE_NATIVE)
#      error Native AVX2 support requires native AVX support
#    else
#      warning Native AVX2 support requires native AVX support, disabling
#      undef SIMDE_AVX2_NATIVE
#    endif
#  elif defined(SIMDE_AVX2_NEON) && !defined(SIMDE_AVX_NEON)
#    warning AVX2 NEON support requires AVX NEON support, disabling
#    undef SIMDE_AVX_NEON
#  endif

#  if defined(SIMDE_AVX2_NATIVE)
#    include <immintrin.h>
#  endif

#  include <stdint.h>

SIMDE__BEGIN_DECLS

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_add_epi8 (simde__m256i a, simde__m256i b) {
  simde__m256i r;

#if defined(SIMDE_AVX2_NATIVE)
  r.n = _mm256_add_epi8(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_add_epi8(a.m128i[0], b.m128i[0]);
  r.m128i[1] = _mm_add_epi8(a.m128i[1], b.m128i[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = a.i8[i] + b.i8[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_add_epi16 (simde__m256i a, simde__m256i b) {
  simde__m256i r;

#if defined(SIMDE_AVX2_NATIVE)
  r.n = _mm256_add_epi16(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_add_epi16(a.m128i[0], b.m128i[0]);
  r.m128i[1] = _mm_add_epi16(a.m128i[1], b.m128i[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = a.i16[i] + b.i16[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_add_epi32 (simde__m256i a, simde__m256i b) {
  simde__m256i r;

#if defined(SIMDE_AVX2_NATIVE)
  r.n = _mm256_add_epi32(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_add_epi32(a.m128i[0], b.m128i[0]);
  r.m128i[1] = _mm_add_epi32(a.m128i[1], b.m128i[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = a.i32[i] + b.i32[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_add_epi64 (simde__m256i a, simde__m256i b) {
  simde__m256i r;

#if defined(SIMDE_AVX2_NATIVE)
  r.n = _mm256_add_epi64(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_add_epi64(a.m128i[0], b.m128i[0]);
  r.m128i[1] = _mm_add_epi64(a.m128i[1], b.m128i[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i64[i] + b.i64[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_and_si256 (simde__m256i a, simde__m256i b) {
  simde__m256i r;

#if defined(SIMDE_AVX2_NATIVE)
  r.n = _mm256_and_si256(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_and_si128(a.m128i[0], b.m128i[0]);
  r.m128i[1] = _mm_and_si128(a.m128i[1], b.m128i[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i64[i] & b.i64[i];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_broadcastsi128_si256 (simde__m128i a) {
  simde__m256i r;

#if defined(SIMDE_AVX2_NATIVE)
  return SIMDE__M256I_C(_mm256_broadcastsi128_si256(a.n));
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = a.n;
  r.m128i[1] = a.n;
#else
  r.i64[0] = a.i64[0];
  r.i64[1] = a.i64[1];
  r.i64[2] = a.i64[0];
  r.i64[3] = a.i64[1];
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_shuffle_epi8 (simde__m256i a, simde__m256i b) {
  simde__m256i r;

#if defined(SIMDE_AVX2_NATIVE)
  r.n = _mm256_shuffle_epi8(a.n, b.n);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < ((sizeof(r.u8) / sizeof(r.u8[0])) / 2) ; i++) {
    r.u8[  i   ] = (b.u8[  i   ] & 0x80) ? 0 : a.u8[(b.u8[  i   ] & 0x0f)     ];
    r.u8[i + 16] = (b.u8[i + 16] & 0x80) ? 0 : a.u8[(b.u8[i + 16] & 0x0f) + 16];
  }
#endif

  return r;
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_srli_epi64 (simde__m256i a, const int imm8) {
  simde__m256i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.u64) / sizeof(r.u64[0])) ; i++) {
    r.u64[i] = a.u64[i] >> imm8;
  }
  return r;
}
#if defined(SIMDE_AVX2_NATIVE)
#  define simde_mm256_srli_epi64(a, imm8) SIMDE__M256I_C(_mm256_srli_epi64(a.n, imm8))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_xor_si256 (simde__m256i a, simde__m256i b) {
  simde__m256i r;

#if defined(SIMDE_AVX2_NATIVE)
  r.n = _mm256_xor_si256(a.n, b.n);
#elif defined(SIMDE_SSE2_NATIVE)
  r.m128i[0] = _mm_xor_si128(a.m128i[0], b.m128i[0]);
  r.m128i[1] = _mm_xor_si128(a.m128i[1], b.m128i[1]);
#else
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i64) / sizeof(r.i64[0])) ; i++) {
    r.i64[i] = a.i64[i] ^ b.i64[i];
  }
#endif

  return r;
}

SIMDE__END_DECLS

#endif /* !defined(SIMDE__AVX2_H) */
