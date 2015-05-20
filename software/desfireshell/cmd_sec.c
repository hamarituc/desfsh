#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "desflua.h"
#include "desfsh.h"



int cmd_auth(lua_State *l)
{
  int result;
  uint8_t num;
  MifareDESFireKey k;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key number expected");
  result = desf_lua_get_key(l, 2, &k);
  if(result)
    return luaL_argerror(l, 2, lua_tostring(l, -1));

  num = lua_tonumber(l, 1);

  result = mifare_desfire_authenticate(tag, num, k);
  desf_lua_handle_result(l, result, tag);


  return lua_gettop(l);
}


int cmd_cks(lua_State *l)
{
  return 0;
}


int cmd_gks(lua_State *l)
{
  int result;
  uint8_t settings, maxkeys;


  result = mifare_desfire_get_key_settings(tag, &settings, &maxkeys);
  desf_lua_handle_result(l, result, tag);

  if(result == 0)
  {
    lua_checkstack(l, 2);
    lua_pushinteger(l, settings);
    lua_pushinteger(l, maxkeys);
  }


  return lua_gettop(l);
}


int cmd_gkv(lua_State *l)
{
  int result;
  uint8_t num;
  uint8_t ver;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key number expected");

  num = lua_tonumber(l, 1);

  result = mifare_desfire_get_key_version(tag, num, &ver);
  desf_lua_handle_result(l, result, tag);

  if(result == 0)
  {
    lua_checkstack(l, 1);
    lua_pushinteger(l, ver);
  }


  return lua_gettop(l);
}
