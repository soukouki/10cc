#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  char* p = argv[1];

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", strtol(p, &p, 10));

  while(*p) {
    if(*p == '+') {
      p++;
      printf("  add rax, %d\n", strtol(p, &p, 10));
      continue;
    } else if(*p == '-') {
      p++;
      printf("  sub rax, %d\n", strtol(p, &p, 10));
      continue;
    } else {
      fprintf(stderr, "予期しない文字です: '%c'\n", *p);
      return 1;
    }
  }

  printf("  ret\n");
  return 0;
}
