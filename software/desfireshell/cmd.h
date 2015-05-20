#ifndef _DESF_CMD_H_
#define _DESF_CMD_H_


/* SEC */
extern int cmd_auth(lua_State *l);
extern int cmd_cks(lua_State *l);
extern int cmd_gks(lua_State *l);
extern int cmd_gkv(lua_State *l);

/* PICC */
extern int cmd_createapp(lua_State *l);
extern int cmd_appids(lua_State *l);
extern int cmd_selapp(lua_State *l);
extern int cmd_getver(lua_State *l);


#endif
