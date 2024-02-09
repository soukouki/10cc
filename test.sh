#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./10cc "$input" > tmp.s
  cc -o tmp.o tmp.s test.s
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

assert 1 "a = 1; 1;"
assert 10 "a = 10; a;"
assert 30 "a = 10; b = 20; a + b;"

assert 12 "abc = 12; abc;"
assert 34 "_ = 3; _ = _ * 10; _ + 4;"

assert 10 "return 10;"
assert 10 "return 10; 20;"
assert 10 "return 10; return 20;"

assert 10 "if(1) return 10;"
assert 10 "if(1) return 10; return 20;"
assert 20 "if(0) return 10; return 20;"
assert 20 "if(0) return 10; else return 20;"
assert 20 "if(0) return 10; else if(1) return 20; else return 30; return 40;"
assert 30 "if(0) return 10; else if(0) return 20; else return 30; return 40;"

assert 34 "while(0) 12; 34;"
assert 56 "while(1) return 56; 78;"
assert 5 "a = 0; while (a < 5) a = a + 1; return a;"
assert 5 "a = 0; while (1) if(a == 5) return a; else a = a + 1;"

assert 3 "for(;;) return 3;"
assert 5 "a = 0; for(;;) if(a == 5) return a; else a = a + 1;"
assert 4 "for(a = 0; a < 9; a = a + 1) return 4;"
assert 9 "for(a = 0; a < 9; a = a + 1) 1; a;"
assert 9 "for(a = 9;;) if(a == 9) return a;"
assert 5 "for(a = 0; a < 9; a = a + 1) if(a == 5) return a;"

assert 10 "{ 10; }"
assert 10 "{ return 10; 20; }"
assert 20 "{ 10; 20; }"
assert 10 "{ a = 10; return a; }"
assert 20 "if(1) { return 20; }"
assert 10 "if(1) { a = 10; return a; }"
assert 20 "if(0) { a = 10; return a; } else { b = 20; return b; }"
assert 30 "a = 0; while(1) { a = a + 10; if(a == 30) { return a; } }"
assert 40 "for(a = 0; a < 100; a = a + 10) { if(a == 40) { return a; } }"

assert 100 "call0();"
assert 110 "call0()+10;"
assert 120 "a = call0(); a + 20;"
assert 100 "return call0();"
assert 100 "{ call0(); call0(); }"

assert 110 "call1(10);"
assert 220 "call1(10)*2;"
assert 110 "a = call1(10); a;"
assert 110 "return call1(10);"
assert 110 "if(1) return call1(10); else return call1(20); return 110;"

assert 30 "call2(10, 20);"
assert 60 "call3(10, 20, 30);"
assert 99 "call4(10, 20, 30, 39);"
assert 15 "call5(1, 2, 3, 4, 5);"
assert 21 "call6(1, 2, 3, 4, 5, 6);"

echo OK
