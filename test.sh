#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./10cc "$input" > tmp.s
  cc -o tmp.o tmp.s
  ./tmp.o
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"

assert 3 "1+2;"
assert 0 "1+2-3;"

assert 21 "20 + 1;"
assert 1 " 1 - 2 + 3 + 4 - 5 ;"

assert 41 "1 + 20 * 2;"
assert 9 "(20 - 1) / 2;"

assert 10 "-10 + 20;"
assert 10 "- -10;"

assert 0 "2 == 3;"
assert 1 "2 != 3;"
assert 1 "2 <= 2;"
assert 0 "2 < 1;"

assert 10 "a = 10; a;"
assert 30 "a = 10; b = 20; a + b;"

echo OK
