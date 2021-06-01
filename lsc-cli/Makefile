CC = gcc
CFLAGS = -Wall -g
OBJECTS   = lsc-cli.o \
			es_status_codes.o \
			crossPlattformBoard.o \
			UIAbstractionLayer.o \
			globals.o \
			lscpcie.o \
			crossPlattformBoard_ll.o
TARGET    = lsc-cli

lsc-cli: $(OBJECTS)  
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

lsc-cli.o: lsc-cli.c
	$(CC) -c $(CFLAGS) -o lsc-cli.o lsc-cli.c

es_status_codes.o: ../shared_src/es_status_codes.c ../shared_src/es_status_codes.h
	$(CC) -c $(CFLAGS) -o es_status_codes.o ../shared_src/es_status_codes.c

crossPlattformBoard.o: ../shared_src/crossPlattformBoard.c ../shared_src/crossPlattformBoard.h \
		../shared_src/crossPlattformBoard_ll.h \
		../shared_src/es_status_codes.h \
		../shared_src/globals.h \
		../shared_src/enum.h \
		../shared_src/struct.h \
		../shared_src/UIAbstractionLayer.h
	$(CC) -c $(CFLAGS) -o crossPlattformBoard.o ../shared_src/crossPlattformBoard.c

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

crossPlattformBoard_ll.o: ../shared_src/linux/crossPlattformBoard_ll.c ../shared_src/crossPlattformBoard_ll.h \
		../shared_src/es_status_codes.h \
		../shared_src/crossPlattformBoard.h \
		../shared_src/enum.h \
		../shared_src/struct.h \
		../shared_src/globals.h \
		../linux-driver/userspace/lscpcie.h \
		../linux-driver/userspace/types.h \
		../linux-driver/kernelspace/registers-common.h \
		../linux-driver/kernelspace/ioctl.h \
		../linux-driver/kernelspace/registers.h \
		../linux-driver/userspace/local-config.h
	$(CC) -c $(CFLAGS) -o crossPlattformBoard_ll.o ../shared_src/linux/crossPlattformBoard_ll.c

clean:
	rm -f *~ *.o

distclean: clean
	rm -f $(programs)
