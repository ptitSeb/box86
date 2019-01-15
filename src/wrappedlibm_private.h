#if defined(GO) && defined(GOM) && defined(GO2) && defined(DATA)

GOW(acos, dFd)
GOW(acosf, fFf)
// __acosf_finite
// __acos_finite
// acosh    // Weak
// acoshf   // Weak
// __acoshf_finite
// __acosh_finite
// acoshl   // Weak
// acosl    // Weak
GOW(asin, dFd)
GOW(asinf, fFf)
// __asinf_finite
// __asin_finite
// asinh    // Weak
// asinhf   // Weak
// asinhl   // Weak
// asinl    // Weak
GO(atan, dFd)
GOW(atan2, dFdd)
GOW(atan2f, fFff)
GO(__atan2f_finite, fFff)
GO(__atan2_finite, dFdd)
// atan2l   // Weak
GOW(atanf, fFf)
// atanh    // Weak
// atanhf   // Weak
// __atanhf_finite
// __atanh_finite
// atanhl   // Weak
// atanl    // Weak
// cabs // Weak
// cabsf    // Weak
// cabsl    // Weak
// cacos    // Weak
// cacosf   // Weak
// cacosh   // Weak
// cacoshf  // Weak
// cacoshl  // Weak
// cacosl   // Weak
// carg // Weak
// cargf    // Weak
// cargl    // Weak
// casin    // Weak
// casinf   // Weak
// casinh   // Weak
// casinhf  // Weak
// casinhl  // Weak
// casinl   // Weak
// catan    // Weak
// catanf   // Weak
// catanh   // Weak
// catanhf  // Weak
// catanhl  // Weak
// catanl   // Weak
// cbrt // Weak
// cbrtf    // Weak
// cbrtl    // Weak
// ccos // Weak
// ccosf    // Weak
// ccosh    // Weak
// ccoshf   // Weak
// ccoshl   // Weak
// ccosl    // Weak
// ceil // Weak
// ceilf    // Weak
// ceill    // Weak
// cexp // Weak
// cexpf    // Weak
// cexpl    // Weak
// cimag    // Weak
// cimagf   // Weak
// cimagl   // Weak
// clog // Weak
// clog10   // Weak
// __clog10
// clog10f  // Weak
// __clog10f
// clog10l  // Weak
// __clog10l
// clogf    // Weak
// clogl    // Weak
// conj // Weak
// conjf    // Weak
// conjl    // Weak
// copysign // Weak
// copysignf    // Weak
// copysignl    // Weak
GOW(cos, dFd)
GOW(cosf, fFf)
// cosh // Weak
// coshf    // Weak
// __coshf_finite
// __cosh_finite
// coshl    // Weak
// cosl // Weak
// cpow // Weak
// cpowf    // Weak
// cpowl    // Weak
// cproj    // Weak
// cprojf   // Weak
// cprojl   // Weak
// creal    // Weak
// crealf   // Weak
// creall   // Weak
// csin // Weak
// csinf    // Weak
// csinh    // Weak
// csinhf   // Weak
// csinhl   // Weak
// csinl    // Weak
// csqrt    // Weak
// csqrtf   // Weak
// csqrtl   // Weak
// ctan // Weak
// ctanf    // Weak
// ctanh    // Weak
// ctanhf   // Weak
// ctanhl   // Weak
// ctanl    // Weak
// __cxa_finalize  // Weak
// drem // Weak
// dremf    // Weak
// dreml    // Weak
// erf  // Weak
// erfc // Weak
// erfcf    // Weak
// erfcl    // Weak
// erff // Weak
// erfl // Weak
GOW(exp, dFd)
GOW(exp10, dFd)
GOW(exp10f, fFf)
// __exp10f_finite
// __exp10_finite
// exp10l   // Weak
GOW(exp2, dFd)
GOW(exp2f, fFf)
// __exp2f_finite
// __exp2_finite
// exp2l    // Weak
GOW(expf, fFf)
// __expf_finite
// __exp_finite
// expl // Weak
// expm1    // Weak
// expm1f   // Weak
// expm1l   // Weak
// fabs // Weak
// fabsf    // Weak
// fabsl    // Weak
// fdim // Weak
// fdimf    // Weak
// fdiml    // Weak
// feclearexcept
// fedisableexcept
// feenableexcept
// fegetenv
// fegetexcept
// fegetexceptflag
// fegetround
// feholdexcept
// feraiseexcept
// fesetenv
// fesetexceptflag
// fesetround
// fetestexcept
// feupdateenv
// finite   // Weak
// __finite
// finitef  // Weak
// __finitef
// finitel  // Weak
// __finitel
GOW(floor, dFd)
GOW(floorf, fFf)
// floorl   // Weak
// fma  // Weak
// fmaf // Weak
// fmal // Weak
// fmax // Weak
// fmaxf    // Weak
// fmaxl    // Weak
// fmin // Weak
// fminf    // Weak
// fminl    // Weak
GOW(fmod, dFdd)
GOW(fmodf, fFff)
// __fmodf_finite
// __fmod_finite
// fmodl    // Weak
// __fpclassify
// __fpclassifyf
GOW(frexp, dFdp)
GOW(frexpf, fFfp)
// frexpl   // Weak
// gamma    // Weak
// gammaf   // Weak
// __gammaf_r_finite
// gammal   // Weak
// __gamma_r_finite
// __gmon_start__  // Weak
// hypot    // Weak
// hypotf   // Weak
// __hypotf_finite
// __hypot_finite
// hypotl   // Weak
// ilogb    // Weak
// ilogbf   // Weak
// ilogbl   // Weak
// __issignaling
// __issignalingf
// _ITM_deregisterTMCloneTable // Weak
// _ITM_registerTMCloneTable   // Weak
// j0
// j0f
// __j0f_finite
// __j0_finite
// j0l
// j1
// j1f
// __j1f_finite
// __j1_finite
// j1l
// jn
// jnf
// __jnf_finite
// __jn_finite
// jnl
// ldexp    // Weak
// ldexpf   // Weak
// ldexpl   // Weak
// lgamma   // Weak
// lgammaf  // Weak
// lgammaf_r    // Weak
// __lgammaf_r_finite
// lgammal  // Weak
// lgammal_r    // Weak
// lgamma_r // Weak
// __lgamma_r_finite
DATAV(_LIB_VERSION, 4)
// llrint   // Weak
// llrintf  // Weak
// llrintl  // Weak
// llround  // Weak
// llroundf // Weak
// llroundl // Weak
GOW(log, dFd)
GOW(log10, dFd)
GOW(log10f, fFf)
// __log10f_finite
// __log10_finite
// log10l   // Weak
// log1p    // Weak
// log1pf   // Weak
// log1pl   // Weak
GOW(log2, dFd)
GOW(log2f, fFf)
// __log2f_finite
// __log2_finite
// log2l    // Weak
// logb // Weak
// logbf    // Weak
// logbl    // Weak
GOW(logf, fFf)
// __logf_finite
// __log_finite
// logl // Weak
// lrint    // Weak
// lrintf   // Weak
// lrintl   // Weak
// lround   // Weak
// lroundf  // Weak
// lroundl  // Weak
// matherr  // Weak
GOW(modf, dFdp)
GOW(modff, fFfp)
// modfl    // Weak
// nan  // Weak
// nanf // Weak
// nanl // Weak
// nearbyint    // Weak
// nearbyintf   // Weak
// nearbyintl   // Weak
// nextafter    // Weak
// nextafterf   // Weak
// nextafterl   // Weak
// nexttoward   // Weak
// nexttowardf  // Weak
// nexttowardl  // Weak
GOW(pow, dFdd)
// pow10    // Weak
// pow10f   // Weak
// pow10l   // Weak
GOW(powf, fFff)
GO(__powf_finite, fFff)
GO(__pow_finite, dFdd)
//GOW(powl, DFDD)
// remainder    // Weak
// remainderf   // Weak
// __remainderf_finite
// __remainder_finite
// remainderl   // Weak
// remquo   // Weak
// remquof  // Weak
// remquol  // Weak
// rint // Weak
// rintf    // Weak
// rintl    // Weak
GOW(round, dFd)
GOW(roundf, fFf)
// roundl   // Weak
// scalb    // Weak
// scalbf   // Weak
// __scalbf_finite
// __scalb_finite
// scalbl   // Weak
// scalbln  // Weak
// scalblnf // Weak
// scalblnl // Weak
// scalbn   // Weak
// scalbnf  // Weak
// scalbnl  // Weak
// __signbit
// __signbitf
DATAB(signgam, 4)
// significand  // Weak
// significandf // Weak
// significandl // Weak
GOW(sin, dFd)
GOW(sincos, vFdpp)
GOW(sincosf, vFfpp)
// sincosl  // Weak
GOW(sinf, fFf)
// sinh // Weak
// sinhf    // Weak
// __sinhf_finite
// __sinh_finite
// sinhl    // Weak
// sinl // Weak
GOW(sqrt, dFd)
GOW(sqrtf, fFf)
// __sqrtf_finite
// __sqrt_finite
// sqrtl    // Weak
GO(tan, dFd)
GOW(tanf, fFf)
GOW(tanh, dFd)
GOW(tanhf, fFf)
// tanhl    // Weak
// tanl // Weak
// tgamma   // Weak
// tgammaf  // Weak
// tgammal  // Weak
GOW(trunc, dFd)
GOW(truncf, fFf)
// truncl   // Weak
// y0
// y0f
// __y0f_finite
// __y0_finite
// y0l
// y1
// y1f
// __y1f_finite
// __y1_finite
// y1l
// yn
// ynf
// __ynf_finite
// __yn_finite
// ynl

#endif