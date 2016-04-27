#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <openssl/err.h>
#include <openssl/cmac.h>

#include "buffer.h"
#include "desflua.h"
#include "fn.h"
#include "key.h"



typedef int (*key_div_fn_t)(lua_State *l,
  uint8_t *key, unsigned int keylen,
  uint8_t *uid, uint32_t *aid, uint8_t *kno,
  uint8_t *pad, unsigned int padlen,
  uint8_t **divkey, unsigned int *divkeylen);



static int key_div_aes(lua_State *l,
  uint8_t *key, unsigned int keylen,
  uint8_t *uid, uint32_t *aid, uint8_t *kno,
  uint8_t *pad, unsigned int padlen,
  uint8_t **divkey, unsigned int *divkeylen);

static int key_create(lua_State *l);
static int key_div(lua_State *l);




int key_gettype(lua_State *l, int idx, enum keytype_e *type, char **_typestr)
{
  const char *typestr;


  if(type == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): type=%p", __FILE__, __LINE__, type);
    return -1;
  }

  if(!lua_isstring(l, idx))
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "string expected");
    return -1;
  }

  typestr = lua_tostring(l, idx);

       if(!strcasecmp(typestr, "DES"))    { *type = _DES_;    }
  else if(!strcasecmp(typestr, "3DES"))   { *type = _3DES_;   }
  else if(!strcasecmp(typestr, "3K3DES")) { *type = _3K3DES_; }
  else if(!strcasecmp(typestr, "AES"))    { *type = _AES_;    }
  else
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "unkown key type '%s'", typestr);
    return -1;
  }

  if(_typestr != NULL)
  {
    *_typestr = NULL;

    switch(*type)
    {
    case _DES_:    *_typestr = "DES";    break;
    case _3DES_:   *_typestr = "3DES";   break;
    case _3K3DES_: *_typestr = "3K3DES"; break;
    case _AES_:    *_typestr = "AES";    break;
    }
  }


  return 0;
}


int key_getraw(lua_State *l, int idx, enum keytype_e *type, uint8_t **key, unsigned int *keylen, uint8_t *_ver, char **keystr)
{
  int result;
  char *typestr;
  unsigned int elen;
  uint8_t ver;



  if(type == NULL || key == NULL || keylen == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): type=%p key=%p keylen=%p", __FILE__, __LINE__, type, key, keylen);
    return -1;
  }

  if(!lua_istable(l, idx))
  {
    lua_checkstack(l, 1);
    lua_pushstring(l, "key must be a LUA table");
    return -1;
  }

  lua_checkstack(l, 1);


  /* Typ auslesen. */
  lua_getfield(l, idx, "t");
  result = key_gettype(l, -1, type, &typestr);
  if(result)
  {
    lua_remove(l, -2);
    lua_pushfstring(l, "key type invalid: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return -1;
  }
  lua_pop(l, 1);


  /* Version auslesen. */
  lua_getfield(l, idx, "v");
  if(!lua_isnil(l, -1) && !lua_isnumber(l, -1))
  {
    lua_pop(l, 1);
    lua_pushfstring(l, "key version invalid, number expected");
    return -1;
  }

  ver = lua_isnumber(l, -1) ? lua_tointeger(l, -1) : 0;
  lua_pop(l, 1);

  if(_ver != NULL)
    *_ver = ver;


  /* Schlüssel auslesen. */
  lua_getfield(l, idx, "k");
  result = buffer_get(l, -1, key, keylen);
  if(result)
  {
    lua_remove(l, -2);
    lua_pushfstring(l, "key string invalid: %s", lua_tostring(l, -1));
    lua_remove(l, -2);
    return -1;
  }
  lua_pop(l, 1);


  /* Schlüssellänge prüfen. */
  switch(*type)
  {
  case _DES_:    elen =  8; break;
  case _3DES_:   elen = 16; break;
  case _3K3DES_: elen = 24; break;
  case _AES_:    elen = 16; break;
  }

  if(*keylen != elen)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "key length %d invalid, expected %d bytes", *keylen, elen);
    free(*key);

    return -1;
  }


  /* Wird keine Debug-Ausgabe benötigt, sind wir fertig. */
  if(keystr == NULL)
    return 0;



  char *keystrpos;
  unsigned int i;


  /* Bei Bedarf Debug-Ausgabe anlegen. */
  keystrpos = *keystr = (char*)malloc(128 * sizeof(char));
  if(keystrpos == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): cannot create key", __FILE__, __LINE__);
    free(*key);

    return -1;
  }

  keystrpos += sprintf(keystrpos, "%s:", typestr);
  for(i = 0; i < *keylen; i++)
    keystrpos += sprintf(keystrpos, "%02x", (*key)[i]);
  keystrpos += sprintf(keystrpos, " (V:%03d)", ver);


  return 0;
}


