#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./10cc "$input" > tmp.s
  cc -no-pie -o tmp.o tmp.s test.s
  ./tmp.o
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

echo "値を返す"
assert 0  "int main() { return 0; }"
assert 42 "int main() { return 42; }"

echo "四則演算"
assert 3  "int main() { return 1 + 2; }"
assert 0  "int main() { return 1 + 2 - 3; }"
assert 1  "int main() { return 1 - 2 + 3 + 4 - 5 ; }"
assert 41 "int main() { return 1 + 20 * 2; }"
assert 9  "int main() { return (20 - 1) / 2; }"

echo "単項+, 単項-"
assert 10 "int main() { return -10 + 20; }"
assert 10 "int main() { return - -10; }"

echo "比較演算子"
assert 0 "int main() { return 2 == 3; }"
assert 1 "int main() { return 2 != 3; }"
assert 1 "int main() { return 2 <= 2; }"
assert 0 "int main() { return 2 < 1; }"

echo "1文字変数"
assert 1  "int main() { int a; a = 1; return 1; }"
assert 10 "int main() { int a; a = 10; return a; }"
assert 30 "int main() { int a; a = 10; int b; b = 20; return a + b; }"

echo "複数文字変数"
assert 12 "int main() { int abc; abc = 12; return abc; }"
assert 34 "int main() { int _; _ = 3; _ = _ * 10; return _ + 4; }"

echo "return文"
assert 10 "int main() { return 10; }"
assert 10 "int main() { return 10; 20; }"
assert 10 "int main() { return 10; return 20; }"

echo "if文"
assert 10 "int main() { if(1) return 10; }"
assert 10 "int main() { if(1) return 10; return 20; }"
assert 20 "int main() { if(0) return 10; return 20; }"
assert 20 "int main() { if(0) return 10; else return 20; }"
assert 20 "int main() { if(0) return 10; else if(1) return 20; else return 30; return 40; }"
assert 30 "int main() { if(0) return 10; else if(0) return 20; else return 30; return 40; }"

echo "while文"
assert 34 "int main() { while(0) 12; return 34; }"
assert 56 "int main() { while(1) return 56; return 78; }"
assert 5  "int main() { int a; a = 0; while (a < 5) a = a + 1; return a; }"
assert 5  "int main() { int a; a = 0; while (1) if(a == 5) return a; else a = a + 1; }"

echo "for文"
assert 3 "int main() { for(;;) return 3; }"
assert 5 "int main() { int a; a = 0; for(;;) if(a == 5) return a; else a = a + 1; }"
assert 4 "int main() { int a; for(a = 0; a < 9; a = a + 1) return 4; }"
assert 9 "int main() { int a; for(a = 0; a < 9; a = a + 1) 1; return a; }"
assert 9 "int main() { int a; for(a = 9;;) if(a == 9) return a; }"
assert 5 "int main() { int a; for(a = 0; a < 9; a = a + 1) if(a == 5) return a; }"

echo "ブロック"
assert 10 "int main() { { return 10; } }"
assert 10 "int main() { { return 10; 20; } }"
assert 20 "int main() { { 10; return 20; } }"
assert 10 "int main() { { int a; a = 10; return a; } }"
assert 20 "int main() { if(1) { return 20; } }"
assert 10 "int main() { if(1) { int a; a = 10; return a; } }"
assert 20 "int main() { if(0) { int a; a = 10; return a; } else { int b; b = 20; return b; } }"
assert 30 "int main() { int a; a = 0; while(1) { a = a + 10; if(a == 30) { return a; } } }"
assert 40 "int main() { int a; for(a = 0; a < 100; a = a + 10) { if(a == 40) { return a; } } }"

echo "関数プロトタイプ宣言"
assert 10 "int func(); int main() { return 10; }"
assert 10 "int func(); int main() { return 10; } int func() { return 10; }"
assert 10 "int func(int); int main() { return 10; } int func(int a) { return a; }"
assert 10 "int func(int, int); int main() { return 10; } int func(int a, int b) { return a + b; }"
assert 10 "int func(int a); int main() { return 10; } int func(int b) { return b; }"
assert 10 "int* func(int*); int main() { return 10; } int* func(int* b) { return b; }"

echo "0引数の関数呼び出し"
assert 100 "int call0(); int main() { return call0(); }"
assert 110 "int call0(); int main() { return call0() + 10; }"
assert 120 "int call0(); int main() { int a; a = call0(); return a + 20; }"
assert 100 "int call0(); int main() { return call0(); }"
assert 100 "int call0(); int main() { { call0(); return call0(); } }"

echo "1引数の関数呼び出し"
assert 110 "int call1(int); int main() { call1(10); }"
assert 220 "int call1(int); int main() { call1(10)*2; }"
assert 110 "int call1(int); int main() { int a; a = call1(10); return a; }"
assert 110 "int call1(int); int main() { return call1(10); }"
assert 110 "int call1(int); int main() { if(1) return call1(10); else return call1(20); return 110; }"

