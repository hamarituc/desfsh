#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "debug.h"
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
  FNPARAM("kno", "Key Number",         0),
  FNPARAM("key", "Authentication Key", 0),
  FNPARAMEND
};
FN_RET(cmd_auth) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_auth, "Authenticate to PICC",
"Establish authentication to PICC via key <kno> of the currently selected\n" \
"application. <key> is the shared secret key to use for authentication.\n" \
"See help(\"key\") for further details.\n");


static int cmd_auth(lua_State *l)
{
  int result;
  uint8_t keyno;
  MifareDESFireKey k;
  char *keystr;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key number expected");
  result = desflua_get_key(l, 2, &k, &keystr); if(result) { desflua_argerror(l, 2, "key"); }

  keyno = lua_tointeger(l, 1);

  debug_gen(DEBUG_IN, "KNO", "%d", keyno);
  debug_gen(DEBUG_IN, "KEY", "%s", keystr);
  free(keystr);

  result = mifare_desfire_authenticate(tag, keyno, k);
  desflua_handle_result(l, result, tag);
  mifare_desfire_key_free(k);


  return lua_gettop(l);
}




FN_ALIAS(cmd_cks) = { "cks", "ChangeKeySettings", NULL };
FN_PARAM(cmd_cks) =
{
  FNPARAM("settings", "New Key Settings", 0),
  FNPARAMEND
};
FN_RET(cmd_cks) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_cks, "Change Master Key Configuration",
"Changes the master key configuration of the currently selected application.\n" \
"The configuration must be given as an 8 bit number. See help(\"keysettings\")\n" \
"for further information.\n");


static int cmd_cks(lua_State *l)
{
  int result;
  uint8_t settings;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key settings must be a number");

  settings = lua_tointeger(l, 1);

  debug_keysettings(DEBUG_IN, settings);

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
  FNPARAM("code",     "Return Code",  0),
  FNPARAM("err",      "Error String", 0),
  FNPARAM("settings", "Key Settings", 1),
  FNPARAMEND
};
FN("cmd", cmd_gks, "Get Master Key Configuration",
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

  debug_keysettings(DEBUG_OUT, settings);
  debug_gen(DEBUG_OUT, "MAXKEYS", "%d", maxkeys);


exit:
  return lua_gettop(l);
}




FN_ALIAS(cmd_ck) = { "ck", "ChangeKey", NULL };
FN_PARAM(cmd_ck) =
{
  FNPARAM("kno",  "Key number", 0),
  FNPARAM("knew", "New Key",    0),
  FNPARAM("kold", "Old Key",    1),
  FNPARAMEND
};
FN_RET(cmd_ck) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAMEND
};
FN("cmd", cmd_ck, "Change Key",
"Changes key <kno> of the currently selected application to <knew>. Depending\n" \
"on the master key settings, a prior authentification has to be achieved. The\n" \
"current value of <kno> has to specified via <kold> when authentication is\n" \
"required an the key change key differs from <kno>.\n");


static int cmd_ck(lua_State *l)
{
  int result;
  uint8_t keyno;
  int oldidx;
  MifareDESFireKey kold, knew;
  char *knewstr, *koldstr;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key number expected");
  result = desflua_get_key(l, 2, &knew, &knewstr); if(result) { desflua_argerror(l, 2, "new key"); }

  oldidx = (lua_gettop(l) < 3 || lua_isnil(l, 3)) ? 2 : 3;
  result = desflua_get_key(l, oldidx, &kold, &koldstr);
  if(result)
  {
    mifare_desfire_key_free(knew);
    free(knewstr);
    desflua_argerror(l, 3, "old key");
  }

  keyno = lua_tointeger(l, 1);

  debug_gen(DEBUG_IN, "KNO",  "%d", keyno);
  debug_gen(DEBUG_IN, "KNEW", "%s", knewstr);
  debug_gen(DEBUG_IN, "KOLD", "%s", koldstr);
  free(knewstr);
  free(koldstr);

  result = mifare_desfire_change_key(tag, keyno, knew, kold);
  desflua_handle_result(l, result, tag);
  mifare_desfire_key_free(kold);
  mifare_desfire_key_free(knew);


  return lua_gettop(l);
}




FN_ALIAS(cmd_gkv) = { "gkv", "GetKeyVersion", NULL };
FN_PARAM(cmd_gkv) =
{
  FNPARAM("kno", "Key Number", 0),
  FNPARAMEND
};
FN_RET(cmd_gkv) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAM("ver",  "Key Version",  1),
  FNPARAMEND
};
FN("cmd", cmd_gkv, "Get Key Version", \
"On success, the commands returns the version number of the key <kno> of the\n"
"currently selected application via the <ver> return value. In case of an\n"
"error, <ver> will be 'nil'.\n");


static int cmd_gkv(lua_State *l)
{
  int result;
  uint8_t keyno;
  uint8_t ver;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "key number expected");

  keyno = lua_tointeger(l, 1);

  debug_gen(DEBUG_IN, "KNO", "%d", keyno);

  result = mifare_desfire_get_key_version(tag, keyno, &ver);
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  lua_checkstack(l, 1);
  lua_pushinteger(l, ver);

  debug_gen(DEBUG_OUT, "KVER", "%d", ver);


exit:
  return lua_gettop(l);
}
