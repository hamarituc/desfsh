#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "desflua.h"
#include "desfsh.h"



static int cmd_read_gen(lua_State *l, char op)
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

  fid  = lua_tointeger(l, 1);
  off  = lua_tointeger(l, 2);
  len  = lua_tointeger(l, 3);
  comm = lua_tointeger(l, 4);

  data = (uint8_t*)malloc(len * sizeof(uint8_t));
  if(data == NULL)
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);

  if(lua_isnumber(l, 4))
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

  desflua_push_buffer(l, data, result);


exit:
  free(data);
  return lua_gettop(l);
}


static int cmd_write_gen(lua_State *l, char op)
{
  int result;
  uint8_t fid;
  uint32_t off, len;
  uint8_t *data;
  uint8_t comm;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l, lua_isnumber(l, 2), 2, "offset must be a number");

  result = desflua_get_buffer(l, 3, &data, &len);
  if(result)
  {
    lua_remove(l, -2);
    lua_pushfstring(l, "buffer invalid: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return luaL_argerror(l, 3, lua_tostring(l, -1));
  }

  luaL_argcheck(l, lua_gettop(l) < 4 || lua_isnumber(l, 4), 4, "comm settings must be a number");

  fid  = lua_tointeger(l, 1);
  off  = lua_tointeger(l, 2);
  comm = lua_tointeger(l, 4);


  if(lua_isnumber(l, 4))
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
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


int cmd_read(lua_State *l)
{
  return cmd_read_gen(l, 'f');
}


int cmd_write(lua_State *l)
{
  return cmd_write_gen(l, 'f');
}


int cmd_getval(lua_State *l)
{
  int result;
  uint8_t fid;
  uint8_t comm;
  int32_t val;


  luaL_argcheck(l,                      lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l, lua_gettop(l) < 2 || lua_isnumber(l, 2), 2, "comm settings must be a number");

  fid  = lua_tointeger(l, 1);
  comm = lua_tointeger(l, 2);

  if(lua_isnumber(l, 2))
    result = mifare_desfire_get_value_ex(tag, fid, &val, comm);
  else
    result = mifare_desfire_get_value(tag, fid, &val);
  desflua_handle_result(l, result, tag);
  if(result < 0)
    goto exit;

  lua_pushinteger(l, val);


exit:
  return lua_gettop(l);
}


static int cmd_value(lua_State *l, char op)
{
  int result;
  uint8_t fid;
  int32_t amount;
  uint8_t comm;


  luaL_argcheck(l,                      lua_isnumber(l, 1), 1, "file number expected");
  luaL_argcheck(l,                      lua_isnumber(l, 2), 2, "amount must be a number");
  luaL_argcheck(l, lua_gettop(l) < 3 || lua_isnumber(l, 3), 3, "comm settings must be a number");

  fid    = lua_tointeger(l, 1);
  amount = lua_tointeger(l, 2);
  comm   = lua_tointeger(l, 3);

  if(lua_isnumber(l, 3))
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


int cmd_credit(lua_State *l)
{
  return cmd_value(l, 'c');
}


int cmd_debit(lua_State *l)
{
  return cmd_value(l, 'd');
}


int cmd_lcredit(lua_State *l)
{
  return cmd_value(l, 'l');
}


int cmd_wrec(lua_State *l)
{
  return cmd_write_gen(l, 'r');
}


int cmd_rrec(lua_State *l)
{
  return cmd_read_gen(l, 'r');
}


int cmd_crec(lua_State *l)
{
  int result;
  uint8_t fid;


  luaL_argcheck(l, lua_isnumber(l, 1), 1, "file number expected");

  fid = lua_tointeger(l, 1);
  result = mifare_desfire_clear_record_file(tag, fid);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


int cmd_commit(lua_State *l)
{
  int result;


  result = mifare_desfire_commit_transaction(tag);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}


int cmd_abort(lua_State *l)
{
  int result;


  result = mifare_desfire_abort_transaction(tag);
  desflua_handle_result(l, result, tag);


  return lua_gettop(l);
}
