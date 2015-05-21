#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "desflua.h"
#include "desfsh.h"



int cmd_fileids(lua_State *l)
{
  int result;
  uint8_t *fids;
  size_t len;
  unsigned int i;


  result = mifare_desfire_get_file_ids(tag, &fids, &len);
  desflua_handle_result(l, result, tag);

  if(result)
    goto exit;

  lua_checkstack(l, 3);
  lua_newtable(l);
  for(i = 0; i < len; i++)
  {
    lua_pushinteger(l, i + 1);
    lua_pushinteger(l, fids[i]);
    lua_settable(l, -3);
  }
  free(fids);


exit:
  return lua_gettop(l);
}


int cmd_gfs(lua_State *l)
{
  int result;
  uint8_t fid;
  struct mifare_desfire_file_settings settings;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");

  fid = lua_tointeger(l, 1);

  result = mifare_desfire_get_file_settings(tag, fid, &settings);
  desflua_handle_result(l, result, tag);

  if(result)
    goto exit;

  lua_checkstack(l, 3);
  lua_newtable(l);

  lua_pushinteger(l, settings.file_type);              lua_setfield(l, -2, "type");
  lua_pushinteger(l, settings.communication_settings); lua_setfield(l, -2, "comm");
  desflua_push_acl(l, settings.access_rights);         lua_setfield(l, -2, "acl");

  switch(settings.file_type)
  {
  case MDFT_STANDARD_DATA_FILE:
  case MDFT_BACKUP_DATA_FILE:
    lua_pushinteger(l, settings.settings.standard_file.file_size);
    lua_setfield(l, -2, "size");
    break;

  case MDFT_VALUE_FILE_WITH_BACKUP:
    lua_pushinteger(l, settings.settings.value_file.lower_limit);
    lua_setfield(l, -2, "lower");
    lua_pushinteger(l, settings.settings.value_file.upper_limit);
    lua_setfield(l, -2, "upper");
    if(settings.settings.value_file.limited_credit_enabled)
    {
      lua_pushinteger(l, settings.settings.value_file.limited_credit_value);
      lua_setfield(l, -2, "lcred");
    }
    break;

  case MDFT_LINEAR_RECORD_FILE_WITH_BACKUP:
  case MDFT_CYCLIC_RECORD_FILE_WITH_BACKUP:
    lua_pushinteger(l, settings.settings.linear_record_file.record_size);
    lua_setfield(l, -2, "recsize");
    lua_pushinteger(l, settings.settings.linear_record_file.max_number_of_records);
    lua_setfield(l, -2, "mrec");
    lua_pushinteger(l, settings.settings.linear_record_file.current_number_of_records);
    lua_setfield(l, -2, "crec");
    break;
  }


exit:
  return lua_gettop(l);
}


int cmd_cfs(lua_State *l)
{
  int result;
  uint8_t fid;
  uint8_t comm;
  uint16_t acl;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "communication settings must be a number");

  result = desflua_get_acl(l, 3, &acl);
  if(result)
    return luaL_argerror(l, 1, lua_tostring(l, -1));
  
  fid  = lua_tointeger(l, 1);
  comm = lua_tointeger(l, 2);


  result = mifare_desfire_change_file_settings(tag, fid, comm, acl);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


int cmd_csdf(lua_State *l)
{
  int result;
  uint8_t fid;
  uint8_t comm;
  uint16_t acl;
  uint32_t size;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "communication settings must be a number");

  result = desflua_get_acl(l, 3, &acl);
  if(result)
    return luaL_argerror(l, 1, lua_tostring(l, -1));
  
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "file size must be a number");

  fid  = lua_tointeger(l, 1);
  comm = lua_tointeger(l, 2);
  size = lua_tointeger(l, 4);


  result = mifare_desfire_create_std_data_file(tag, fid, comm, acl, size);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


/*int cmd_getver(lua_State *l)
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
}*/
