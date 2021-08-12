#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "dictionnary.h"
#include "custommem.h"
#include "khash.h"

KHASH_SET_INIT_STR(store);
KHASH_MAP_INIT_STR(default, const char*);

typedef struct dic_s {
    kh_store_t      *dic;
    kh_default_t    *def;
} dic_t;


dic_t *NewDictionnary()
{
    dic_t *dic = (dic_t*)calloc(1, sizeof(dic_t));
    dic->dic = kh_init(store);
    dic->def = kh_init(default);
    return dic;
}

void FreeDictionnary(dic_t **d)
{
    if(!d || !*d)
        return;
    dic_t* dic = *d;
    const char* k;
    kh_foreach_value(dic->def, k, free((void*)k));
    kh_destroy(default, dic->def);
    kh_foreach_key(dic->dic, k, free((void*)k));
    kh_destroy(store, dic->dic);
    free(dic);
    *d = NULL;
}

const char* AddDictionnary(dic_t* dic, const char* s)
{
    khint_t k = kh_get(store, dic->dic, s);
    if(k!=kh_end(dic->dic))
        return kh_key(dic->dic, k);
    char* n = strdup(s);
    int ret;
    k = kh_put(store, dic->dic, n, &ret);
    return n;
}

int ExistDictionnary(dic_t* dic, const char* s)
{
    khint_t k = kh_get(store, dic->dic, s);
    if(k!=kh_end(dic->dic))
        return 1;
    return 0;

}

const char* AddDefault(dic_t* dic, const char* s, const char** vername)
{
    char name[strlen(s)+1];
    strcpy(name, s);
    char* p = strchr(name, '@');
    if(!p) {
        printf_log(LOG_NONE, "Warning, Added Default symbol without version mark %s\n", s);
        return NULL; 
    }
    *p=0;
    const char* key = AddDictionnary(dic, name);
    char* def_ver = strdup(p+2);
    *vername = def_ver;
    int ret;
    khint_t k = kh_put(default, dic->def, key, &ret);
    if(!ret) {
        free(def_ver);
        *vername = kh_value(dic->def, k);
        return key;
    }
    kh_value(dic->def, k) = def_ver;
    return key;
}

const char* ExistDefault(dic_t* dic, const char* s)
{
    khint_t k = kh_get(default, dic->def, s);
    if(k!=kh_end(dic->def))
        return kh_value(dic->def, k);
    return NULL;

}