CFLAGS=-std=c11 -g -static -Wall -Wno-unused-variable
# tmp.cを除く全ての.cファイルをコンパイルする
SRCS=$(filter-out tmp.c, $(filter-out test.c, $(wildcard *.c)))
OBJS=$(SRCS:.c=.o)

10cc: $(OBJS)
	$(CC) -g -o 10cc $(OBJS) $(LDFLAGS)

$(OBJS): 10cc.h map.h

test: 10cc test_mock.s
	./test.sh

test.s: test.c
	gcc -S -o test.s test.c

test_mock.s: test_mock.c
	gcc -S -o test_mock.s test_mock.c

self1: 10cc *.c
	./10cc map.c > map.s
	./10cc main.c > main.s
	./10cc tokenize.c > tokenize.s
	./10cc parse.c > parse.s
	./10cc analyze.c > analyze.s
	./10cc gen.c > gen.s
	gcc -o 10cc-1 map.s main.s tokenize.s parse.s analyze.s gen.s

clean:
	rm -f 10cc *.o *~ tmp* test.s

.PHONY: test clean
