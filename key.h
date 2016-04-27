#ifndef _DESF_KEY_H_
#define _DESF_KEY_H_

#include <lua.h>
#include <freefare.h>

#include "fn.h"


enum keytype_e { _DES_, _3DES_, _3K3DES_, _AES_ };

extern int key_gettype(lua_State *l, int idx, enum keytype_e *type, char **typestr);
extern int key_getraw(lua_State *l, int idx, enum keytype_e *type, uint8_t **key, unsigned int *keylen, uint8_t *_ver, char **keystr);
extern int key_get(lua_State *l, int idx, MifareDESFireKey *k, char **keystr);
extern void key_push(lua_State *l, enum keytype_e type, uint8_t *key, unsigned int keylen, uint8_t ver);

extern FNDECL(key_create);
extern FNDECL(key_div);

#endif
