#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(XRRQueryVersion,iFppp)
GO(XRRFreeScreenConfigInfo,vFp)
GO(XRRGetCrtcInfo, pFppu)
GO(XRRListOutputProperties, pFpup)
GO(XRRQueryExtension, iFppp)
//GO(XRRAllocModeInfo
//GO(XRRGetProviderProperty
GO(XRRQueryExtension,iFppp)
//GO(XRRListProviderProperties
GO(XRRRotations,uFip)
//GO(XRRSetCrtcTransform
GO(XRRGetCrtcGammaSize, iFpu)
GO(XRRConfigRotations,uFp)
//GO(XRRGetProviderInfo
//GO(XRRConfigureOutputProperty
//GO(XRRSetOutputPrimary
GO(XRRFreeOutputInfo, vFp)
GO(XRRUpdateConfiguration,iF)
GO(XRRGetScreenResources, pFpp)
GO(XRRConfigSizes,pFpp)
//GO(XRRQueryProviderProperty
//GO(XRRDeleteOutputProperty
//GO(XRRDeleteProviderProperty
GO(XRRSetScreenConfig,iFpppiuu)
GO(XRRAllocGamma, pFi)
GO(XRRSetScreenSize, vFppiiii)
GO(XRRSetScreenConfigAndRate,iFpppiuwu)
GO(XRRFreeScreenResources, vFp)
GO(XRRGetOutputPrimary, uFpp)
//GO(XRRCreateMode
GO(XRRConfigCurrentRate,wFp)
//GO(XRRDestroyMode
GO(XRRSetCrtcConfig, iFppuuiiuupi)
GO(XRRConfigCurrentConfiguration,uFpp)
GO(XRRSizes,pFip)
//GO(XRRAddOutputMode
GO(XRRFreePanning, vFp)
//GO(XRRFreeProviderResources
//GO(XRRChangeProviderProperty
GO(XRRGetPanning, pFppu)
//GO(XRRSetProviderOffloadSink
GO(XRRGetScreenResourcesCurrent, pFpp)
GO(XRRConfigTimes,uFpp)
GO(XRRSetCrtcGamma, vFppp)
//GO(XRRSetProviderOutputSource
GO(XRRGetScreenSizeRange, iFpppppp)
GO(XRRRates,pFpiip)
//GO(XRRFreeProviderInfo
GO(XRRConfigRates,pFpip)
GO(XRRQueryOutputProperty, pFpup)
GO(XRRGetOutputProperty, iFpupiiiipppppp)
GO(XRRFreeGamma, vFp)
GO(XRRRootToScreen,iFp)
GO(XRRGetScreenInfo,pFpp)
GO(XRRFreeCrtcInfo, vFp)
//GO(XRRGetProviderResources
//GO(XRRFreeModeInfo
//GO(XRRChangeOutputProperty
GO(XRRGetCrtcGamma, pFpu)
GO(XRRSetPanning, pFppu)
GO(XRRSelectInput,vFpi)
//GO(XRRGetCrtcTransform
GO(XRRTimes,uFpip)
//GO(XRRDeleteOutputMode
GO(XRRGetOutputInfo, pFppu)
//GO(XRRConfigureProviderProperty
