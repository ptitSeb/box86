#if defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(END)

// dladdr
// dladdr1
// dlclose
// dlerror
DATAB(_dlfcn_hook, 4)
// dlinfo
// dlmopen
// dlopen
// dlsym
// dlvsym   // Weak

END()

#else
#error Mmmm...
#endif