
./10cc test.c > tmp.s
cc -no-pie -o tmp.o tmp.s test_mock.s
./tmp.o
