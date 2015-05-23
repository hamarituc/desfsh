#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>

#include "cmd.h"
#include "fn.h"



static int fn_help(lua_State *l)
{
  lua_CFunction cfn;
  const struct fn_t *fn;


  // TODO: Allgemeine Hilfe
  luaL_argcheck(l, lua_isfunction(l, 1), 1, "invalid function");
  cfn = lua_tocfunction(l, 1);

  lua_checkstack(l, 2);
  lua_getfield(l, LUA_REGISTRYINDEX, "desfsh");
  lua_pushlightuserdata(l, cfn);
  lua_gettable(l, -2);
  fn = lua_touserdata(l, -1);
  lua_settop(l, 0);

  if(fn == NULL)
    return luaL_argerror(l, 1, "function not known");

  printf("%s\n", fn->brief);
  printf("%s\n", fn->desc);


  return 0;
}


static void fn_register(lua_State *l, const struct fn_t *fn)
{
  unsigned int nalias, i;
  luaL_Reg *list;


  nalias = 0;
  while(fn->alias[nalias] != NULL)
    nalias++;

  if(nalias == 0)
    return;

  list = (luaL_Reg*)malloc((nalias + 1) * sizeof(luaL_Reg));
  if(list == NULL)
    luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);

  for(i = 0; i < nalias; i++)
  {
    list[i].name = fn->alias[i];
    list[i].func = fn->fn;
  }

  list[nalias].name = NULL;
  list[nalias].func = NULL;
  
  luaL_register(l, fn->class, list);
  lua_pop(l, 1);
  free(list);

  /*
   * Um die Funktion zu identifierzieren nutzen wir ihre Adresse.
   * Nicht schön, funktioniert aber im Gegensatz zum Funktionsobjekt.
   */
  lua_checkstack(l, 3);
  lua_getfield(l, LUA_REGISTRYINDEX, "desfsh");
  lua_pushlightuserdata(l, fn->fn);
  lua_pushlightuserdata(l, (void*)fn);
  lua_settable(l, -3);
  lua_pop(l, 1);
}


void fn_init(lua_State *l)
{
  lua_checkstack(l, 1);
  lua_newtable(l);
  lua_setfield(l, LUA_REGISTRYINDEX, "desfsh");

  lua_register(l, "help", fn_help);


  fn_register(l, FNREF(cmd_auth)); 
  fn_register(l, FNREF(cmd_cks));
  fn_register(l, FNREF(cmd_gks));
  fn_register(l, FNREF(cmd_ck));
  fn_register(l, FNREF(cmd_gkv));
}
