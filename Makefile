PHP_PREFIX=/home/johannes/src/php/5.3/_src
CFLAGS=-Wall -g -O0 -I/home/johannes/src/php/5.3/embed -I/home/johannes/src/php/5.3/embed/Zend -I/home/johannes/src/php/5.3/embed/TSRM -I/home/johannes/src/php/5.3/embed/main

all: pconnect-test

pconnect-sapi.o: pconnect-sapi.c
	gcc -c $(CFLAGS) -opconnect-sapi.o -I$(PHP_PREFIX) -I$(PHP_PREFIX)/Zend -I$(PHP_PREFIX)/TSRM -I$(PHP_PREFIX)/main -I$(PHP_PREFIX)/sapi/embed pconnect-sapi.c

main.o: main.c
	gcc -c $(CFLAGS) -omain.o main.c

pconnect-test: pconnect-sapi.o main.o
	gcc -opconnect-test -L. -Wl,-rpath,. -lphp5 pconnect-sapi.o main.o

clean:
	rm pconnect-test
	rm *.o
	
