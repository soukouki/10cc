CFLAGS=-std=c11 -g -static -Wall -Wno-unused-variable
# tmp.cを除く全ての.cファイルをコンパイルする
SRCS=$(filter-out tmp.c tmp2.c tmp_doughnut.c, $(filter-out test.c, $(wildcard *.c)))
OBJS=$(SRCS:.c=.o)

10cc: $(OBJS)
	$(CC) -g -o 10cc $(OBJS) $(LDFLAGS)

$(OBJS): 10cc.h map.h

self1-test: 10cc test_mock.s
	./10cc test.c > tmp.s
	cc -static -o tmp.o tmp.s test_mock.s
	./tmp.o

self2-all-test: self2-all test_mock.s
	./10cc-2 test.c > tmp.s
	cc -static -o tmp.o tmp.s test_mock.s
	./tmp.o

self3-all-test: self3-all test_mock.s
	./10cc-3 test.c > tmp.s
	cc -static -o tmp.o tmp.s test_mock.s
	./tmp.o

test.s: test.c
	gcc -S -o test.s test.c

test_mock.s: test_mock.c
	gcc -S -o test_mock.s test_mock.c

self2-all: 10cc *.c
	./10cc map.c > map2.s
	./10cc main.c > main2.s
	./10cc preprocess.c > preprocess2.s
	./10cc tokenize.c > tokenize2.s
	./10cc parse.c > parse2.s
	./10cc analyze.c > analyze2.s
	./10cc gen.c > gen2.s
	gcc -g -o 10cc-2 map2.s main2.s preprocess2.s tokenize2.s parse2.s analyze2.s gen2.s -no-pie
	./10cc-2 tmp.c # while文で失敗

self2-map: 10cc *.c
	./10cc map.c > map.s
	$(CC) main.c -S
	$(CC) preprocess.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s preprocess.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

self2-main: 10cc *.c
	$(CC) map.c -S
	./10cc main.c > main.s
	$(CC) preprocess.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s preprocess.s tokenize.s parse.s analyze.s gen.s -g -no-pie
	./10cc-2 tmp.c # 成功

self2-preprocess: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	./10cc preprocess.c > preprocess.s
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s preprocess.s tokenize.s parse.s analyze.s gen.s -g -no-pie
	./10cc-2 tmp.c # 成功

self2-tokenize: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) preprocess.c -S
	./10cc tokenize.c > tokenize.s
	$(CC) parse.c -S
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s preprocess.s tokenize.s parse.s analyze.s gen.s -g -no-pie
	./10cc-2 tmp.c # 成功

self2-parse: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) preprocess.c -S
	$(CC) tokenize.c -S
	./10cc parse.c > parse.s
	$(CC) analyze.c -S
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s preprocess.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

self2-analyze: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) preprocess.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	./10cc analyze.c > analyze.s
	$(CC) gen.c -S
	gcc -g -o 10cc-2 map.s main.s preprocess.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

self2-gen: 10cc *.c
	$(CC) map.c -S
	$(CC) main.c -S
	$(CC) preprocess.c -S
	$(CC) tokenize.c -S
	$(CC) parse.c -S
	$(CC) analyze.c -S
	./10cc gen.c > gen.s
	gcc -g -o 10cc-2 map.s main.s preprocess.s tokenize.s parse.s analyze.s gen.s -no-pie
	./10cc-2 tmp.c # 成功

self3-all: self2-all *.c
	./10cc-2 map.c > map3.s
	./10cc-2 main.c > main3.s
	./10cc-2 preprocess.c > preprocess3.s
	./10cc-2 tokenize.c > tokenize3.s
	./10cc-2 parse.c > parse3.s
	./10cc-2 analyze.c > analyze3.s
	./10cc-2 gen.c > gen3.s
	gcc -g -o 10cc-3 map3.s main3.s preprocess3.s tokenize3.s parse3.s analyze3.s gen3.s -no-pie
	./10cc-3 tmp.c
	diff map2.s map3.s
	diff main2.s main3.s
	diff preprocess2.s preprocess3.s
	diff tokenize2.s tokenize3.s
	diff parse2.s parse3.s
	diff analyze2.s analyze3.s
	diff gen2.s gen3.s

clean:
	rm 10cc *.o *.s
	rm tmp*.s tmp*.o

.PHONY: test clean
