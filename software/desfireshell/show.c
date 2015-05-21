#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <lua.h>
#include <lauxlib.h>
#include <freefare.h>

#include "cmd.h"
#include "desfsh.h"



static void show_handle_error(MifareTag tag, const char *fmt, ...)
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


int show_picc(__attribute__((unused)) lua_State *l)
{
  int result;
  struct mifare_desfire_version_info info;
  unsigned int e1, e2, s1, s2, u1, u2;
  static const char units[] = { ' ', 'K', 'M', 'G', 'T' };


  result = mifare_desfire_get_version(tag, &info);
  if(result)
  {
    show_handle_error(tag, "GetVersion()");
    return 0;
  }

  printf("       VEND  TYPE    VER     PROT  SIZE\n");


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
    printf(".. %d%c", s2, units[u2]);
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


  printf("  UID: 0x%02x%02x%02x%02x%02x%02x%02x\n",
    info.uid[0], info.uid[1], info.uid[2], info.uid[3], info.uid[4], info.uid[5], info.uid[6]);
  printf("BATCH: 0x%02x%02x%02x%02x%02x\n",
    info.batch_number[0], info.batch_number[1], info.batch_number[2], info.batch_number[3], info.batch_number[4]);
  printf(" PROD: %02x/%02x\n",
    info.production_week, info.production_year);


  return 0;
}


int show_apps(__attribute__((unused)) lua_State *l)
{
  int result;
  MifareDESFireAID *apps;
  size_t len, i;
  uint32_t aid;
  MifareDESFireAID piccapp;

  static const char *akc[] =
  {
    "AMK ", "K01 ", "K02 ", "K03 ",
    "K04 ", "K05 ", "K06 ", "K07 ",
    "K08 ", "K09 ", "K10 ", "K11 ",
    "K12 ", "K13 ", "SELF", "----",
  };


  result = mifare_desfire_get_application_ids(tag, &apps, &len);
  if(result)
  {
    show_handle_error(tag, "GetApplicationIDs()");
    return 0;
  }

  printf("AID        AKC   SETC  APP   FILE  AMKC  KEYS\n");

  for(i = 0; i < len; i++)
  {
    uint8_t settings, maxkeys;

    
    aid = mifare_desfire_aid_get_aid(apps[i]);

    result = mifare_desfire_select_application(tag, apps[i]);
    if(result)
    {
      show_handle_error(tag, "SelectApplication(0x%06x)", aid);
      continue;
    }

    result = mifare_desfire_get_key_settings(tag, &settings, &maxkeys);
    if(result)
    {
      show_handle_error(tag, "GetKeySettings()", aid);
      continue;
    }


    printf("0x%06x : %s  %s  %s  %s  %s   %2d\n",
      aid,
      akc[(settings >> 4) & 0x0f],
      (settings & 0x08) ? "--- " : "AMK ",
      (settings & 0x04) ? "AMK " : "*** ",
      (settings & 0x02) ? "AMK " : "*** ",
      (settings & 0x01) ? "--- " : "AMK ",
      maxkeys);
  }
  mifare_desfire_free_application_ids(apps);

  piccapp = mifare_desfire_aid_new(0);
  result = mifare_desfire_select_application(tag, piccapp);
  if(result)
    show_handle_error(tag, "SelectApplication(0x000000)");
  printf("Application 0x000000 selected.\n");
  free(piccapp);


  return 0;
}


int show_files(__attribute__((unused)) lua_State *l)
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
  if(result)
  {
    show_handle_error(tag, "GetFileIDs()");
    return 0;
  }

  printf("ID   TYP  COMM    RD  WR  RW  CA\n");

  for(i = 0; i < len; i++)
  {
    struct mifare_desfire_file_settings settings;


    result = mifare_desfire_get_file_settings(tag, fids[i], &settings);
    if(result)
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

    printf("[%s][%s][%s][%s]",
      keystr[MDAR_READ(settings.access_rights)       & 0x0f],
      keystr[MDAR_WRITE(settings.access_rights)      & 0x0f],
      keystr[MDAR_READ_WRITE(settings.access_rights) & 0x0f],
      keystr[MDAR_CHANGE_AR(settings.access_rights)  & 0x0f]);

    switch(settings.file_type)
    {
    case MDFT_STANDARD_DATA_FILE:
    case MDFT_BACKUP_DATA_FILE:
      printf("  %3d bytes", settings.settings.standard_file.file_size);
      break;

    case MDFT_VALUE_FILE_WITH_BACKUP:
      printf("  LO: %d  UP: %d",
        settings.settings.value_file.lower_limit,
        settings.settings.value_file.upper_limit);
      if(settings.settings.value_file.limited_credit_enabled)
      {
        printf("  LC: %d",
          settings.settings.value_file.limited_credit_value);
      }
      break;

    case MDFT_LINEAR_RECORD_FILE_WITH_BACKUP:
    case MDFT_CYCLIC_RECORD_FILE_WITH_BACKUP:
      printf("  %3d bytes  %3d/%3d recs",
        settings.settings.linear_record_file.record_size,
        settings.settings.linear_record_file.current_number_of_records,
        settings.settings.linear_record_file.max_number_of_records);
      break;
    }

    printf("\n");
  }
  free(fids);


  return 0;
}
