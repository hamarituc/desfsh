SOURCE	:= $(sort $(wildcard *.c))
OBJS	:= $(patsubst %.c, %.o, $(SOURCE))
BIN	:= desfsh
CC	:= gcc
CFLAGS	:= -Wall -Wextra -g -I/usr/include/lua5.1
LDFLAGS	:= -lnfc -lfreefare -lreadline -llua5.1 -lcrypto -lz


default: all

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

fn.o: luainit.inc

luainit.inc: luainit.lua
	echo -n -e "\0" | cat $< - | xxd -i > $@

clean:
	rm -f *.o *.inc
	rm -f $(BIN)
