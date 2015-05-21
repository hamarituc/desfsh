#ifndef _DESF_LUA_H_
#define _DESF_LUA_H_

#include <lua.h>
#include <freefare.h>


enum keytype_e { _DES_, _3DES_, _3K3DES_, _AES_ };

extern int desflua_get_buffer(lua_State *l, int idx, uint8_t **buffer, unsigned int *len);
extern int desflua_get_keytype(lua_State *l, int idx, enum keytype_e *type);
extern int desflua_get_key(lua_State *l, int idx, MifareDESFireKey *k);
extern int desflua_get_acl(lua_State *l, int idx, uint16_t *acl);
extern void desflua_push_acl(lua_State *l, uint16_t acl);
extern void desflua_handle_result(lua_State *l, int result, MifareTag tag);
extern void desflua_shell();


#endif