echo "複数引数の関数呼び出し"
assert 30 "int call2(int, int);                     int main() { return call2(10, 20); }"
assert 60 "int call3(int, int, int);                int main() { return call3(10, 20, 30); }"
assert 99 "int call4(int, int, int, int);           int main() { return call4(10, 20, 30, 39); }"
assert 15 "int call5(int, int, int, int, int);      int main() { return call5(1, 2, 3, 4, 5); }"
assert 21 "int call6(int, int, int, int, int, int); int main() { return call6(1, 2, 3, 4, 5, 6); }"

echo "0引数の関数定義"
assert 42 "int func() {} int main() { return 42; }"
assert 42 "int func() { return 42; } int main() { return func(); }"
assert 42 "int a() { return 42; } int b() { return a(); } int main() { return b(); }"
assert 42 "int func() { int a; a = 20; return a; } int main() { int a; a = 42; func(); return a; }"

echo "複数引数の関数定義"
assert 13 "int id(int x) { return x; } int main() { return id(13); }"
assert 13 "int add(int x, int y) { return x + y; } int main() { return add(3, 10); }"
assert 13 "int fib(int n) { if(n <= 1) return n; return fib(n - 1) + fib(n - 2); } int main() { return fib(7); }"

echo "単項&, 単項*"
assert 42 "int main() { int a; a = 42; int* b; b = &a; return *b; }"

echo "ポインタ型(1段階の参照まで)"
assert 42 "int main() { int a; a = 42; int* b; b = &a; return *b; }"
assert 42 "int main() { int a; int* b; b = &a; *b = 42; return a; }"
assert 42 "int f(int* x) { return *x; } int main() { int a; a = 42; return f(&a); }"
assert 42 "int* f(int* x) { *x = 42; return x; } int main() { int a; f(&a); return a; }"
assert 42 "int f(int* x, int* y) { return *x + *y; } int main() { int a; a = 20; int b; b = 22; return f(&a, &b); }"

echo "ポインタ型(複数段階の参照まで)"
assert 42 "int main() { int a; a = 42; int* b; b = &a; int** c; c = &b; return **c; }"
assert 42 "int main() { int a; int* b; int** c; b = &a; c = &b; **c = 42; return a; }"

echo "ポインタ型(ポインタの演算)"
assert 2 "int* alloc_int(int, int, int, int); int main() { int* a; a = alloc_int(1, 2, 3, 4); return *(a + 1); }"
assert 4 "int* alloc_int(int, int, int, int); int main() { int* a; a = alloc_int(1, 2, 3, 4); return *(a + 3); }"
assert 2 "int* alloc_int(int, int, int, int); int main() { int* a; a = alloc_int(1, 2, 3, 4); return *((a + 3) - 2); }"

echo "sizeof演算子"
assert 4 "int main() { return sizeof(1); }"
assert 4 "int main() { return sizeof 1; }"
assert 8 "int main() { int* a; return sizeof(a); }"
assert 8 "int main() { int* a; return sizeof(a + 4); }"
assert 4 "int main() { return sizeof(sizeof(1)); }"

echo "sizeof演算子(多重ポインタ)"
assert 8 "int main() { int** a; return sizeof(a); }"
assert 8 "int main() { int** a; return sizeof(a + 4); }"
assert 8 "int main() { int** a; return sizeof(*a); }"

echo "配列"
assert 1 "int main() { int a[1]; return 1; }"
assert 2 "int main() { int a[1]; *a = 2; return *a; }"

echo "配列の添字アクセス"
assert 1 "int main() { int a[3]; *a = 1; return a[0]; }"
assert 2 "int main() { int a[3]; a[0] = 2; return *a; }"
assert 3 "int main() { int a[3]; a[1] = 3; return a[1]; }"
assert 4 "int main() { int a[3]; a[1] = 4; return *(a + 1); }"
assert 5 "int main() { int a[3]; a[1] = 5; int* b; b = a; return *(b + 1); }"
assert 6 "int main() { int a[3]; (a)[1] = 6; return (a)[1]; }"
assert 7 "int main() { int a[3]; 1[a] = 7; return 1[a]; }"
assert 8 "int main() { int *a[3]; int b; b = 8; a[1] = &b; return *(a[1]); }"

echo "グローバル変数"
assert 0 "int a;    int main() { return a; }"
assert 1 "int a;    int main() { a = 1; return a; }"
assert 2 "int* a;   int main() { int b; b = 2; a = &b; return *a; }"
assert 3 "int a[3]; int main() { a[1] = 3; return a[1]; }"
assert 4 "int a;    int f() { return a; } int main() { a = 4; return f(); }"
assert 0 "int a;    int f() { return a; } int main() { int a; a = 0; return f(); }"
assert 5 "int a;    int main() { int* b; b = &a; *b = 5; return a; }"

echo OK