int key_get(lua_State *l, int idx, MifareDESFireKey *k, char **keystr)
{
  int result;
  enum keytype_e type;
  uint8_t *key;
  unsigned int keylen;
  uint8_t ver;



  if(k == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): k=%p", __FILE__, __LINE__, k);
    return -1;
  }

  result = key_getraw(l, idx, &type, &key, &keylen, &ver, keystr);
  if(result)
    return -1;

  switch(type)
  {
  case _DES_:    *k = mifare_desfire_des_key_new(key);                   break;
  case _3DES_:   *k = mifare_desfire_3des_key_new(key);                  break;
  case _3K3DES_: *k = mifare_desfire_3k3des_key_new(key);                break;
  case _AES_:    *k = mifare_desfire_aes_key_new_with_version(key, ver); break;
  }

  free(key);

  if(*k == NULL)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "internal error (%s:%d): cannot create key", __FILE__, __LINE__);
    return -1;
  }

  switch(type)
  {
  case _DES_:
  case _3DES_:
  case _3K3DES_:
    mifare_desfire_key_set_version(*k, ver);
    break;

  default:
    break;
  }


  return 0;
}


void key_push(lua_State *l, enum keytype_e type, uint8_t *key, unsigned int keylen, uint8_t ver)
{
  lua_checkstack(l, 2);
  lua_newtable(l);

  switch(type)
  {
  case _DES_:    lua_pushstring(l, "DES");    break;
  case _3DES_:   lua_pushstring(l, "3DES");   break;
  case _3K3DES_: lua_pushstring(l, "3K3DES"); break;
  case _AES_:    lua_pushstring(l, "AES");    break;
  }

  lua_setfield(l, -2, "t");

  buffer_push(l, key, keylen); lua_setfield(l, -2, "k");
  lua_pushinteger(l, ver);     lua_setfield(l, -2, "v");

  lua_checkstack(l, 2);
  lua_newtable(l);
  lua_getglobal(l, "key");
  lua_setfield(l, -2, "__index");
  lua_setmetatable(l, -2);
}




static int key_div_aes(lua_State *l,
  uint8_t *key, unsigned int keylen,
  uint8_t *uid, uint32_t *aid, uint8_t *kno,
  uint8_t *pad, unsigned int padlen,
  uint8_t **_divkey, unsigned int *_divkeylen)
{
  uint8_t magic[1];
  uint8_t aid_buf[3];
  uint8_t kno_buf[1];
  CMAC_CTX *ctx;
  unsigned int i;
  static uint8_t divkey[EVP_MAX_BLOCK_LENGTH];
  size_t divkeylen;


  magic[0] = 0x01;
  if(aid != NULL)
  {
    aid_buf[0] = (*aid >> 16) & 0xff;
    aid_buf[1] = (*aid >>  8) & 0xff;
    aid_buf[2] =  *aid        & 0xff;
  }
  if(kno != NULL)
    kno_buf[0] = *kno;

  ctx = CMAC_CTX_new();
  if(!CMAC_Init(ctx, key, keylen, EVP_aes_128_cbc(), NULL)) { goto fail; }
  if(!CMAC_Update(ctx, magic, 1))                           { goto fail; }
  for(i = 0; i < 2; i++)
  {
    if(!CMAC_Update(ctx, uid, 7))                           { goto fail; }
    if(aid != NULL)
      if(!CMAC_Update(ctx, aid_buf, 3))                     { goto fail; }
    if(kno != NULL)
      if(!CMAC_Update(ctx, kno_buf, 1))                     { goto fail; }
    if(pad != NULL)
      if(!CMAC_Update(ctx, pad, padlen))                    { goto fail; }
  }
  if(!CMAC_Final(ctx, divkey, &divkeylen))                  { goto fail; }
  CMAC_CTX_free(ctx);

  *_divkey    = divkey;
  *_divkeylen = divkeylen;


  return 0;


fail:;
  unsigned long err;

  lua_checkstack(l, 2);
  lua_pushstring(l, "Crypto error:\n");
  while((err = ERR_get_error()))
  {
    lua_pushfstring(l, "%s\n", ERR_error_string(err, NULL));
    lua_concat(l, 2);
  }

  return -1;
}




