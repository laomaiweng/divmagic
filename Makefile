all: divmagic32 divmagic64

CFLAGS ?= -Wall -Wextra -O3

divmagic32: divmagic.c
	$(CC) $(CFLAGS) -DBITS=32 -o $@ $<

divmagic64: divmagic.c
	$(CC) $(CFLAGS) -DBITS=64 -o $@ $<

clean:
	$(RM) divmagic32 divmagic64
