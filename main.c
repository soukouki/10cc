#include <stdio.h>
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

// 入力ファイル名
char *filename;


// エラーを報告するための関数
// printfと同じ引数を取る
void error3(char* file_name, int file_line, char* fmt, char* arg1, char* arg2, char* arg3) {
  fprintf(stderr, fmt, arg1, arg2, arg3);
  fprintf(stderr, "\nerror reported at %s:%d\n", file_name, file_line);
  exit(1);
}
void error0(char* file_name, int file_line, char* fmt) {
  error3(file_name, file_line, fmt, "", "", "");
}
void error1(char* file_name, int file_line, char* fmt, char* arg1) {
  error3(file_name, file_line, fmt, arg1, "", "");
}
void error2(char* file_name, int file_line, char* fmt, char* arg1, char* arg2) {
  error3(file_name, file_line, fmt, arg1, arg2, "");
}

// エラーの起きた場所を報告するための関数
// 下のようなフォーマットでエラーメッセージを表示する
//
// foo.c:10: x = y + + 5;
//                   ^ 式ではありません
void error_at2(char* file_name, int file_line, char* loc, char* fmt, char* arg1, char* arg2) {
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
  fprintf(stderr, fmt, arg1, arg2);
  fprintf(stderr, "\nerror reported at %s:%d\n", file_name, file_line);
  exit(1);
}
void error_at0(char* file_name, int file_line, char *loc, char* fmt) {
  error_at2(file_name, file_line, loc, fmt, "", "");
}
void error_at1(char* file_name, int file_line, char *loc, char* fmt, char* arg1) {
  error_at2(file_name, file_line, loc, fmt, arg1, "");
}

// 指定されたファイルの内容を返す
char *read_file(char *path) {
  // ファイルを開く
  FILE *fp = fopen(path, "r");
  if (!fp)
    error2(__FILE__, __LINE__, "cannot open %s: %s", path, strerror(errno));

  // ファイルの長さを調べる
  if (fseek(fp, 0, SEEK_END) == -1)
    error2(__FILE__, __LINE__, "%s: fseek: %s", path, strerror(errno));
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    error2(__FILE__, __LINE__, "%s: fseek: %s", path, strerror(errno));

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
    "ND_DOT",

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
    "ND_SWITCH",
    "ND_BLOCK",
    "ND_FUNCDEF",
    "ND_FUNCPROT",
    "ND_STRDEF",
    "ND_STRUCT",
    "ND_ENUM",
    "ND_BREAK",
    "ND_CONTINUE",
    "ND_CASE",
    "ND_DEFAULT",

    "ND_DECL",
    "ND_GDECL",
    "ND_GDECL_EXTERN",
    "ND_TYPE",
    "ND_IDENT",

    "ND_PROGRAM",
  };

  type_kinds = (char*[]){
    "TY_CHAR",
    "TY_INT",
    "TY_PTR",
    "TY_ARR",
    "TY_STRUCT",
    "TY_VOID",
  };

  user_input = read_file(argv[1]);
  filename = argv[1];
  
  printf("# tokenize\n");
  token = tokenize(user_input, filename);
  printf("# parse\n");
  Node* code = parse();
  printf("# analyze semantics\n");
  Node* analyzed_code = analyze_semantics(code);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  gen(analyzed_code);

  return 0;
}
