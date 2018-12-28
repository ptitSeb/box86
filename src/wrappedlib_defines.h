#define GO(N, W) \
    if(strcmp(name, #N)==0) { \
        symbol=dlsym(lib->priv.w.lib, #N); \
        size = 12; \
        addr = AddBridge(lib->priv.w.bridge, W, symbol); \
    } else
#define GOM(N, W) \
    if(strcmp(name, #N)==0) { \
        size = 12; \
        addr = AddBridge(lib->priv.w.bridge, W, my_##N); \
    } else
#define GO2(N, W, O) \
    if(strcmp(name, #N)==0) { \
        size = 12; \
        symbol=dlsym(lib->priv.w.lib, #O); \
        addr = AddBridge(lib->priv.w.bridge, W, symbol); \
    } else
#define DATA(N, S) \
    if(strcmp(name, #N)==0) { \
        symbol=dlsym(lib->priv.w.lib, #N); \
        size = S; \
        addr = (uintptr_t)symbol; \
    } else
#define END() {}
