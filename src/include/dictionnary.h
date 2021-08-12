#ifndef __DICTIONNARY_H_
#define __DICTIONNARY_H_
#include <stdint.h>

typedef struct dic_s dic_t;

dic_t *NewDictionnary();
void FreeDictionnary(dic_t **dic);

const char* AddDictionnary(dic_t* dic, const char* s);
int ExistDictionnary(dic_t* dic, const char* s);

const char* AddDefault(dic_t* dic, const char* s, const char** vername);
const char* ExistDefault(dic_t* dic, const char* s);

#endif //__DICTIONNARY_H_