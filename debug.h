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

#ifndef _DESF_DEBUG_H_
#define _DESF_DEBUG_H_

#include "fn.h"


#define DEBUG_STAT	0x01
#define DEBUG_IN	0x02
#define DEBUG_OUT	0x04
#define DEBUG_INFO	0x08

#define DEBUG_BLACK	0
#define DEBUG_RED	1
#define DEBUG_GREEN	2
#define DEBUG_YELLOW	3
#define DEBUG_BLUE	4
#define DEBUG_MAGENTA	5
#define DEBUG_CYAN	6
#define DEBUG_WHITE	7
#define DEBUG_NORMAL	0
#define DEBUG_BOLD	1


extern FNDECL(debug);
extern void debug_gen(unsigned char dir, const char *label, const char *fmt, ...);
extern void debug_cmd(const char *name);
extern void debug_info(const char *fmt, ...);
extern void debug_result(uint8_t err, const char *str);
extern void debug_keysettings(unsigned char dir, uint8_t settings);
extern void debug_comm(unsigned char dir, uint8_t comm);
extern void debug_acl(unsigned char dir, uint16_t acl);
extern void debug_buffer(unsigned char dir, uint8_t *buf, unsigned int len, unsigned int offset);


#endif
