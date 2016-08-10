#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <nfc/nfc.h>
#include <freefare.h>
#include <openssl/evp.h>

#include "desfsh.h"
#include "shell.h"


#define MAXDEVS		16


static int help = 0;
static const char *devstr = NULL;
static const char *tagstr = NULL;
static int devnr = -1;
static int tagnr = -1;
static int online = 1;
static int interactive = 0;
static const char *command = NULL;


FreefareTag tag = NULL;



static int parse(int argc, char *argv[])
{
  static struct option longopts[] =
  {
    { .name = "help",        .has_arg = 0, .flag = NULL, .val = 'h' },
    { .name = "device",      .has_arg = 1, .flag = NULL, .val = 'd' },
    { .name = "tag",         .has_arg = 1, .flag = NULL, .val = 't' },
    { .name = "devicename",  .has_arg = 1, .flag = NULL, .val = 'D' },
    { .name = "tagname",     .has_arg = 1, .flag = NULL, .val = 'T' },
    { .name = "offline",     .has_arg = 0, .flag = NULL, .val = 'o' },
    { .name = "interactive", .has_arg = 0, .flag = NULL, .val = 'i' },
    { .name = "command",     .has_arg = 1, .flag = NULL, .val = 'c' },
  };


  while(1)
  {
    int c;

    c = getopt_long(argc, argv, "hd:t:D:T:oic:", longopts, NULL);
    if(c == -1)
      break;

    switch(c)
    {
    case 'h': help = 1;                             break;
    case 'D': devstr = optarg;       devnr  = -1;   break;
    case 'T': tagstr = optarg;       devnr  = -1;   break;
    case 'd': devnr  = atoi(optarg); devstr = NULL; break;
    case 't': tagnr  = atoi(optarg); tagstr = NULL; break;
    case 'o': online = 0;                           break;
    case 'i': interactive = 1;                      break;
    case 'c': command = optarg;                     break;
    case '?': break;
    default:  return -1;
    }
  }

  if(command == NULL)
    interactive = 1;

  return 0;
}


static void show_help(const char *name)
{
  printf("Usage: %s [OPTIONS]\n", name);
  printf("\n");
  printf("Without options all present NFC devices and tags are shown.\n");
  printf("\n");
  printf("Options:\n");
  printf("  -h               Show this help text.\n");
  printf("  -D <devstring>   NFC device to connect\n");
  printf("  -T <tagstring>   NFC tag (UID) to connect\n");
  printf("  -d <devnumber>   NFC device number to connect (can be used instead of -D)\n");
  printf("  -t <tagnumber>   NFC tag number to connect (can be used instead of -T)\n");
  printf("  -o               Offline Mode. Don't connect to any NFC device.\n");
  printf("  -i               Enter interactive shell mode.\n");
  printf("                   Default if no command specified via -c.\n");
  printf("  -c <command>     Execute the specified command.\n");
  printf("\n");
}


static void show_tags(nfc_device *dev, const char *indent)
{
  FreefareTag *tags;
  unsigned int i;


  if(indent == NULL)
    indent = "";

  tags = freefare_get_tags(dev);
  for(i = 0; tags[i] != NULL; i++)
  {
    FreefareTag tag;

    tag = tags[i];
    printf("%s%2d: %s --> %s\n", indent, i,
        freefare_get_tag_friendly_name(tag),
        freefare_get_tag_uid(tag));
  }
  freefare_free_tags(tags);
}


static void show_devs(nfc_context *ctx)
{
  nfc_connstring connstr[MAXDEVS];
  unsigned int n, i;


  n = nfc_list_devices(ctx, connstr, MAXDEVS);
  if(n == 0)
  {
    printf("No devices found.\n");
    return;
  }

  printf("%d devices found:\n", n);
  for(i = 0; i < n; i++)
  {
    nfc_device *dev;

    printf("%2d: %s\n", i, connstr[i]);
    dev = nfc_open(ctx, connstr[i]);
    show_tags(dev, "  ");
    nfc_close(dev);
  }
}


int main(int argc, char *argv[])
{
  int result;
  nfc_context *ctx;
  nfc_device *dev;
  FreefareTag *tags;


  result = parse(argc, argv);
  if(result)
  {
    fprintf(stderr, "Failure parsing arguments. Exiting.\n");
    return -1;
  }

  if(help)
  {
    show_help(argv[0]);
    return 0;
  }

  OpenSSL_add_all_algorithms();

  if(online)
  {
    nfc_connstring connstr[MAXDEVS];
    int i, n;

    nfc_init(&ctx);
    if(devstr == NULL && devnr < 0)
    {
      show_devs(ctx);
      goto end_exit;
    }

    if(devstr == NULL)
    {
      n = nfc_list_devices(ctx, connstr, MAXDEVS);
      if(devnr >= n)
      {
        printf("Device number %d invalid. Only %d devices present.\n", devnr, n);
        goto end_exit;
      }
      devstr = connstr[devnr];
    }

    dev = nfc_open(ctx, devstr);
    if(dev == NULL)
    {
      printf("Unable to open device.\n");
      goto end_exit;
    }

    if(tagstr == NULL && tagnr < 0)
    {
      show_tags(dev, NULL);
      goto end_close;
    }

    tags = freefare_get_tags(dev);
    tag = NULL;
    for(i = 0; tags[i] != NULL; i++)
    {
      const char *uidstr;

      tag = tags[i];
      uidstr = freefare_get_tag_uid(tag);

      if((tagnr > 0 && tagnr == i) || \
         (tagstr != NULL && !strcasecmp(uidstr, tagstr)))
        break;
    }

    if(tag == NULL)
    {
      fprintf(stderr, "Tag not found.\n");
      goto end_free;
    }

    if(freefare_get_tag_type(tag) != MIFARE_DESFIRE)
    {
      fprintf(stderr, "Tag is not a DESFire card.\n");
      goto end_free;
    }

    mifare_desfire_connect(tag);
  }

  shell(online, interactive, command);

  if(online)
    mifare_desfire_disconnect(tag);

end_free:
  if(online)
    freefare_free_tags(tags);

end_close:
  if(online)
    nfc_close(dev);

end_exit:
  if(online)
    nfc_exit(ctx);

  EVP_cleanup();


  return 0;
}
