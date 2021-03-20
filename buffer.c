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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <lua.h>
#include <lauxlib.h>

#include "desflua.h"
#include "fn.h"
#include "hexdump.h"



static int buffer_get_table(lua_State *l, int idx, uint8_t **buffer, unsigned int *len);
static int buffer_get_hexstr(lua_State *l, int idx, uint8_t **buffer, unsigned int *len);
static int buffer_get_ascii(lua_State *l, int idx, uint8_t **buffer, unsigned int *len);
static void buffer_push_table(lua_State *l, uint8_t *buffer, unsigned int len);
static void buffer_push_hexstr(lua_State *l, uint8_t *buffer, unsigned int len);
static void buffer_push_ascii(lua_State *l, uint8_t *buffer, unsigned int len);
static void buffer_push_hexdump(lua_State *l, uint8_t *buffer, unsigned int len);
static int buffer_from(lua_State *l, int (*fn)(lua_State*, int, uint8_t**, unsigned int*), const char *id);
static int buffer_to(lua_State *l, void (*fn)(lua_State*, uint8_t*, unsigned int));

static int buffer_from_table(lua_State *l);
static int buffer_from_hexstr(lua_State *l);
static int buffer_from_ascii(lua_State *l);
static int buffer_to_table(lua_State *l);
static int buffer_to_hexstr(lua_State *l);
static int buffer_to_ascii(lua_State *l);
static int buffer_to_hexdump(lua_State *l);
static int buffer_concat(lua_State *l);




static int buffer_get_table(lua_State *l, int idx, uint8_t **buffer, unsigned int *len)
{
  unsigned int i;
/*  long int lval;
  int result;*/


  if(!lua_istable(l, idx))
  {
    lua_checkstack(l, 1);
    lua_pushstring(l, "table expected");
    return -1;
  }

#if LUA_VERSION_NUM > 501
  *len = lua_rawlen(l, idx);
#else
  *len = lua_objlen(l, idx);
#endif

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

    if(!lua_isnumber(l, -1))
    {
      lua_pushfstring(l, "index %d --> '%s' is not a valid number", i, lua_tostring(l, -1));
      lua_remove(l, -2);
      free(*buffer);
      *buffer = NULL;
      return -1;
    }

    (*buffer)[i] = lua_tointeger(l, -1) % 256;
    lua_pop(l, 1);
  }


  return 0;
}



static int buffer_get_hexstr(lua_State *l, int idx, uint8_t **buffer, unsigned int *len)
{
  const char *hexstr;
  unsigned int i;


  if(!lua_isstring(l, idx))
  {
    lua_checkstack(l, 1);
    lua_pushstring(l, "string expected");
    return -1;
  }

#if LUA_VERSION_NUM > 501
  *len = lua_rawlen(l, idx);
#else
  *len = lua_objlen(l, idx);
#endif
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


  return 0;
}


static int buffer_get_ascii(lua_State *l, int idx, uint8_t **buffer, unsigned int *len)
{
  const char *str;
  unsigned int i;


  if(!lua_isstring(l, idx))
  {
    lua_checkstack(l, 1);
    lua_pushstring(l, "string expected");
    return -1;
  }

#if LUA_VERSION_NUM > 501
  *len = lua_rawlen(l, idx);
#else
  *len = lua_objlen(l, idx);
#endif

  *buffer = (uint8_t*)malloc(*len * sizeof(uint8_t));
  if(*buffer == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);
    return -1;
  }

  str = lua_tostring(l, idx);

  for(i = 0; i < *len; i++)
    (*buffer)[i] = str[i];


  return 0;
}


int buffer_get(lua_State *l, int idx, uint8_t **buffer, unsigned int *len)
{
  if(buffer == NULL || len == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): buffer=%p len=%p", __FILE__, __LINE__, buffer, len);
    return -1;
  }

  /* Den Index positiv machen, damit wir absolut referenzieren können. */
  if(idx < 0)
    idx = lua_gettop(l) + 1 + idx;

  lua_checkstack(l, 1);

  if(lua_istable(l, idx))
    return buffer_get_table(l, idx, buffer, len);
  else if(lua_type(l, idx) == LUA_TSTRING)
    return buffer_get_hexstr(l, idx, buffer, len);
  else
  {
    lua_checkstack(l, 1);
    lua_pushstring(l, "array or hexstring expected");
    return -1;
  }
}


static void buffer_push_table(lua_State *l, uint8_t *buffer, unsigned int len)
{
  unsigned int i;


  lua_checkstack(l, 3);

  if(buffer == NULL)
  {
    lua_pushnil(l);
    return;
  }

  lua_newtable(l);

  for(i = 0; i < len; i++)
  {
    lua_pushinteger(l, i + 1);
    lua_pushinteger(l, buffer[i]);
    lua_settable(l, -3);
  }
}


