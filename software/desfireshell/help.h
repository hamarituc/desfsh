#ifndef _DESF_HELP_H_
#define _DESF_HELP_H_

#include <lua.h>

#include "fn.h"


extern void help_regfn(lua_State *l, const struct fn_t *fn);
extern void help_regtopic(lua_State *l, const char *topic, const char *brief, const char *desc);
extern FNDECL(help);
extern void help_init(lua_State *l);


#endif
