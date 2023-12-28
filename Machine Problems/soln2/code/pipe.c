#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc,char **argv) {

    int fd[2];

    // create pipe
    if (pipe(fd) == -1) {
      fprintf(stderr, "can't create pipe, error %d\n", errno);
      exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

   if (pid == -1) {
      fprintf(stderr, "can't fork, error %d\n", errno);
      exit(EXIT_FAILURE);
   }
   if (pid == 0) {
        // close read-end of the pipe
        close(fd[0]);
        // redirect stdout (1) to write-end of the pipe
        close(1);
        dup2(fd[1], 1);
        // exec ls
        execlp("ls", "ls", "-l", NULL);
    }
    else {
        char buffer[128];

        // close unused write-end of the pipe
        close(fd[1]);

        // read from pipe and print until EOF
        size_t ret = 1;
        while (ret > 0) {
            ret = read(fd[0], buffer, 128);
            buffer[ret] = '\0';
            printf("READ: %s", buffer);
        }

        // wait for child to exit first
        int status;
        wait(&status);
    }

    return 0;
}
