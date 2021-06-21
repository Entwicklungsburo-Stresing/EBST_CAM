CC = gcc
CFLAGS = -Wall -g
OBJECTS   = lsc-cli.o \
			es_status_codes.o \
			Board.o \
			UIAbstractionLayer.o \
			globals.o \
			lscpcie.o \
			Board_ll.o
TARGET    = lsc-cli
LIBS      = -lpthread -lm

lsc-cli: $(OBJECTS)  
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

lsc-cli.o: lsc-cli.c
	$(CC) -c $(CFLAGS) -o lsc-cli.o lsc-cli.c

es_status_codes.o: ../shared_src/es_status_codes.c ../shared_src/es_status_codes.h
	$(CC) -c $(CFLAGS) -o es_status_codes.o ../shared_src/es_status_codes.c

Board.o: ../shared_src/Board.c ../shared_src/Board.h \
		../shared_src/Board_ll.h \
		../shared_src/es_status_codes.h \
		../shared_src/globals.h \
		../shared_src/enum.h \
		../shared_src/struct.h \
		../shared_src/UIAbstractionLayer.h
	$(CC) -c $(CFLAGS) -o Board.o ../shared_src/Board.c

UIAbstractionLayer.o: UIAbstractionLayer.c ../shared_src/UIAbstractionLayer.h
	$(CC) -c $(CFLAGS) -o UIAbstractionLayer.o UIAbstractionLayer.c

globals.o: ../shared_src/globals.c ../shared_src/globals.h
	$(CC) -c $(CFLAGS) -o globals.o ../shared_src/globals.c

lscpcie.o: ../linux-driver/userspace/lscpcie.c ../linux-driver/userspace/lscpcie.h \
		../linux-driver/userspace/types.h \
		../linux-driver/kernelspace/registers-common.h \
		../shared_src/enum.h \
		../linux-driver/kernelspace/ioctl.h \
		../linux-driver/userspace/constants.h \
		../linux-driver/kernelspace/registers.h \
		../linux-driver/userspace/local-config.h
	$(CC) -c $(CFLAGS) -o lscpcie.o ../linux-driver/userspace/lscpcie.c

Board_ll.o: ../shared_src/linux/Board_ll.c ../shared_src/Board_ll.h \
		../shared_src/es_status_codes.h \
		../shared_src/Board.h \
		../shared_src/enum.h \
		../shared_src/struct.h \
		../shared_src/globals.h \
		../linux-driver/userspace/lscpcie.h \
		../linux-driver/userspace/types.h \
		../linux-driver/kernelspace/registers-common.h \
		../linux-driver/kernelspace/ioctl.h \
		../linux-driver/kernelspace/registers.h \
		../linux-driver/userspace/local-config.h
	$(CC) -c $(CFLAGS) -o Board_ll.o ../shared_src/linux/Board_ll.c

clean:
	rm -f *~ *.o

distclean: clean
	rm -f lsc-cli
