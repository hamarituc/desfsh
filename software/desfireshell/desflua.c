#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <lua.h>
#include <lauxlib.h>

#include "cmd.h"
#include "desflua.h"



/* Diese Version brücksichtigt zusätzlich Oktalzahlen. */
/*int desflua_get_long(lua_State *l, int idx, long int *val)
{
  const char *str;
  char *end;


  if(val == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): val=%p", __FILE__, __LINE__, val);
    return -1;
  }

  switch(lua_type(l, idx))
  {
  case LUA_TNUMBER:
    *val = lua_tointeger(l, idx);
    break;

  case LUA_TSTRING:
    str = lua_tostring(l, idx);
    *val = strtol(str, &end, 0);
    if(str[0] == '\0' || end[0] != '\0')
    {
      lua_checkstack(l, 1);
      lua_pushfstring(l, "'%s' is not a valid integer", str);
      return -1;
    }
    break;

  default:
    lua_checkstack(l, 1);
    lua_pushstring(l, "number or string expected");
    return -1;
  }

  return 0;
}*/


int desflua_get_buffer(lua_State *l, int idx, uint8_t **buffer, unsigned int *len)
{
  unsigned int i;


  if(buffer == NULL || len == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): buffer=%p len=%p", __FILE__, __LINE__, buffer, len);
    return -1;
  }

  lua_checkstack(l, 1);

  if(lua_istable(l, idx))
  {
/*    long int lval;
    int result;*/

    *len = lua_objlen(l, idx);

    *buffer = (uint8_t*)malloc(*len * sizeof(uint8_t));
    if(*buffer == NULL)
    {
      lua_checkstack(l, 1);
      lua_pushfstring(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);
      return -1;
    }

    for(i = 0; i < *len; i++)
    {
      lua_pushinteger(l, i + 1);
      lua_gettable(l, idx);

/*      result = desflua_get_long(l, -1, &lval) % 256;
      if(result)
      {
        lua_checkstack(l, 1);
        lua_pushfstring(l, "index %d --> %s", i, lua_tointeger(l, -1));
	lua_remove(l, -2);
	free(*buffer);
	*buffer = NULL;
	return -1;
      }

      (*buffer)[i] = lval % 256;*/

      if(!lua_isnumber(l, -1))
      {
        lua_checkstack(l, 1);
        lua_pushfstring(l, "index %d --> '%s' is not a valid number", i, lua_tostring(l, -1));
	lua_remove(l, -2);
	free(*buffer);
	*buffer = NULL;
	return -1;
      }

      (*buffer)[i] = lua_tointeger(l, -1) % 256;
    }
  }
  else if(lua_isstring(l, idx))
  {
    const char *hexstr;


    *len = lua_objlen(l, idx);
    if(*len % 2)
    {
      lua_checkstack(l, 1);
      lua_pushstring(l, "length of hexstring must be even");
      return -1;
    }
    
    *len /= 2;
    *buffer = (uint8_t*)malloc(*len * sizeof(uint8_t));
    if(*buffer == NULL)
    {
      lua_checkstack(l, 1);
      lua_pushfstring(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);
      return -1;
    }

    hexstr = lua_tostring(l, idx);

    for(i = 0; i < *len; i++)
    {
      char c1, c2;
      uint8_t val;

      c1 = tolower(hexstr[2 * i]);
      c2 = tolower(hexstr[2 * i + 1]);

      val = 0;

      if(c1 >= '0' && c1 <= '9')
        val += c1 - '0';
      else if(c1 >= 'a' && c1 <= 'f')
        val += c1 - 'a' + 10;
      else
      {
        lua_checkstack(l, 1);
        lua_pushfstring(l, "invalid character '%c' at index %d", c1, 2 * i);
	free(*buffer);
	*buffer = NULL;
	return -1;
      }

      val *= 16;

      if(c2 >= '0' && c2 <= '9')
        val += c2 - '0';
      else if(c2 >= 'a' && c2 <= 'f')
        val += c2 - 'a' + 10;
      else
      {
        lua_checkstack(l, 1);
        lua_pushfstring(l, "invalid character '%c' at index %d", c2, 2 * i + 1);
	free(*buffer);
	*buffer = NULL;
	return -1;
      }

      (*buffer)[i] = val;
    }
  }
  else
  {
    lua_checkstack(l, 1);
    lua_pushstring(l, "array or hexstring expected");
    return -1;
  }


  return 0;
}


int desflua_get_keytype(lua_State *l, int idx, enum keytype_e *type)
{
  const char *typestr;


  if(type == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): type=%p", __FILE__, __LINE__, type);
    return -1;
  }

  if(!lua_isstring(l, idx))
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "string expected");
    return -1;
  }

  typestr = lua_tostring(l, idx);

       if(!strcasecmp(typestr, "DES"))    { *type = _DES_;    }
  else if(!strcasecmp(typestr, "3DES"))   { *type = _3DES_;   }
  else if(!strcasecmp(typestr, "3K3DES")) { *type = _3K3DES_; }
  else if(!strcasecmp(typestr, "AES"))    { *type = _AES_;    }
  else
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "unkown key type '%s'", typestr);
    return -1;
  }

  return 0;
}


