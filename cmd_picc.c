/*
 * DESFire-Shell: Modify MIFARE DESFire Cards
 *
 * Copyright (C) 2015-2021 Mario Haustein
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see https://www.gnu.org/licenses/.
 */

#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "debug.h"
#include "desflua.h"
#include "desfsh.h"
#include "fn.h"
#include "key.h"



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
  FNPARAM("keytype",  "Key Type",            0),
  FNPARAM("aid",      "Application ID",      0),
  FNPARAM("settings", "Master Key Settings", 0),
  FNPARAM("nkeys",    "Number of Keys",      0),
  FNPARAMEND
};
FN_RET(cmd_createapp) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_createapp, "Create Application", NULL);


static int cmd_createapp(lua_State *l)
{
  int result;
  enum keytype_e type;
  char *typestr;
  uint32_t aid;
  MifareDESFireAID app;
  uint8_t settings;
  uint8_t maxkeys;


  result = key_gettype(l, 1, &type, &typestr); if(result) { desflua_argerror(l, 1, "keytype"); }
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "AID must be a number");
  luaL_argcheck(l, lua_isnumber(l, 3), 3, "key settings must be a number");
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "number of keys expected");

  aid = lua_tointeger(l, 2);
  app = mifare_desfire_aid_new(aid);
  if(app == NULL)
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);

  settings = lua_tointeger(l, 3);
  maxkeys  = lua_tointeger(l, 4);

  if(maxkeys > 14)
    return luaL_argerror(l, 4, "at most 14 keys allowed");

  switch(type)
  {
  case _DES_:                                          break;
  case _3DES_:                                         break;
  case _3K3DES_: maxkeys |= APPLICATION_CRYPTO_3K3DES; break;
  case _AES_:    maxkeys |= APPLICATION_CRYPTO_AES;    break;
  }

  debug_cmd("CreateApplication");
  debug_gen(DEBUG_IN, "KTYPE", "%s", typestr);
  debug_gen(DEBUG_IN, "AID", "0x%06x", aid);
  debug_keysettings(DEBUG_IN, settings);
  debug_gen(DEBUG_IN, "MAXKEYS",  "%d", maxkeys & 0x0f);

  result = mifare_desfire_create_application(tag, app, settings, maxkeys);
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
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_deleteapp, "Delete Application", NULL);


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

  debug_cmd("DeleteApplication");
  debug_gen(DEBUG_IN, "AID", "0x%06x", aid);

  result = mifare_desfire_delete_application(tag, app);

  free(app);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_appids) = { "appids", "aids", "GetApplicationIDs", NULL };
FN_PARAM(cmd_appids) =
{
  FNPARAMEND
};
FN_RET(cmd_appids) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAM("aids", "List of AIDs", 1),
  FNPARAMEND
};
FN("cmd", cmd_appids, "Get Application List", NULL);


static int cmd_appids(lua_State *l)
{
  int result;
  MifareDESFireAID *apps;
  size_t len, i;
  uint32_t aid;
  char buffer[10];


  debug_cmd("GetApplicationIDs");

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

    debug_gen(DEBUG_OUT, "AID", "0x%06x", aid);
  }
  mifare_desfire_free_application_ids(apps);


exit:
  return lua_gettop(l);
}




FN_ALIAS(cmd_selapp) = { "selapp", "select", "SelectApplication", NULL };
FN_PARAM(cmd_selapp) =
{
  FNPARAM("aid", "AID", 0),
  FNPARAMEND
};
FN_RET(cmd_selapp) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_selapp, "Select Application", NULL);


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

  debug_cmd("SelectApplication");
  debug_gen(DEBUG_IN, "AID", "0x%06x", aid);

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
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_format, "Format PICC", NULL);


static int cmd_format(lua_State *l)
{
  int result;


  debug_cmd("FormatPICC");
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
  FNPARAM("code", "Return Code",      0),
  FNPARAM("err",  "Error String",     0),
  FNPARAM("info", "PICC Information", 1),
  FNPARAMEND
};
FN("cmd", cmd_getver, "Get PICC Information", NULL);


