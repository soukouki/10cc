CFLAGS=-std=c11 -g -static -Wall -Wno-unused-variable
# tmp.cを除く全ての.cファイルをコンパイルする
SRCS=$(filter-out tmp.c tmp2.c, $(filter-out test.c, $(wildcard *.c)))
OBJS=$(SRCS:.c=.o)

10cc: $(OBJS)
	$(CC) -g -o 10cc $(OBJS) $(LDFLAGS)

$(OBJS): 10cc.h map.h

self1-test: 10cc test_mock.s
	./10cc test.c > tmp.s
	cc -static -o tmp.o tmp.s test_mock.s
	./tmp.o

self2-all-test: self2-all
	./10cc-2 test.c > tmp.s
	cc -static -o tmp.o tmp.s test_mock.s
	./tmp.o

test.s: test.c
	gcc -S -o test.s test.c

test_mock.s: test_mock.c
	gcc -S -o test_mock.s test_mock.c

self2-all: 10cc *.c
	./10cc map.c > map.s
	./10cc main.c > main.s
	./10cc tokenize.c > tokenize.s
	./10cc parse.c > parse.s
	./10cc analyze.c > analyze.s
	./10cc gen.c > gen.s
	gcc -g -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

self2-map: 10cc *.c
	./10cc map.c > map.s
	$(CC) main.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

self2-main: 10cc *.c
	$(CC) map.c -S
	./10cc main.c > main.s
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -g -no-pie
	./10cc-2 tmp.c # 成功

self2-tokenize: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	./10cc tokenize.c > tokenize.s
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -g -no-pie
	./10cc-2 tmp.c # 成功

self2-parse: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) tokenize.c -S
	./10cc parse.c > parse.s
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

self2-analyze: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	./10cc analyze.c > analyze.s
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

self2-gen: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	./10cc gen.c > gen.s
	gcc -g -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

clean:
	rm -f 10cc *.o *~ tmp* test.s

.PHONY: test clean
