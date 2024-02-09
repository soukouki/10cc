#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "10cc.h"

// 現在着目しているトークン
Token* token;

// 入力プログラム
char* user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所の報告付きエラー
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  printf("# tokenize\n");
  token = tokenize(user_input);
  printf("# parse\n");
  Node* code = program();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  
  gen(code);

  return 0;
}
