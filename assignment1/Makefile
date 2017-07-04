CC = gcc
CCOPTS = -c -g -Wall
LINKOPTS = -g

all: main

main: main.o testrunner.o assignment1_test.o
	$(CC) $(LINKOPTS) -o $@ $^

main.o: main.c assignment1_test.h
	$(CC) $(CCOPTS) -o $@ $<

testrunner.o: testrunner.c testrunner.h
	$(CC) $(CCOPTS) -o $@ $<

assignment1_test.o: assignment1_test.c testrunner.h
	$(CC) $(CCOPTS) -o $@ $<

test: main
	./main -test -f7 all

clean:
	rm -rf *.o main main.exe *~
