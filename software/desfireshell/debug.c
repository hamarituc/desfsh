#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <lua.h>
#include <lauxlib.h>

#include "debug.h"
#include "fn.h"



static uint8_t debug_flags = 0;

static int debug(lua_State *l);
static void debug_color(int fg, int bg, int attr);




FN_ALIAS(debug) = { "dbg", NULL };
FN_PARAM(debug) =
{
  FNPARAM("stat", "Show Status", 0),
  FNPARAM("in",   "Show Input",  0),
  FNPARAM("out",  "Show Output", 0),
  FNPARAMEND
};
FN_RET(debug) =
{
  FNPARAMEND
};
FN(NULL, debug, "Set Debug Flags",
"Sets the debug flags. A flag is set, when the corresponding argument is 'true'\n" \
"and cleared when the corresponding argument is 'false'. The flag remains its\n" \
"state when the corresponding parameter is 'nil'.\n");


static int debug(lua_State *l)
{
  luaL_argcheck(l, lua_gettop(l) < 1 || lua_isnil(l, 1) || lua_isboolean(l, 1), 1, "stat: boolean or nil expected");
  luaL_argcheck(l, lua_gettop(l) < 2 || lua_isnil(l, 2) || lua_isboolean(l, 2), 2, "in: boolean or nil expected");
  luaL_argcheck(l, lua_gettop(l) < 3 || lua_isnil(l, 3) || lua_isboolean(l, 3), 3, "out: boolean or nil expected");

  if(lua_isboolean(l, 1))
  {
    if(lua_toboolean(l, 1))
      debug_flags |=  DEBUG_STAT;
    else
      debug_flags &= ~DEBUG_STAT;
  }

  if(lua_isboolean(l, 2))
  {
    if(lua_toboolean(l, 2))
      debug_flags |=  DEBUG_IN;
    else
      debug_flags &= ~DEBUG_IN;
  }

  if(lua_isboolean(l, 3))
  {
    if(lua_toboolean(l, 3))
      debug_flags |=  DEBUG_OUT;
    else
      debug_flags &= ~DEBUG_OUT;
  }


  return 0;
}




static void debug_color(int fg, int bg, int attr)
{
  if(fg < 0 && bg < 0 && attr < 0)
    return;

  printf("\033[");
  if(attr >= 0)
    printf("%d%s", attr & 0x0f,      fg >= 0 || bg >= 0 ? ";" : "");
  if(fg >= 0)
    printf("%d%s", 30 + (fg & 0x07), bg >= 0 ? ";" : "");
  if(bg >= 0)
    printf("%d",   40 + (bg & 0x07));
  printf("m");
}


void debug_gen(unsigned char dir, const char *label, const char *fmt, ...)
{
  va_list args;
  const char *arrow;


  if(dir & debug_flags & DEBUG_IN)
  {
    debug_color(DEBUG_BLUE, 0, DEBUG_BOLD);
    arrow = " =>";
  }
  else if(dir & debug_flags & DEBUG_OUT)
  {
    debug_color(DEBUG_CYAN, 0, DEBUG_BOLD);
    arrow = "<= ";
  }
  else
    return;

  va_start(args, fmt);
  printf("%8s %s ", label, arrow);
  vprintf(fmt, args);
  printf("\n");
  va_end(args);
  debug_color(-1, -1, 0);
}


void debug_result(uint8_t err, const char *str)
{
  if(!(debug_flags & DEBUG_STAT))
    return;

  debug_color(err == 0 ? DEBUG_GREEN : DEBUG_RED, 0, DEBUG_BOLD);
  printf("%8s <=> %d: %s\n", "STAT", err, str);
  debug_color(-1, -1, 0);
}


void debug_keysettings(unsigned char dir, uint8_t settings)
{
  static const char *akc[] =
  {
    "AMK", "K01", "K02", "K03",
    "K04", "K05", "K06", "K07",
    "K08", "K09", "K10", "K11",
    "K12", "K13", "SLF", "---",
  };

  debug_gen(dir, "KEYSET", "0x%02x = AKC:%s CONF:%s CA/F:%s DA/F:%s LIST:%s MKC:%s",
    settings,
    akc[(settings >> 4) & 0x0f],
    (settings & 0x08) ? "M"   : "-",
    (settings & 0x04) ? "*"   : "M",
    (settings & 0x04) ? "M/*" : "M",
    (settings & 0x02) ? "*"   : "M",
    (settings & 0x01) ? "M"   : "-");
}
