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

clean:
	rm -f 10cc *.o *~ tmp* test.s

.PHONY: test clean
