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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>

#include "buffer.h"
#include "desflua.h"
#include "fn.h"



static int crypto_cmac(lua_State *l);
static int crypto_hmac(lua_State *l);




FN_ALIAS(crypto_cmac) = { "cmac", NULL };
FN_PARAM(crypto_cmac) =
{
  FNPARAM("cipher", "Block Chipher Identifier", 0),
  FNPARAM("input",  "Input Buffer",             0),
  FNPARAM("key",    "Secret Key",               0),
  FNPARAMEND
};
FN_RET(crypto_cmac) =
{
  FNPARAM("mac", "MAC", 0),
  FNPARAMEND
};
FN("crypto", crypto_cmac, "Calculate CMAC",
"Calculates the CMAC of the input buffer <input> using the specified\n" \
"symmetric cipher. The size of <key> and <mac> depends on the choosen\n" \
"cipher.\n");


static int crypto_cmac(lua_State *l)
{
  int result;
  const char *cipherstr;
  const EVP_CIPHER *cipher;
  OSSL_PARAM macparams[2];
  unsigned int klen;
  uint8_t *input, *key, *mac;
  unsigned int inputlen, keylen;
  EVP_MAC *macalg;
  EVP_MAC_CTX *ctx;
  size_t maclen, buflen;



  luaL_argcheck(l, lua_isstring(l, 1), 1, "cipher identifier expected");

  cipherstr = lua_tostring(l, 1);
  cipher = EVP_get_cipherbyname(cipherstr);
  if(cipher == NULL)
    return luaL_error(l, "cipher '%s' unknown", cipherstr);

  macparams[0] = OSSL_PARAM_construct_utf8_string("cipher", (char*)cipherstr, 0);
  macparams[1] = OSSL_PARAM_construct_end();

  klen = EVP_CIPHER_key_length(cipher);

  result = buffer_get(l, 2, &input, &inputlen);
  if(result)
    desflua_argerror(l, 2, "input");

  result = buffer_get(l, 3, &key, &keylen);
  if(result)
  {
    free(input);
    desflua_argerror(l, 2, "key");
  }

  if(klen != keylen)
  {
    free(input);
    memset(key, 0, keylen);
    free(key);

    lua_settop(l, 0);
    lua_checkstack(l, 1);
    lua_pushfstring(l, "key length %d invalid, expected %d bytes", keylen, klen);

    return luaL_argerror(l, 3, lua_tostring(l, -1));
  }

  buflen = EVP_MAX_BLOCK_LENGTH * sizeof(uint8_t);
  mac = (uint8_t*)malloc(buflen);
  if(mac == NULL)
  {
    free(input);
    memset(key, 0, keylen);
    free(key);

    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);
  }

  macalg = EVP_MAC_fetch(NULL, "CMAC", NULL);
  if(macalg == NULL)
    goto fail;

  ctx = EVP_MAC_CTX_new(macalg);
  if(!EVP_MAC_init(ctx, key, keylen, macparams)) { goto fail; }
  if(!EVP_MAC_update(ctx, input, inputlen))      { goto fail; }
  if(!EVP_MAC_final(ctx, mac, &maclen, buflen))  { goto fail; }
  EVP_MAC_CTX_free(ctx);
  EVP_MAC_free(macalg);

  lua_settop(l, 0);
  buffer_push(l, mac, maclen);

  free(input);
  memset(key, 0, keylen);
  free(key);
  memset(mac, 0, EVP_MAX_BLOCK_LENGTH);
  free(mac);


  return lua_gettop(l);


fail:;
  unsigned long err;

  EVP_MAC_free(macalg);

  lua_settop(l, 0);

  free(input);
  memset(key, 0, keylen);
  free(key);
  memset(mac, 0, EVP_MAX_BLOCK_LENGTH);
  free(mac);

  lua_checkstack(l, 2);
  lua_pushstring(l, "Crypto error:\n");
  while((err = ERR_get_error()))
  {
    lua_pushfstring(l, "%s\n", ERR_error_string(err, NULL));
    lua_concat(l, 2);
  }

  return lua_error(l);
}




FN_ALIAS(crypto_hmac) = { "hmac", NULL };
FN_PARAM(crypto_hmac) =
{
  FNPARAM("hash",  "Hash Function", 0),
  FNPARAM("input", "Input Buffer",  0),
  FNPARAM("key",   "Secret Key",    0),
  FNPARAMEND
};
FN_RET(crypto_hmac) =
{
  FNPARAM("mac", "MAC", 0),
  FNPARAMEND
};
FN("crypto", crypto_hmac, "Calculate HMAC",
"Calculates the HMAC of the input buffer <input> using the specified\n" \
"message digest. The size of <key> and <mac> depends on the choosen\n" \
"digest.\n");


static int crypto_hmac(lua_State *l)
{
  int result;
  const char *digeststr;
  const EVP_MD *digest;
  OSSL_PARAM macparams[2];
  uint8_t *input, *key, *mac;
  unsigned int inputlen, keylen;
  EVP_MAC *macalg;
  EVP_MAC_CTX *ctx;
  size_t maclen, buflen;



  luaL_argcheck(l, lua_isstring(l, 1), 1, "message digest identifier expected");

  digeststr = lua_tostring(l, 1);
  digest = EVP_get_digestbyname(digeststr);
  if(digest == NULL)
    return luaL_error(l, "message digest '%s' unknown", digeststr);

  macparams[0] = OSSL_PARAM_construct_utf8_string("digest", (char*)digeststr, 0);
  macparams[1] = OSSL_PARAM_construct_end();

  result = buffer_get(l, 2, &input, &inputlen);
  if(result)
    desflua_argerror(l, 2, "input");

  result = buffer_get(l, 3, &key, &keylen);
  if(result)
  {
    free(input);
    desflua_argerror(l, 2, "key");
  }

  buflen = EVP_MAX_MD_SIZE * sizeof(uint8_t);
  mac = (uint8_t*)malloc(buflen);
  if(mac == NULL)
  {
    free(input);
    memset(key, 0, keylen);
    free(key);

    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);
  }

  macalg = EVP_MAC_fetch(NULL, "HMAC", NULL);
  if(macalg == NULL)
    goto fail;

  ctx = EVP_MAC_CTX_new(macalg);
  if(!EVP_MAC_init(ctx, key, keylen, macparams)) { goto fail; }
  if(!EVP_MAC_update(ctx, input, inputlen))      { goto fail; }
  if(!EVP_MAC_final(ctx, mac, &maclen, buflen))  { goto fail; }
  EVP_MAC_CTX_free(ctx);
  EVP_MAC_free(macalg);

  lua_settop(l, 0);
  buffer_push(l, mac, maclen);

  free(input);
  memset(key, 0, keylen);
  free(key);
  memset(mac, 0, EVP_MAX_MD_SIZE);
  free(mac);


  return lua_gettop(l);


fail:;
  unsigned long err;

  EVP_MAC_free(macalg);

  lua_settop(l, 0);

  free(input);
  memset(key, 0, keylen);
  free(key);
  memset(mac, 0, EVP_MAX_MD_SIZE);
  free(mac);

  lua_checkstack(l, 2);
  lua_pushstring(l, "");
  while((err = ERR_get_error()))
  {
    lua_pushfstring(l, "%s\n", ERR_error_string(err, NULL));
    lua_concat(l, 2);
  }

  return luaL_error(l, "Crypto error:\n%s", lua_tostring(l, 1));
}
