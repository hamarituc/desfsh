#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "cmd.h"
#include "shell.h"



static void shell_registerfn(lua_State *l, lua_CFunction f, const char *name)
{
  lua_checkstack(l, 1);
  lua_pushcfunction(l, f);
  lua_setfield(l, LUA_GLOBALSINDEX, name);
}


static void shell_register(lua_State *l)
{
  shell_registerfn(l, cmd_auth,         "cmd_auth");
//  shell_registerfn(l, cmd_cks,          "cmd_cks");
  shell_registerfn(l, cmd_gks,          "cmd_gks");
  shell_registerfn(l, cmd_ck,           "cmd_ck");
  shell_registerfn(l, cmd_gkv,          "cmd_gkv");
  shell_registerfn(l, cmd_createapp,    "cmd_createapp");
  shell_registerfn(l, cmd_deleteapp,    "cmd_deleteapp");
  shell_registerfn(l, cmd_appids,       "cmd_appids");
  shell_registerfn(l, cmd_selapp,       "cmd_selapp");
  shell_registerfn(l, cmd_format,       "cmd_format");
  shell_registerfn(l, cmd_getver,       "cmd_getver");
  shell_registerfn(l, cmd_fileids,      "cmd_fileids");
  shell_registerfn(l, cmd_gfs,          "cmd_gfs");
  shell_registerfn(l, cmd_cfs,          "cmd_cfs");
  shell_registerfn(l, cmd_csdf,         "cmd_csdf");
//  shell_registerfn(l, cmd_cbdf,         "cmd_cbdf");
//  shell_registerfn(l, cmd_cvf,          "cmd_cvf");
//  shell_registerfn(l, cmd_clrl,         "cmd_clrf");
//  shell_registerfn(l, cmd_ccrf,         "cmd_ccrf");
  shell_registerfn(l, show_picc,        "show_picc");
  shell_registerfn(l, show_apps,        "show_apps");
  shell_registerfn(l, show_files,       "show_files");
}


void shell()
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
  shell_register(l);


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
      fprintf(stderr, "%s\n", lua_tostring(l, -1));
    lua_settop(l, 0);
  }


  lua_close(l);
}
