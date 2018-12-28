// regular symbol mapped to itself
#define GO(N, W) \
    if(strcmp(name, #N)==0) { \
        symbol=dlsym(lib->priv.w.lib, #N); \
        size = 12; \
        addr = AddBridge(lib->priv.w.bridge, W, symbol); \
    } else
// regular symbol mapped to itself, but weak (unimplemented for now)
#define GOW(N, W) \
    if(strcmp(name, #N)==0) { \
        symbol=dlsym(lib->priv.w.lib, #N); \
        size = 12; \
        addr = AddBridge(lib->priv.w.bridge, W, symbol); \
    } else
// symbol mapped to my_symbol
#define GOM(N, W) \
    if(strcmp(name, #N)==0) { \
        size = 12; \
        addr = AddBridge(lib->priv.w.bridge, W, my_##N); \
    } else
// symbol mapped to another one
#define GO2(N, W, O) \
    if(strcmp(name, #N)==0) { \
        size = 12; \
        symbol=dlsym(lib->priv.w.lib, #O); \
        addr = AddBridge(lib->priv.w.bridge, W, symbol); \
    } else
// data
#define DATA(N, S) \
    if(strcmp(name, #N)==0) { \
        symbol=dlsym(lib->priv.w.lib, #N); \
        size = S; \
        addr = (uintptr_t)symbol; \
    } else
// data, Weak (type V)
#define DATAV(N, S) \
    if(strcmp(name, #N)==0) { \
        symbol=dlsym(lib->priv.w.lib, #N); \
        size = S; \
        addr = (uintptr_t)symbol; \
    } else
// data, Uninitialized (type B) // should be ignored?
#define DATAB(N, S) \
    if(strcmp(name, #N)==0) { \
        symbol=dlsym(lib->priv.w.lib, #N); \
        size = S; \
        addr = (uintptr_t)symbol; \
    } else

// end marker
#define END() {}
