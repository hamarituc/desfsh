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
