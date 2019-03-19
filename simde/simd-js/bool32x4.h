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
#  error bool32x4.h must not be included directly; use <simde/simde-js/simde-js.h> instead.
#endif

#if !defined(SIMDE__EM_BOOL32X4_H)
#define SIMDE__EM_BOOL32X4_H

SIMDE__BEGIN_DECLS

SIMDE__FUNCTION_ATTRIBUTES
simde_em_bool32x4
simde_x_em_bool32x4_set (_Bool s0, _Bool s1, _Bool s2, _Bool s3) {
#if defined(SIMDE_EM_NATIVE)
  int32x4 a = emscripten_int32x4_set(s0, s1, s2, s3);
  int32x4 b = emscripten_int32x4_splat(0);
  return SIMDE_EM_BOOL32X4_C(emscripten_int32x4_notEqual(a, b));
#elif defined(SIMDE_EM_SSE2)
  __m128i a = _mm_set_epi32(s3, s2, s1, s0);
  __m128i b = _mm_set1_epi32(0);
  return SIMDE_EM_BOOL32X4_SSE_C(_mm_andnot_si128(_mm_cmpeq_epi32(a, b), _mm_set1_epi32(~INT32_C(0))));
#elif defined(SIMDE_EM_NEON)
  SIMDE__ALIGN(16) int32_t data[4] = { s0, s1, s2, s3 };
  return SIMDE_EM_BOOL32X4_NEON_UC(vmvnq_u32(vceqq_s32(vld1q_s32(data), vdupq_n_s32(0))));
#else
  return (simde_em_bool32x4) {
    .v = {
      s0 ? ~INT32_C(0) : 0,
      s1 ? ~INT32_C(0) : 0,
      s2 ? ~INT32_C(0) : 0,
      s3 ? ~INT32_C(0) : 0
    }
  };
#endif
}

SIMDE__END_DECLS

#endif /* !defined(SIMDE__EM_BOOL32X4_H) */
