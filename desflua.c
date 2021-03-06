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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>

#include "buffer.h"
#include "cmd.h"
#include "debug.h"
#include "desflua.h"



int desflua_get_comm(lua_State *l, int idx, uint8_t *comm)
{
  const char *commstr;


  if(comm == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): comm=%p", __FILE__, __LINE__, comm);
    return -1;
  }

  if(lua_isnumber(l, idx))
    *comm = lua_tonumber(l, idx);
  else if(lua_isstring(l, idx))
  {
    commstr = lua_tostring(l, idx);

         if(!strcasecmp(commstr, "PLAIN")) { *comm = MDCM_PLAIN;      }
    else if(!strcasecmp(commstr, "MAC"))   { *comm = MDCM_MACED;      }
    else if(!strcasecmp(commstr, "CRYPT")) { *comm = MDCM_ENCIPHERED; }
    else
    {
      lua_checkstack(l, 1);
      lua_pushfstring(l, "unkown communication mode '%s'", commstr);
      return -1;
    }
  }
  else
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "number or string expected");
    return -1;
  }


  return 0;
}


int desflua_get_acl(lua_State *l, int idx, uint16_t *acl)
{
  uint16_t rd, wr, rw, ca;


  if(acl == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): acl=%p", __FILE__, __LINE__, acl);
    return -1;
  }

  if(lua_isnumber(l, idx))
    *acl = lua_tointeger(l, idx);
  else if(lua_istable(l, idx))
  {
    lua_checkstack(l, 4);
    lua_getfield(l, idx, "rd");
    lua_getfield(l, idx, "wr");
    lua_getfield(l, idx, "rw");
    lua_getfield(l, idx, "ca");

    if(!lua_isnumber(l, -4) ||
       !lua_isnumber(l, -3) ||
       !lua_isnumber(l, -2) ||
       !lua_isnumber(l, -1))
    {
      lua_checkstack(l, 1);
           if(!lua_isnumber(l, -4)) { lua_pushstring(l, "read access: number expected");       }
      else if(!lua_isnumber(l, -3)) { lua_pushstring(l, "write access: number expected");      }
      else if(!lua_isnumber(l, -2)) { lua_pushstring(l, "read/write access: number expected"); }
      else if(!lua_isnumber(l, -1)) { lua_pushstring(l, "change access: number expected");     }
      lua_remove(l, -2);
      lua_remove(l, -2);
      lua_remove(l, -2);
      lua_remove(l, -2);
      return -1;
    }

    rd = lua_tointeger(l, -4);
    wr = lua_tointeger(l, -3);
    rw = lua_tointeger(l, -2);
    ca = lua_tointeger(l, -1);
    lua_pop(l, 4);

    *acl = MDAR(rd, wr, rw, ca);
  }
  else
  {
    lua_checkstack(l, 1);
    lua_pushstring(l, "number or table expected");
    return -1;
  }


  return 0;
}


void desflua_push_acl(lua_State *l, uint16_t acl)
{
  lua_checkstack(l, 2);
  lua_newtable(l);
  lua_pushinteger(l, MDAR_READ(acl));       lua_setfield(l, -2, "rd");
  lua_pushinteger(l, MDAR_WRITE(acl));      lua_setfield(l, -2, "wr");
  lua_pushinteger(l, MDAR_READ_WRITE(acl)); lua_setfield(l, -2, "rw");
  lua_pushinteger(l, MDAR_CHANGE_AR(acl));  lua_setfield(l, -2, "ca");
}


void desflua_handle_result(lua_State *l, int result, FreefareTag tag)
{
  uint8_t err;
  const char *str;


  lua_settop(l, 0);
  lua_checkstack(l, 2);

  err = mifare_desfire_last_picc_error(tag);
  str = result >= 0 ? "OK" : freefare_strerror(tag);
  lua_pushinteger(l, err);
  lua_pushstring(l, str);

  debug_result(err, str);
}


void desflua_argerror(lua_State *l, int argnr, const char *prefix)
{
  lua_checkstack(l, 1);
  lua_pushfstring(l, "%s: %s", prefix, lua_tostring(l, -1));
  lua_remove(l, -2);
  luaL_argerror(l, argnr, lua_tostring(l, -1));
}
