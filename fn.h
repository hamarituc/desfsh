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

#ifndef _DESF_FN_H_
#define _DESF_FN_H_

#include <lua.h>


#define FN_ID(n)	__ ## n ## __
#define FN_ALIAS_ID(n)	__ ## n ## _ ## alias ## __
#define FN_PARAM_ID(n)	__ ## n ## _ ## param ## __
#define FN_RET_ID(n)	__ ## n ## _ ## ret ## __

#define FN(c, n, b, d) \
  struct fn_t FN_ID(n) = \
  { \
    .fn    = n, \
    .class = c, \
    .alias = FN_ALIAS_ID(n), \
    .brief = b, \
    .desc  = d, \
    .param = FN_PARAM_ID(n), \
    .ret   = FN_RET_ID(n), \
  }

#define FN_ALIAS(n)	static const char * FN_ALIAS_ID(n) []
#define FN_PARAM(n)	static const struct fn_param_t FN_PARAM_ID(n) []
#define FN_RET(n)	static const struct fn_param_t FN_RET_ID(n) []

#define FNPARAM(n, d, o)	{ .name = n, .desc = d, .opt = o }
#define FNPARAMEND		FNPARAM(NULL, NULL, 0)

#define FNDECL(n)	struct fn_t FN_ID(n)
#define FNREF(n)	(&FN_ID(n))


struct fn_param_t
{
  const char *name;
  const char *desc;
  const unsigned char opt:1;
};

struct fn_t
{
  lua_CFunction fn;
  const char *class;
  const char **alias;
  const char *brief;
  const char *desc;
  const struct fn_param_t *param;
  const struct fn_param_t *ret;
};


extern int fn_init(lua_State *l, int online);


#endif
