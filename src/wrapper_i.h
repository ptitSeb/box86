#ifdef GOW

GOW(iFpV, iFpp_t, p(0), (void*)stack(4))
GOW(iF1pV, iFipp_t, 1, p(0), (void*)stack(4))
GOW(iFopV, iFppp_t, (void*)stdout, p(0), (void*)stack(4))
GOW(iFvopV, iFppp_t, (void*)stdout, p(4), (void*)stack(8))

#else
#error You must not include wrapper_i.h but wrapper.h
#endif
