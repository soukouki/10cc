// テストに、その段階では書けないコードが必要な場合に使う

#include <stdio.h>

int call0() {
  printf("call0 OK\n");
  return 100;
}

int call1(int a) {
  printf("call1 %d\n", a);
  return a + 100;
}

int call2(int a, int b) {
  printf("call2 %d %d\n", a, b);
  return a + b;
}

int call3(int a, int b, int c) {
  printf("call3 %d %d %d\n", a, b, c);
  return a + b + c;
}

int call4(int a, int b, int c, int d) {
  printf("call4 %d %d %d %d\n", a, b, c, d);
  return a + b + c + d;
}

int call5(int a, int b, int c, int d, int e) {
  printf("call5 %d %d %d %d %d\n", a, b, c, d, e);
  return a + b + c + d + e;
}

int call6(int a, int b, int c, int d, int e, int f) {
  printf("call6 %d %d %d %d %d %d\n", a, b, c, d, e, f);
  return a + b + c + d + e + f;
}
