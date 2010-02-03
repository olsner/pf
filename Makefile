LDFLAGS = -Lgmp-prefix/lib -Wl,-rpath,\$$ORIGIN/gmp-prefix/lib -lmpfr -lgmp -lm -lrt
CFLAGS = -Wall -ggdb

pf: pf.c buf.c rewr.c mympfr.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
