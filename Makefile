CC		?= gcc
PKG_CONFIG	?= pkg-config

LUAPKG	?= \
	$(shell \
		for p in lua5.4 lua-5.4 lua54 \
		         lua5.3 lua-5.3 lua53 \
		         lua5.2 lua-5.2 lua52 \
		         lua5.1 lua-5.1 lua51 \
		         lua; \
		do \
			$(PKG_CONFIG) --exists $$p && echo $$p && break; \
		done \
	)

SOURCE	:= $(sort $(wildcard *.c))
OBJS	:= $(patsubst %.c, %.o, $(SOURCE))
BIN	:= desfsh
CFLAGS	?= -Wall -Wextra
CFLAGS	+= $(shell pkg-config $(LUAPKG) --cflags)
LDFLAGS	?=
LDFLAGS	+= -lnfc -lfreefare -lreadline $(shell pkg-config $(LUAPKG) --libs) -lcrypto -lz


default: all

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

install:
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL) $(BIN) $(DESTDIR)/usr/bin

clean:
	rm -f *.o *.inc
	rm -f $(BIN)