static void buffer_push_hexstr(lua_State *l, uint8_t *buffer, unsigned int len)
{
  unsigned int i;
  static char hexchar[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };


  lua_checkstack(l, 2);

  if(buffer == NULL)
  {
    lua_pushnil(l);
    return;
  }

  lua_pushstring(l, "");
  for(i = 0; i < len; i++)
  {
    uint8_t v1, v2;

    v1 = (buffer[i] >> 4) & 0x0f;
    v2 =  buffer[i]       & 0x0f;

    lua_pushfstring(l, "%c%c", hexchar[v1], hexchar[v2]);
    lua_concat(l, 2);
  }
}


static void buffer_push_ascii(lua_State *l, uint8_t *buffer, unsigned int len)
{
  unsigned int i;


  lua_checkstack(l, 2);

  if(buffer == NULL)
  {
    lua_pushnil(l);
    return;
  }

  lua_pushstring(l, "");
  for(i = 0; i < len; i++)
  {
    lua_pushfstring(l, "%c", buffer[i]);
    lua_concat(l, 2);
  }
}



static void buffer_push_hexdump(lua_State *l, uint8_t *buffer, unsigned int len)
{
  unsigned int idx;
  char *line;


  lua_checkstack(l, 3);

  if(buffer == NULL)
  {
    lua_pushnil(l);
    return;
  }

  lua_pushstring(l, "");

  for(idx = 0; idx < len; idx += 8)
  {
    if(idx > 0)
      lua_pushstring(l, "\n");
    line = hexdump_line(buffer + idx, len - idx, idx);
    lua_pushstring(l, line);
    lua_concat(l, idx > 0 ? 3 : 2);
  }
}



void buffer_push(lua_State *l, uint8_t *buffer, unsigned int len)
{
  buffer_push_table(l, buffer, len);

  lua_checkstack(l, 2);
  lua_newtable(l);
  lua_getglobal(l, "buf");
  lua_setfield(l, -2, "__index");
  lua_pushcfunction(l, buffer_concat);
  lua_setfield(l, -2, "__concat");
  lua_setmetatable(l, -2);
}



static int buffer_from(lua_State *l, int (*fn)(lua_State*, int, uint8_t**, unsigned int*), const char *id)
{
  int result;
  uint8_t *buffer;
  unsigned int len;


  result = fn(l, 1, &buffer, &len); 
  if(result)
    desflua_argerror(l, 1, id);

  lua_settop(l, 0);
  buffer_push(l, buffer, len);
  free(buffer);


  return lua_gettop(l);
}



static int buffer_to(lua_State *l, void (*fn)(lua_State*, uint8_t*, unsigned int))
{
  int result;
  uint8_t *buffer;
  unsigned int len;


  result = buffer_get(l, 1, &buffer, &len); 
  if(result)
    desflua_argerror(l, 1, "buffer");

  lua_settop(l, 0);
  fn(l, buffer, len);
  free(buffer);


  return lua_gettop(l);
}




FN_ALIAS(buffer_from_table) = { "fromtable", "ft", NULL };
FN_PARAM(buffer_from_table) =
{
  FNPARAM("table", "Buffer as Table", 0),
  FNPARAMEND
};
FN_RET(buffer_from_table) =
{
  FNPARAM("buffer", "Output Buffer", 0),
  FNPARAMEND
};
FN("buf", buffer_from_table, "Read Buffer from Table", NULL);


static int buffer_from_table(lua_State *l)
{
  return buffer_from(l, buffer_get_table, "table");
}




FN_ALIAS(buffer_from_hexstr) = { "fromhexstr", "fh", NULL };
FN_PARAM(buffer_from_hexstr) =
{
  FNPARAM("hexstr", "Buffer as HEX String", 0),
  FNPARAMEND
};
FN_RET(buffer_from_hexstr) =
{
  FNPARAM("buffer", "Output Buffer", 0),
  FNPARAMEND
};
FN("buf", buffer_from_hexstr, "Read Buffer from HEX String", NULL);


static int buffer_from_hexstr(lua_State *l)
{
  return buffer_from(l, buffer_get_hexstr, "hexstr");
}




FN_ALIAS(buffer_from_ascii) = { "fromascii", "fa", NULL };
FN_PARAM(buffer_from_ascii) =
{
  FNPARAM("str", "Buffer as ASCII String", 0),
  FNPARAMEND
};
FN_RET(buffer_from_ascii) =
{
  FNPARAM("buffer", "Output Buffer", 0),
  FNPARAMEND
};
FN("buf", buffer_from_ascii, "Read Buffer from ASCII String", NULL);


static int buffer_from_ascii(lua_State *l)
{
  return buffer_from(l, buffer_get_ascii, "str");
}




