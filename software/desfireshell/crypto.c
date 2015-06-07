#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <openssl/err.h>
#include <openssl/cmac.h>
#include <openssl/hmac.h>

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
  unsigned int klen;
  uint8_t *input, *key, *mac;
  unsigned int inputlen, keylen;
  CMAC_CTX *ctx;
  size_t maclen;



  luaL_argcheck(l, lua_isstring(l, 1), 1, "cipher identifier expected");

  cipherstr = lua_tostring(l, 1);
  cipher = EVP_get_cipherbyname(cipherstr);
  if(cipher == NULL)
    return luaL_error(l, "cipher '%s' unknown", cipherstr);

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

  mac = (uint8_t*)malloc(EVP_MAX_BLOCK_LENGTH * sizeof(uint8_t));
  if(mac == NULL)
  {
    free(input);
    memset(key, 0, keylen);
    free(key);

    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);
  }

  ctx = CMAC_CTX_new();
  if(!CMAC_Init(ctx, key, keylen, cipher, NULL)) { goto fail; }
  if(!CMAC_Update(ctx, input, inputlen))         { goto fail; }
  if(!CMAC_Final(ctx, mac, &maclen))             { goto fail; }
  CMAC_CTX_free(ctx);

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

  lua_settop(l, 0);

  free(input);
  memset(key, 0, keylen);
  free(key);
  memset(mac, 0, EVP_MAX_BLOCK_LENGTH);
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
  uint8_t *input, *key, *mac;
  unsigned int inputlen, keylen, maclen;
  HMAC_CTX ctx;



  luaL_argcheck(l, lua_isstring(l, 1), 1, "message digest identifier expected");

  digeststr = lua_tostring(l, 1);
  digest = EVP_get_digestbyname(digeststr);
  if(digest == NULL)
    return luaL_error(l, "message digest '%s' unknown", digeststr);

  result = buffer_get(l, 2, &input, &inputlen);
  if(result)
    desflua_argerror(l, 2, "input");

  result = buffer_get(l, 3, &key, &keylen);
  if(result)
  {
    free(input);
    desflua_argerror(l, 2, "key");
  }

  mac = (uint8_t*)malloc(EVP_MAX_MD_SIZE * sizeof(uint8_t));
  if(mac == NULL)
  {
    free(input);
    memset(key, 0, keylen);
    free(key);

    return luaL_error(l, "internal error (%s:%d): out of memory", __FILE__, __LINE__);
  }

  HMAC_CTX_init(&ctx);
  if(!HMAC_Init(&ctx, key, keylen, digest)) { goto fail; }
  if(!HMAC_Update(&ctx, input, inputlen))   { goto fail; }
  if(!HMAC_Final(&ctx, mac, &maclen))       { goto fail; }
  HMAC_CTX_cleanup(&ctx);

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
