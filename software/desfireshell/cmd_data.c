#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "desflua.h"
#include "desfsh.h"



int cmd_read(lua_State *l)
{
  int result;
  uint8_t fid;
  uint32_t off, len;
  uint8_t comm;
  uint8_t *data;


  luaL_argcheck(l,                      lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l,                      lua_isnumber(l, 2), 2, "offset must be a number");
  luaL_argcheck(l,                      lua_isnumber(l, 3), 3, "length must be a number");
  luaL_argcheck(l, lua_gettop(l) < 4 || lua_isnumber(l, 4), 4, "comm settings must be a number");

  fid  = lua_tonumber(l, 1);
  off  = lua_tonumber(l, 2);
  len  = lua_tonumber(l, 3);
  comm = lua_tonumber(l, 4);

  data = (uint8_t*)malloc(len * sizeof(uint8_t));
  if(data == NULL)
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);

  if(lua_isnumber(l, 4))
    result = mifare_desfire_read_data_ex(tag, fid, off, len, data, comm);
  else
    result = mifare_desfire_read_data(tag, fid, off, len, data);
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  desflua_push_buffer(l, data, result);


exit:
  free(data);
  return lua_gettop(l);
}


int cmd_write(lua_State *l)
{
  int result;
  uint8_t fid;
  uint32_t off, len;
  uint8_t *data;
  uint8_t comm;


  luaL_argcheck(l,                      lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l,                      lua_isnumber(l, 2), 2, "offset must be a number");

  result = desflua_get_buffer(l, -1, &data, &len);
  if(result)
  {
    lua_remove(l, -2);
    lua_pushfstring(l, "buffer invalid: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 3, lua_tostring(l, -1));
  }

  luaL_argcheck(l, lua_gettop(l) < 4 || lua_isnumber(l, 4), 4, "comm settings must be a number");

  fid  = lua_tonumber(l, 1);
  off  = lua_tonumber(l, 2);
  comm = lua_tonumber(l, 4);


  if(lua_isnumber(l, 4))
    result = mifare_desfire_write_data_ex(tag, fid, off, len, data, comm);
  else
    result = mifare_desfire_write_data(tag, fid, off, len, data);
  desflua_handle_result(l, result, tag);

  
  return lua_gettop(l);
}
