CC = gcc
CFLAGS = -Wall

main: main.o threads.o
	$(CC) $(CFLAGS) main.o threads.o -o main

main.o: main.c threads.h
	$(CC) $(CFLAGS) -c main.c

threads.o: threads.c threads.h
	$(CC) $(CFLAGS) -c threads.c

clean:
	rm *.o main

