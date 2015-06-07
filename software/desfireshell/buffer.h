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
