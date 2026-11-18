CC = gcc
CFLAGS = -Wall -Wextra

prog: main.o
	$(CC) -o prog main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c
