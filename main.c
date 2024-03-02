#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "10cc.h"

// 現在着目しているトークン
Token* token;

// 入力プログラム
char* user_input;

char** node_kinds;
char** type_kinds;

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

typedef enum {
  RUN_NORMAL,
  RUN_PARSE,
  RUN_ANALYZE,
} RunMode;

int main(int argc, char **argv) {
  node_kinds = (char*[]){
    "ND_ADD",
    "ND_SUB",
    "ND_MUL",
    "ND_DIV",
    "ND_EQ",
    "ND_NE",
    "ND_LT",
    "ND_LE",

    "ND_SIZEOF",
    "ND_ADDR",
    "ND_DEREF",

    "ND_NUM",

    "ND_VARREF",
    "ND_GVARREF",
    "ND_ARRAYREF",
    "ND_CALL",
    "ND_ASSIGN",
    "ND_RETURN",
    "ND_IF",
    "ND_WHILE",
    "ND_FOR",
    "ND_BLOCK",
    "ND_FUNCDEF",
    "ND_FUNCPROT",

    "ND_DECL",
    "ND_GDECL",
    "ND_TYPE",
    "ND_IDENT",

    "ND_PROGRAM",
  };

  type_kinds = (char*[]){
    "TY_INT",
    "TY_PTR",
    "TY_ARR",
  };

  RunMode mode = RUN_NORMAL;
  if(argc == 3) {
    if(strcmp(argv[2], "-p") == 0) {
      mode = RUN_PARSE;
    } else if(strcmp(argv[2], "-a") == 0) {
      mode = RUN_ANALYZE;
    } else {
      error("不明なオプションです: %s", argv[1]);
    }
    user_input = argv[1];
  } else if(argc != 2) {
    error("引数の個数が正しくありません");
  } else {
    user_input = argv[1];
  }

  printf("# tokenize\n");
  token = tokenize(user_input);
  printf("# parse\n");
  Node* code = parse();
  if(mode == RUN_PARSE) {
    print_node(code);
    return 0;
  }
  printf("# analyze semantics\n");
  Node* analyzed_code = analyze_semantics(code);
  if(mode == RUN_ANALYZE) {
    print_node(analyzed_code);
    return 0;
  }

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  gen(analyzed_code);

  return 0;
}
