CFLAGS=-std=c11 -g -static -Wall -Wno-unused-variable
# tmp.cを除く全ての.cファイルをコンパイルする
SRCS=$(filter-out tmp.c tmp2.c, $(filter-out test.c, $(wildcard *.c)))
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

self2: 10cc *.c
	./10cc map.c > map.s
	./10cc main.c > main.s
	./10cc tokenize.c > tokenize.s
	./10cc parse.c > parse.s
	./10cc analyze.c > analyze.s
	./10cc gen.c > gen.s
	gcc -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie

selftest1: 10cc *.c
	./10cc map.c > map.s
	$(CC) main.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

selftest2: 10cc *.c
	$(CC) map.c -S
	./10cc main.c > main.s
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -g -no-pie
	./10cc-2 tmp.c # 失敗(なぜか変数も関数も出力されない)

selftest4: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	./10cc tokenize.c > tokenize.s
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -g -no-pie
	./10cc-2 tmp.c # 成功

selftest5: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) tokenize.c -S
	./10cc parse.c > parse.s
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

selftest6: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	./10cc analyze.c > analyze.s
	$(CC) gen.c -S
	gcc -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

selftest7: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	./10cc gen.c > gen.s
	gcc -o 10cc-2 map.s main.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

clean:
	rm -f 10cc *.o *~ tmp* test.s

.PHONY: test clean
