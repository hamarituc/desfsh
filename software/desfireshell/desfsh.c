#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <nfc/nfc.h>
#include <freefare.h>

#include "desfsh.h"
#include "shell.h"


#define MAXDEVS		16


static const char *devstr = NULL;
static const char *tagstr = NULL;
static int devnr = -1;
static int tagnr = -1;


FreefareTag tag = NULL;



static int parse(int argc, char *argv[])
{
  static struct option longopts[] =
  {
    { .name = "device",     .has_arg = 1, .flag = NULL, .val = 'd' },
    { .name = "tag",        .has_arg = 1, .flag = NULL, .val = 't' },
    { .name = "devicename", .has_arg = 1, .flag = NULL, .val = 'D' },
    { .name = "tagname",    .has_arg = 1, .flag = NULL, .val = 'T' },
  };


  while(1)
  {
    int c;

    c = getopt_long(argc, argv, "d:t:", longopts, NULL);
    if(c == -1)
      break;

    switch(c)
    {
    case 'D': devstr = optarg;       devnr  = -1;   break;
    case 'T': tagstr = optarg;       devnr  = -1;   break;
    case 'd': devnr  = atoi(optarg); devstr = NULL; break;
    case 't': tagnr  = atoi(optarg); tagstr = NULL; break;
    case '?': break;
    default:  return -1;
    }
  }

  return 0;
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
  unsigned int i;


  result = parse(argc, argv);
  if(result)
  {
    fprintf(stderr, "Failure parsing arguments. Exiting.\n");
    return -1;
  }


  nfc_init(&ctx);
  if(devstr == NULL && devnr < 0)
  {
    show_devs(ctx);
    goto end_exit;
  }

  dev = nfc_open(ctx, devstr);
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

    if((tagnr > 0 && (unsigned int)tagnr == i) || \
       (tagstr != NULL && !strcmp(uidstr, tagstr)))
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
  shell();
  mifare_desfire_disconnect(tag);

end_free:
  freefare_free_tags(tags);

end_close:
  nfc_close(dev);

end_exit:
  nfc_exit(ctx);


  return 0;
}
