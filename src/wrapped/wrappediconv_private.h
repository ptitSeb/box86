#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(libiconv, uFppppp)
GO(libiconv_close, iFp)
GO(libiconv_open, pFpp)
GO(libiconv_open_into, iFppp)
GO(libiconv_set_relocation_prefix, vFpp)
GO(libiconvctl, iFpip)
//GO(libiconvlist, vFpp) // this one have callbacks
