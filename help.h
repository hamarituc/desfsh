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

#ifndef _DESF_HELP_H_
#define _DESF_HELP_H_

#include <lua.h>

#include "fn.h"


extern void help_regfn(lua_State *l, const struct fn_t *fn);
extern void help_regtopic(lua_State *l, const char *topic, const char *brief, const char *desc);
extern FNDECL(help);
extern void help_init(lua_State *l);


#endif
