// テストに、その段階では書けないコードが必要な場合に使う

#include <stdio.h>
#include <stdlib.h>

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

int* alloc_int(int a, int b, int c, int d) {
  int* p = malloc(4 * sizeof(int));
  p[0] = a;
  p[1] = b;
  p[2] = c;
  p[3] = d;
  return p;
}

int** alloc_ptr(int a, int b, int c, int d) {
  int** p = malloc(4 * sizeof(int*));
  p[0] = malloc(sizeof(int));
  p[1] = malloc(sizeof(int));
  p[2] = malloc(sizeof(int));
  p[3] = malloc(sizeof(int));
  *p[0] = a;
  *p[1] = b;
  *p[2] = c;
  *p[3] = d;
  return p;
}
