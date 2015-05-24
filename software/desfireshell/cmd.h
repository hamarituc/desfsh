#ifndef _DESF_CMD_H_
#define _DESF_CMD_H_

#include "fn.h"


/* SEC */
extern FNDECL(cmd_auth);
extern FNDECL(cmd_cks);
extern FNDECL(cmd_gks);
extern FNDECL(cmd_ck);
extern FNDECL(cmd_gkv);

/* PICC */
extern FNDECL(cmd_createapp);
extern FNDECL(cmd_deleteapp);
extern FNDECL(cmd_appids);
extern FNDECL(cmd_selapp);
extern FNDECL(cmd_format);
extern FNDECL(cmd_getver);
extern FNDECL(cmd_freemem);
extern FNDECL(cmd_carduid);

/* APP */
extern int cmd_fileids(lua_State *l);
extern int cmd_gfs(lua_State *l);
extern int cmd_cfs(lua_State *l);
extern int cmd_csdf(lua_State *l);
extern int cmd_cbdf(lua_State *l);
extern int cmd_cvf(lua_State *l);
extern int cmd_clrf(lua_State *l);
extern int cmd_ccrf(lua_State *l);
extern int cmd_delf(lua_State *l);

/* DATA */
extern int cmd_read(lua_State *l);
extern int cmd_write(lua_State *l);
extern int cmd_getval(lua_State *l);
extern int cmd_credit(lua_State *l);
extern int cmd_debit(lua_State *l);
extern int cmd_lcredit(lua_State *l);
extern int cmd_wrec(lua_State *l);
extern int cmd_rrec(lua_State *l);
extern int cmd_crec(lua_State *l);
extern int cmd_commit(lua_State *l);
extern int cmd_abort(lua_State *l);


/* show.c */
extern int show_picc(lua_State *l);
extern int show_apps(lua_State *l);
extern int show_files(lua_State *l);
// Hexdump File


//get_df_names
//set_default_key
//set_ats

#endif
