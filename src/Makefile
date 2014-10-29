CC=clang
CFLAGS=-lgit2

all: churny

churny: churny.c
	$(CC) $(CFLAGS) -o churny churny.c

debug: churny.c
	$(CC) $(CFLAGS) -DDEBUG -o churny churny.c
