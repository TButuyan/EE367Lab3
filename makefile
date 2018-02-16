all: client367 server367

client367:
	gcc client367.c -lm -o myclient

server367:
	gcc server367.c -o myserver

clean: clean
	rm -f myclient myserver
