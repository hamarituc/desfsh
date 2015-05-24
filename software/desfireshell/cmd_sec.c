#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "desflua.h"
#include "desfsh.h"
#include "fn.h"



static int cmd_auth(lua_State *l);
static int cmd_cks(lua_State *l);
static int cmd_gks(lua_State *l);
static int cmd_ck(lua_State *l);
static int cmd_gkv(lua_State *l);




FN_ALIAS(cmd_auth) = { "auth", "Authenticate", NULL };
FN_PARAM(cmd_auth) =
{
  FNPARAM("kno", "Key number",         0),
  FNPARAM("key", "Authentication key", 0),
  FNPARAMEND
};
FN_RET(cmd_auth) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAMEND
};
FN("cmd", cmd_auth, "Authenticate to PICC",
"Establish authentication to PICC via key <kno> of the currently selected\n" \
"application. <key> is the shared secret key to use for authentication.\n" \
"See help(\"key\") for further details.\n");


static int cmd_auth(lua_State *l)
{
  int result;
  uint8_t num;
  MifareDESFireKey k;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key number expected");
  result = desflua_get_key(l, 2, &k);
  if(result)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "key: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 2, lua_tostring(l, -1));
  }

  num = lua_tointeger(l, 1);

  result = mifare_desfire_authenticate(tag, num, k);
  desflua_handle_result(l, result, tag);
  mifare_desfire_key_free(k);


  return lua_gettop(l);
}




FN_ALIAS(cmd_cks) = { "cks", "ChangeKeySettings", NULL };
FN_PARAM(cmd_cks) =
{
  FNPARAM("settings", "New key settings", 0),
  FNPARAMEND
};
FN_RET(cmd_cks) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAMEND
};
FN("cmd", cmd_cks, "Change master key configuration",
"Changes the master key configuration of the currently selected application.\n" \
"The configuration must be given as an 8 bit number. See help(\"keysettings\")\n" \
"for further information.\n");


static int cmd_cks(lua_State *l)
{
  int result;
  uint8_t settings;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key settings must be a number");

  settings = lua_tointeger(l, 1);

  result = mifare_desfire_change_key_settings(tag, settings);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_gks) = { "gks", "GetKeySettings", NULL };
FN_PARAM(cmd_gks) =
{
  FNPARAMEND
};
FN_RET(cmd_gks) =
{
  FNPARAM("code",     "Return code",  0),
  FNPARAM("err",      "Error string", 0),
  FNPARAM("settings", "Key settings", 1),
  FNPARAMEND
};
FN("cmd", cmd_gks, "Get master key configuration",
"Retrieves the master key configuration of the currently selected application.\n" \
"When successful, <settings> contains the binary coded master key\n" \
"configuration. In case of an error, <settings> will be 'nil'. For further\n" \
"information about master key configurations see help(\"keysettings\").\n");


static int cmd_gks(lua_State *l)
{
  int result;
  uint8_t settings, maxkeys;


  result = mifare_desfire_get_key_settings(tag, &settings, &maxkeys);
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  lua_checkstack(l, 2);
  lua_pushinteger(l, settings);
  lua_pushinteger(l, maxkeys);


exit:
  return lua_gettop(l);
}




FN_ALIAS(cmd_ck) = { "ck", "ChangeKey", NULL };
FN_PARAM(cmd_ck) =
{
  FNPARAM("kno",  "Key number", 0),
  FNPARAM("knew", "Key number", 0),
  FNPARAM("kold", "Key number", 1),
  FNPARAMEND
};
FN_RET(cmd_ck) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAMEND
};
FN("cmd", cmd_ck, "Changes a key",
"Changes key <kno> of the currently selected application to <knew>. Depending\n" \
"on the master key settings, a prior authentification has to be achieved. The\n" \
"current value of <kno> has to specified via <kold> when authentication is\n" \
"required an the key change key differs from <kno>.\n");


static int cmd_ck(lua_State *l)
{
  int result;
  uint8_t num;
  int oldidx;
  MifareDESFireKey kold, knew;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key number expected");

  result = desflua_get_key(l, 2, &knew);
  if(result)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "new key: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 2, lua_tostring(l, -1));
  }

  oldidx = (lua_gettop(l) < 3 || lua_isnil(l, 3)) ? 2 : 3;
  result = desflua_get_key(l, oldidx, &kold);
  if(result)
  {
    mifare_desfire_key_free(knew);
    lua_checkstack(l, 1);
    lua_pushfstring(l, "old key: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 3, lua_tostring(l, -1));
  }


  num = lua_tointeger(l, 1);

  result = mifare_desfire_change_key(tag, num, knew, kold);
  desflua_handle_result(l, result, tag);
  mifare_desfire_key_free(kold);
  mifare_desfire_key_free(knew);


  return lua_gettop(l);
}




FN_ALIAS(cmd_gkv) = { "gkv", "GetKeyVersion", NULL };
FN_PARAM(cmd_gkv) =
{
  FNPARAM("kno", "Key number", 0),
  FNPARAMEND
};
FN_RET(cmd_gkv) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAM("ver",  "Key version",  1),
  FNPARAMEND
};
FN("cmd", cmd_gkv, "Get key version", \
"On success, the commands returns the version number of the key <kno> of the\n"
"currently selected application via the <ver> return value. In case of an\n"
"error, <ver> will be 'nil'.\n");


static int cmd_gkv(lua_State *l)
{
  int result;
  uint8_t num;
  uint8_t ver;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key number expected");

  num = lua_tointeger(l, 1);

  result = mifare_desfire_get_key_version(tag, num, &ver);
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  lua_checkstack(l, 1);
  lua_pushinteger(l, ver);


exit:
  return lua_gettop(l);
}
