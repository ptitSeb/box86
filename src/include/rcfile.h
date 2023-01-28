#ifndef __RCFILE_H__
#define __RCFILE_H__

typedef struct path_collection_s path_collection_t;

void LoadRCFile(const char* filename);
void DeleteParams();
void ApplyParams(const char* name, path_collection_t* preload);

#endif //__RCFILE_H__