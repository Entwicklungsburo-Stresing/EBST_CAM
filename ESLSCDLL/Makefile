CC = gcc
CFLAGS = -Wall -shared -fPIC  -g
OBJECTS   = es_status_codes.o \
			Board.o \
			UIAbstractionLayer.o \
			globals.o \
			lscpcie.o \
			Board_ll.o \
			ESLSCDLL.o
TARGET    = libESLSCDLL.so
LIBS      = -lpthread -lm

libESLSCDLL.so: $(OBJECTS)  
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	chmod -x $(TARGET)

ESLSCDLL.o: ESLSCDLL.c
	$(CC) -c $(CFLAGS) -o ESLSCDLL.o ESLSCDLL.c

es_status_codes.o: ../shared_src/es_status_codes.c ../shared_src/es_status_codes.h
	$(CC) -c $(CFLAGS) -o es_status_codes.o ../shared_src/es_status_codes.c

Board.o: Board.c Board.h \
		Board_ll.h \
		../shared_src/es_status_codes.h \
		globals.h \
		../shared_src/enum_hardware.h \
		../shared_src/enum_settings.h \
		../shared_src/struct.h \
		UIAbstractionLayer.h
	$(CC) -c $(CFLAGS) -o Board.o Board.c

UIAbstractionLayer.o: UIAbstractionLayer.c UIAbstractionLayer.h
	$(CC) -c $(CFLAGS) -o UIAbstractionLayer.o UIAbstractionLayer.c

globals.o: globals.c globals.h
	$(CC) -c $(CFLAGS) -o globals.o globals.c

lscpcie.o: ../linux-driver/userspace/lscpcie.c ../linux-driver/userspace/lscpcie.h \
		../linux-driver/userspace/types.h \
		../linux-driver/kernelspace/registers-common.h \
		../shared_src/enum_hardware.h \
		../shared_src/enum_settings.h \
		../linux-driver/kernelspace/ioctl.h \
		../linux-driver/userspace/constants.h \
		../linux-driver/kernelspace/registers.h \
		../linux-driver/userspace/local-config.h
	$(CC) -c $(CFLAGS) -o lscpcie.o ../linux-driver/userspace/lscpcie.c

Board_ll.o: linux/Board_ll.c Board_ll.h \
		../shared_src/es_status_codes.h \
		Board.h \
		../shared_src/enum_hardware.h \
		../shared_src/enum_settings.h \
		../shared_src/struct.h \
		globals.h \
		../linux-driver/userspace/lscpcie.h \
		../linux-driver/userspace/types.h \
		../linux-driver/kernelspace/registers-common.h \
		../linux-driver/kernelspace/ioctl.h \
		../linux-driver/kernelspace/registers.h \
		../linux-driver/userspace/local-config.h
	$(CC) -c $(CFLAGS) -o Board_ll.o linux/Board_ll.c

clean:
	rm -f *.so *.o
