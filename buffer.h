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

#ifndef _DESF_BUFFER_H_
#define _DESF_BUFFER_H_

#include "fn.h"


extern int buffer_get(lua_State *l, int idx, uint8_t **buffer, unsigned int *len);
extern void buffer_push(lua_State *l, uint8_t *buffer, unsigned int len);

extern FNDECL(buffer_from_table);
extern FNDECL(buffer_from_hexstr);
extern FNDECL(buffer_from_ascii);
extern FNDECL(buffer_to_table);
extern FNDECL(buffer_to_hexstr);
extern FNDECL(buffer_to_ascii);
extern FNDECL(buffer_to_hexdump);
extern FNDECL(buffer_concat);


#endif
