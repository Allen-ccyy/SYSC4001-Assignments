CC = gcc
CCOPTS = -Wall -c -g
LINKOPTS = -g -pthread -lrt

EXEC = assignment5
OBJECTS = restart.o testrunner.o fileio.o assignment5_tests.o bonnie.o util.o

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(LINKOPTS) -o $@ $^

%.o: %.c
	$(CC) $(CCOPTS) -o $@ $^

test:
	./$(EXEC) -test -f0 all

clean:
	- $(RM) $(EXEC)
	- $(RM) $(OBJECTS)
	- $(RM) *~
	- $(RM) core.*
	- $(RM) -rf testdata
	
pretty: 
	indent *.c *.h -kr
