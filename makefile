CC=gcc
CFLAGS=-Werror -Wall

all:shell

shell: shell.o fonctionShell.o
	$(CC) -o shell fonctionShell.o shell.o $(CFLAGS)

shell.o:shell.c
	$(CC) -o shell.o -c shell.c $(CFLAGS)

fonctionShell.o:fonctionShell.c
	$(CC) -o fonctionShell.o -c fonctionShell.c $(CFLAGS)

clean:
	rm *.o

mrproper: clean
	rm shell
