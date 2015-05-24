#include <string.h>
#include <lua.h>
#include <lauxlib.h>

#include "fn.h"
#include "help.h"



static int help(lua_State *l);



static int help_sort(lua_State *l)
{
  const char *t1, *t2;
  int result;


  lua_settop(l, 2);
  lua_checkstack(l, 2);
  lua_getfield(l, -2, "topic");
  lua_getfield(l, -2, "topic");
  t1 = lua_tostring(l, -2);
  t2 = lua_tostring(l, -1);
  result = strcasecmp(t1, t2);

  lua_settop(l, 0);
  lua_pushboolean(l, result < 0);


  return 1;
}


static void help_gen(lua_State *l)
{
  unsigned int i, n;


  lua_checkstack(l, 1);
  lua_newtable(l);


  /* Alle Hilfethemen durchgehen und in einer Key-Value-Liste speichern. */
  lua_checkstack(l, 5);
  lua_getfield(l, LUA_REGISTRYINDEX, "desfsh");
  lua_pushnil(l);
  while(lua_next(l, -2) != 0)
  {
    if(lua_isuserdata(l, -1))
    {
      struct fn_t *fn;

      fn = lua_touserdata(l, -1);
      if(fn->class != NULL)
        lua_pushfstring(l, "%s.%s", fn->class, fn->alias[0]);
      else
        lua_pushstring(l, fn->alias[0]);
      lua_pushstring(l, fn->brief);
      lua_settable(l, -6);
    }
    else if(lua_istable(l, -1))
    {
      lua_pushvalue(l, -2);
      lua_getfield(l, -2, "brief");
      lua_settable(l, -6);
    }
      
    lua_pop(l, 1);
  }
  lua_pop(l, 1);


  /* Die Key-Value-Liste in ein Array umwandeln. */
  lua_checkstack(l, 5);
  lua_newtable(l);
  lua_pushnil(l);
  i = 0;
  while(lua_next(l, -3) != 0)
  {
    lua_pushinteger(l, ++i);

    /* 
     * Die Einträge haben die Form:
     *
     * { topic = <Name>, brief = <Kurzbeschreibung> }
     */
    lua_newtable(l);
    lua_pushvalue(l, -4); lua_setfield(l, -2, "topic");
    lua_pushvalue(l, -3); lua_setfield(l, -2, "brief");
    lua_settable(l, -5);

    lua_pop(l, 1);
  }
  n = i;
  lua_remove(l, -2);


  /* Die Liste sortieren. */
  lua_getglobal(l, "table");
  lua_getfield(l, -1, "sort");
  lua_remove(l, -2);
  lua_pushvalue(l, -2);
  lua_pushcfunction(l, help_sort);
  lua_call(l, 2, 0);


  /* Ausgabe */
  printf("\n");
  lua_checkstack(l, 3);
  for(i = 0; i < n; i++)
  {
    lua_pushinteger(l, i + 1);
    lua_gettable(l, -2);

    lua_getfield(l, -1, "topic");
    lua_getfield(l, -2, "brief");
    printf("%-20s  %s\n", lua_tostring(l, -2), lua_tostring(l, -1));

    lua_pop(l, 3);
  }
  printf("\n");
}


static void help_fn(const struct fn_t *fn)
{
  unsigned int nparam, nret, i;


  nparam = 0;
  while(fn->param[nparam].name != NULL)
    nparam++;

  nret = 0;
  while(fn->ret[nret].name != NULL)
    nret++;

  printf("\n");

  if(fn->brief != NULL)
    printf("%s\n\n", fn->brief);

  for(i = 0; i < nret; i++)
  {
    const struct fn_param_t *ret;

    ret = &fn->ret[i];
    printf("%s%s%s%s",
      i > 0 ? ", " : "",
      ret->opt ? "[" : "",
      ret->name,
      ret->opt ? "]" : "");
  }

  if(nret > 0)
    printf(" = ");

  printf("%s(", fn->alias[0]);

  for(i = 0; i < nparam; i++)
  {
    const struct fn_param_t *param;

    param = &fn->param[i];
    printf("%s%s%s%s",
      i > 0 ? ", " : "",
      param->opt ? "[" : "",
      param->name,
      param->opt ? "]" : "");
  }

  printf(")\n\n");

  printf("Aliasses: ");
  for(i = 0; fn->alias[i] != NULL; i++)
    printf("%s%s", i > 0 ? ", " : "", fn->alias[i]);
  printf("\n\n");

  for(i = 0; i < nret; i++)
  {
    const struct fn_param_t *ret;

    ret = &fn->ret[i];
    if(ret->name != NULL && ret->desc != NULL)
      printf("    %-10s  %s\n", ret->name, ret->desc);
  }

  for(i = 0; i < nparam; i++)
  {
    const struct fn_param_t *param;

    param = &fn->param[i];
    if(param->name != NULL && param->desc != NULL)
      printf("    %-10s  %s\n", param->name, param->desc);
  }

  if(nret > 0 || nparam > 0)
    printf("\n");

  if(fn->desc != NULL)
    printf("%s\n", fn->desc);
}


