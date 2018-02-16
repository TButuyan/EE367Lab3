/* Jaymark Ganibi / Tyler Butuyan
 * EE367
 * 2/15/18
 * server367.c -- a stream socket server demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3509"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold

void sigchld_handler(int s) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

//Child process
void error(char *s);

int main() {
	int sockfd, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	char rec_cmd[100];

	//For child Process
	int pid;
	int status;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while (1) { // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *) &their_addr),
				s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (fork() == 0) {
			close(sockfd);
			if ((numbytes = recv(new_fd, rec_cmd, 100 - 1, 0)) == -1) {
				perror("recv");
				exit(1);
			}

			rec_cmd[numbytes] = '\0';

			if (!strcmp(rec_cmd, "list")) /// list contents and send back////
			{
				//ls command through child
				if ((pid = fork()) == 0) {
					dup2(new_fd, 1);
					execl("/usr/bin/ls", "ls", (char *) NULL);
					error("could not exec 'ls'");
				}

				wait(&status);
				exit(0);
			}

			if (!strcmp(rec_cmd, "disp")) /// display file and send back ////
			{
				FILE *fp;
				if ((numbytes = recv(new_fd, rec_cmd, 100 - 1, 0)) == -1) {
					perror("recvdisplay");
					exit;
				}
				rec_cmd[numbytes] = '\0';

				fp = fopen(rec_cmd, "r"); // Open target file
				if (fp == NULL) {
					send(new_fd, "0", 1, 0); // Send file size 0 to let client know not found
					printf("file not found");
				} else {
					fseek(fp, 0L, SEEK_END); // Jump to end of file
					int filesize = ftell(fp); // Find byte size from byte 0 to EOF
					fseek(fp, 0, SEEK_SET); // Reset fp to top of file
					char *filebuff = calloc(filesize, sizeof(char)); //Allocate array for file
					fread(filebuff, sizeof(char), filesize, fp); // Read file size # of bytes to filebuff
					char sendSize[20] = {0}; // Allocate char array to send over socket
					snprintf(sendSize, 20, "%d", filesize); // Change int to char to send to clinet
					send(new_fd, sendSize, 20, 0); // Send file size to client as char
					send(new_fd, filebuff, filesize, MSG_DONTWAIT);
					fclose(fp); // Release file io
					free(filebuff); // Free dynamic array
				}
			}

			//Check command
			if (!strcmp(rec_cmd, "chek")) {
				FILE *fp;
				char filename[1000];
				int numb;

				if ((numb = recv(new_fd, filename, 100 - 1, 0)) == -1) {
					perror("recv");
					exit(1);
				}
				filename[numb] = '\0';

				printf("Looking for %s\n", filename);

				fp = fopen(filename, "r");
				if (fp == NULL)
					send(new_fd, "no", 2, 0);
				else
					send(new_fd, "yes", 3, 0);

			}

			close(new_fd);
			exit(0);
		}//end fork
		close(new_fd);
	}

	return 0;
}
