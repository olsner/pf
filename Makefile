LDFLAGS = -lmpfr -lgmp -lm -lrt
CFLAGS = -Wall -Wno-unused-function -ggdb

default: pf conv

pf: pf.c buf.c rewr.c mympfr.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

conv: conv.c buf.c mympfr.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
