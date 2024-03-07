#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "10cc.h"

// 入力ファイル名
char *filename;

// 現在着目しているトークン
Token* token;

// 入力プログラム
char* user_input;

char** node_kinds;
char** type_kinds;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char* file_name, int file_line, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\nerror reported at %s:%d\n", file_name, file_line);
  exit(1);
}

// 入力ファイル名
char *filename;

// エラーの起きた場所を報告するための関数
// 下のようなフォーマットでエラーメッセージを表示する
//
// foo.c:10: x = y + + 5;
//                   ^ 式ではありません
void error_at(char* file_name, int file_line, char *loc, char *msg, ...) {
  va_list ap;
  va_start(ap, msg);

  // locが含まれている行の開始地点と終了地点を取得
  char *line = loc;
  while (user_input < line && line[-1] != '\n')
    line--;

  char* end = loc;
  while (*end != '\n')
    end++;

  // 見つかった行が全体の何行目なのかを調べる
  int line_num = 1;
  for (char *p = user_input; p < line; p++)
    if (*p == '\n')
      line_num++;

  // 見つかった行を、ファイル名と行番号と一緒に表示
  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // エラー箇所を"^"で指し示して、エラーメッセージを表示
  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\nerror reported at %s:%d\n", file_name, file_line);
  exit(1);
}

// 指定されたファイルの内容を返す
char *read_file(char *path) {
  // ファイルを開く
  FILE *fp = fopen(path, "r");
  if (!fp)
    ERROR("cannot open %s: %s", path, strerror(errno));

  // ファイルの長さを調べる
  if (fseek(fp, 0, SEEK_END) == -1)
    ERROR("%s: fseek: %s", path, strerror(errno));
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    ERROR("%s: fseek: %s", path, strerror(errno));

  // ファイル内容を読み込む
  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  // ファイルが必ず"\n\0"で終わっているようにする
  if (size == 0 || buf[size - 1] != '\n')
    buf[size++] = '\n';
  buf[size] = '\0';
  fclose(fp);
  return buf;
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
    "ND_MOD",
    "ND_EQ",
    "ND_NE",
    "ND_LT",
    "ND_LE",

    "ND_SIZEOF",
    "ND_ADDR",
    "ND_DEREF",

    "ND_NUM",
    "ND_STR",

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
    "ND_STRDEF",

    "ND_DECL",
    "ND_GDECL",
    "ND_TYPE",
    "ND_IDENT",

    "ND_PROGRAM",
  };

  type_kinds = (char*[]){
    "TY_INT",
    "TY_CHAR",
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
      ERROR("不明なオプションです: %s", argv[1]);
    }
    user_input = read_file(argv[1]);
    filename = argv[1];
  } else if(argc != 2) {
    ERROR("引数の個数が正しくありません");
  } else {
    user_input = read_file(argv[1]);
    filename = argv[1];
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
