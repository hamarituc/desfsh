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

  if(result < 0)
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

  if(result < 0)
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
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "acl: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 1, lua_tostring(l, -1));
  }
  
  fid  = lua_tointeger(l, 1);
  comm = lua_tointeger(l, 2);


  result = mifare_desfire_change_file_settings(tag, fid, comm, acl);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


static int cmd_create_df(lua_State *l, unsigned char backup)
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
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "acl: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 3, lua_tostring(l, -1));
  }
  
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "file size must be a number");

  fid  = lua_tointeger(l, 1);
  comm = lua_tointeger(l, 2);
  size = lua_tointeger(l, 4);


  if(backup)
    result = mifare_desfire_create_backup_data_file(tag, fid, comm, acl, size);
  else
    result = mifare_desfire_create_std_data_file(tag, fid, comm, acl, size);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


int cmd_csdf(lua_State *l)
{
  return cmd_create_df(l, 0);
}


int cmd_cbdf(lua_State *l)
{
  return cmd_create_df(l, 1);
}


int cmd_cvf(lua_State *l)
{
  int result;
  uint8_t fid;
  uint8_t comm;
  uint16_t acl;
  uint32_t lower, upper, value;
  unsigned char lcredit;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "communication settings must be a number");

  result = desflua_get_acl(l, 3, &acl);
  if(result)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "acl: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 1, lua_tostring(l, -1));
  }
  
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "lower limit must be a number");
  luaL_argcheck(l, lua_isnumber(l, 5), 5, "upper limit must be a number");
  luaL_argcheck(l, lua_isnumber(l, 6), 6, "initial value must be a number");

  fid     = lua_tointeger(l, 1);
  comm    = lua_tointeger(l, 2);
  lower   = lua_tointeger(l, 4);
  upper   = lua_tointeger(l, 5);
  value   = lua_tointeger(l, 6);
  lcredit = lua_toboolean(l, 7);


  result = mifare_desfire_create_value_file(tag, fid, comm, acl, lower, upper, value, lcredit);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


static int cmd_create_rf(lua_State *l, unsigned char cyclic)
{
  int result;
  uint8_t fid;
  uint8_t comm;
  uint16_t acl;
  uint32_t recsize, maxrecs;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "communication settings must be a number");

  result = desflua_get_acl(l, 3, &acl);
  if(result)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "acl: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 1, lua_tostring(l, -1));
  }
  
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "record size must be a number");
  luaL_argcheck(l, lua_isnumber(l, 5), 5, "maximum number of recotds must be a number");

  fid     = lua_tointeger(l, 1);
  comm    = lua_tointeger(l, 2);
  recsize = lua_tointeger(l, 4);
  maxrecs = lua_tointeger(l, 5);


  if(cyclic)
    result = mifare_desfire_create_cyclic_record_file(tag, fid, comm, acl, recsize, maxrecs);
  else
    result = mifare_desfire_create_linear_record_file(tag, fid, comm, acl, recsize, maxrecs);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


int cmd_clrf(lua_State *l)
{
  return cmd_create_rf(l, 0);
}


int cmd_ccrf(lua_State *l)
{
  return cmd_create_rf(l, 1);
}


int cmd_delf(lua_State *l)
{
  int result;
  uint8_t fid;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");

  fid = lua_tointeger(l, 1);

  result = mifare_desfire_delete_file(tag, fid);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}
