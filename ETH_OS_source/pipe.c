#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


int main(int argc, char **argv) {
	int x;
	mywait(&x);
        printf("%i\n", x);
}

int mywait(int* foo) {

  int f = 5;
  *foo = f;

  return 0;
}
