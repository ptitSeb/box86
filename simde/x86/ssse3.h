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

#if !defined(SIMDE__SSSE3_H)
#  if !defined(SIMDE__SSSE3_H)
#    define SIMDE__SSSE3_H
#  endif
#  include "sse3.h"

#  if defined(SIMDE_SSSE3_NATIVE)
#    undef SIMDE_SSSE3_NATIVE
#  endif
#  if defined(SIMDE_SSSE3_FORCE_NATIVE)
#    define SIMDE_SSSE3_NATIVE
#  elif defined(__SSSE3__) && !defined(SIMDE_SSSE3_NO_NATIVE) && !defined(SIMDE_NO_NATIVE)
#    define SIMDE_SSSE3_NATIVE
#  elif defined(__ARM_NEON) && !defined(SIMDE_SSSE3_NO_NEON) && !defined(SIMDE_NO_NEON)
#    define SIMDE_SSSE3_NEON
#  endif

#  if defined(SIMDE_SSSE3_NATIVE) && !defined(SIMDE_SSE3_NATIVE)
#    if defined(SIMDE_SSSE3_FORCE_NATIVE)
#      error Native SSSE3 support requires native SSE3 support
#    else
#      warning Native SSSE3 support requires native SSE3 support, disabling
#      undef SIMDE_SSSE3_NATIVE
#    endif
#  elif defined(SIMDE_SSSE3_NEON) && !defined(SIMDE_SSE3_NEON)
#    warning SSSE3 NEON support requires SSE3 NEON support, disabling
#    undef SIMDE_SSSE3_NEON
#  endif

#  if defined(SIMDE_SSSE3_NATIVE)
#    include <tmmintrin.h>
#  else
#    if defined(SIMDE_SSSE3_NEON)
#      include <arm_neon.h>
#    endif
#  endif

SIMDE__BEGIN_DECLS

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_abs_epi8 (simde__m128i a) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_abs_epi8(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.u8[i] = (a.i8[i] < 0) ? (- a.i8[i]) : a.i8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_abs_epi16 (simde__m128i a) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_abs_epi16(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.u16[i] = (a.i16[i] < 0) ? (- a.i16[i]) : a.i16[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_abs_epi32 (simde__m128i a) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_abs_epi32(a.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.u32[i] = (a.i32[i] < 0) ? (- a.i32[i]) : a.i32[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_abs_pi8 (simde__m64 a) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_abs_pi8(a.n));
#else
  simde__m64 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.u8[i] = (a.i8[i] < 0) ? (- a.i8[i]) : a.i8[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_abs_pi16 (simde__m64 a) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_abs_pi16(a.n));
#else
  simde__m64 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.u16[i] = (a.i16[i] < 0) ? (- a.i16[i]) : a.i16[i];
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_abs_pi32 (simde__m64 a) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_abs_pi32(a.n));
#else
  simde__m64 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.u32[i] = (a.i32[i] < 0) ? (- a.i32[i]) : a.i32[i];
  }
  return r;
#endif
}

#if defined(simde_mm_alignr_epi8)
#  undef simde_mm_alignr_epi8
#endif
SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_alignr_epi8 (simde__m128i a, simde__m128i b, int count) {
  simde__m128i r;
  const int bits = (8 * count) % 64;
  const int eo = count / 8;

  switch (eo) {
    case 0:
      r.u64[0]  = b.u64[0] >> bits;
      r.u64[0] |= b.u64[1] << (64 - bits);
      r.u64[1]  = b.u64[1] >> bits;
      r.u64[1] |= a.u64[0] << (64 - bits);
      break;
    case 1:
      r.u64[0]  = b.u64[1] >> bits;
      r.u64[0] |= a.u64[0] << (64 - bits);
      r.u64[1]  = a.u64[0] >> bits;
      r.u64[1] |= a.u64[1] << (64 - bits);
      break;
    case 2:
      r.u64[0]  = a.u64[0] >> bits;
      r.u64[0] |= a.u64[1] << (64 - bits);
      r.u64[1]  = a.u64[1] >> bits;
      break;
    case 3:
      r.u64[0]  = a.u64[1] >> bits;
      r.u64[1]  = 0;
      break;
    default:
      HEDLEY_UNREACHABLE();
      break;
  }

  return r;
}
#if defined(SIMDE_SSSE3_NATIVE)
#  define simde_mm_alignr_epi8(a, b, count) SIMDE__M128I_C(_mm_alignr_epi8(a.n, b.n, count))
#endif

#if defined(simde_mm_alignr_pi8)
#  undef simde_mm_alignr_pi8
#endif
SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_alignr_pi8 (simde__m64 a, simde__m64 b, const int count) {
  simde__m64 r;
#if defined(__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) && defined(__SIZEOF_INT128__)
  unsigned __int128 t = a.u64[0];
  t <<= 64;
  t |= b.u64[0];
  t >>= count * 8;
  r.u64[0] = (uint64_t) t;
#else
  const int cb = count * 8;

  if (cb > 64) {
    r.u64[0] = a.u64[0] >> (cb - 64);
  } else {
    r.u64[0] = (a.u64[0] << (64 - cb)) | (b.u64[0] >> cb);
  }
#endif
  return r;
}
#if defined(SIMDE_SSSE3_NATIVE)
#  define simde_mm_alignr_pi8(a, b, count) SIMDE__M64_C(_mm_alignr_pi8(a.n, b.n, count))
#endif

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_shuffle_epi8 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_shuffle_epi8(a.n, b.n));
#else
  simde__m128i r;
  for (size_t i = 0 ; i < (sizeof(r.u8) / sizeof(r.u8[0])) ; i++) {
    r.u8[i] = a.u8[b.u8[i] & 15] * ((~(b.u8[i]) >> 7) & 1);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_shuffle_pi8 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_shuffle_pi8(a.n, b.n));
