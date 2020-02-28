#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "buffer.h"
#include "cmd.h"
#include "debug.h"
#include "desflua.h"
#include "desfsh.h"
#include "fn.h"



static int cmd_read(lua_State *l);
static int cmd_write(lua_State *l);
static int cmd_getval(lua_State *l);
static int cmd_credit(lua_State *l);
static int cmd_debit(lua_State *l);
static int cmd_lcredit(lua_State *l);
static int cmd_wrec(lua_State *l);
static int cmd_rrec(lua_State *l);
static int cmd_crec(lua_State *l);
static int cmd_commit(lua_State *l);
static int cmd_abort(lua_State *l);




static int cmd_read_gen(lua_State *l, char op)
{
  int result;
  unsigned char hascomm;
  uint8_t fid;
  uint32_t off, len;
  uint8_t comm;
  int nocheck;
  uint8_t *data;
  struct mifare_desfire_file_settings settings;
  uint32_t datalen;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "offset must be a number");
  luaL_argcheck(l, lua_isnumber(l, 3), 3, "length must be a number");
  hascomm = lua_gettop(l) >= 4;
  if(hascomm)
  {
    result = desflua_get_comm(l, 4, &comm);
    if(result)
      desflua_argerror(l, 4, "comm");
  }

  nocheck = 0;
  if(lua_gettop(l) >= 5)
    nocheck = lua_toboolean(l, 5);

  fid = lua_tointeger(l, 1);
  off = lua_tointeger(l, 2);
  len = lua_tointeger(l, 3);

  switch(op)
  {
    case 'f': debug_cmd("ReadData");   break;
    case 'r': debug_cmd("ReadRecord"); break;
  }

  debug_gen(DEBUG_IN, "FID", "%d", fid);
  debug_gen(DEBUG_IN, "OFF", "%d", off);
  debug_gen(DEBUG_IN, "LEN", "%d", len);


  /*
   * Wir benötigen die Größe der Datei, damit wir einen entsprechend großen
   * Puffer allokieren können, wenn das len-Argument 0 ist.
   *
   * Außerdem müssen wir den Fall abfangen, dass read() oder rrec() auf
   * ein Werte-File aufgerufen wird, da die entsprechende Operation in
   * libfreefare dann abort() aufruft.
   *
   * Unglücklicher Weise kann der Zugriff auf diese Information auf den
   * Application Master Key beschränkt sein :-/ Dann wird aber auch ein
   * Aufruf der read()-Funktion mit einem Fehler abbrechen.
   *
   * Wir können die Einstellungen auch nicht auf gut Glück abfragen, da
   * wir dann u.U. die Auth. verlieren. Deswegen erlauben wir die
   * Möglichkeit diese Prüfung zu überspringen. Der Nutzer muss dann
   * sicherstellen, dass er auf dem korrekten File-Typ operiert und
   * der Längenparameter größer als Null ist.
   */

  datalen = len;

  if(!nocheck)
  {
    debug_info("Executing GetFileSettings() to determine file type and size.");
    result = mifare_desfire_get_file_settings(tag, fid, &settings);
    if(result >= 0)
    {
      if(len == 0)
      {
        switch(settings.file_type)
        {
        case MDFT_STANDARD_DATA_FILE:
        case MDFT_BACKUP_DATA_FILE:
          datalen = settings.settings.standard_file.file_size;
          debug_info("  --> %d bytes", datalen);
          break;

        case MDFT_LINEAR_RECORD_FILE_WITH_BACKUP:
        case MDFT_CYCLIC_RECORD_FILE_WITH_BACKUP:
          datalen = settings.settings.linear_record_file.current_number_of_records *
                    settings.settings.linear_record_file.record_size;
          debug_info("  --> %d bytes (%d records, %d bytes/record)", datalen,
            settings.settings.linear_record_file.current_number_of_records,
            settings.settings.linear_record_file.record_size);
          break;

        case MDFT_VALUE_FILE_WITH_BACKUP:
          return luaL_error(l, "Operation not supported for value files.");

        default:
          return luaL_error(l, "Operation not supported for file type %d.", settings.file_type);
        }
      }
    }
    else
    {
      debug_info("  --> Command Failed. Unable to determine file size. Authentication lost.");
      debug_info("  --> To skip this check set the nocheck parameter to true and ensure to operate on the proper file type and specify a nonzero length parameter.");
    }
  }


  /*
   * Prüfen, ob wir die Anzahl der zu lesenden Bytes kennen.
   */

  if(datalen == 0)
    return luaL_error(l, "Length parameter is 0 and unable to determine file size. You have to specify a non zero length.");


  /*
   * Jetzt haben wir alle Hindernisse für das read()-Kommando aus dem Weg
   * geräumt. Eventuelle Fehler werden nun über den Rückgabewert von read()
   * abgehandelt.
   */

  data = (uint8_t*)malloc(datalen * sizeof(uint8_t));
  if(data == NULL)
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);

  if(hascomm)
  {
    switch(op)
    {
    case 'f': result = mifare_desfire_read_data_ex(tag, fid, off, len, data, comm);    break;
    case 'r': result = mifare_desfire_read_records_ex(tag, fid, off, len, data, comm); break;
    }
  }
  else
  {
    switch(op)
    {
    case 'f': result = mifare_desfire_read_data(tag, fid, off, len, data);    break;
    case 'r': result = mifare_desfire_read_records(tag, fid, off, len, data); break;
    }
  }
  desflua_handle_result(l, result, tag);

  if(result < 0)
    goto exit;

  buffer_push(l, data, result);

  debug_buffer(DEBUG_OUT, data, result, op == 'r' ? 0 : off);


