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
#include <stdio.h>
#include <stdarg.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "debug.h"
#include "fn.h"
#include "hexdump.h"



static uint8_t debug_flags = 0;

static int debug(lua_State *l);
static void debug_color(int fg, int bg, int attr);




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


void debug_cmd(const char *name)
{
  if(!(debug_flags & DEBUG_STAT))
    return;

  debug_color(DEBUG_MAGENTA, -1, 0);
  printf(">>> %s <<<\n", name);
  debug_color(-1, -1, 0);
}


void debug_info(const char *fmt, ...)
{
  va_list args;


  if(!(debug_flags & DEBUG_INFO))
    return;

  debug_color(DEBUG_WHITE, -1, DEBUG_BOLD);
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
    debug_color(DEBUG_BLUE, -1, DEBUG_BOLD);
    arrow = " =>";
  }
  else if(dir & debug_flags & DEBUG_OUT)
  {
    debug_color(DEBUG_CYAN, -1, DEBUG_BOLD);
    arrow = "<= ";
  }
  else
    return;

  printf("%8s %s ", label, arrow);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  debug_color(-1, -1, 0);
  printf("\n");
}


void debug_result(uint8_t err, const char *str)
{
  if(!(debug_flags & DEBUG_STAT))
    return;

  debug_color(err == 0 ? DEBUG_GREEN : DEBUG_RED, -1, DEBUG_BOLD);
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
  unsigned int idx;
  char *line;


  for(idx = 0; idx < len; idx += 8)
  {
    line = hexdump_line(buf + idx, len - idx, offset + idx);
    debug_gen(dir, "BUF", "%s", line);
  }
}
