// テストに、その段階では書けないコードが必要な場合に使う

#include <stdio.h>
#include <stdlib.h>

int call0() {
  return 100;
}

int call1(int a) {
  return a + 100;
}

int call2(int a, int b) {
  return a + b;
}

int call3(int a, int b, int c) {
  return a + b + c;
}

int call4(int a, int b, int c, int d) {
  return a + b + c + d;
}

int call5(int a, int b, int c, int d, int e) {
  return a + b + c + d + e;
}

int call6(int a, int b, int c, int d, int e, int f) {
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
