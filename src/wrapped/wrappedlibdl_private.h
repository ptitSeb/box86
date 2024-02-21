#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(GOS))
#error Meh...
#endif

GOM(dladdr, iFEpp)
GOM(dladdr1, iFEpppi)
GOM(dlclose, iFEp)
GOM(dlerror, pFEv)
DATAB(_dlfcn_hook, 4)
GOM(dlinfo, iFEpip)
GOM(dlmopen, pFEppi)
GOM(dlopen, pFEpi)
GOM(dlsym, pFEpp)
GOM(dlvsym, pFEppp)   // Weak
GOM(_dl_find_object, iFEpp)
