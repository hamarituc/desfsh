LUAPKG	:= \
	$(shell \
		for p in lua5.4 lua-5.4 lua54 \
		         lua5.3 lua-5.3 lua53 \
		         lua5.2 lua-5.2 lua52 \
		         lua5.1 lua-5.1 lua51 \
		         lua; \
		do \
			pkg-config --exists $$p && echo $$p && break; \
		done \
	)

SOURCE	:= $(sort $(wildcard *.c))
OBJS	:= $(patsubst %.c, %.o, $(SOURCE))
BIN	:= desfsh
CC	:= gcc
CFLAGS	:= -Wall -Wextra -g $(shell pkg-config $(LUAPKG) --cflags)
LDFLAGS	:= -lnfc -lfreefare -lreadline $(shell pkg-config $(LUAPKG) --libs) -lcrypto -lz


default: all

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.inc
	rm -f $(BIN)
