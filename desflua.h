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

#ifndef _DESF_LUA_H_
#define _DESF_LUA_H_

#include <lua.h>
#include <freefare.h>


extern int desflua_get_comm(lua_State *l, int idx, uint8_t *comm);
extern int desflua_get_acl(lua_State *l, int idx, uint16_t *acl);
extern void desflua_push_acl(lua_State *l, uint16_t acl);
extern void desflua_handle_result(lua_State *l, int result, MifareTag tag);
extern void desflua_argerror(lua_State *l, int argnr, const char *prefix);
extern void desflua_shell();

#endif