static int cmd_getver(lua_State *l)
{
  int result;
  struct mifare_desfire_version_info info;
  char buffer[20], *bufferpos;
  unsigned int e1, e2, s1, s2, u1, u2;
  static const char units[] = { ' ', 'K', 'M', 'G', 'T' };


  debug_cmd("GetVersion");
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

  e1 = (info.hardware.storage_size >> 1);
  e2 = (info.hardware.storage_size >> 1) + 1;
  u1 = 0;
  u2 = 0;
  while(u1 < 5 && e1 >= 10) { u1++; e1 -= 10; } 
  while(u2 < 5 && e2 >= 10) { u2++; e2 -= 10; } 
  s1 = 1 << e1;
  s2 = 1 << e2;

  bufferpos = buffer;
  bufferpos += sprintf(bufferpos, "%d%c", s1, units[u1]);
  if(info.hardware.storage_size & 0x01)
    bufferpos += sprintf(bufferpos, " .. %d%c", s2, units[u2]);

  debug_gen(DEBUG_OUT, "PICCINFO", "   [HW]  Vend: 0x%02x  Type: %d.%d  Ver: %d.%d  Size: %s  Prot: %d",
    info.hardware.vendor_id,
    info.hardware.type, info.hardware.subtype,
    info.hardware.version_major, info.hardware.version_minor,
    buffer, info.hardware.protocol);


  lua_newtable(l);
  lua_pushinteger(l, info.software.vendor_id);     lua_setfield(l, -2, "vendor");
  lua_pushinteger(l, info.software.type);          lua_setfield(l, -2, "type");
  lua_pushinteger(l, info.software.subtype);       lua_setfield(l, -2, "subtype");
  lua_pushinteger(l, info.software.version_major); lua_setfield(l, -2, "major");
  lua_pushinteger(l, info.software.version_minor); lua_setfield(l, -2, "minor");
  lua_pushinteger(l, info.software.storage_size);  lua_setfield(l, -2, "size");
  lua_pushinteger(l, info.software.protocol);      lua_setfield(l, -2, "protocol");
  lua_setfield(l, -2, "software");

  e1 = (info.software.storage_size >> 1);
  e2 = (info.software.storage_size >> 1) + 1;
  u1 = 0;
  u2 = 0;
  while(u1 < 5 && e1 >= 10) { u1++; e1 -= 10; } 
  while(u2 < 5 && e2 >= 10) { u2++; e2 -= 10; } 
  s1 = 1 << e1;
  s2 = 1 << e2;

  bufferpos = buffer;
  bufferpos += sprintf(bufferpos, "%d%c", s1, units[u1]);
  if(info.software.storage_size & 0x01)
    bufferpos += sprintf(bufferpos, " .. %d%c", s2, units[u2]);

  debug_gen(DEBUG_OUT, "PICCINFO", "   [SW]  Vend: 0x%02x  Type: %d.%d  Ver: %d.%d  Size: %s  Prot: %d",
    info.software.vendor_id,
    info.software.type, info.software.subtype,
    info.software.version_major, info.software.version_minor,
    buffer, info.software.protocol);


  snprintf(buffer, 20, "%02x%02x%02x%02x%02x%02x%02x", 
    info.uid[0], info.uid[1], info.uid[2], info.uid[3], info.uid[4], info.uid[5], info.uid[6]);
  lua_pushstring(l, buffer); lua_setfield(l, -2, "uid");

  debug_gen(DEBUG_OUT, "PICCINFO", "  [UID]  %02x%02x%02x%02x%02x%02x%02x",
    info.uid[0], info.uid[1], info.uid[2], info.uid[3], info.uid[4], info.uid[5], info.uid[6]);


  snprintf(buffer, 20, "%02x%02x%02x%02x%02x",
    info.batch_number[0], info.batch_number[1], info.batch_number[2], info.batch_number[3], info.batch_number[4]);
  lua_pushstring(l, buffer); lua_setfield(l, -2, "batch");

  debug_gen(DEBUG_OUT, "PICCINFO", "[BATCH]  %02x%02x%02x%02x%02x",
    info.batch_number[0], info.batch_number[1], info.batch_number[2], info.batch_number[3], info.batch_number[4]);


  snprintf(buffer, 20, "%x", info.production_week);
  lua_pushstring(l, buffer); lua_setfield(l, -2, "prodweek");

  snprintf(buffer, 20, "%x", info.production_year);
  lua_pushstring(l, buffer); lua_setfield(l, -2, "prodyear");

  debug_gen(DEBUG_OUT, "PICCINFO", " [PROD]  %02x/%02x", info.production_week, info.production_year);


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
  FNPARAM("code", "Return Code",         0),
  FNPARAM("err",  "Error String",        0),
  FNPARAM("size", "Size of free Memory", 1),
  FNPARAMEND
};
FN("cmd", cmd_freemem, "Get Size of remaining Memory", NULL);


static int cmd_freemem(lua_State *l)
{
  int result;
  uint32_t freemem;


  debug_cmd("FreeMem");
  result = mifare_desfire_free_mem(tag, &freemem);
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  lua_pushinteger(l, freemem);

  debug_gen(DEBUG_OUT, "FREE", "%d", freemem);


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
  FNPARAM("code", "Return Code",   0),
  FNPARAM("err",  "Error String",  0),
  FNPARAM("uid",  "Real Card UID", 1),
  FNPARAMEND
};
FN("cmd", cmd_carduid, "Get Real Card UID", NULL);


static int cmd_carduid(lua_State *l)
{
  int result;
  char *uid;


  debug_cmd("GetCardUID");
  result = mifare_desfire_get_card_uid(tag, &uid);
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  lua_pushstring(l, uid);

  debug_gen(DEBUG_OUT, "UID", "0x%s", uid);


exit:
  return lua_gettop(l);
}
