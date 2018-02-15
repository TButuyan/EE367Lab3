/*
 ** server367.c - an edited copy of server.c
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

#define PORT "3502"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 1000    // max number of bytes we can get at once

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char command[MAXDATASIZE]; // command string for client
    int quit = 0; // flag to exit command loop
    char buff[MAXDATASIZE]; // buffer string
    FILE *buffer;   // buffer file
    char file[MAXDATASIZE]; // string for file name

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                    sizeof(int)) == -1) {
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

    if (p == NULL)  {
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

    while(1) {  // main accept() loop
        quit = 0;
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            if (send(new_fd, "Command (type 'h' for help): ", 29, 0) == -1){
                perror("send");
            }

            if (!fork()) {
                execl("/bin/ls", "ls", "-l", (char *)0);
            }

            while (!quit) { // loop for command input
                recv(new_fd, command, 1000, 0);

                switch(command[0]) { // extract command input
                    case 'l':
                        if(!fork()) {
                            printf("client called ls\n");
                        }
                        send(new_fd, "l", 1, 0);
                        break;
                    case 'c':
                        if(fork() == 0) {
                            printf("client called check\n");
                        }
                        send(new_fd, "c", 1, 0);
                        break;
                    case 'p':
                        if(fork() == 0) {
                            printf("client called display\n");
                        }
                        send(new_fd, "p", 1, 0);
                        break;
                    case 'd':
                        if(fork() == 0) {
                            printf("client called download\n");
                        }
                        break;
                    case 'q': 
                        if (fork() == 0) {
                            printf("client quit\n");
                        }
                        quit = 1;
                        send(new_fd, "q", 1, 0); //q is quit
                        break;
                    case 'h':
                        send(new_fd, "h", 1, 0);
                        break;
                    default: 
                        send(new_fd, "~", 1, 0);
                        break;
                }
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}

