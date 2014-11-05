CC=clang
CFLAGS=-lgit2

all: churny

churny: churny.c loc.c loc.h utils.c utils.h
	$(CC) -march=native -O3 $(CFLAGS) -o churny churny.c loc.c utils.c

debug: churny.c loc.c loc.h utils.c utils.h
	$(CC) $(CFLAGS) -DDEBUG -g -o churny churny.c loc.c utils.c

trace: churny.c loc.c loc.h utils.c utils.h
	$(CC) $(CFLAGS) -DTRACE -o churny churny.c loc.c utils.c

valgrind: churny.c loc.c loc.h utils.c utils.h
	$(CC) $(CFLAGS) -g -O0 -o churny churny.c loc.c utils.c

