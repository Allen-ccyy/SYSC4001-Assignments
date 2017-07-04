CC = gcc
CCOPTS = -c -g -Wall -ggdb 
LINKOPTS = -g -pthread 

all: enzyme

enzyme: enzyme.o assignment3_tests.o testrunner.o
	$(CC) $(LINKOPTS) -o $@ $^

assignment3_tests.o: assignment3_tests.c enzyme.h
	$(CC) $(CCOPTS) -o $@ assignment3_tests.c

enzyme.o: enzyme.c enzyme.h
	$(CC) $(CCOPTS) -o $@ enzyme.c

testrunner.o: testrunner.c testrunner.h
	$(CC) $(CCOPTS) -o $@ $<

test: enzyme
	./enzyme -test -f0 all

clean:
	rm -rf enzyme enzyme.exe *.o *~ *.err *.out
