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
extern FNDECL(cmd_fileids);
extern FNDECL(cmd_gfs);
extern FNDECL(cmd_cfs);
extern FNDECL(cmd_csdf);
extern FNDECL(cmd_cbdf);
extern FNDECL(cmd_cvf);
extern FNDECL(cmd_clrf);
extern FNDECL(cmd_ccrf);
extern FNDECL(cmd_delf);

/* DATA */
extern FNDECL(cmd_read);
extern FNDECL(cmd_write);
extern FNDECL(cmd_getval);
extern FNDECL(cmd_credit);
extern FNDECL(cmd_debit);
extern FNDECL(cmd_lcredit);
extern FNDECL(cmd_wrec);
extern FNDECL(cmd_rrec);
extern FNDECL(cmd_crec);
extern FNDECL(cmd_commit);
extern FNDECL(cmd_abort);

//get_df_names
//set_default_key
//set_ats

#endif
