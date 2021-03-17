#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

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
    fprintf(stdout, "Failed to create LUA state.\n");
    return;
  }

  luaL_openlibs(l);
  fn_init(l, online);



  /*
   * Auf der Kommandozeile angegebenes Kommando ausführen.
   */
  if(command != NULL)
  {
    lua_settop(l, 0);
    luaL_dostring(l, command);
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
        p   = msg + len - strlen(LUA_QL("<eof>"));
        if(strstr(msg, LUA_QL("<eof>")) != p)
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
  }


  lua_close(l);
}
