#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GOW(acos, dFd)
GOW(acosf, fFf)
GO(__acosf_finite, fFf)
GO(__acos_finite, dFd)
GOW(acosh, dFd)
GOW(acoshf, fFf)
GO(__acoshf_finite, fFf)
GO(__acosh_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(acoshl, DFD)
#else
GO2(acoshl, KFK, acosh)
#endif
#ifdef HAVE_LD80BITS
GOW(acpsl, DFD)
#else
GO2(acosl, KFK, acos)
#endif
GOW(asin, dFd)
GOW(asinf, fFf)
GO(__asinf_finite, fFf)
GO(__asin_finite, dFd)
GOW(asinh, dFd)
GOW(asinhf, fFf)
#ifdef HAVE_LD80BITS
GOW(asinhl, DFD)
#else
GO2(asinhl, KFK, asinh)
#endif
#ifdef HAVE_LD80BITS
GOW(asinl, DFD)
#else
GO2(asinl, KFK, asin)
#endif
GO(atan, dFd)
GOW(atan2, dFdd)
GOW(atan2f, fFff)
GO(__atan2f_finite, fFff)
GO(__atan2_finite, dFdd)
// atan2l   // Weak
GOW(atanf, fFf)
GOW(atanh, dFd)
GOW(atanhf, fFf)
// __atanhf_finite
// __atanh_finite
#ifdef HAVE_LD80BITS
GOW(atanhl, DFD)
#else
GO2(atanhl, KFK, atanh)
#endif
// atanl    // Weak
GOW(cabs, dFdd)     // only 1 arg, but is a double complex
GOW(cabsf, fFff)    // only 1 arg, but is a float complex
// cabsl    // Weak
// cacos    // Weak
// cacosf   // Weak
// cacosh   // Weak
// cacoshf  // Weak
// cacoshl  // Weak
// cacosl   // Weak
GOW(carg, dFdd)     // 1arg, double complex
GOW(cargf, fFff)    // 1arg, float complex
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
GOW(cbrt, dFd)
GOW(cbrtf, fFf)
#ifdef HAVE_LD80BITS
GOW(cbrtl, DFD)
#else
GO2(cbrtl, KFK, cbrt)
#endif
// ccos // Weak
// ccosf    // Weak
// ccosh    // Weak
// ccoshf   // Weak
// ccoshl   // Weak
// ccosl    // Weak
GOW(ceil, dFd)
GOW(ceilf, fFf)
// ceill    // Weak
// cexp // Weak
// cexpf    // Weak
// cexpl    // Weak
// cimag    // Weak
// cimagf   // Weak
// cimagl   // Weak
GOS(clog, pFpV)   // return a double complex, that is a struct
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
GOW(copysign, dFdd)
GOW(copysignf, fFff)
// copysignl    // Weak
GOW(cos, dFd)
GOW(cosf, fFf)
GOW(cosh, dFd)
GOW(coshf, fFf)
GO(__coshf_finite, fFf)
GO(__cosh_finite, dFd)
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
GOS(csqrt, pFpV)
GOS(csqrtf, pFpV)   // Weak
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
GOW(erf, dFd)
GOW(erfc, dFd)
GOW(erfcf, fFf)
#ifdef HAVE_LD80BITS
GOW(erfcl, DFD)
#else
GO2(erfcl, KFK, erfc)
#endif
GOW(erff, fFf)
#ifdef HAVE_LD80BITS
GOW(erfl, DFD)
#else
GO2(erfl, KFK, erf)
#endif
GOW(exp, dFd)
GOW(exp10, dFd)
GOW(exp10f, fFf)
// __exp10f_finite
// __exp10_finite
// exp10l   // Weak
GOW(exp2, dFd)
GOW(exp2f, fFf)
GO(__exp2f_finite, fFf)
GO(__exp2_finite, dFd)
// exp2l    // Weak
GOW(expf, fFf)
GO(__expf_finite, fFf)
GO(__exp_finite, dFd)
// expl // Weak
GOW(expm1, dFd)
GOW(expm1f, fFf)
// expm1l   // Weak
GOW(fabs, dFd)
GOW(fabsf, fFf)
// fabsl    // Weak
// fdim // Weak
// fdimf    // Weak
// fdiml    // Weak
GO(feclearexcept, iFi)
GO(fedisableexcept, iFi)
GO(feenableexcept, iFi)
GO(fegetenv, iFp)
GO(fegetexcept, iFv)
GO(fegetexceptflag, iFpi)
GO(fegetround, iFv)
GO(feholdexcept, iFp)
GO(feraiseexcept, iFi)
GO(fesetenv, iFp)
GO(fesetexceptflag, iFpi)
GO(fesetround, iFi)
GO(fetestexcept, iFi)
GO(feupdateenv, iFp)
GOW(finite, iFd)
// __finite
GOW(finitef, iFf)
GO(__finitef, iFf)
// finitel  // Weak
// __finitel
GOW(floor, dFd)
GOW(floorf, fFf)
// floorl   // Weak
GOW(fma, dFddd)
GOW(fmaf, fFfff)
// fmal // Weak
GOW(fmax, dFdd)
GOW(fmaxf, fFff)
// fmaxl    // Weak
GOW(fmin, dFdd)
GOW(fminf, fFff)
// fminl    // Weak
GOW(fmod, dFdd)
GOW(fmodf, fFff)
// __fmodf_finite
// __fmod_finite
#ifdef HAVE_LD80BITS
GOW(fmodl, DFDD)
#else
GO2(fmodl, KFKK, fmod)
#endif
GO(__fpclassify, iFd)
GO(__fpclassifyf, iFf)
GOW(frexp, dFdp)
GOW(frexpf, fFfp)
#ifdef HAVE_LD80BITS
GOW(frexpl, DFDp)
#else
GO2(frexpl, KFKp, frexp)
#endif
// gamma    // Weak
// gammaf   // Weak
// __gammaf_r_finite
// gammal   // Weak
// __gamma_r_finite
// __gmon_start__  // Weak
GOW(hypot, dFdd)
GOW(hypotf, fFff)
GO(__hypotf_finite, fFff)
GO(__hypot_finite, dFdd)
// hypotl   // Weak
GOW(ilogb, iFd)
GOW(ilogbf, iFf)
// ilogbl   // Weak
// __issignaling
// __issignalingf
// _ITM_deregisterTMCloneTable // Weak
// _ITM_registerTMCloneTable   // Weak
GO(j0, dFd)
GO(j0f, fFf)
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
GOW(ldexp, dFdi)
GOW(ldexpf, fFfi)
#ifdef HAVE_LD80BITS
GOW(ldexpl, DFD)
#else
GO2(ldexpl, KFK, ldexp)
#endif
GOW(lgamma, dFd)
GOW(lgammaf, fFf)
GOW(lgammaf_r, fFfp)
// __lgammaf_r_finite
#ifdef HAVE_LD80BITS
GOW(lgammal, DFD)
#else
GO2(lgammal, KFK, lgamma)
#endif
#ifdef HAVE_LD80BITS
GOW(lgammal_r, DFDp)
#else
GO2(lgammal_r, KFKp, lgamma_r)
#endif
GOW(lgamma_r, dFdp)
// __lgamma_r_finite
DATAV(_LIB_VERSION, 4)
GOW(llrint, IFd)
GOW(llrintf, IFf)
// llrintl  // Weak
// llround  // Weak
// llroundf // Weak
// llroundl // Weak
GOW(log, dFd)
GOW(log10, dFd)
GOW(log10f, fFf)
GO(__log10f_finite, fFf)
GO(__log10_finite, dFd)
// log10l   // Weak
GOW(log1p, dFd)
GOW(log1pf, fFf)
// log1pl   // Weak
GOW(log2, dFd)
GOW(log2f, fFf)
GO(__log2f_finite, fFf)
GO(__log2_finite, dFd)
// log2l    // Weak
GOW(logb, dFd)
GOW(logbf, fFf)
// logbl    // Weak
GOW(logf, fFf)
GO(__logf_finite, fFf)
GO(__log_finite, dFd)
// logl // Weak
GOW(lrint, iFd)
GOW(lrintf, iFf)
// lrintl   // Weak
GOW(lround, iFd)
GOW(lroundf, iFf)
// lroundl  // Weak
// matherr  // Weak
GOW(modf, dFdp)
GOW(modff, fFfp)
// modfl    // Weak
// nan  // Weak
// nanf // Weak
// nanl // Weak
GOW(nearbyint, dFd)
GOW(nearbyintf, fFf)
// nearbyintl   // Weak
GOW(nextafter, dFdd)
GOW(nextafterf, fFff)
// nextafterl   // Weak
GOW(nexttoward, dFdd)
GOW(nexttowardf, fFff)
// nexttowardl  // Weak
GOW(pow, dFdd)
// pow10    // Weak
// pow10f   // Weak
// pow10l   // Weak
GOW(powf, fFff)
GO(__powf_finite, fFff)
GO(__pow_finite, dFdd)
#ifdef HAVE_LD80BITS
GOW(powl, DFDD)
#else
GO2(powl, KFKK, pow)
#endif
// remainder    // Weak
// remainderf   // Weak
// __remainderf_finite
// __remainder_finite
// remainderl   // Weak
// remquo   // Weak
// remquof  // Weak
// remquol  // Weak
GOW(rint, dFd)
GOW(rintf, fFf)
// rintl    // Weak
GOW(round, dFd)
GOW(roundf, fFf)
// roundl   // Weak
// scalb    // Weak
// scalbf   // Weak
// __scalbf_finite
// __scalb_finite
// scalbl   // Weak
GOW(scalbln, dFdi)
GOW(scalblnf, fFfi)
// scalblnl // Weak
GOW(scalbn, dFdi)
GOW(scalbnf, fFfi)
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
GOW(sinh, dFd)
GOW(sinhf, fFf)
GO(__sinhf_finite, fFf)
GO(__sinh_finite, dFd)
// sinhl    // Weak
// sinl // Weak
GOW(sqrt, dFd)
GOW(sqrtf, fFf)
GO(__sqrtf_finite, fFf)
GO(__sqrt_finite, dFd)
// sqrtl    // Weak
GO(tan, dFd)
GOW(tanf, fFf)
GOW(tanh, dFd)
GOW(tanhf, fFf)
// tanhl    // Weak
// tanl // Weak
GOW(tgamma, dFd)
GOW(tgammaf, fFf)
#ifdef HAVE_LD80BITS
GOW(tgammal, DFD)
#else
GO2(tgammal, KFK, tgamma)
#endif
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
