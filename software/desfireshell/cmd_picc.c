#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "desflua.h"
#include "desfsh.h"



static int cmd_createapp(lua_State *l);
static int cmd_deleteapp(lua_State *l);
static int cmd_appids(lua_State *l);
static int cmd_selapp(lua_State *l);
static int cmd_format(lua_State *l);
static int cmd_getver(lua_State *l);
static int cmd_freemem(lua_State *l);
static int cmd_carduid(lua_State *l);




FN_ALIAS(cmd_createapp) = { "createapp", "capp", "CreateApplication", NULL };
FN_PARAM(cmd_createapp) =
{
  FNPARAM("aid",      "Application ID",      0),
  FNPARAM("settings", "Master key settings", 0),
  FNPARAM("nkeys",    "Number of keys",      0),
  FNPARAMEND
};
FN_RET(cmd_createapp) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAMEND
};
FN("cmd", cmd_createapp, "Create application", NULL);


static int cmd_createapp(lua_State *l)
{
  int result;
  enum keytype_e type;
  uint32_t aid;
  MifareDESFireAID app;
  uint8_t settings;
  uint8_t keyno;


  result = desflua_get_keytype(l, 1, &type);
  if(result)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "keytype: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 1, lua_tostring(l, -1));
  }
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "AID must be a number");
  luaL_argcheck(l, lua_isnumber(l, 3), 3, "key settings must be a number");
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "number of keys expected");

  aid = lua_tointeger(l, 2);
  app = mifare_desfire_aid_new(aid);
  if(app == NULL)
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);

  settings = lua_tointeger(l, 3);
  keyno    = lua_tointeger(l, 4);

  if(keyno >= 14)
    return luaL_argerror(l, 4, "at most 14 key allowed");

  switch(type)
  {
  case _DES_:                                        break;
  case _3DES_:                                       break;
  case _3K3DES_: keyno |= APPLICATION_CRYPTO_3K3DES; break;
  case _AES_:    keyno |= APPLICATION_CRYPTO_AES;    break;
  }

  result = mifare_desfire_create_application(tag, app, settings, keyno);
  free(app);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_deleteapp) = { "deleteapp", "dapp", "DeleteApplication", NULL };
FN_PARAM(cmd_deleteapp) =
{
  FNPARAM("aid", "Application ID", 0),
  FNPARAMEND
};
FN_RET(cmd_deleteapp) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAMEND
};
FN("cmd", cmd_deleteapp, "Delete application", NULL);


static int cmd_deleteapp(lua_State *l)
{
  int result;
  uint32_t aid;
  MifareDESFireAID app;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "AID must be a number");

  aid = lua_tointeger(l, 1);
  app = mifare_desfire_aid_new(aid);
  if(app == NULL)
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);

  result = mifare_desfire_delete_application(tag, app);

  free(app);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_appids) = { "appids", "GetApplicationIDs", NULL };
FN_PARAM(cmd_appids) =
{
  FNPARAMEND
};
FN_RET(cmd_appids) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAM("aids", "List of AIDs", 1),
  FNPARAMEND
};
FN("cmd", cmd_appids, "Get application list", NULL);


static int cmd_appids(lua_State *l)
{
  int result;
  MifareDESFireAID *apps;
  size_t len, i;
  uint32_t aid;
  char buffer[10];


  result = mifare_desfire_get_application_ids(tag, &apps, &len);
  desflua_handle_result(l, result, tag);

  if(result < 0)
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




FN_ALIAS(cmd_selapp) = { "selapp", "select", "SelectApplication", NULL };
FN_PARAM(cmd_selapp) =
{
  FNPARAM("aids", "List of AIDs", 1),
  FNPARAMEND
};
FN_RET(cmd_selapp) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAMEND
};
FN("cmd", cmd_selapp, "Select application", NULL);


static int cmd_selapp(lua_State *l)
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




FN_ALIAS(cmd_format) = { "format", "FormatPICC", NULL };
FN_PARAM(cmd_format) =
{
  FNPARAMEND
};
FN_RET(cmd_format) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAMEND
};
FN("cmd", cmd_format, "Format PICC", NULL);


static int cmd_format(lua_State *l)
{
  int result;


  result = mifare_desfire_format_picc(tag);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_getver) = { "getver", "GetVersion", NULL };
FN_PARAM(cmd_getver) =
{
  FNPARAMEND
};
FN_RET(cmd_getver) =
{
  FNPARAM("code", "Return code",      0),
  FNPARAM("err",  "Error string",     0),
  FNPARAM("info", "PICC information", 1),
  FNPARAMEND
};
FN("cmd", cmd_getver, "Format PICC", NULL);


static int cmd_getver(lua_State *l)
{
  int result;
  struct mifare_desfire_version_info info;
  char buffer[20];


  result = mifare_desfire_get_version(tag, &info);
  desflua_handle_result(l, result, tag);

  if(result < 0)
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




FN_ALIAS(cmd_freemem) = { "freemem", "FreeMem", NULL };
FN_PARAM(cmd_freemem) =
{
  FNPARAMEND
};
FN_RET(cmd_freemem) =
{
  FNPARAM("code", "Return code",         0),
  FNPARAM("err",  "Error string",        0),
  FNPARAM("size", "Size of free memory", 1),
  FNPARAMEND
};
FN("cmd", cmd_freemem, "Get size of remaining memory", NULL);


static int cmd_freemem(lua_State *l)
{
  int result;
  uint32_t freemem;


  result = mifare_desfire_free_mem(tag, &freemem);
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  lua_pushinteger(l, freemem);


exit:
  return lua_gettop(l);
}




FN_ALIAS(cmd_carduid) = { "carduid", "cuid", "GetCardUID", NULL };
FN_PARAM(cmd_carduid) =
{
  FNPARAMEND
};
FN_RET(cmd_carduid) =
{
  FNPARAM("code", "Return code",   0),
  FNPARAM("err",  "Error string",  0),
  FNPARAM("uid",  "Real card UID", 1),
  FNPARAMEND
};
FN("cmd", cmd_carduid, "Get real card UID", NULL);


static int cmd_carduid(lua_State *l)
{
  int result;
  char *uid;


  result = mifare_desfire_get_card_uid(tag, &uid);
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  lua_pushstring(l, uid);


exit:
  return lua_gettop(l);
}
