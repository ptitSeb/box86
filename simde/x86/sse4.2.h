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

#if !defined(SIMDE__SSE4_2_H)
#  if !defined(SIMDE__SSE4_2_H)
#    define SIMDE__SSE4_2_H
#  endif
#  include "sse4.1.h"

#  if defined(SIMDE_SSE4_2_NATIVE)
#    undef SIMDE_SSE4_2_NATIVE
#  endif
#  if defined(SIMDE_SSE4_2_FORCE_NATIVE)
#    define SIMDE_SSE4_2_NATIVE
#  elif defined(__SSE4_2__) && !defined(SIMDE_SSE4_2_NO_NATIVE) && !defined(SIMDE_NO_NATIVE)
#    define SIMDE_SSE4_2_NATIVE
#  elif defined(__ARM_NEON) && !defined(SIMDE_SSE4_2_NO_NEON) && !defined(SIMDE_NO_NEON)
#    define SIMDE_SSE4_1_NEON
#  endif

#  if defined(SIMDE_SSE4_2_NATIVE) && !defined(SIMDE_SSE4_1_NATIVE)
#    if defined(SIMDE_SSE4_2_FORCE_NATIVE)
#      error Native SSE4.2 support requires native SSE4.1 support
#    else
#      warning Native SSE4.2 support requires native SSE4.1 support, disabling
#      undef SIMDE_SSE4_2_NATIVE
#    endif
#  elif defined(SIMDE_SSE4_2_NEON) && !defined(SIMDE_SSE4_1_NEON)
#    warning SSE4.2 NEON support requires SSE4.1 NEON support, disabling
#    undef SIMDE_SSE4_2_NEON
#  endif

#  if defined(SIMDE_SSE4_2_NATIVE)
#    include <nmmintrin.h>
#  else
#    if defined(SIMDE_SSE4_1_NEON)
#      include <arm_neon.h>
#    endif
#  endif

SIMDE__BEGIN_DECLS

SIMDE__END_DECLS

#endif /* !defined(SIMDE__SSE4_2_H) */
