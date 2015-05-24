#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "desflua.h"
#include "desfsh.h"



static int show_picc(lua_State *l);
static int show_apps(lua_State *l);
static int show_files(lua_State *l);



static void show_handle_error(FreefareTag tag, const char *fmt, ...)
{
  va_list args;


  printf("%3d: '%s'", mifare_desfire_last_picc_error(tag), freefare_strerror(tag));

  if(fmt != NULL)
  {
    printf(" during ");
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }

  printf("\n");
}




FN_ALIAS(show_picc) = { "picc", NULL };
FN_PARAM(show_picc) =
{
  FNPARAM("key", "PICC Master Key", 1),
  FNPARAMEND
};
FN_RET(show_picc) =
{
  FNPARAMEND
};
FN("show", show_picc, "Show PICC Information",
"When <key> is specified, it will be used for authentication. The commands\n" \
"selects application 0x000000 before performing any other actions, so a\n" \
"previous authentication becomes invalid.\n");


static int show_picc(lua_State *l)
{
  int result;
  unsigned char haskey;
  MifareDESFireKey pmk;
  MifareDESFireAID piccapp;



  /* Sollte ein PMK angegeben sein, lesen wir ihn aus. */
  haskey = lua_gettop(l) >= 1 && !lua_isnil(l, 1);
  if(haskey)
  {
    result = desflua_get_key(l, 1, &pmk, NULL);
    if(result)
    {
      lua_checkstack(l, 1);
      lua_pushfstring(l, "PICC key: %s", lua_tostring(l, -1));
      lua_remove(l, -2);
      return luaL_argerror(l, 1, lua_tostring(l, -1));
    }
  }

  /*
   * Master-APP auswählen. Wir benötigen Sie, um am Ende die
   * PICC-Schlüsseleinstellungen auslesen zu können.
   */
  piccapp = mifare_desfire_aid_new(0);
  result = mifare_desfire_select_application(tag, piccapp);
  if(result < 0)
    show_handle_error(tag, "SelectApplication(0x000000)");
  free(piccapp);

  /* Wenn wir einen Schlüssel haben, authentifizieren wir uns. */
  if(haskey)
  {
    result = mifare_desfire_authenticate(tag, 0, pmk);
    if(result < 0)
    {
      show_handle_error(tag, "Authenticate(0)");
      haskey = 0;
    }
  }



  /*
   * Versionsinformationen ausgeben.
   */

  struct mifare_desfire_version_info info;
  unsigned int e1, e2, s1, s2, u1, u2;
  static const char units[] = { ' ', 'K', 'M', 'G', 'T' };


  result = mifare_desfire_get_version(tag, &info);
  if(result < 0)
  {
    show_handle_error(tag, "GetVersion()");
    goto fail_version;
  }

  printf("\n");
  printf("       VEND  TYPE    VER     PROT  SIZE\n");
  printf("-----------------------------------------------\n");


  printf("   HW: 0x%02x  0x%02x%02x  0x%02x%02x  0x%02x  ",
    info.hardware.vendor_id,
    info.hardware.type, info.hardware.subtype,
    info.hardware.version_major, info.hardware.version_minor,
    info.hardware.protocol);

  e1 = (info.hardware.storage_size >> 1);
  e2 = (info.hardware.storage_size >> 1) + 1;
  u1 = 0;
  u2 = 0;
  while(u1 < 5 && e1 >= 10) { u1++; e1 -= 10; }
  while(u2 < 5 && e2 >= 10) { u2++; e2 -= 10; }
  s1 = 1 << e1;
  s2 = 1 << e2;

  printf("%d%c", s1, units[u1]);
  if(info.hardware.storage_size & 0x01)
    printf(" .. %d%c", s2, units[u2]);
  printf("\n");


  printf("   SW: 0x%02x  0x%02x%02x  0x%02x%02x  0x%02x  ",
    info.software.vendor_id,
    info.software.type, info.software.subtype,
    info.software.version_major, info.software.version_minor,
    info.software.protocol);

  e1 = (info.software.storage_size >> 1);
  e2 = (info.software.storage_size >> 1) + 1;
  u1 = 0;
  u2 = 0;
  while(u1 < 5 && e1 >= 10) { u1++; e1 -= 10; }
  while(u2 < 5 && e2 >= 10) { u2++; e2 -= 10; }
  s1 = 1 << e1;
  s2 = 1 << e2;

  printf("%d%c", s1, units[u1]);
  if(info.software.storage_size & 0x01)
    printf(".. %d%c", s2, units[u2]);
  printf("\n");


  printf("\n");
  printf("  UID: 0x%02x%02x%02x%02x%02x%02x%02x\n",
    info.uid[0], info.uid[1], info.uid[2], info.uid[3], info.uid[4], info.uid[5], info.uid[6]);
  printf("BATCH: 0x%02x%02x%02x%02x%02x\n",
    info.batch_number[0], info.batch_number[1], info.batch_number[2], info.batch_number[3], info.batch_number[4]);
  printf(" PROD: %02x/%02x\n",
    info.production_week, info.production_year);
  printf("\n");

fail_version:;



  /*
   * Freien Speicher und tatsächliche UID ausgeben. Für letzteres ist eine
   * Authentifikation erforderlich.
   */

  uint32_t freemem;
  char *cuid;

  result = mifare_desfire_free_mem(tag, &freemem);
  if(result < 0)
    show_handle_error(tag, "FreeMem()");
  else
    printf(" FREE: %d\n", freemem);

  result = mifare_desfire_get_card_uid(tag, &cuid);
  if(result < 0)
  {
    uint8_t err;
    
    err = mifare_desfire_last_picc_error(tag);
    if(err == AUTHENTICATION_ERROR)
      printf(" CUID: ??? (authentication required)\n");
    else
      show_handle_error(tag, "GetCardUID()");
  }
  else
    printf(" CUID: 0x%s\n", cuid);

  printf("\n");



  /* Schlüsseleinstellungen ausgeben. */

  uint8_t settings, maxkeys;


  result = mifare_desfire_get_key_settings(tag, &settings, &maxkeys);
  if(result < 0)
    show_handle_error(tag, "GetKeySettings()");
  else
  {
    printf("CONF  CAPP  DAPP  LIST  PMKC  KEYS\n");
    printf("----------------------------------\n");
    printf("%s  %s  %s  %s  %s   %2d\n",
      (settings & 0x08) ? "PMK " : "--- ",
      (settings & 0x04) ? "*** " : "PMK ",
      (settings & 0x04) ? "?MK " : "PMK ",
      (settings & 0x02) ? "*** " : "PMK ",
      (settings & 0x01) ? "PMK " : "--- ",
      maxkeys);

    printf("\n");
  }



  /* Info zu Auth.-Status ausgeben. */
  printf("Application 0x000000 selected.\n");
  if(haskey)
    printf("Authentificated with PICC master key.\n");
  else
    printf("Unauthenticated.\n");
  printf("\n");


  return 0;
}




