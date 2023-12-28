#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {

    printf("Before exec\n");

    // call ls with an argument (-l) and terminate the args list with NULL
    execlp("ls", "ls", "-l", NULL);

    // this will never be printed since exec replaces 
    // this processes image by ls
    printf("After exec\n");

    return 0;

}


