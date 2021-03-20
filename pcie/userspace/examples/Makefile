examples: readout-polling readout-blocking-proc readout-blocking-filesystem

readout-polling: readout-polling.o common.o to-library.o ../lscpcie.o
	gcc -Wall -g -o $@ $^

readout-blocking-proc: readout-blocking-proc.o common.o to-library.o ../lscpcie.o
	gcc -Wall -g -o $@ $^

readout-blocking-filesystem: readout-blocking-filesystem.o common.o \
  to-library.o ../lscpcie.o
	gcc -Wall -g -o $@ $^

%.o: %.c
	gcc -Wall -g -D__user_space__ -I.. -c $< -o $@

clean:
	rm -f *~ *.o
