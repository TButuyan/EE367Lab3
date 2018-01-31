all: client367 server367 client server

client367: client367.o
	gcc client367.c -lm -o client367

server367: server367.o
	gcc server367.c -lm -o server367

client: client.o
	gcc client.c -lm -o client

server: server.o
	gcc server.c -lm -o server

clean:
	rm -f *.o

real_clean: clean
	rm -f client367 server367 client server
