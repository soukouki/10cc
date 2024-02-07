CFLAGS=-std=c11 -g -static

10cc: 10cc.c

test: 10cc
	./test.sh

clean:
	rm -f 10cc *.o *~ tmp*

.PHONY: test clean