static void help_topic(lua_State *l, int idx)
{
  lua_checkstack(l, 1);
  lua_getfield(l, idx, "desc");

  printf("\n%s\n", lua_tostring(l, -1));
  lua_pop(l, 1);
}




FN_ALIAS(help) = { "help", NULL };
FN_PARAM(help) =
{
  FNPARAM("item", "Function or topic", 0),
  FNPARAMEND
};
FN_RET(help) =
{
  FNPARAMEND
};
FN(NULL, help, "Show help text", NULL);


static int help(lua_State *l)
{
  lua_CFunction cfn;


  luaL_argcheck(l, \
    lua_gettop(l) < 1 || lua_isnil(l, 1) || lua_isstring(l, 1) || lua_isfunction(l, 1), \
    1, "invalid function or topic");


  /* Allgmeine Hilfe. */
  if(lua_gettop(l) < 1)
  {
    lua_settop(l, 0);
    help_gen(l);
    return 0;
  }


  /* Argument aus der Registry abrufen. */
  lua_checkstack(l, 2);
  lua_getfield(l, LUA_REGISTRYINDEX, "desfsh");

  if(lua_isfunction(l, 1))
  {
    cfn = lua_tocfunction(l, 1);
    lua_pushlightuserdata(l, cfn);
  }
  else
    lua_pushvalue(l, 1);

  lua_gettable(l, -2);


  /* Unterscheiden, ob es sich um eine Funktion oder ein Topic handelt. */
  if(lua_isuserdata(l, -1))
    help_fn(lua_touserdata(l, -1));
  else if(lua_istable(l, -1))
    help_topic(l, -1);
  else
    return luaL_argerror(l, 1, "function or topic not known");

  lua_settop(l, 0);


  return 0;
}




void help_regfn(lua_State *l, const struct fn_t *fn)
{
  unsigned int i;


  /*
   * Um die Funktion zu identifizieren nutzen wir ihre Adresse.
   * Nicht schön, funktioniert aber im Gegensatz zum Funktionsobjekt.
   */
  lua_checkstack(l, 3);
  lua_getfield(l, LUA_REGISTRYINDEX, "desfsh");

  for(i = 0; fn->alias[i] != NULL; i++)
  {
    /* Aliasse in der Hilfe registrieren. */
    if(fn->class == NULL)
      lua_pushstring(l, fn->alias[i]);
    else
      lua_pushfstring(l, "%s.%s", fn->class, fn->alias[i]);

    lua_pushvalue(l, -1);
    lua_gettable(l, -3);
    if(!lua_isnil(l, -1))
      luaL_error(l, "internal error (%s:%d): function alias %s already defined", \
        __FILE__, __LINE__, lua_tostring(l, -1));
    lua_pop(l, 1);

    lua_pushlightuserdata(l, (void*)fn);
    lua_settable(l, -3);
  }

  lua_pushlightuserdata(l, fn->fn);
  lua_pushlightuserdata(l, (void*)fn);
  lua_settable(l, -3);
  lua_pop(l, 1);
}


void help_regtopic(lua_State *l, const char *topic, const char *brief, const char *desc)
{
  if(topic == NULL || brief == NULL || desc == NULL)
    return;

  lua_checkstack(l, 5);
  lua_getfield(l, LUA_REGISTRYINDEX, "desfsh");
  lua_pushstring(l, topic);

  lua_newtable(l);
  lua_pushstring(l, brief); lua_setfield(l, -2, "brief");
  lua_pushstring(l, desc);  lua_setfield(l, -2, "desc");

  lua_settable(l, -3);
  lua_pop(l, 1);
}


void help_init(lua_State *l)
{
  lua_checkstack(l, 1);
  lua_newtable(l);
  lua_setfield(l, LUA_REGISTRYINDEX, "desfsh");
}
