/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3502" // the port client will be connecting to 

#define MAXDATASIZE 1000 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char command[MAXDATASIZE]; // command input
	int quit = 0; // command loop flag

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    printf("Command (type 'h' for help): ");

	while (!quit) {
		scanf("%s", command);
        send(sockfd, command, 1000, 0);
        
        recv(sockfd, buf, 1000, 0); // extract command details
        
        switch(buf[0]) {
            case 'l':
                if(fork() == 0) {
                    execl("/bin/ls", "ls", "-l", (char *)0);
                }
                break;
            case 'c':
                printf("Check\n");
                break;
            case 'p':
                printf("Display\n");
                break;
            case 'd':
                printf("Download\n");
                break;
            case 'q':
                quit = 1;
                printf("Quiting program\n");
                break;
            case 'h':
                printf("Help menu:\n");
                printf("l: List the contents of the directory\n");
                printf("c: Check <filename>\n");
                printf("p: Display <filename>\n");
                printf("d: Download <filename>\n");
                printf("q: Quit\n");
                printf("h: Help\n");
                break;
            case '~':
                printf("Invalid Command!\n");
                break;
            default:
                printf("Command (type 'h' for help): ");
                break;
        }

	}
		close(sockfd);

	return 0;
}