FN_ALIAS(show_apps) = { "apps", NULL };
FN_PARAM(show_apps) =
{
  FNPARAM("key",     "PICC Master Key",                 1),
  FNPARAM("keylist", "List of Application Master Keys", 1),
  FNPARAMEND
};
FN_RET(show_apps) =
{
  FNPARAMEND
};
FN("show", show_apps, "Show Application Information",
"When <key> is specified, it will be used for authentication to application\n" \
"0x000000. <keylist> is a table where each index specifies an application id\n" \
"and the corresponding value will be used as application master key. When no\n" \
"application master key is given, the application settings are read out\n" \
"unauthenticated.\n");


static int show_apps(lua_State *l)
{
  int result;
  unsigned char haspmk;
  MifareDESFireKey pmk;
  MifareDESFireAID piccapp;
  MifareDESFireAID *apps;
  size_t len, i;

  static const char *akc[] =
  {
    "AMK ", "K01 ", "K02 ", "K03 ",
    "K04 ", "K05 ", "K06 ", "K07 ",
    "K08 ", "K09 ", "K10 ", "K11 ",
    "K12 ", "K13 ", "SELF", "----",
  };



  /* Sollte ein PMK angegeben sein, lesen wir ihn aus. */
  haspmk = lua_gettop(l) >= 1 && !lua_isnil(l, 1);
  if(haspmk)
  {
    result = desflua_get_key(l, 1, &pmk, NULL);
    if(result)
    {
      lua_checkstack(l, 1);
      lua_pushfstring(l, "PICC key: %s", lua_tostring(l, -1));
      lua_remove(l, -2);
      return luaL_argerror(l, 1, lua_tostring(l, -1));
    }
  }

  luaL_argcheck(l, lua_gettop(l) < 2 || lua_isnil(l, 2) || lua_istable(l, 2), 2,
    "application master keys must be stored inside a table");


  /*
   * Master-APP auswählen. Wir benötigen Sie ggf., um die gespeicherten APPs
   * auflisten zu können.
   */
  piccapp = mifare_desfire_aid_new(0);
  result = mifare_desfire_select_application(tag, piccapp);
  if(result < 0)
    show_handle_error(tag, "SelectApplication(0x000000)");


  /* Wenn wir einen Schlüssel haben, authentifizieren wir uns. */
  if(haspmk)
  {
    result = mifare_desfire_authenticate(tag, 0, pmk);
    if(result < 0)
    {
      show_handle_error(tag, "Authenticate(0)");
      haspmk = 0;
    }
  }


  /* APPs abfragen. */
  result = mifare_desfire_get_application_ids(tag, &apps, &len);
  if(result < 0)
  {
    show_handle_error(tag, "GetApplicationIDs()");
    return 0;
  }


  /* APPs auflisten. */
  printf("\n");
  printf("AID        AKC   CONF  FILE  LIST  AMKC  KEYS\n");
  printf("---------------------------------------------\n");

  lua_checkstack(l, 1);

  for(i = 0; i < len; i++)
  {
    uint32_t aid;
    uint8_t err;
    MifareDESFireKey amk;
    uint8_t settings, maxkeys;


    /* ID auflisten. */
    aid = mifare_desfire_aid_get_aid(apps[i]);
    printf("0x%06x : ", aid);

    /* APP auswählen. */
    result = mifare_desfire_select_application(tag, apps[i]);
    if(result < 0)
    {
      show_handle_error(tag, "SelectApplication(0x%06x)", aid);
      continue;
    }

    /* Auf gut Glück versuchen, die Einstellungen auszulesen. */
    result = mifare_desfire_get_key_settings(tag, &settings, &maxkeys);
    if(result < 0)
    {
      err = mifare_desfire_last_picc_error(tag);
      
      if(err != AUTHENTICATION_ERROR)
      {
        show_handle_error(tag, "GetKeySettings()");
        continue;
      }
    }
    else
      goto skip_amk;


    /*
     * Wir müssen uns zunächst authentifizieren.
     */

    /* AMK laden. */
    if(lua_gettop(l) < 2 || lua_isnil(l, 2))
    {
      printf("authentication required\n");
      continue;
    }

    lua_pushinteger(l, aid);
    lua_gettable(l, -2);
    if(lua_isnil(l, -1))
    {
      lua_pop(l, 1);
      printf("authentication required\n");
      continue;
    }

    result = desflua_get_key(l, -1, &amk, NULL);
    if(result < 0)
    {
      printf("APP Master Key invalid: %s\n", lua_tostring(l, -1));
      lua_pop(l, 2);
      continue;
    }

    /* Authentifizierung vornehmen. */
    result = mifare_desfire_authenticate(tag, 0, amk);
    if(result < 0)
    {
      show_handle_error(tag, "Authenticate(0)");
      continue;
    }

skip_amk:

    /* Schlüsseleinstellungen authentifiziert auslesen. */
    result = mifare_desfire_get_key_settings(tag, &settings, &maxkeys);
    if(result < 0)
    {
      show_handle_error(tag, "GetKeySettings()");
      continue;
    }

    /* Einstellungen ausgeben. */
    printf("%s  %s  %s  %s  %s   %2d\n",
      akc[(settings >> 4) & 0x0f],
      (settings & 0x08) ? "AMK " : "--- ",
      (settings & 0x04) ? "*** " : "AMK ",
      (settings & 0x02) ? "*** " : "AMK ",
      (settings & 0x01) ? "AMK " : "--- ",
      maxkeys);
  }
  mifare_desfire_free_application_ids(apps);
  printf("\n");


  result = mifare_desfire_select_application(tag, piccapp);
  if(result < 0)
    show_handle_error(tag, "SelectApplication(0x000000)");
  else
    printf("Application 0x000000 selected.\nUnauthenticated.\n");
  free(piccapp);
  printf("\n");


  return 0;
}




