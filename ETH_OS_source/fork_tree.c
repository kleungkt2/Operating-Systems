#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/* define the number of forks */
const int numberOfForks = 3;

int main(int argc, char** argv) {
    int i = 0;
    int level = 0;
    int status;
    pid_t child[numberOfForks];

    /* fork numberOfForks child processes */
    for (i = 0; i < numberOfForks; ++i) {
        child[i] = fork();

        if (child[i] == -1) {
            fprintf(stderr, "can't fork, error %d\n", errno);
            exit(EXIT_FAILURE);
        }

        if (child[i] == 0) {
            level++;
        }
    }

    /* print process info */
    sleep(level);
    printf("pid(%i)%.*s -> level %i\n", getpid(), level, "\t\t\t\t\t\t", level);

    /* wait for all children of the process */
    i = numberOfForks - 1;
    while(i >= 0 && child[i] != 0) {
        waitpid(child[i], &status, 0);
        --i;
    }

    /* print a process done message */
    sleep(10);
    printf("pid(%i)%.*s -> done\n", getpid(), level, "\t\t\t\t\t\t");

    return 0;
}

