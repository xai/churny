CC=clang
CFLAGS=-lgit2

all: churny

churny: src/churny.c src/churny.h src/loc.c src/loc.h src/utils.c src/utils.h src/jdime.c src/jdime.h src/list.c src/list.h
	$(CC) -march=native -O3 $(CFLAGS) -o churny src/churny.c src/loc.c src/utils.c src/jdime.c src/list.c

debug: src/churny.c src/churny.h src/loc.c src/loc.h src/utils.c src/utils.h src/jdime.c src/jdime.h src/list.c src/list.h
	$(CC) $(CFLAGS) -DDEBUG -g -o churny src/churny.c src/loc.c src/utils.c src/jdime.c src/list.c

trace: src/churny.c src/churny.h src/loc.c src/loc.h src/utils.c src/utils.h src/jdime.c src/jdime.h src/list.c src/list.h
	$(CC) $(CFLAGS) -DTRACE -o churny src/churny.c src/loc.c src/utils.c src/jdime.c src/list.c

valgrind: src/churny.c src/churny.h src/loc.c src/loc.h src/utils.c src/utils.h src/jdime.c src/jdime.h src/list.c src/list.h
	$(CC) $(CFLAGS) -g -O0 -o churny src/churny.c src/loc.c src/utils.c src/jdime.c src/list.c

clean:
	rm churny

