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
    luaL_argerror(l, 2, lua_tostring(l, -1));

  num = lua_tonumber(l, 1);

  result = mifare_desfire_authenticate(tag, num, k);
  result = desf_lua_handle_result(l, result, tag);


  return result;
}


int cmd_cks(lua_State *l)
{
  return 0;
}