FN_ALIAS(buffer_to_table) = { "totable", "tt", NULL };
FN_PARAM(buffer_to_table) =
{
  FNPARAM("buffer", "Input Buffer", 0),
  FNPARAMEND
};
FN_RET(buffer_to_table) =
{
  FNPARAM("table", "Buffer as Table", 0),
  FNPARAMEND
};
FN("buf", buffer_to_table, "Convert Buffer to Table", NULL);


static int buffer_to_table(lua_State *l)
{
  return buffer_to(l, buffer_push_table);
}




FN_ALIAS(buffer_to_hexstr) = { "tohexstr", "th", NULL };
FN_PARAM(buffer_to_hexstr) =
{
  FNPARAM("buffer", "Input Buffer", 0),
  FNPARAMEND
};
FN_RET(buffer_to_hexstr) =
{
  FNPARAM("hexstr", "Buffer as HEX String", 0),
  FNPARAMEND
};
FN("buf", buffer_to_hexstr, "Convert Buffer to HEX String", NULL);


static int buffer_to_hexstr(lua_State *l)
{
  return buffer_to(l, buffer_push_hexstr);
}




FN_ALIAS(buffer_to_ascii) = { "toascii", "ta", NULL };
FN_PARAM(buffer_to_ascii) =
{
  FNPARAM("buffer", "Input Buffer", 0),
  FNPARAMEND
};
FN_RET(buffer_to_ascii) =
{
  FNPARAM("str", "Buffer as ASCII String", 0),
  FNPARAMEND
};
FN("buf", buffer_to_ascii, "Convert Buffer to ASCII String", NULL);


static int buffer_to_ascii(lua_State *l)
{
  return buffer_to(l, buffer_push_ascii);
}




FN_ALIAS(buffer_to_hexdump) = { "hexdump", NULL };
FN_PARAM(buffer_to_hexdump) =
{
  FNPARAM("buffer", "Input Buffer", 0),
  FNPARAMEND
};
FN_RET(buffer_to_hexdump) =
{
  FNPARAM("dump", "Buffer as HEX Dump", 0),
  FNPARAMEND
};
FN("buf", buffer_to_hexdump, "Convert Buffer to HEX Dump", NULL);


static int buffer_to_hexdump(lua_State *l)
{
  return buffer_to(l, buffer_push_hexdump);
}




FN_ALIAS(buffer_concat) = { "concat", NULL };
FN_PARAM(buffer_concat) =
{
  FNPARAM("buffer", "Input Buffer", 0),
  FNPARAM("...",    "Input Buffer", 1),
  FNPARAMEND
};
FN_RET(buffer_concat) =
{
  FNPARAM("buffer", "Output Buffer", 0),
  FNPARAMEND
};
FN("buf", buffer_concat, "Concatenate buffers", NULL);


static int buffer_concat(lua_State *l)
{
  unsigned int n, i;
  uint8_t **buffers;
  unsigned int *lens;
  int result;
  unsigned char fail;
  unsigned int failidx;
  uint8_t *sumbuffer;
  unsigned int sumlen, pos;


  n = lua_gettop(l);
  if(n == 0)
    return 0;

  buffers = (uint8_t**)malloc(n * sizeof(uint8_t*));
  lens    = (unsigned int*)malloc(n * sizeof(unsigned int));

  if(buffers == NULL || lens == NULL)
  {
    free(buffers);
    free(lens);
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);
  }

  for(i = 0; i < n; i++)
    buffers[i] = NULL;
    
  fail = 0;
  for(i = 0; i < n; i++)
  {
    result = buffer_get(l, i + 1, &buffers[i], &lens[i]);
    if(result)
    {
      fail    = 1;
      failidx = i + 1;
      break;
    }
  }

  if(fail)
  {
    for(i = 0; i < n; i++)
      free(buffers[i]);
    free(buffers);
    free(lens);

    desflua_argerror(l, failidx, "buffer");
  }

  lua_settop(l, 0);

  sumlen = 0;
  for(i = 0; i < n; i++)
    sumlen += lens[i];

  sumbuffer = (uint8_t*)malloc(sumlen * sizeof(uint8_t));
  if(sumbuffer == NULL)
  {
    for(i = 0; i < n; i++)
      free(buffers[i]);
    free(buffers);
    free(lens);
    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);
  }

  pos = 0;
  for(i = 0; i < n; i++)
  {
    memcpy(sumbuffer + pos, buffers[i], lens[i] * sizeof(uint8_t));
    pos += lens[i];
  }

  buffer_push(l, sumbuffer, sumlen);

  free(sumbuffer);
  for(i = 0; i < n; i++)
    free(buffers[i]);
  free(buffers);
  free(lens);


  return lua_gettop(l);
}
