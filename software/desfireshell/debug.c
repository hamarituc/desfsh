#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "debug.h"
#include "fn.h"



static uint8_t debug_flags = 0;

static int debug(lua_State *l);
static void debug_color(int fg, int bg, int attr);




/*FN_ALIAS(debug) = { "debugset", NULL };
FN_PARAM(debug) =
{
  FNPARAM("stat", "Show Status",                     1),
  FNPARAM("in",   "Show Input",                      1),
  FNPARAM("out",  "Show Output",                     1),
  FNPARAM("info", "Show miscellaneous Informations", 1),
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
  luaL_argcheck(l, lua_gettop(l) < 4 || lua_isnil(l, 4) || lua_isboolean(l, 4), 4, "info: boolean or nil expected");

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

  if(lua_isboolean(l, 4))
  {
    if(lua_toboolean(l, 4))
      debug_flags |=  DEBUG_INFO;
    else
      debug_flags &= ~DEBUG_INFO;
  }


  return 0;
}*/




FN_ALIAS(debug) = { "debugset", NULL };
FN_PARAM(debug) =
{
  FNPARAM("mask", "Debug Bitmask", 0),
  FNPARAMEND
};
FN_RET(debug) =
{
  FNPARAMEND
};
FN(NULL, debug, "Set Debug Flags",
"Sets the debug flags. The debug bitmask has to be specified as an integer.\n" \
" 1 ==> Show command status\n" \
" 2 ==> Show command input parameters\n" \
" 4 ==> Show command results\n" \
" 8 ==> Show other informations\n");


static int debug(lua_State *l)
{
  luaL_argcheck(l, lua_isnumber(l, 1), 1, "mask: number expected");

  debug_flags = lua_tonumber(l, 1);

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


void debug_info(const char *fmt, ...)
{
  va_list args;


  if(!(debug_flags & DEBUG_INFO))
    return;

  debug_color(DEBUG_WHITE, 0, DEBUG_BOLD);
  va_start(args, fmt);
  printf("         *I* ");
  vprintf(fmt, args);
  printf("\n");
  va_end(args);
  debug_color(-1, -1, 0);
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


void debug_comm(unsigned char dir, uint8_t comm)
{
  const char *commstr;


  switch(comm)
  {
  case MDCM_PLAIN:      
  case MDCM_PLAIN | 0x02: commstr = "PLAIN"; break;
  case MDCM_MACED:        commstr = "MAC";   break;
  case MDCM_ENCIPHERED:   commstr = "CRYPT"; break;
  default:                commstr = "???";   break;
  }

  debug_gen(dir, "COMM", "0x%02x (%s)", comm, commstr);
}


void debug_acl(unsigned char dir, uint16_t acl)
{
  static const char *keystr[] =
  {
    "00", "01", "02", "03",
    "04", "05", "06", "07",
    "08", "09", "10", "11",
    "12", "13", "**", "--",
  };

  debug_gen(dir, "ACL", "RD:%s WR:%s RW:%s CA:%s",
    keystr[MDAR_READ(acl)       & 0x0f],
    keystr[MDAR_WRITE(acl)      & 0x0f],
    keystr[MDAR_READ_WRITE(acl) & 0x0f],
    keystr[MDAR_CHANGE_AR(acl)  & 0x0f]);
}


void debug_buffer(unsigned char dir, uint8_t *buf, unsigned int len, unsigned int offset)
{
  unsigned int idx, col;
  char line[80], *linepos;


  idx = 0;
  while(idx < len)
  {
    linepos = line;

    linepos += sprintf(linepos, "%08x ", offset + idx);

    for(col = 0; col < 8; col++)
    {
      linepos += sprintf(linepos, col != 4 ? " " : "  ");
      if(idx + col < len)
        linepos += sprintf(linepos, "%02x", buf[idx + col]);
      else
        linepos += sprintf(linepos, "  ");
    }

    linepos += sprintf(linepos, "  |");

    for(col = 0; col < 8; col++)
    {
      if(idx + col < len)
      {
        uint8_t c = buf[idx + col];
        linepos += sprintf(linepos, "%c",
          c >= 0x20 && c <= 0x7f ? c : '.');
      }
      else
        linepos += sprintf(linepos, " ");
    }

    linepos += sprintf(linepos, "|");
    idx += 8;

    debug_gen(dir, "BUF", "%s", line);
  }
}
