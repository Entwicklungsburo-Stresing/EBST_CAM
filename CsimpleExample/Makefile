CC = gcc
CFLAGS = -Wall -g
OBJECTS   = CsimpleExample.o
TARGET    = CsimpleExample
LIBS      = -lESLSCDLL

CsimpleExample: $(OBJECTS)  
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

CsimpleExample.o: CsimpleExample.c
	$(CC) -c $(CFLAGS) -o CsimpleExample.o CsimpleExample.c

clean:
	rm -f *~ *.o CsimpleExample
