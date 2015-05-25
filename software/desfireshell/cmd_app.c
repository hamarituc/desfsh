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



static int cmd_fileids(lua_State *l);
static int cmd_gfs(lua_State *l);
static int cmd_cfs(lua_State *l);
static int cmd_csdf(lua_State *l);
static int cmd_cbdf(lua_State *l);
static int cmd_cvf(lua_State *l);
static int cmd_clrf(lua_State *l);
static int cmd_ccrf(lua_State *l);
static int cmd_delf(lua_State *l);




FN_ALIAS(cmd_fileids) = { "fileids", "fids", "GetFileIDs", NULL };
FN_PARAM(cmd_fileids) =
{
  FNPARAMEND
};
FN_RET(cmd_fileids) =
{
  FNPARAM("code", "Return Code",      0),
  FNPARAM("err",  "Error String",     0),
  FNPARAM("fids", "List of File IDs", 1),
  FNPARAMEND
};
FN("cmd", cmd_fileids, "Get File List", NULL);


static int cmd_fileids(lua_State *l)
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

    debug_gen(DEBUG_OUT, "FID", "%d", fids[i]);
  }
  free(fids);


exit:
  return lua_gettop(l);
}




FN_ALIAS(cmd_gfs) = { "gfs", "GetFileSettings", NULL };
FN_PARAM(cmd_gfs) =
{
  FNPARAM("fid", "File ID", 0),
  FNPARAMEND
};
FN_RET(cmd_gfs) =
{
  FNPARAM("code",     "Return Code",   0),
  FNPARAM("err",      "Error String",  0),
  FNPARAM("settings", "File Settings", 1),
  FNPARAMEND
};
FN("cmd", cmd_gfs, "Get File Settings", NULL);


static int cmd_gfs(lua_State *l)
{
  int result;
  uint8_t fid;
  struct mifare_desfire_file_settings settings;
  const char *ftypestr;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");

  fid = lua_tointeger(l, 1);

  debug_gen(DEBUG_IN, "FID", "%d", fid);

  result = mifare_desfire_get_file_settings(tag, fid, &settings);
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  lua_checkstack(l, 3);
  lua_newtable(l);

  lua_pushinteger(l, settings.file_type);              lua_setfield(l, -2, "type");
  lua_pushinteger(l, settings.communication_settings); lua_setfield(l, -2, "comm");
  desflua_push_acl(l, settings.access_rights);         lua_setfield(l, -2, "acl");

  debug_comm(DEBUG_OUT, settings.communication_settings);
  debug_acl(DEBUG_OUT, settings.access_rights);

  switch(settings.file_type)
  {
  case MDFT_STANDARD_DATA_FILE:             ftypestr = "SDF"; break;
  case MDFT_BACKUP_DATA_FILE:               ftypestr = "BDF"; break;
  case MDFT_VALUE_FILE_WITH_BACKUP:         ftypestr = "VF";  break;
  case MDFT_LINEAR_RECORD_FILE_WITH_BACKUP: ftypestr = "LRF"; break;
  case MDFT_CYCLIC_RECORD_FILE_WITH_BACKUP: ftypestr = "CRF"; break;
  default:                                  ftypestr = "???"; break;
  }

  switch(settings.file_type)
  {
  case MDFT_STANDARD_DATA_FILE:
  case MDFT_BACKUP_DATA_FILE:
    lua_pushinteger(l, settings.settings.standard_file.file_size);
    lua_setfield(l, -2, "size");

    debug_gen(DEBUG_OUT, "FSET", "[%s]  SIZE: %d", ftypestr,
      settings.settings.standard_file.file_size);
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

    if(settings.settings.value_file.limited_credit_enabled)
      debug_gen(DEBUG_OUT, "FSET", "[%s]  LOWER: %d  UPPER: %d  LCRED: %d", ftypestr,
        settings.settings.value_file.lower_limit,
        settings.settings.value_file.upper_limit,
        settings.settings.value_file.limited_credit_value);
    else
      debug_gen(DEBUG_OUT, "FSET", "[%s]  LOWER: %d  UPPER: %d", ftypestr,
        settings.settings.value_file.lower_limit,
        settings.settings.value_file.upper_limit);
    break;

  case MDFT_LINEAR_RECORD_FILE_WITH_BACKUP:
  case MDFT_CYCLIC_RECORD_FILE_WITH_BACKUP:
    lua_pushinteger(l, settings.settings.linear_record_file.record_size);
    lua_setfield(l, -2, "recsize");
    lua_pushinteger(l, settings.settings.linear_record_file.max_number_of_records);
    lua_setfield(l, -2, "mrec");
    lua_pushinteger(l, settings.settings.linear_record_file.current_number_of_records);
    lua_setfield(l, -2, "crec");

    debug_gen(DEBUG_OUT, "FSET", "[%s]  RECSIZE: %d  MREC: %d  CREC: %d", ftypestr,
      settings.settings.linear_record_file.record_size,
      settings.settings.linear_record_file.max_number_of_records,
      settings.settings.linear_record_file.current_number_of_records);
    break;
  }


exit:
  return lua_gettop(l);
}