exit:
  free(data);
  return lua_gettop(l);
}


static int cmd_write_gen(lua_State *l, char op)
{
  int result;
  unsigned char hascomm;
  uint8_t fid;
  uint32_t off, len;
  uint8_t *data;
  uint8_t comm;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "offset must be a number");
  result = buffer_get(l, 3, &data, &len);  if(result) { desflua_argerror(l, 3, "buffer"); }
  hascomm = lua_gettop(l) >= 4;
  if(hascomm)
  {
    result = desflua_get_comm(l, 4, &comm);
    if(result)
      desflua_argerror(l, 4, "comm");
  }

  fid = lua_tointeger(l, 1);
  off = lua_tointeger(l, 2);

  switch(op)
  {
    case 'f': debug_cmd("WriteData");   break;
    case 'r': debug_cmd("WriteRecord"); break;
  }

  debug_gen(DEBUG_IN, "FID", "%d", fid);
  debug_gen(DEBUG_IN, "OFF", "%d", off);
  debug_buffer(DEBUG_IN, data, len, off);

  if(hascomm)
  {
    switch(op)
    {
    case 'f': result = mifare_desfire_write_data_ex(tag, fid, off, len, data, comm);   break;
    case 'r': result = mifare_desfire_write_record_ex(tag, fid, off, len, data, comm); break;
    }
  }
  else
  {
    switch(op)
    {
    case 'f': result = mifare_desfire_write_data(tag, fid, off, len, data);   break;
    case 'r': result = mifare_desfire_write_record(tag, fid, off, len, data); break;
    }
  }

  free(data);

  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_read) = { "read", "ReadData", NULL };
FN_PARAM(cmd_read) =
{
  FNPARAM("fid",     "File ID",                       0),
  FNPARAM("offset",  "Offset",                        0),
  FNPARAM("len",     "Length",                        0),
  FNPARAM("comm",    "Communication Settings",        1),
  FNPARAM("nocheck", "Skip file type and size check", 1),
  FNPARAMEND
};
FN_RET(cmd_read) =
{
  FNPARAM("code",   "Return Code",  0),
  FNPARAM("err",    "Error String", 0),
  FNPARAM("buffer", "File Content", 1),
  FNPARAMEND
};
FN("cmd", cmd_read, "Read from File", NULL);


static int cmd_read(lua_State *l)
{
  return cmd_read_gen(l, 'f');
}




