#ifndef _DESF_DEBUG_H_
#define _DESF_DEBUG_H_

#include "fn.h"


#define DEBUG_STAT	0x01
#define DEBUG_IN	0x02
#define DEBUG_OUT	0x04

#define DEBUG_BLACK	0
#define DEBUG_RED	1
#define DEBUG_GREEN	2
#define DEBUG_YELLOW	3
#define DEBUG_BLUE	4
#define DEBUG_MAGENTA	5
#define DEBUG_CYAN	6
#define DEBUG_WHITE	7
#define DEBUG_NORMAL	0
#define DEBUG_BOLD	1


extern FNDECL(debug);
extern void debug_gen(unsigned char dir, const char *label, const char *fmt, ...);
extern void debug_result(uint8_t err, const char *str);
extern void debug_keysettings(unsigned char dir, uint8_t settings);
extern void debug_comm(unsigned char dir, uint8_t comm);
extern void debug_acl(unsigned char dir, uint16_t acl);
extern void debug_buffer(unsigned char dir, uint8_t *buf, unsigned int len, unsigned int offset);


#endif
