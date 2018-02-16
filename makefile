all: client367 server367

client367: client367.o
	gcc client367.c -lm myclient

server367: server367.o
	gcc server367.c -lm myserver

clean:
	rm -f *.o

real_clean: clean
	rm -f myclient myserver