FN_ALIAS(cmd_write) = { "write", "WriteData", NULL };
FN_PARAM(cmd_write) =
{
  FNPARAM("fid",    "File ID",                0),
  FNPARAM("offset", "Offset",                 0),
  FNPARAM("buffer", "Content",                0),
  FNPARAM("comm",   "Communication Settings", 1),
  FNPARAMEND
};
FN_RET(cmd_write) =
{
  FNPARAM("code",   "Return Code",  0),
  FNPARAM("err",    "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_write, "Write to File", NULL);


static int cmd_write(lua_State *l)
{
  return cmd_write_gen(l, 'f');
}




FN_ALIAS(cmd_getval) = { "getval", "value", "GetValue", NULL };
FN_PARAM(cmd_getval) =
{
  FNPARAM("fid",  "File ID",                0),
  FNPARAM("comm", "Communication Settings", 1),
  FNPARAMEND
};
FN_RET(cmd_getval) =
{
  FNPARAM("code",  "Return Code",  0),
  FNPARAM("err",   "Error String", 0),
  FNPARAM("value", "File Value",   1),
  FNPARAMEND
};
FN("cmd", cmd_getval, "Get Value of File", NULL);


static int cmd_getval(lua_State *l)
{
  int result;
  unsigned char hascomm;
  uint8_t fid;
  uint8_t comm;
  int32_t val;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  hascomm = lua_gettop(l) >= 2;
  if(hascomm)
  {
    result = desflua_get_comm(l, 2, &comm);
    if(result)
      desflua_argerror(l, 2, "comm");
  }

  fid = lua_tointeger(l, 1);

  debug_cmd("GetValue");
  debug_gen(DEBUG_IN, "FID", "%d", fid);

  if(hascomm)
    result = mifare_desfire_get_value_ex(tag, fid, &val, comm);
  else
    result = mifare_desfire_get_value(tag, fid, &val);
  desflua_handle_result(l, result, tag);
  if(result < 0)
    goto exit;

  lua_pushinteger(l, val);

  debug_gen(DEBUG_OUT, "VAL", "%d", val);


exit:
  return lua_gettop(l);
}




static int cmd_value(lua_State *l, char op)
{
  int result;
  unsigned char hascomm;
  uint8_t fid;
  int32_t amount;
  uint8_t comm;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "amount must be a number");
  hascomm = lua_gettop(l) >= 3;
  if(hascomm)
  {
    result = desflua_get_comm(l, 3, &comm);
    if(result)
      desflua_argerror(l, 3, "comm");
  }

  fid    = lua_tointeger(l, 1);
  amount = lua_tointeger(l, 2);

  switch(op)
  {
    case 'c': debug_cmd("Credit");        break;
    case 'd': debug_cmd("Debit");         break;
    case 'l': debug_cmd("LimitedCredit"); break;
  }

  debug_gen(DEBUG_IN, "FID", "%d", fid);
  debug_gen(DEBUG_IN, "AMOUNT", "%d", amount);

  if(hascomm)
  {
    switch(op)
    {
    case 'c': result = mifare_desfire_credit_ex(tag, fid, amount, comm);         break;
    case 'd': result = mifare_desfire_debit_ex(tag, fid, amount, comm);          break;
    case 'l': result = mifare_desfire_limited_credit_ex(tag, fid, amount, comm); break;
    }
  }
  else
  {
    switch(op)
    {
    case 'c': result = mifare_desfire_credit(tag, fid, amount);         break;
    case 'd': result = mifare_desfire_debit(tag, fid, amount);          break;
    case 'l': result = mifare_desfire_limited_credit(tag, fid, amount); break;
    }
  }
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_credit) = { "credit", "Credit", NULL };
FN_PARAM(cmd_credit) =
{
  FNPARAM("fid",    "File ID",                0),
  FNPARAM("amount", "Credit Amount",          0),
  FNPARAM("comm",   "Communication Settings", 1),
  FNPARAMEND
};
FN_RET(cmd_credit) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_credit, "Increase a Files Value", NULL);


static int cmd_credit(lua_State *l)
{
  return cmd_value(l, 'c');
}




