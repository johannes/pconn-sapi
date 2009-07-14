# Environment specific:
CC=gcc
PHP_PREFIX=/home/johannes/src/php/5.3/_src

# That's specific to my setup, in a "proper" setup you can drop the -I/home/joh... flags
CFLAGS=-I$(PHP_PREFIX) -I$(PHP_PREFIX)/Zend -I$(PHP_PREFIX)/TSRM -I$(PHP_PREFIX)/main -I$(PHP_PREFIX)/sapi/embed -I/home/johannes/src/php/5.3/embed -I/home/johannes/src/php/5.3/embed/Zend -I/home/johannes/src/php/5.3/embed/TSRM -I/home/johannes/src/php/5.3/embed/main
LDFLAGS=-L. -Wl,-rpath,.

all: pconnect-test

pconnect-sapi.o: pconnect-sapi.c
	gcc -c -Wall -g $(CFLAGS) -opconnect-sapi.o pconnect-sapi.c

main.o: main.c
	gcc -c -Wall -g -O0 $(CFLAGS) -omain.o main.c

pconnect-test: pconnect-sapi.o main.o
	gcc $(LDFLAGS) -opconnect-test -lphp5 pconnect-sapi.o main.o

clean:
	rm pconnect-test
	rm *.o
	
