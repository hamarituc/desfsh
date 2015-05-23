#ifndef _DESF_CMD_H_
#define _DESF_CMD_H_


/* SEC */
extern int cmd_auth(lua_State *l);
//extern int cmd_cks(lua_State *l);
extern int cmd_gks(lua_State *l);
extern int cmd_ck(lua_State *l);
extern int cmd_gkv(lua_State *l);

/* PICC */
extern int cmd_createapp(lua_State *l);
extern int cmd_deleteapp(lua_State *l);
extern int cmd_appids(lua_State *l);
extern int cmd_selapp(lua_State *l);
extern int cmd_format(lua_State *l);
extern int cmd_getver(lua_State *l);
extern int cmd_freemem(lua_State *l);
extern int cmd_carduid(lua_State *l);

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
// read
// write
// getval
// credit
// debit
// lcredit
// wrec
// rrec
// crec
// commit
// abort


/* show.c */
extern int show_picc(lua_State *l);
extern int show_apps(lua_State *l);
extern int show_files(lua_State *l);
// Hexdump File


//get_df_names
//free_mem
//set_default_key
//set_ats
//get_card_uid

#endif
