CC = gcc

CFLAGS += -g
CFLAGS += -fno-strict-aliasing
CFLAGS += -std=gnu99
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -Wno-unused-value
CFLAGS += -Wdeclaration-after-statement
CFLAGS  += -Werror

BIN      = simulator

HDRS = list.h workload.h sim.h stats.h

all: $(BIN)

.PHONY: clean
clean:
	$(RM) *.a *.so *.o $(BIN) $(LIBBIN)

%.o: %.c $(HDRS) Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

simulator: simulator.o workloads.o sched_rr.o stats.o
	$(CC) $(CFLAGS) -o $@ $^