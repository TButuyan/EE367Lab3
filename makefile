all: server367.c client367.c

	gcc -o server367 server367.c
	gcc -o client367 client367.c
clean:
	rm server367 client367
