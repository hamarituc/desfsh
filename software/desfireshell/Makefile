SOURCE	:= $(sort $(wildcard *.c))
OBJS	:= $(patsubst %.c, %.o, $(SOURCE))
BIN	:= desfsh
CC	:= gcc
CFLAGS	:= -Wall -Wextra -g
LDFLAGS	:= -lnfc -lfreefare -lreadline -llua


default: all

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o
	rm -f $(BIN)
