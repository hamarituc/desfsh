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