#else
  simde__m64 r;
  for (size_t i = 0 ; i < (sizeof(r.u8) / sizeof(r.u8[0])) ; i++) {
    r.u8[i] = a.u8[b.u8[i] & 7] * ((~(b.u8[i]) >> 7) & 1);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_hadd_epi16 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_hadd_epi16(a.n, b.n));
#else
  simde__m128i r;

  r.i16[0] = a.i16[0] + a.i16[1];
  r.i16[1] = a.i16[2] + a.i16[3];
  r.i16[2] = a.i16[4] + a.i16[5];
  r.i16[3] = a.i16[6] + a.i16[7];
  r.i16[4] = b.i16[0] + b.i16[1];
  r.i16[5] = b.i16[2] + b.i16[3];
  r.i16[6] = b.i16[4] + b.i16[5];
  r.i16[7] = b.i16[6] + b.i16[7];

  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_hadd_epi32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_hadd_epi32(a.n, b.n));
#else
  simde__m128i r;

  r.i32[0] = a.i32[0] + a.i32[1];
  r.i32[1] = a.i32[2] + a.i32[3];
  r.i32[2] = b.i32[0] + b.i32[1];
  r.i32[3] = b.i32[2] + b.i32[3];

  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_hadd_pi16 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_hadd_pi16(a.n, b.n));
#else
  simde__m64 r;

  r.i16[0] = a.i16[0] + a.i16[1];
  r.i16[1] = a.i16[2] + a.i16[3];
  r.i16[2] = b.i16[0] + b.i16[1];
  r.i16[3] = b.i16[2] + b.i16[3];

  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_hadd_pi32 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_hadd_pi32(a.n, b.n));
#else
  simde__m64 r;

  r.i32[0] = a.i32[0] + a.i32[1];
  r.i32[1] = b.i32[0] + b.i32[1];

  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_hadds_epi16 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_hadds_epi16(a.n, b.n));
