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

assert 0  "main() { return 0; }"
assert 42 "main() { return 42; }"

assert 3  "main() { return 1 + 2; }"
assert 0  "main() { return 1 + 2 - 3; }"
assert 1  "main() { return 1 - 2 + 3 + 4 - 5 ; }"
assert 41 "main() { return 1 + 20 * 2; }"
assert 9  "main() { return (20 - 1) / 2; }"

assert 10 "main() { return -10 + 20; }"
assert 10 "main() { return - -10; }"

assert 0 "main() { return 2 == 3; }"
assert 1 "main() { return 2 != 3; }"
assert 1 "main() { return 2 <= 2; }"
assert 0 "main() { return 2 < 1; }"

assert 1  "main() { a = 1; return 1; }"
assert 10 "main() { a = 10; return a; }"
assert 30 "main() { a = 10; b = 20; return a + b; }"

assert 12 "main() { abc = 12; return abc; }"
assert 34 "main() { _ = 3; _ = _ * 10; return _ + 4; }"

assert 10 "main() { return 10; }"
assert 10 "main() { return 10; 20; }"
assert 10 "main() { return 10; return 20; }"

assert 10 "main() { if(1) return 10; }"
assert 10 "main() { if(1) return 10; return 20; }"
assert 20 "main() { if(0) return 10; return 20; }"
assert 20 "main() { if(0) return 10; else return 20; }"
assert 20 "main() { if(0) return 10; else if(1) return 20; else return 30; return 40; }"
assert 30 "main() { if(0) return 10; else if(0) return 20; else return 30; return 40; }"

assert 34 "main() { while(0) 12; return 34; }"
assert 56 "main() { while(1) return 56; return 78; }"
assert 5  "main() { a = 0; while (a < 5) a = a + 1; return a; }"
assert 5  "main() { a = 0; while (1) if(a == 5) return a; else a = a + 1; }"

assert 3 "main() { for(;;) return 3; }"
assert 5 "main() { a = 0; for(;;) if(a == 5) return a; else a = a + 1; }"
assert 4 "main() { for(a = 0; a < 9; a = a + 1) return 4; }"
assert 9 "main() { for(a = 0; a < 9; a = a + 1) 1; return a; }"
assert 9 "main() { for(a = 9;;) if(a == 9) return a; }"
assert 5 "main() { for(a = 0; a < 9; a = a + 1) if(a == 5) return a; }"

assert 10 "main() { { return 10; } }"
assert 10 "main() { { return 10; 20; } }"
assert 20 "main() { { 10; return 20; } }"
assert 10 "main() { { a = 10; return a; } }"
assert 20 "main() { if(1) { return 20; } }"
assert 10 "main() { if(1) { a = 10; return a; } }"
assert 20 "main() { if(0) { a = 10; return a; } else { b = 20; return b; } }"
assert 30 "main() { a = 0; while(1) { a = a + 10; if(a == 30) { return a; } } }"
assert 40 "main() { for(a = 0; a < 100; a = a + 10) { if(a == 40) { return a; } } }"

assert 100 "main() { return call0(); }"
assert 110 "main() { return call0() + 10; }"
assert 120 "main() { a = call0(); return a + 20; }"
assert 100 "main() { return call0(); }"
assert 100 "main() { { call0(); return call0(); } }"

assert 110 "main() { call1(10); }"
assert 220 "main() { call1(10)*2; }"
assert 110 "main() { a = call1(10); return a; }"
assert 110 "main() { return call1(10); }"
assert 110 "main() { if(1) return call1(10); else return call1(20); return 110; }"

assert 30 "main() { return call2(10, 20); }"
assert 60 "main() { return call3(10, 20, 30); }"
assert 99 "main() { return call4(10, 20, 30, 39); }"
assert 15 "main() { return call5(1, 2, 3, 4, 5); }"
assert 21 "main() { return call6(1, 2, 3, 4, 5, 6); }"

echo OK
