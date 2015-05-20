#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <freefare.h>

#include "desfsh.h"
#include "cmd.h"



/*
 * Interne Funktionen
 */

static void desf_lua_registerfn(lua_State *l, lua_CFunction f, const char *name)
{
  lua_checkstack(l, 1);
  lua_pushcfunction(l, f);
  lua_setfield(l, LUA_GLOBALSINDEX, name);
}


static void desf_lua_register(lua_State *l)
{
  desf_lua_registerfn(l, cmd_auth, "auth");
  desf_lua_registerfn(l, cmd_cks,  "cks");
}



/*
 * Bibliotheksfunktionen.
 */

int desf_lua_get_long(lua_State *l, int idx, long int *val)
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
}


int desf_lua_get_buffer(lua_State *l, int idx, uint8_t **buffer, unsigned int *len)
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
    long int lval;
    int result;

    *len = lua_objlen(l, idx);

    *buffer = (uint8_t*)malloc(*len + sizeof(uint8_t));
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
      result = desf_lua_get_long(l, -1, &lval);
      if(result)
      {
        lua_checkstack(l, 1);
        lua_pushfstring(l, "inddex %d --> %s", i, lua_tointeger(l, -1));
	lua_remove(l, -2);
	free(*buffer);
	*buffer = NULL;
	return -1;
      }
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
    *buffer = (uint8_t*)malloc(*len + sizeof(uint8_t));
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

      val *= 10;

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

      *buffer[i] = val;
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


/* Buffer nach Hex-String */
/* Buffer nach Int-Array */
/* Buffer nach Hex-Array */
/* Buffer nach String */
/* Int to hex */
/* hex to int */


/*
 * Die LUA-Eingabeshell.
 */

void desf_lua_shell()
{
  lua_State *l;
  const char *prompt;
  char *s;
  int result;


  l = luaL_newstate();
  if(l == NULL)
  {
    fprintf(stdout, "Failed to create LUA state.\n");
    return;
  }

  luaL_openlibs(l);
  desf_lua_register(l);


  prompt = "> ";
  while((s = readline(prompt)) != NULL)
  {
    add_history(s);

    lua_checkstack(l, 1);
    lua_pushstring(l, s);
    if(lua_gettop(l) > 1)
      lua_concat(l, 2);

    /* Den übergebenen Code übersetzen. */
    result = luaL_loadbuffer(l, lua_tostring(l, -1), lua_strlen(l, -1), "shell");

    /*
     * Wenn das Kommando lediglich unvollständig ist, nicht mit einem
     * Syntax-Fehler abbrechen, sondern weitere Eingaben entgegennehmen.
     */
    if(result == LUA_ERRSYNTAX)
    {
      size_t len;
      const char *msg, *p;

      msg = lua_tolstring(l, -1, &len);
      p   = msg + len - strlen(LUA_QL("<eof>"));
      if(strstr(msg, LUA_QL("<eof>")) != p)
        fprintf(stderr, "%s\n", lua_tostring(l, -1));
      else
      {
	prompt = ">> ";
        lua_pop(l, 1);
        continue;
      }
    }
    
    prompt = "> ";

    /* Im Fehlerfall den Kommando-Code verwerfen. */
    if(result)
    {
      lua_settop(l, 0);
      continue;
    }

    /* Den übergebenen Code ausführen. */
    if(lua_pcall(l, 0, 0, 0))
      fprintf(stderr, "%s", lua_tostring(l, -1));
    lua_settop(l, 0);
    printf("\n");
  }


  lua_close(l);
}