FN_ALIAS(cmd_debit) = { "debit", "Debit", NULL };
FN_PARAM(cmd_debit) =
{
  FNPARAM("fid",    "File ID",                0),
  FNPARAM("amount", "Debit Amount",           0),
  FNPARAM("comm",   "Communication Settings", 1),
  FNPARAMEND
};
FN_RET(cmd_debit) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_debit, "Decrease a Files Value", NULL);


static int cmd_debit(lua_State *l)
{
  return cmd_value(l, 'd');
}




FN_ALIAS(cmd_lcredit) = { "lcredit", "LimitedCredit", NULL };
FN_PARAM(cmd_lcredit) =
{
  FNPARAM("fid",    "File ID",                0),
  FNPARAM("amount", "Credit Amount",          0),
  FNPARAM("comm",   "Communication Settings", 1),
  FNPARAMEND
};
FN_RET(cmd_lcredit) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_lcredit, "Increase a Files Value by a Limited Amount", NULL);


static int cmd_lcredit(lua_State *l)
{
  return cmd_value(l, 'l');
}




FN_ALIAS(cmd_wrec) = { "wrec", "WriteRecord", NULL };
FN_PARAM(cmd_wrec) =
{
  FNPARAM("fid",    "File ID",                0),
  FNPARAM("offset", "Offset",                 0),
  FNPARAM("buffer", "Content",                0),
  FNPARAM("comm",   "Communication Settings", 1),
  FNPARAMEND
};
FN_RET(cmd_wrec) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_wrec, "Write to Record", NULL);


static int cmd_wrec(lua_State *l)
{
  return cmd_write_gen(l, 'r');
}




FN_ALIAS(cmd_rrec) = { "rrec", "ReadRecord", NULL };
FN_PARAM(cmd_rrec) =
{
  FNPARAM("fid",     "File ID",                       0),
  FNPARAM("offset",  "Backlog",                       0),
  FNPARAM("len",     "Number of records",             0),
  FNPARAM("comm",    "Communication Settings",        1),
  FNPARAM("nocheck", "Skip file type and size check", 1),
  FNPARAMEND
};
FN_RET(cmd_rrec) =
{
  FNPARAM("code",   "Return Code",  0),
  FNPARAM("err",    "Error String", 0),
  FNPARAM("buffer", "File Content", 1),
  FNPARAMEND
};
FN("cmd", cmd_rrec, "Read from Record", NULL);


static int cmd_rrec(lua_State *l)
{
  return cmd_read_gen(l, 'r');
}




FN_ALIAS(cmd_crec) = { "crec", "ClearRecordFile", NULL };
FN_PARAM(cmd_crec) =
{
  FNPARAM("fid", "File ID", 0),
  FNPARAMEND
};
FN_RET(cmd_crec) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_crec, "Clear Record File", NULL);


int cmd_crec(lua_State *l)
{
  int result;
  uint8_t fid;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");

  fid = lua_tointeger(l, 1);

  debug_cmd("ClearRecordFile");
  debug_gen(DEBUG_IN, "FID", "%d", fid);

  result = mifare_desfire_clear_record_file(tag, fid);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_commit) = { "commit", "CommitTransaction", NULL };
FN_PARAM(cmd_commit) =
{
  FNPARAMEND
};
FN_RET(cmd_commit) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_commit, "Commit Transaction", NULL);


static int cmd_commit(lua_State *l)
{
  int result;


  debug_cmd("Commit");
  result = mifare_desfire_commit_transaction(tag);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}




FN_ALIAS(cmd_abort) = { "abort", "AbortTransaction", NULL };
FN_PARAM(cmd_abort) =
{
  FNPARAMEND
};
FN_RET(cmd_abort) =
{
  FNPARAM("code", "Return Code",  0),
  FNPARAM("err",  "Error String", 0),
  FNPARAMEND
};
FN("cmd", cmd_abort, "Abort Transaction", NULL);


static int cmd_abort(lua_State *l)
{
  int result;


  debug_cmd("Abort");
  result = mifare_desfire_abort_transaction(tag);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}
