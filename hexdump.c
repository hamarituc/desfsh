#include <stdint.h>
#include <stdio.h>

#include "hexdump.h"



char *hexdump_line(uint8_t *buffer, unsigned int len, unsigned int offset)
{
  static char line[80];
  unsigned int col;
  char *linepos;


  if(len > 8)
    len = 8;

  linepos = line;
  linepos += sprintf(linepos, "%08x ", offset);

  for(col = 0; col < 8; col++)
  {
    linepos += sprintf(linepos, col != 4 ? " " : "  ");
    if(col < len)
      linepos += sprintf(linepos, "%02x", buffer[col]);
    else
      linepos += sprintf(linepos, "  ");
  }

  linepos += sprintf(linepos, "  |");

  for(col = 0; col < 8; col++)
  {
    if(col < len)
    {
      uint8_t c = buffer[col];
      linepos += sprintf(linepos, "%c",
        c >= 0x20 && c <= 0x7f ? c : '.');
    }
    else
      linepos += sprintf(linepos, " ");
  }

  linepos += sprintf(linepos, "|");


  return line;
}