FN_ALIAS(cmd_cfs) = { "cfs", "ChangeFileSettings", NULL };
FN_PARAM(cmd_cfs) =
{
  FNPARAM("fid",  "File ID",                    0),
  FNPARAM("comm", "New Communication Settings", 0),
  FNPARAM("acl",  "New ACL",                    0),
  FNPARAMEND
};
FN_RET(cmd_cfs) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_cfs, "Change File Settings", NULL);


static int cmd_cfs(lua_State *l)
{
  int result;
  uint8_t fid;
  uint8_t comm;
  uint16_t acl;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  result = desflua_get_comm(l, 2, &comm);  if(result) { desflua_argerror(l, 2, "comm"); }
  result = desflua_get_acl(l, 3, &acl);    if(result) { desflua_argerror(l, 3, "acl");  }
  
  fid  = lua_tointeger(l, 1);
  comm = lua_tointeger(l, 2);

  debug_gen(DEBUG_IN, "FID", "%d", fid);
  debug_comm(DEBUG_IN, comm);

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
  result = desflua_get_comm(l, 2, &comm); if(result) { desflua_argerror(l, 2, "comm"); }
  result = desflua_get_acl(l, 3, &acl);   if(result) { desflua_argerror(l, 3, "acl");  }
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "file size must be a number");

  fid  = lua_tointeger(l, 1);
  comm = lua_tointeger(l, 2);
  size = lua_tointeger(l, 4);

  debug_gen(DEBUG_IN, "FID", "%d", fid);
  debug_comm(DEBUG_IN, comm);
  debug_acl(DEBUG_IN, acl);
  debug_gen(DEBUG_IN, "FSET", "[%s]  SIZE: %d", backup ? "BDF" : "SDF", size);

  if(backup)
    result = mifare_desfire_create_backup_data_file(tag, fid, comm, acl, size);
  else
    result = mifare_desfire_create_std_data_file(tag, fid, comm, acl, size);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_csdf) = { "csdf", "createsdf", "CreateStandardDataFile", NULL };
FN_PARAM(cmd_csdf) =
{
  FNPARAM("fid",  "File ID",                0),
  FNPARAM("comm", "Communication Settings", 0),
  FNPARAM("acl",  "ACL",                    0),
  FNPARAM("size", "File Size",              0),
  FNPARAMEND
};
FN_RET(cmd_csdf) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_csdf, "Create Standard Data File", NULL);


static int cmd_csdf(lua_State *l)
{
  return cmd_create_df(l, 0);
}




FN_ALIAS(cmd_cbdf) = { "cbdf", "createbdf", "CreateBackupDataFile", NULL };
FN_PARAM(cmd_cbdf) =
{
  FNPARAM("fid",  "File ID",                0),
  FNPARAM("comm", "Communication Settings", 0),
  FNPARAM("acl",  "ACL",                    0),
  FNPARAM("size", "File Size",              0),
  FNPARAMEND
};
FN_RET(cmd_cbdf) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_cbdf, "Create Backup Data File", NULL);


static int cmd_cbdf(lua_State *l)
{
  return cmd_create_df(l, 1);
}




FN_ALIAS(cmd_cvf) = { "cfv", "createvf", "CreateValueFile", NULL };
FN_PARAM(cmd_cvf) =
{
  FNPARAM("fid",   "File ID",                 0),
  FNPARAM("comm",  "Communication Settings",  0),
  FNPARAM("acl",   "ACL",                     0),
  FNPARAM("lower", "Lower Limit",             0),
  FNPARAM("upper", "Upper Limit",             0),
  FNPARAM("value", "Initial Value",           0),
  FNPARAM("lcred", "Limited Credit enabled?", 1),
  FNPARAMEND
};
FN_RET(cmd_cvf) =
{
  FNPARAM("code", "Return code",  0),
  FNPARAM("err",  "Error string", 0),
  FNPARAMEND
};
FN("cmd", cmd_cvf, "Create Value File", NULL);