#else
  simde__m128i r;
  for (size_t i = 0 ; i < ((sizeof(r.i16) / sizeof(r.i16[0])) / 2) ; i++) {
    int32_t ta = ((int32_t) a.i16[i * 2]) + ((int32_t) a.i16[(i * 2) + 1]);
    r.i16[  i  ] = HEDLEY_LIKELY(ta > INT16_MIN) ? (HEDLEY_LIKELY(ta < INT16_MAX) ? ((int16_t) ta) : INT16_MAX) : INT16_MIN;
    int32_t tb = ((int32_t) b.i16[i * 2]) + ((int32_t) b.i16[(i * 2) + 1]);
    r.i16[i + 4] = HEDLEY_LIKELY(tb > INT16_MIN) ? (HEDLEY_LIKELY(tb < INT16_MAX) ? ((int16_t) tb) : INT16_MAX) : INT16_MIN;
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_hadds_pi16 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_hadds_pi16(a.n, b.n));
#else
  simde__m64 r;
  for (size_t i = 0 ; i < ((sizeof(r.i16) / sizeof(r.i16[0])) / 2) ; i++) {
    int32_t ta = ((int32_t) a.i16[i * 2]) + ((int32_t) a.i16[(i * 2) + 1]);
    r.i16[  i  ] = HEDLEY_LIKELY(ta > INT16_MIN) ? (HEDLEY_LIKELY(ta < INT16_MAX) ? ((int16_t) ta) : INT16_MAX) : INT16_MIN;
    int32_t tb = ((int32_t) b.i16[i * 2]) + ((int32_t) b.i16[(i * 2) + 1]);
    r.i16[i + 2] = HEDLEY_LIKELY(tb > INT16_MIN) ? (HEDLEY_LIKELY(tb < INT16_MAX) ? ((int16_t) tb) : INT16_MAX) : INT16_MIN;
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_hsub_epi16 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_hsub_epi16(a.n, b.n));
#else
  simde__m128i r;

  r.i16[0] = a.i16[0] - a.i16[1];
  r.i16[1] = a.i16[2] - a.i16[3];
  r.i16[2] = a.i16[4] - a.i16[5];
  r.i16[3] = a.i16[6] - a.i16[7];
  r.i16[4] = b.i16[0] - b.i16[1];
  r.i16[5] = b.i16[2] - b.i16[3];
  r.i16[6] = b.i16[4] - b.i16[5];
  r.i16[7] = b.i16[6] - b.i16[7];

  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_hsub_epi32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_hsub_epi32(a.n, b.n));
#else
  simde__m128i r;

  r.i32[0] = a.i32[0] - a.i32[1];
  r.i32[1] = a.i32[2] - a.i32[3];
  r.i32[2] = b.i32[0] - b.i32[1];
  r.i32[3] = b.i32[2] - b.i32[3];

  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_hsub_pi16 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_hsub_pi16(a.n, b.n));
#else
  simde__m64 r;

  r.i16[0] = a.i16[0] - a.i16[1];
  r.i16[1] = a.i16[2] - a.i16[3];
  r.i16[2] = b.i16[0] - b.i16[1];
  r.i16[3] = b.i16[2] - b.i16[3];

  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_hsub_pi32 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_hsub_pi32(a.n, b.n));
#else
  simde__m64 r;

  r.i32[0] = a.i32[0] - a.i32[1];
  r.i32[1] = b.i32[0] - b.i32[1];

  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_hsubs_epi16 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_hsubs_epi16(a.n, b.n));