int desflua_get_key(lua_State *l, int idx, MifareDESFireKey *k)
{
  int result;
  enum keytype_e type;
  uint8_t *key;
  unsigned int len, elen;
  uint8_t ver;


  if(k == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): k=%p", __FILE__, __LINE__, k);
    return -1;
  }

  if(!lua_istable(l, idx))
  {
    lua_checkstack(l, 1);
    lua_pushstring(l, "key must be a LUA table");
    return -1;
  }


  lua_checkstack(l, 1);

  /* Typ auslesen. */
  lua_getfield(l, idx, "t");
  result = desflua_get_keytype(l, -1, &type);
  if(result)
  {
    lua_remove(l, -2);
    lua_pushfstring(l, "key type invalid: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return -1;
  }
  lua_pop(l, 1);

  /* Version auslesen. */
  lua_getfield(l, idx, "v");
  if(!lua_isnil(l, -1) && !lua_isnumber(l, -1))
  {
    lua_pop(l, 1);
    lua_pushfstring(l, "key version invalid, number expected");
    return -1;
  }

  ver = lua_isnumber(l, -1) ? lua_tointeger(l, -1) : 0;
  lua_pop(l, 1);

  /* Schlüssel auslesen. */
  lua_getfield(l, idx, "k");
  result = desflua_get_buffer(l, -1, &key, &len);
  if(result)
  {
    lua_remove(l, -2);
    lua_pushfstring(l, "key string invalid: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return -1;
  }
  lua_pop(l, 1);


  switch(type)
  {
  case _DES_:    elen =  8; break;
  case _3DES_:   elen = 16; break;
  case _3K3DES_: elen = 24; break;
  case _AES_:    elen = 16; break;
  }

  if(len != elen)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "key length %d invalid, expeced %d bytes", len, elen);
    free(key);
    return -1;
  }

  switch(type)
  {
  case _DES_:    *k = mifare_desfire_des_key_new(key);                   break;
  case _3DES_:   *k = mifare_desfire_3des_key_new(key);                  break;
  case _3K3DES_: *k = mifare_desfire_3k3des_key_new(key);                break;
  case _AES_:    *k = mifare_desfire_aes_key_new_with_version(key, ver); break;
  }

  if(*k == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): cannot create key", __FILE__, __LINE__);
    free(key);
    return -1;
  }

  switch(type)
  {
  case _DES_:
  case _3DES_:
  case _3K3DES_:
    mifare_desfire_key_set_version(*k, ver);
    break;

  default: break;
  }


  return 0;
}


int desflua_get_acl(lua_State *l, int idx, uint16_t *acl)
{
  uint16_t rd, wr, rw, ca;


  if(acl == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): acl=%p", __FILE__, __LINE__, acl);
    return -1;
  }

  if(lua_isnumber(l, idx))
    *acl = lua_tointeger(l, idx);
  else if(lua_istable(l, idx))
  {
    lua_checkstack(l, 4);
    lua_getfield(l, idx, "rd");
    lua_getfield(l, idx, "wr");
    lua_getfield(l, idx, "rw");
    lua_getfield(l, idx, "ca");

    if(!lua_isnumber(l, -4) ||
       !lua_isnumber(l, -3) ||
       !lua_isnumber(l, -2) ||
       !lua_isnumber(l, -1))
    {
      lua_checkstack(l, 1);
           if(!lua_isnumber(l, -4)) { lua_pushstring(l, "read access: number expected");       }
      else if(!lua_isnumber(l, -3)) { lua_pushstring(l, "write access: number expected");      }
      else if(!lua_isnumber(l, -2)) { lua_pushstring(l, "read/write access: number expected"); }
      else if(!lua_isnumber(l, -1)) { lua_pushstring(l, "change access: number expected");     }
      lua_remove(l, -2);
      lua_remove(l, -2);
      lua_remove(l, -2);
      lua_remove(l, -2);
      return -1;
    }

    rd = lua_tointeger(l, -4);
    wr = lua_tointeger(l, -3);
    rw = lua_tointeger(l, -2);
    ca = lua_tointeger(l, -1);
    lua_pop(l, 4);

    *acl = MDAR(rd, wr, rw, ca);
  }
  else
  {
    lua_checkstack(l, 1);
    lua_pushstring(l, "number or table expected");
    return -1;
  }


  return 0;
}


void desflua_push_acl(lua_State *l, uint16_t acl)
{
  lua_checkstack(l, 2);
  lua_newtable(l);
  lua_pushinteger(l, MDAR_READ(acl));       lua_setfield(l, -2, "rd");
  lua_pushinteger(l, MDAR_WRITE(acl));      lua_setfield(l, -2, "wr");
  lua_pushinteger(l, MDAR_READ_WRITE(acl)); lua_setfield(l, -2, "rw");
  lua_pushinteger(l, MDAR_CHANGE_AR(acl));  lua_setfield(l, -2, "ca");
}


void desflua_handle_result(lua_State *l, int result, FreefareTag tag)
{
  lua_settop(l, 0);
  lua_checkstack(l, 2);

  lua_pushinteger(l, mifare_desfire_last_picc_error(tag));
  lua_pushstring(l, result == 0 ? "OK" : freefare_strerror(tag));
}


/* Buffer nach Hex-String */
/* Buffer nach Int-Array */
/* Buffer nach Hex-Array */
/* Buffer nach String */
/* Int to hex */
/* hex to int */
