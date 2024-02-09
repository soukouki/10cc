CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

10cc: $(OBJS)
	$(CC) -o 10cc $(OBJS) $(LDFLAGS)

$(OBJS): 10cc.h

test: 10cc test.s
	./test.sh

test.s: test.c
	gcc -S -o test.s test.c

clean:
	rm -f 10cc *.o *~ tmp* test.s

.PHONY: test clean
