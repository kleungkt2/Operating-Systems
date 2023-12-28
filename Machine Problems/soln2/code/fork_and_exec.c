#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char** argv) {

   pid_t pid = fork();
 
   if (pid == -1) {
      fprintf(stderr, "can't fork, error %d\n", errno);
      exit(EXIT_FAILURE);
   }
   else if (pid == 0) {

     /* When fork() returns 0, we are in the child process. */

     // call ls with an argument (-l) and terminate the args list with NULL
     execlp("ls", "ls", "-l", NULL);
   }
   else {
      /* When fork() returns a positive number, we are the parent */
      int status;
      wait(&status);
      printf("After ls (in parent)\n");
   }

   return 0;

}

