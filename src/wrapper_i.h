#ifdef GO

GO(iFv)
GO(iFpp, p(0), p(4))
GO(iFppi, p(0), p(4), i32(8))
GO(iFi, i32(0))
GO(iFp, p(0))
GO(iFu, u32(0))
GO(iFii, i32(0), i32(4))
GO(iFip, i32(0), p(4))
GO(iFuu, u32(0), u32(4))
GO(iFup, u32(0), p(4))
GOW(iFpV, iFpp_t, p(0), (void*)stack(4))
GO(iFpip, p(0), i32(4), p(8))
GOW(iF1pV, iFipp_t, 1, p(0), (void*)stack(4))
GO(iFuipp, u32(0), i32(4), p(8), p(12))
GO(iFpuup, p(0), u32(4), u32(8), p(12))
GOW(iFopV, iFppp_t, (void*)stdout, p(0), (void*)stack(4))
GOW(iFvopV, iFppp_t, (void*)stdout, p(4), (void*)stack(8))
GO(iFEpppp, emu, p(0), p(4), p(8), p(12))
GO(iFEpippppp, emu, p(0), i32(4), p(8), p(12), p(16), p(20), p(24))

#else
#error Meh
#endif