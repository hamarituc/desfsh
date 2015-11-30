#ifndef _DESF_LUA_H_
#define _DESF_LUA_H_

#include <lua.h>
#include <freefare.h>


extern int desflua_get_comm(lua_State *l, int idx, uint8_t *comm);
extern int desflua_get_acl(lua_State *l, int idx, uint16_t *acl);
extern void desflua_push_acl(lua_State *l, uint16_t acl);
extern void desflua_handle_result(lua_State *l, int result, FreefareTag tag);
extern void desflua_argerror(lua_State *l, int argnr, const char *prefix);
extern void desflua_shell();

#endif
