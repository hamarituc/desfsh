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
#include <lua.h>
#include <lauxlib.h>
#include <zlib.h>

#include "buffer.h"
#include "desflua.h"
#include "fn.h"



static int crc_crc32(lua_State *l);




FN_ALIAS(crc_crc32) = { "crc32", NULL };
FN_PARAM(crc_crc32) =
{
  FNPARAM("input", "Input Buffer", 0),
  FNPARAMEND
};
FN_RET(crc_crc32) =
{
  FNPARAM("crc32", "Checksum", 0),
  FNPARAMEND
};
FN("crc", crc_crc32, "Calculate a CRC-32 checksum",
"Calculates the CRC-32 checksum of the input buffer <input>. The checksum\n" \
"is returned as a buffer.\n");


static int crc_crc32(lua_State *l)
{
  int result;
  uint8_t *input, crcbuf[4];
  unsigned int inputlen;
  uint32_t crc;



  result = buffer_get(l, 1, &input, &inputlen);
  if(result)
    desflua_argerror(l, 1, "input");

  crc = crc32(0, Z_NULL, 0);
  crc = crc32(crc, input, inputlen);
  crcbuf[0] = (crc >> 24) & 0xff;
  crcbuf[1] = (crc >> 16) & 0xff;
  crcbuf[2] = (crc >>  8) & 0xff;
  crcbuf[3] =  crc        & 0xff;

  lua_settop(l, 0);
  buffer_push(l, crcbuf, 4);


  return lua_gettop(l);
}