FN_ALIAS(key_create) = { "create", NULL };
FN_PARAM(key_create) =
{
  FNPARAM("key", "Key", 0),
  FNPARAMEND
};
FN_RET(key_create) =
{
  FNPARAM("key", "Key", 0),
  FNPARAMEND
};
FN("key", key_create, "Create key object", NULL);


static int key_create(lua_State *l)
{
  int result;
  unsigned int failidx;
  const char *failarg;
  enum keytype_e type;
  uint8_t *key;
  unsigned int keylen;
  uint8_t ver;



  key  = NULL;


  /* Schlüssel auslesen. */
  result = key_getraw(l, 1, &type, &key, &keylen, &ver, NULL);
  if(result)
  {
    failidx = 1;
    failarg = "key";
    goto fail;
  }

  /* Argumente verwerfen. */
  lua_settop(l, 0);

  key_push(l, type, key, keylen, ver);


  return lua_gettop(l);


fail:
  free(key);

  /* Kehrt nicht zurück. */
  desflua_argerror(l, failidx, failarg);

  /* Macht den Compiler glücklich. */
  return 0;
}



FN_ALIAS(key_div) = { "diversify", "div", NULL };
FN_PARAM(key_div) =
{
  FNPARAM("key", "Master Key",                  0),
  FNPARAM("uid", "PICC UID",                    0),
  FNPARAM("aid", "Application ID",              1),
  FNPARAM("kno", "Key number",                  1),
  FNPARAM("pad", "Padding, e.g. System String", 1),
  FNPARAMEND
};
FN_RET(key_div) =
{
  FNPARAM("key", "Diversified Key", 0),
  FNPARAMEND
};
FN("key", key_div, "Calculate diversified Key", NULL);


static int key_div(lua_State *l)
{
  int result;
  unsigned int failidx;
  const char *failarg;
  enum keytype_e type;
  uint8_t *key;
  unsigned int keylen;
  uint8_t ver;
  key_div_fn_t fn;
  uint8_t *uid, *pad;
  unsigned int uidlen, padlen;
  uint32_t aid, *paid;
  uint8_t kno, *pkno;
  uint8_t *divkey;
  unsigned int divkeylen;



  key  = NULL;
  uid  = NULL;
  paid = NULL;
  pkno = NULL;
  pad  = NULL;


  /* Schlüssel auslesen. */
  result = key_getraw(l, 1, &type, &key, &keylen, &ver, NULL);
  if(result)
  {
    failidx = 1;
    failarg = "key";
    goto fail;
  }


  /* Diversification-Funktion setzen. */
  switch(type)
  {
  case _AES_: fn = key_div_aes; break;

  default:
    free(key);
    luaL_argerror(l, 1, "key type not implemented");
    break;
  }


  /* UID auslesen. */
  result = buffer_get(l, 2, &uid, &uidlen);
  if(result)
  {
    failidx = 2;
    failarg = "uid";
    goto fail;
  }

  /* Länge der UID prüfen. */
  if(uidlen != 7)
  {
    lua_checkstack(l, 1);
    lua_pushfstring(l, "UID length %d invalid, expected 7 bytes", uidlen);
    failidx = 2;
    failarg = "uid";
    goto fail;
  }


  /* AID auslesen */
  if(lua_gettop(l) >= 3 && lua_isnumber(l, 3))
  {
    aid = (uint32_t)lua_tonumber(l, 3) & 0x00ffffff;
    paid = &aid;
  }

  /* KNO auslesen. */ 
  if(lua_gettop(l) >= 4 && lua_isnumber(l, 4))
  {
    kno = (uint8_t)lua_tonumber(l, 4) & 0x000000ff;
    pkno = &kno;
  }

  /* Wenn gegeben, Padding auslesen. */
  if(lua_gettop(l) >= 5 && !lua_isnil(l, 5))
  {
    result = buffer_get(l, 5, &pad, &padlen);
    if(result)
    {
      failidx = 5;
      failarg = "pad";
      goto fail;
    }
  }
  else
  {
    pad    = NULL;
    padlen = 0;
  }


  /* Argumente verwerfen. */
  lua_settop(l, 0);

  result = fn(l, key, keylen, uid, paid, pkno, pad, padlen, &divkey, &divkeylen);

  free(key);
  free(uid);
  free(pad);

  if(result)
    luaL_error(l, "Key generation error: %s", lua_tostring(l, -1));

  key_push(l, type, divkey, divkeylen, ver);


  return lua_gettop(l);


fail:
  free(key);
  free(uid);
  free(pad);

  /* Kehrt nicht zurück. */
  desflua_argerror(l, failidx, failarg);

  /* Macht den Compiler glücklich. */
  return 0;
}
