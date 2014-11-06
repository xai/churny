CC=clang
CFLAGS=-lgit2

all: churny

churny: churny.c loc.c loc.h utils.c utils.h list.c list.h
	$(CC) -march=native -O3 $(CFLAGS) -o churny churny.c loc.c utils.c list.c

debug: churny.c loc.c loc.h utils.c utils.h list.c list.h
	$(CC) $(CFLAGS) -DDEBUG -g -o churny churny.c loc.c utils.c list.c

trace: churny.c loc.c loc.h utils.c utils.h list.c list.h
	$(CC) $(CFLAGS) -DTRACE -o churny churny.c loc.c utils.c list.c

valgrind: churny.c loc.c loc.h utils.c utils.h list.c list.h
	$(CC) $(CFLAGS) -g -O0 -o churny churny.c loc.c utils.c list.c