static int cmd_cvf(lua_State *l)
{
  int result;
  uint8_t fid;
  uint8_t comm;
  uint16_t acl;
  uint32_t lower, upper, value;
  unsigned char lcredit;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  result = desflua_get_comm(l, 2, &comm); if(result) { desflua_argerror(l, 2, "comm"); }
  result = desflua_get_acl(l, 3, &acl);   if(result) { desflua_argerror(l, 3, "acl");  }
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "lower limit must be a number");
  luaL_argcheck(l, lua_isnumber(l, 5), 5, "upper limit must be a number");
  luaL_argcheck(l, lua_isnumber(l, 6), 6, "initial value must be a number");

  fid     = lua_tointeger(l, 1);
  comm    = lua_tointeger(l, 2);
  lower   = lua_tointeger(l, 4);
  upper   = lua_tointeger(l, 5);
  value   = lua_tointeger(l, 6);
  lcredit = lua_toboolean(l, 7);

  debug_gen(DEBUG_IN, "FID", "%d", fid);
  debug_comm(DEBUG_IN, comm);
  debug_acl(DEBUG_IN, acl);
  debug_gen(DEBUG_IN, "FSET", "[VF]  LOWER: %d  UPPER: %d  INIT: %d%s",
    lower, upper, value, lcredit ? "  LCRED" : "");

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
  result = desflua_get_comm(l, 2, &comm); if(result) { desflua_argerror(l, 2, "comm"); }
  result = desflua_get_acl(l, 3, &acl);   if(result) { desflua_argerror(l, 3, "acl");  }
  luaL_argcheck(l, lua_isnumber(l, 4), 4, "record size must be a number");
  luaL_argcheck(l, lua_isnumber(l, 5), 5, "maximum number of recotds must be a number");

  fid     = lua_tointeger(l, 1);
  comm    = lua_tointeger(l, 2);
  recsize = lua_tointeger(l, 4);
  maxrecs = lua_tointeger(l, 5);

  debug_gen(DEBUG_IN, "FID", "%d", fid);
  debug_comm(DEBUG_IN, comm);
  debug_acl(DEBUG_IN, acl);
  debug_gen(DEBUG_IN, "FSET", "[%s]  RECSIZE: %d  MREC: %d",
    cyclic ? "CRF" : "LRF", recsize, maxrecs);

  if(cyclic)
    result = mifare_desfire_create_cyclic_record_file(tag, fid, comm, acl, recsize, maxrecs);
  else
    result = mifare_desfire_create_linear_record_file(tag, fid, comm, acl, recsize, maxrecs);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_clrf) = { "clrf", "createlrf", "CreateLinearRecordFile", NULL };
FN_PARAM(cmd_clrf) =
{
  FNPARAM("fid",     "File ID",                 0),
  FNPARAM("comm",    "Communication Settings",  0),
  FNPARAM("acl",     "ACL",                     0),
  FNPARAM("recsize", "Record Size",             0),
  FNPARAM("nrecs",   "Number of Records",       0),
  FNPARAMEND
};
FN_RET(cmd_clrf) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_clrf, "Create Linear Record File", NULL);


static int cmd_clrf(lua_State *l)
{
  return cmd_create_rf(l, 0);
}




FN_ALIAS(cmd_ccrf) = { "ccrf", "createcrf", "CreateCyclicRecordFile", NULL };
FN_PARAM(cmd_ccrf) =
{
  FNPARAM("fid",     "File ID",                 0),
  FNPARAM("comm",    "Communication Settings",  0),
  FNPARAM("acl",     "ACL",                     0),
  FNPARAM("recsize", "Record Size",             0),
  FNPARAM("nrecs",   "Number of Records",       0),
  FNPARAMEND
};
FN_RET(cmd_ccrf) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_ccrf, "Create Cyclic Record File", NULL);


static int cmd_ccrf(lua_State *l)
{
  return cmd_create_rf(l, 1);
}




FN_ALIAS(cmd_delf) = { "delf", "df", "DeleteFile", NULL };
FN_PARAM(cmd_delf) =
{
  FNPARAM("fid", "File ID", 0),
  FNPARAMEND
};
FN_RET(cmd_delf) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_delf, "Delete File", NULL);


static int cmd_delf(lua_State *l)
{
  int result;
  uint8_t fid;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");

  fid = lua_tointeger(l, 1);

  debug_gen(DEBUG_IN, "FID", "%d", fid);

  result = mifare_desfire_delete_file(tag, fid);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}
