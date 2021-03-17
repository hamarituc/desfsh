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

#ifndef _DESF_KEY_H_
#define _DESF_KEY_H_

#include <lua.h>
#include <freefare.h>

#include "fn.h"


enum keytype_e { _DES_, _3DES_, _3K3DES_, _AES_ };

extern int key_gettype(lua_State *l, int idx, enum keytype_e *type, char **typestr);
extern int key_getraw(lua_State *l, int idx, enum keytype_e *type, uint8_t **key, unsigned int *keylen, uint8_t *_ver, char **keystr);
extern int key_get(lua_State *l, int idx, MifareDESFireKey *k, char **keystr);
extern void key_push(lua_State *l, enum keytype_e type, uint8_t *key, unsigned int keylen, uint8_t ver);

extern FNDECL(key_create);
extern FNDECL(key_div);

#endif
