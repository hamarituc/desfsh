#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "desflua.h"
#include "desfsh.h"



int cmd_createapp(lua_State *l)
{
  int result;
  enum keytype_e type;
  uint32_t aid;
  MifareDESFireAID app;
  uint8_t settings;
  uint8_t keyno;


  result = desflua_get_keytype(l, 1, &type);
  if(result)
    return luaL_argerror(l, 1, lua_tostring(l, -1));
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "AID must be a number");
  luaL_argcheck(l, lua_isnumber(l, 3), 3, "key settings must be a number");
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "number of keys expected");

  aid = lua_tointeger(l, 2);
  app = mifare_desfire_aid_new(aid);
  if(app == NULL)
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);

  settings = lua_tointeger(l, 3);
  keyno    = lua_tointeger(l, 4);

  switch(type)
  {
  case _DES_:                                        break;
  case _3DES_:                                       break;
  case _3K3DES_: keyno |= APPLICATION_CRYPTO_3K3DES; break;
  case _AES_:    keyno |= APPLICATION_CRYPTO_AES;    break;
  }

  result = mifare_desfire_create_application_aes(tag, app, settings, keyno);
  free(app);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


/*int cmd_deleteapp(lua_State *l)
{
}*/


int cmd_appids(lua_State *l)
{
  int result;
  MifareDESFireAID *apps;
  size_t len, i;
  uint32_t aid;
  char buffer[10];


  result = mifare_desfire_get_application_ids(tag, &apps, &len);
  desflua_handle_result(l, result, tag);

  if(result)
    goto exit;

  lua_checkstack(l, 3);
  lua_newtable(l);
  for(i = 0; i < len; i++)
  {
    aid = mifare_desfire_aid_get_aid(apps[i]);
    snprintf(buffer, 10, "0x%06x", aid);
    lua_pushinteger(l, i + 1);
    lua_pushstring(l, buffer);
    lua_settable(l, -3);
  }
  mifare_desfire_free_application_ids(apps);


exit:
  return lua_gettop(l);
}


int cmd_selapp(lua_State *l)
{
  int result;
  uint32_t aid;
  MifareDESFireAID app;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "AID must be a number");

  aid = lua_tointeger(l, 1);
  app = mifare_desfire_aid_new(aid);
  if(app == NULL)
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);

  result = mifare_desfire_select_application(tag, app);
  free(app);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


int cmd_getver(lua_State *l)
{
  int result;
  struct mifare_desfire_version_info info;
  char buffer[20];


  result = mifare_desfire_get_version(tag, &info);
  desflua_handle_result(l, result, tag);

  if(result)
    goto exit;

  lua_checkstack(l, 3);
  lua_newtable(l);

  lua_newtable(l);
  lua_pushinteger(l, info.hardware.vendor_id);     lua_setfield(l, -2, "vendor");
  lua_pushinteger(l, info.hardware.type);          lua_setfield(l, -2, "type");
  lua_pushinteger(l, info.hardware.subtype);       lua_setfield(l, -2, "subtype");
  lua_pushinteger(l, info.hardware.version_major); lua_setfield(l, -2, "major");
  lua_pushinteger(l, info.hardware.version_minor); lua_setfield(l, -2, "minor");
  lua_pushinteger(l, info.hardware.storage_size);  lua_setfield(l, -2, "size");
  lua_pushinteger(l, info.hardware.protocol);      lua_setfield(l, -2, "protocol");
  lua_setfield(l, -2, "hardware");

  lua_newtable(l);
  lua_pushinteger(l, info.software.vendor_id);     lua_setfield(l, -2, "vendor");
  lua_pushinteger(l, info.software.type);          lua_setfield(l, -2, "type");
  lua_pushinteger(l, info.software.subtype);       lua_setfield(l, -2, "subtype");
  lua_pushinteger(l, info.software.version_major); lua_setfield(l, -2, "major");
  lua_pushinteger(l, info.software.version_minor); lua_setfield(l, -2, "minor");
  lua_pushinteger(l, info.software.storage_size);  lua_setfield(l, -2, "size");
  lua_pushinteger(l, info.software.protocol);      lua_setfield(l, -2, "protocol");
  lua_setfield(l, -2, "software");

  snprintf(buffer, 20, "0x%02x%02x%02x%02x%02x%02x%02x", 
    info.uid[0], info.uid[1], info.uid[2], info.uid[3], info.uid[4], info.uid[5], info.uid[6]);
  lua_pushstring(l, buffer); lua_setfield(l, -2, "uid");

  snprintf(buffer, 20, "0x%02x%02x%02x%02x%02x",
    info.batch_number[0], info.batch_number[1], info.batch_number[2], info.batch_number[3], info.batch_number[4]);
  lua_pushstring(l, buffer); lua_setfield(l, -2, "batch");

  snprintf(buffer, 20, "%x", info.production_week);
  lua_pushstring(l, buffer); lua_setfield(l, -2, "prodweek");

  snprintf(buffer, 20, "%x", info.production_year);
  lua_pushstring(l, buffer); lua_setfield(l, -2, "prodyear");


exit:
  return lua_gettop(l);
}
