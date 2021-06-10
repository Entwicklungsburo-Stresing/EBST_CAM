CC = gcc
CFLAGS = -Wall -g

programs = readout-polling readout-blocking-proc readout-blocking-filesystem \
  readout-blocking-filesystem-trigger readout-loop

examples: $(programs)

readout-polling: readout-polling.o common.o
	$(CC) $(CFLAGS) -o $@ $^ -llscpcie -L..

readout-blocking-proc: readout-blocking-proc.o common.o
	$(CC) $(CFLAGS) -o $@ $^ -llscpcie -L..

readout-blocking-filesystem: readout-blocking-filesystem.o common.o
	$(CC) $(CFLAGS) -o $@ $^ -llscpcie -L..

readout-blocking-filesystem-trigger: readout-blocking-filesystem-trigger.o \
  common.o
	$(CC) $(CFLAGS) -o $@ $^ -llscpcie -L..

readout-loop: readout-loop.o
	$(CC) $(CFLAGS) -o $@ $^ -llscpcie -L..

%.o: %.c
	$(CC) $(CFLAGS) -D__user_space__ -I.. -c $< -o $@

clean:
	rm -f *~ *.o

distclean: clean
	rm -f $(programs)
