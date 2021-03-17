#include <stdlib.h>
#include <stdint.h>
#include <lua.h>
#include <lauxlib.h>

#include "buffer.h"
#include "cmd.h"
#include "crc.h"
#include "crypto.h"
#include "debug.h"
#include "fn.h"
#include "help.h"
#include "key.h"
#include "show.h"



static const char fn_initcode[] =
{
  #include "luainit.inc"
};



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
  
#if LUA_VERSION_NUM > 501
  if(fn->class == NULL)
    lua_getglobal(l, "_G");
  else
  {
    lua_getglobal(l, fn->class);
    if(lua_isnil(l, -1))
    {
      lua_pop(l, 1);
      lua_checkstack(l, 2);
      lua_newtable(l);
      lua_pushvalue(l, -1);
      lua_setglobal(l, fn->class);
    }
  }

  luaL_setfuncs(l, list, 0);
#else
  if(fn->class == NULL)
    lua_getglobal(l, "_G");
  luaL_register(l, fn->class, list);
#endif

  lua_pop(l, 1);
  free(list);


  help_regfn(l, fn);
}


int fn_init(lua_State *l, int online)
{
  int result;


  help_init(l);

  fn_register(l, FNREF(help)); 

  if(online)
    fn_register(l, FNREF(debug)); 

  if(online)
  {
    fn_register(l, FNREF(cmd_auth)); 
    fn_register(l, FNREF(cmd_cks));
    fn_register(l, FNREF(cmd_gks));
    fn_register(l, FNREF(cmd_ck));
    fn_register(l, FNREF(cmd_gkv));

    fn_register(l, FNREF(cmd_createapp));
    fn_register(l, FNREF(cmd_deleteapp));
    fn_register(l, FNREF(cmd_appids));
    fn_register(l, FNREF(cmd_selapp));
    fn_register(l, FNREF(cmd_format));
    fn_register(l, FNREF(cmd_getver));
    fn_register(l, FNREF(cmd_freemem));
    fn_register(l, FNREF(cmd_carduid));

    fn_register(l, FNREF(cmd_fileids));
    fn_register(l, FNREF(cmd_gfs));
    fn_register(l, FNREF(cmd_cfs));
    fn_register(l, FNREF(cmd_csdf));
    fn_register(l, FNREF(cmd_cbdf));
    fn_register(l, FNREF(cmd_cvf));
    fn_register(l, FNREF(cmd_clrf));
    fn_register(l, FNREF(cmd_ccrf));
    fn_register(l, FNREF(cmd_delf));

    fn_register(l, FNREF(cmd_read));
    fn_register(l, FNREF(cmd_write));
    fn_register(l, FNREF(cmd_getval));
    fn_register(l, FNREF(cmd_credit));
    fn_register(l, FNREF(cmd_debit));
    fn_register(l, FNREF(cmd_lcredit));
    fn_register(l, FNREF(cmd_wrec));
    fn_register(l, FNREF(cmd_rrec));
    fn_register(l, FNREF(cmd_crec));
    fn_register(l, FNREF(cmd_commit));
    fn_register(l, FNREF(cmd_abort));

    fn_register(l, FNREF(show_picc));
    fn_register(l, FNREF(show_apps));
    fn_register(l, FNREF(show_files));
  }

  fn_register(l, FNREF(buffer_from_table));
  fn_register(l, FNREF(buffer_from_hexstr));
  fn_register(l, FNREF(buffer_from_ascii));
  fn_register(l, FNREF(buffer_to_table));
  fn_register(l, FNREF(buffer_to_hexstr));
  fn_register(l, FNREF(buffer_to_ascii));
  fn_register(l, FNREF(buffer_to_hexdump));
  fn_register(l, FNREF(buffer_concat));

  fn_register(l, FNREF(key_create));
  fn_register(l, FNREF(key_div));

  fn_register(l, FNREF(crc_crc32));

  fn_register(l, FNREF(crypto_cmac));
  fn_register(l, FNREF(crypto_hmac));

//  help_regtopic(l, "key", "Key datastructure", "TODO\n");

  result = luaL_dostring(l, fn_initcode);

  return result;
}
