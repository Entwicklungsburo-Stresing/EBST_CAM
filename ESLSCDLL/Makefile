all: ESLSCDLL.o
	gcc -shared -fPIC ESLSCDLL.o -o ESLSCDLL.so

ESLSCDLL.o:
	gcc -c -fPIC ESLSCDLL.c -o ESLSCDLL.o

clean:
	rm ESLSCDLL.so ESLSCDLL.o
