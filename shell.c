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

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM > 501
#define QL(x)	(x)
#else
#define QL(x)	LUA_QL(x)
#endif

#include "fn.h"
#include "shell.h"



void shell(int online, int interactive, const char *command)
{
  lua_State *l;
  const char *prompt;
  char *s;
  int result;


  l = luaL_newstate();
  if(l == NULL)
  {
    fprintf(stderr, "Failed to create LUA state.\n");
    return;
  }

  luaL_openlibs(l);
  result = fn_init(l, online);
  if(result)
    goto fail;



  /*
   * Auf der Kommandozeile angegebenes Kommando ausführen.
   */
  if(command != NULL)
  {
    lua_settop(l, 0);
    result = luaL_dostring(l, command);
    if(result)
      goto fail;
  }


  /*
   * Interaktive Shell starten.
   */
  if(interactive)
  {
    lua_settop(l, 0);
    prompt = "> ";
    while((s = readline(prompt)) != NULL)
    {
      add_history(s);

      lua_checkstack(l, 1);
      lua_pushstring(l, s);
      if(lua_gettop(l) > 1)
        lua_concat(l, 2);

      /* Den übergebenen Code übersetzen. */
      size_t len;

#if LUA_VERSION_NUM > 501
      len = lua_rawlen(l, -1);
#else
      len = lua_objlen(l, -1);
#endif
      result = luaL_loadbuffer(l, lua_tostring(l, -1), len, "shell");

      /*
       * Wenn das Kommando lediglich unvollständig ist, nicht mit einem
       * Syntax-Fehler abbrechen, sondern weitere Eingaben entgegennehmen.
       */
      if(result == LUA_ERRSYNTAX)
      {
        size_t len;
        const char *msg, *p;

        msg = lua_tolstring(l, -1, &len);
        p   = msg + len - strlen(QL("<eof>"));
        if(strstr(msg, QL("<eof>")) != p)
          fprintf(stderr, "%s\n", lua_tostring(l, -1));
        else
        {
          prompt = ">> ";
          lua_pop(l, 1);
          lua_pushliteral(l, "\n");
          lua_concat(l, 2);
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
        fprintf(stderr, "%s\n", lua_tostring(l, -1));
      lua_settop(l, 0);
    }

    result = 0;
  }


fail:
  if(result)
    fprintf(stderr, "%s\n", lua_tostring(l, -1));

  lua_close(l);
}
