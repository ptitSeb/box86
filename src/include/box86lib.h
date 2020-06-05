#ifndef _BOX86LIB_H_
#define _BOX86LIB_H_

typedef struct path_collection_s path_collection_t;
typedef struct box86context_s box86context_t;
typedef struct x86emu_s x86emu_t;

void LoadLogEnv();
void LoadEnvPath(path_collection_t *col, const char* defpath, const char* env);
int CountEnv(const char** env);
int GatherEnv(char*** dest, const char** env, const char* prog);
void LoadEnvVars(box86context_t *context);
void setupTraceInit(box86context_t* context);
void setupTrace(box86context_t* context);
void initAllHelpers(box86context_t* context);
void finiAllHelpers();

#ifdef BUILD_DYNAMIC
box86context_t* GetBox86Context();
x86emu_t* GetX86Emu();
#endif

#endif //_BOX86LIB_H_