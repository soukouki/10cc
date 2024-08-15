
// ここから#includeの代わりに展開した部分

typedef long size_t;

void exit();
void* calloc();
char* strerror();

// stdio.h
struct _IO_FILE {};
typedef struct _IO_FILE FILE;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
int fprintf();
int printf();

typedef struct Token Token;
typedef struct Node Node;
char* read_file(char *path);
Token* tokenize(char *p);
Node* parse();
Node* analyze_semantics(Node* node);
void gen(Node* node);

char true = 1;
char false = 0;

// ここまで#includeの代わりに展開した部分

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "10cc.h"

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
  int space_len = end - line;
  fprintf(stderr, "%.*s\n", space_len, line);

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

int main(int argc, char **argv) {
  node_kinds = calloc(1, sizeof(char*) * 100);
  int i = 0;

  node_kinds[i++] = "ND_ADD";
  node_kinds[i++] = "ND_SUB";
  node_kinds[i++] = "ND_MUL";
  node_kinds[i++] = "ND_DIV";
  node_kinds[i++] = "ND_MOD";
  node_kinds[i++] = "ND_EQ";
  node_kinds[i++] = "ND_NE";
  node_kinds[i++] = "ND_LT";
  node_kinds[i++] = "ND_LE";
  node_kinds[i++] = "ND_LAND";
  node_kinds[i++] = "ND_LOR";
  node_kinds[i++] = "ND_ASSIGN";
  node_kinds[i++] = "ND_ASSIGN_ADD";
  node_kinds[i++] = "ND_ASSIGN_SUB";
  node_kinds[i++] = "ND_ASSIGN_MUL";
  node_kinds[i++] = "ND_ASSIGN_DIV";
  node_kinds[i++] = "ND_ASSIGN_MOD";

  node_kinds[i++] = "ND_NOT";
  node_kinds[i++] = "ND_ADDR";
  node_kinds[i++] = "ND_DEREF";
  node_kinds[i++] = "ND_DOT";

  node_kinds[i++] = "ND_NUM";
  node_kinds[i++] = "ND_STR";
  node_kinds[i++] = "ND_CHAR";

  node_kinds[i++] = "ND_CALL";
  node_kinds[i++] = "ND_SIZEOF";
  node_kinds[i++] = "ND_OFFSETOF";

  node_kinds[i++] = "ND_VARREF";
  node_kinds[i++] = "ND_GVARREF";
  node_kinds[i++] = "ND_ARRAYREF";
  node_kinds[i++] = "ND_RETURN";
  node_kinds[i++] = "ND_IF";
  node_kinds[i++] = "ND_WHILE";
  node_kinds[i++] = "ND_FOR";
  node_kinds[i++] = "ND_SWITCH";
  node_kinds[i++] = "ND_BLOCK";
  node_kinds[i++] = "ND_FUNCDEF";
  node_kinds[i++] = "ND_FUNCPROT";
  node_kinds[i++] = "ND_STRDEF";
  node_kinds[i++] = "ND_STRUCT";
  node_kinds[i++] = "ND_ENUM";
  node_kinds[i++] = "ND_BREAK";
  node_kinds[i++] = "ND_CONTINUE";
  node_kinds[i++] = "ND_CASE";
  node_kinds[i++] = "ND_DEFAULT";

  node_kinds[i++] = "ND_DECL";
  node_kinds[i++] = "ND_GDECL";
  node_kinds[i++] = "ND_GDECL_EXTERN";
  node_kinds[i++] = "ND_TYPE";
  node_kinds[i++] = "ND_IDENT";
  node_kinds[i++] = "ND_CONVERT";

  node_kinds[i++] = "ND_PROGRAM";

  i = 0;
  type_kinds = calloc(1, sizeof(char*) * 10);
  
  type_kinds[i++] = "TY_CHAR";
  type_kinds[i++] = "TY_INT";
  type_kinds[i++] = "TY_LONG";
  type_kinds[i++] = "TY_PTR";
  type_kinds[i++] = "TY_ARR";
  type_kinds[i++] = "TY_STRUCT";
  type_kinds[i++] = "TY_VOID";

  user_input = read_file(argv[1]);
  filename = argv[1];
  
  printf("# tokenize\n");
  token = tokenize(user_input);
  printf("# parse\n");
  Node* code = parse();
  printf("# analyze semantics\n");
  Node* analyzed_code = analyze_semantics(code);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  gen(analyzed_code);

  return 0;
}