FN_ALIAS(show_files) = { "files", NULL };
FN_PARAM(show_files) =
{
  FNPARAMEND
};
FN_RET(show_files) =
{
  FNPARAMEND
};
FN("show", show_files, "Show Files of an Application",
"The command preserves the current authentication status.\n");


static int show_files(__attribute__((unused)) lua_State *l)
{
  int result;
  uint8_t *fids;
  size_t len;
  unsigned int i;

  static const char *keystr[] =
  {
    "00", "01", "02", "03",
    "04", "05", "06", "07",
    "08", "09", "10", "11",
    "12", "13", "**", "--",
  };



  result = mifare_desfire_get_file_ids(tag, &fids, &len);
  if(result < 0)
  {
    show_handle_error(tag, "GetFileIDs()");
    return 0;
  }

  printf("\n");
  printf("ID   TYP  COMM    RD  WR  RW  CA\n");
  printf("----------------------------------------------------------------------\n");

  for(i = 0; i < len; i++)
  {
    struct mifare_desfire_file_settings settings;


    result = mifare_desfire_get_file_settings(tag, fids[i], &settings);
    if(result < 0)
    {
      show_handle_error(tag, "GetFileSettings(%d)", fids[i]);
      continue;
    }


    printf("%2d : ", fids[i]);

    switch(settings.file_type)
    {
    case MDFT_STANDARD_DATA_FILE:             printf("SDF  "); break;
    case MDFT_BACKUP_DATA_FILE:               printf("BDF  "); break;
    case MDFT_VALUE_FILE_WITH_BACKUP:         printf("VF   "); break;
    case MDFT_LINEAR_RECORD_FILE_WITH_BACKUP: printf("LRF  "); break;
    case MDFT_CYCLIC_RECORD_FILE_WITH_BACKUP: printf("CRF  "); break;
    }

    switch(settings.communication_settings)
    {
    case MDCM_PLAIN:      printf("PLAIN  "); break;
    case MDCM_MACED:      printf("MAC    "); break;
    case MDCM_ENCIPHERED: printf("CRYPT  "); break;
    }

    printf("[%s][%s][%s][%s]  ",
      keystr[MDAR_READ(settings.access_rights)       & 0x0f],
      keystr[MDAR_WRITE(settings.access_rights)      & 0x0f],
      keystr[MDAR_READ_WRITE(settings.access_rights) & 0x0f],
      keystr[MDAR_CHANGE_AR(settings.access_rights)  & 0x0f]);

    switch(settings.file_type)
    {
    case MDFT_STANDARD_DATA_FILE:
    case MDFT_BACKUP_DATA_FILE:
      printf("%d bytes", settings.settings.standard_file.file_size);
      break;

    case MDFT_VALUE_FILE_WITH_BACKUP:
      printf("%d .. %d",
        settings.settings.value_file.lower_limit,
        settings.settings.value_file.upper_limit);
      if(settings.settings.value_file.limited_credit_enabled)
      {
        printf(", limit: %d",
          settings.settings.value_file.limited_credit_value);
      }
      break;

    case MDFT_LINEAR_RECORD_FILE_WITH_BACKUP:
    case MDFT_CYCLIC_RECORD_FILE_WITH_BACKUP:
      printf("%d bytes/rec, %d/%d recs used",
        settings.settings.linear_record_file.record_size,
        settings.settings.linear_record_file.current_number_of_records,
        settings.settings.linear_record_file.max_number_of_records);
      break;
    }

    printf("\n");
  }
  free(fids);
  printf("\n");


  return 0;
}
