#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <nfc/nfc.h>
#include <freefare.h>

#include "desfsh.h"


#define MAXDEVS		16


const char *devstr;
const char *tagstr;

MifareTag tag = NULL;



static int parse(int argc, char *argv[])
{
  static struct option longopts[] =
  {
    { .name = "device", .has_arg = 1, .flag = NULL, .val = 'd' },
    { .name = "tag",    .has_arg = 1, .flag = NULL, .val = 't' },
  };


  while(1)
  {
    int c;

    c = getopt_long(argc, argv, "d:t:", longopts, NULL);
    if(c == -1)
      break;

    switch(c)
    {
    case 'd': devstr = optarg; break;
    case 't': tagstr = optarg; break;
    case '?': break;
    default:  return -1;
    }
  }

  return 0;
}


static void show_tags(nfc_device *dev, const char *indent)
{
  MifareTag *tags;
  unsigned int i;


  if(indent == NULL)
    indent = "";

  tags = freefare_get_tags(dev);
  for(i = 0; tags[i] != NULL; i++)
  {
    MifareTag tag;

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
  MifareTag *tags;
  unsigned int i;


  result = parse(argc, argv);
  if(result)
  {
    fprintf(stderr, "Failure parsing arguments. Exiting.\n");
    return -1;
  }


  nfc_init(&ctx);
  if(devstr == NULL)
  {
    show_devs(ctx);
    goto end_exit;
  }

  dev = nfc_open(ctx, devstr);
  if(tagstr == NULL)
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

    if(!strcmp(uidstr, tagstr))
      break;
  }

  if(tag == NULL)
  {
    fprintf(stderr, "Tag '%s' not found.\n", tagstr);
    goto end_free;
  }

  if(freefare_get_tag_type(tag) != DESFIRE)
  {
    fprintf(stderr, "Tag '%s' is not a DESFire card.\n", tagstr);
    goto end_free;
  }

  desf_lua_shell();

end_free:
  freefare_free_tags(tags);

end_close:
  nfc_close(dev);

end_exit:
  nfc_exit(ctx);


  return 0;
}