#else
  simde__m128i r;
  for (size_t i = 0 ; i < ((sizeof(r.i16) / sizeof(r.i16[0])) / 2) ; i++) {
    int32_t ta = ((int32_t) a.i16[i * 2]) - ((int32_t) a.i16[(i * 2) + 1]);
    r.i16[  i  ] = HEDLEY_LIKELY(ta > INT16_MIN) ? (HEDLEY_LIKELY(ta < INT16_MAX) ? ((int16_t) ta) : INT16_MAX) : INT16_MIN;
    int32_t tb = ((int32_t) b.i16[i * 2]) - ((int32_t) b.i16[(i * 2) + 1]);
    r.i16[i + 4] = HEDLEY_LIKELY(tb > INT16_MIN) ? (HEDLEY_LIKELY(tb < INT16_MAX) ? ((int16_t) tb) : INT16_MAX) : INT16_MIN;
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_hsubs_pi16 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_hsubs_pi16(a.n, b.n));
#else
  simde__m64 r;
  for (size_t i = 0 ; i < ((sizeof(r.i16) / sizeof(r.i16[0])) / 2) ; i++) {
    int32_t ta = ((int32_t) a.i16[i * 2]) - ((int32_t) a.i16[(i * 2) + 1]);
    r.i16[  i  ] = HEDLEY_LIKELY(ta > INT16_MIN) ? (HEDLEY_LIKELY(ta < INT16_MAX) ? ((int16_t) ta) : INT16_MAX) : INT16_MIN;
    int32_t tb = ((int32_t) b.i16[i * 2]) - ((int32_t) b.i16[(i * 2) + 1]);
    r.i16[i + 2] = HEDLEY_LIKELY(tb > INT16_MIN) ? (HEDLEY_LIKELY(tb < INT16_MAX) ? ((int16_t) tb) : INT16_MAX) : INT16_MIN;
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_maddubs_epi16 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_maddubs_epi16(a.n, b.n));
#else
  simde__m128i r;
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    const int idx = i * 2;
    int32_t ts =
      (((int16_t) a.u8[  idx  ]) * ((int16_t) b.i8[  idx  ])) +
      (((int16_t) a.u8[idx + 1]) * ((int16_t) b.i8[idx + 1]));
    r.i16[i] = HEDLEY_LIKELY(ts > INT16_MIN) ? (HEDLEY_LIKELY(ts < INT16_MAX) ? ((int16_t) ts) : INT16_MAX) : INT16_MIN;
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_maddubs_pi16 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_maddubs_pi16(a.n, b.n));
#else
  simde__m64 r;
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    const int idx = i * 2;
    int32_t ts =
      (((int16_t) a.u8[  idx  ]) * ((int16_t) b.i8[  idx  ])) +
      (((int16_t) a.u8[idx + 1]) * ((int16_t) b.i8[idx + 1]));
    r.i16[i] = HEDLEY_LIKELY(ts > INT16_MIN) ? (HEDLEY_LIKELY(ts < INT16_MAX) ? ((int16_t) ts) : INT16_MAX) : INT16_MIN;
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_mulhrs_epi16 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_mulhrs_epi16(a.n, b.n));
#else
  simde__m128i r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = (int16_t) (((((int32_t) a.i16[i]) * ((int32_t) b.i16[i])) + 0x4000) >> 15);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_mulhrs_pi16 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_mulhrs_pi16(a.n, b.n));
#else
  simde__m64 r;
  SIMDE__VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = (int16_t) (((((int32_t) a.i16[i]) * ((int32_t) b.i16[i])) + 0x4000) >> 15);
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_sign_epi8 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_sign_epi8(a.n, b.n));
#else
  simde__m128i r;
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = (b.i8[i] < 0) ? (- a.i8[i]) : ((b.i8[i] > 0) ? (a.i8[i]) : INT8_C(0));
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_sign_epi16 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_sign_epi16(a.n, b.n));
#else
  simde__m128i r;
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = (b.i16[i] < 0) ? (- a.i16[i]) : ((b.i16[i] > 0) ? (a.i16[i]) : INT16_C(0));
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m128i
simde_mm_sign_epi32 (simde__m128i a, simde__m128i b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M128I_C(_mm_sign_epi32(a.n, b.n));
#else
  simde__m128i r;
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = (b.i32[i] < 0) ? (- a.i32[i]) : ((b.i32[i] > 0) ? (a.i32[i]) : INT32_C(0));
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_sign_pi8 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_sign_pi8(a.n, b.n));
#else
  simde__m64 r;
  for (size_t i = 0 ; i < (sizeof(r.i8) / sizeof(r.i8[0])) ; i++) {
    r.i8[i] = (b.i8[i] < 0) ? (- a.i8[i]) : ((b.i8[i] > 0) ? (a.i8[i]) : INT8_C(0));
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_sign_pi16 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_sign_pi16(a.n, b.n));
#else
  simde__m64 r;
  for (size_t i = 0 ; i < (sizeof(r.i16) / sizeof(r.i16[0])) ; i++) {
    r.i16[i] = (b.i16[i] < 0) ? (- a.i16[i]) : ((b.i16[i] > 0) ? (a.i16[i]) : INT16_C(0));
  }
  return r;
#endif
}

SIMDE__FUNCTION_ATTRIBUTES
simde__m64
simde_mm_sign_pi32 (simde__m64 a, simde__m64 b) {
#if defined(SIMDE_SSSE3_NATIVE)
  return SIMDE__M64_C(_mm_sign_pi32(a.n, b.n));
#else
  simde__m64 r;
  for (size_t i = 0 ; i < (sizeof(r.i32) / sizeof(r.i32[0])) ; i++) {
    r.i32[i] = (b.i32[i] < 0) ? (- a.i32[i]) : ((b.i32[i] > 0) ? (a.i32[i]) : INT32_C(0));
  }
  return r;
#endif
}

SIMDE__END_DECLS

#endif /* !defined(SIMDE__SSE2_H) */
