/*
 * This is an example program that will launch the 'ls' syhstem call
 * It shows how to call a program from within
 * a C program using "execl", which is a variation of exec.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <unistd.h> 
#include <string.h>

void error(char *s); 
char *data = "Some input data\n"; 
 
void main() 
{ 
  int pid;
  int status;

  if ((pid=fork()) == 0) {

    /* This is the child process */ 
    /* 
     * The zeroth argument is the path to the program 'ls' 
     * The first argument is the name of the program to be run, which
     * is 'ls'.  This is a bit redundant to the zeroth argument, 
     * but it's how it works.   The third argument is a terminating
     * symbol -- a NULL pointer -- to indicate the end of the arguments.
     * In general, execl accepts an arbitrary number of arguments.
     */
    execl("/usr/bin/ls", "ls", (char *)NULL); 

    /* If 'ls' wasn't executed then we would still have the following
     * function, which would indicate an error 
     */
    error("Could not exec 'ls'"); 
  } 

  /*  The following is in the parent process */ 
  wait(&status);
  printf("Spawned 'ls' is a child process at pid %d\n", pid); 
 
  /* This is the parent process */ 

  exit(0); 
} 
 
void error(char *s) 
{ 
  perror(s); 
  exit(1); 
}
 
