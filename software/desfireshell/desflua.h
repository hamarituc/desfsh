#ifndef _DESF_LUA_H_
#define _DESF_LUA_H_

#include <lua.h>
#include <freefare.h>


enum keytype_e { _DES_, _3DES_, _3K3DES_, _AES_ };

extern int desf_lua_get_buffer(lua_State *l, int idx, uint8_t **buffer, unsigned int *len);
extern int desf_lua_get_keytype(lua_State *l, int idx, enum keytype_e *type);
extern int desf_lua_get_key(lua_State *l, int idx, MifareDESFireKey *k);
extern int desf_lua_handle_result(lua_State *l, int result, MifareTag tag);
extern void desf_lua_shell();


#endif
