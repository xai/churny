CC=clang
CFLAGS=-lgit2

all: churny

churny: churny.c
	$(CC) -march=native -O3 $(CFLAGS) -o churny churny.c

debug: churny.c
	$(CC) $(CFLAGS) -DDEBUG -o churny churny.c

valgrind: churny.c
	$(CC) $(CFLAGS) -g -O0 -o churny churny.c

