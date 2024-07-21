
// ここから#includeの代わりに展開した部分

extern void* NULL;
typedef char bool;
extern char true;
extern char false;
void* calloc();
int strlen();
int printf();

typedef struct Map Map;

Map* map_new();
void* map_get (Map* map, const char* key);
void map_put (Map* map, const char* key, void* value); // 同じキーがある場合はうまく動作しない
void map_delete(Map* map, const char* key);
void** map_values(Map* map);
char** map_keys(Map* map);

// ここを書き換えたときには、main.cのtype_kindsも書き換えること！
typedef enum TypeKind TypeKind;
enum TypeKind {
  TY_CHAR,
  TY_INT,
  TY_LONG,
  TY_PTR,
  TY_ARRAY,
  TY_STRUCT,
  TY_VOID,
};

typedef struct Type Type;
struct Type {
  TypeKind kind;
  Type* ptr_to;
  int array_size;
  char* struct_name;
};

typedef struct StructMember StructMember;
struct StructMember {
  Type* type;
  int offset;
};

typedef struct Struct Struct;
struct Struct {
  char* name;
  Map* members;
  int size;
};

typedef struct Var Var;
struct Var {
  char* name;
  int offset; // 意味解析時に計算される, ローカル変数の場合のみ
  Type* type; // 意味解析時でのみ使う
  int size;
  bool is_extern;
};

typedef enum NodeKind NodeKind;
// ここを書き換えたときには、main.cのnode_kindsも書き換えること！
enum NodeKind {
  // 2項演算子(lhs, rhsを持つ)
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_MOD,
  // 比較演算子
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,     // GT, GEはLT, LEを使って表現できる
  // 論理演算子
  ND_LAND,
  ND_LOR,
  // 代入演算子
  ND_ASSIGN,     // 代入, lhs, rhsを持つ
  ND_ASSIGN_ADD, // 加算代入, lhs, rhsを持つ
  ND_ASSIGN_SUB, // 減算代入, lhs, rhsを持つ
  ND_ASSIGN_MUL, // 乗算代入, lhs, rhsを持つ
  ND_ASSIGN_DIV, // 除算代入, lhs, rhsを持つ
  ND_ASSIGN_MOD, // 剰余代入, lhs, rhsを持つ
  // 単項演算子(lhsを持つ)
  ND_SIZEOF, // sizeof演算子, lhsを持つ
  ND_NOT,    // 単項not, lhsを持つ
  // ポインタ
  ND_ADDR,   // 単項&, lhsを持つ
  ND_DEREF,  // 単項*, lhsを持つ
  // 構造体
  ND_DOT,    // lhs, nameを持つ

  // リテラル
  ND_NUM, // valを持つ
  ND_STR, // valを持つ
  ND_CHAR,// valを持つ(意味解析時にND_NUMに置き換える)

  // 構文
  ND_VARREF,   // 変数の参照, name, varを持つ
  ND_GVARREF,  // グローバル変数の参照, name, varを持つ
  ND_ARRAYREF, // 配列の参照, lhs, rhsを持つ
  ND_CALL,     // 関数呼び出し, name, args_callを持つ
  ND_RETURN,   // return, lhsを持つ
  ND_IF,       // if文, cond, then, elsを持つ
  ND_WHILE,    // while文, cond, bodyを持つ
  ND_FOR,      // for文, init, cond, inc, bodyを持つ
  ND_SWITCH,   // switch文, cond, bodyを持つ
  ND_BLOCK,    // ブロック, stmtsを持つ
  ND_FUNCDEF,  // 関数定義, name, args_name, args_type, ret_type, bodyを持つ
  ND_FUNCPROT, // 関数プロトタイプ, name, args_name, args_type, ret_typeを持つ
  ND_STRDEF,   // 文字列定義, name, str_valを持つ
  ND_STRUCT,   // 構造体, name, struct_membersを持つ
  ND_ENUM,     // 列挙型, name, enum_membersを持つ
  ND_BREAK,    // break, goto_labelを持つ
  ND_CONTINUE, // continue, goto_labelを持つ
  ND_CASE,     // case, int_val, goto_labelを持つ
  ND_DEFAULT,  // default, goto_labelを持つ

  ND_DECL,         // 宣言, name, type, var, lhs, rhs(Nullable)を持つ 変数定義や関数の仮引数で使う
  ND_GDECL,        // グローバル変数宣言, name, type, var, lhs(Nullable)を持つ
  ND_GDECL_EXTERN, // グローバル変数宣言(外部参照), name, typeを持つ
  ND_TYPE,         // 型, typeを持つ
  ND_IDENT,        // 識別子(意味解析時に置き換える), nameを持つ

  // その他
  ND_PROGRAM, // プログラム全体, funcs, stringsを持つ
};

typedef struct Node Node;
struct Node {
  NodeKind kind;
  char*  loc;            // エラー表示用
  Node*  init;           // forで使う
  Node*  inc;            // forで使う
  Node*  cond;           // if, while, for, switchで使う
  Node*  then;           // ifで使う
  Node*  els;            // ifで使う
  Node*  body;           // while, for, switchで使う
  Map*   case_map;       // switchで使う ラベルをキーにして、caseの値を値にもつ
  bool   has_default;    // switchで使う
  Node** stmts;          // ブロックで使う
  Node** args_call;      // 関数呼び出しで使う
  Node** funcs;          // プログラムで使う
  Node** strings;        // プログラムで使う
  Node** struct_members; // 構造体のメンバ, 中身はND_DECL
  Node** enum_members;   // 列挙型のメンバ, 中身はND_IDENT
  Node*  lhs;            // 2項演算子, 代入(型), return, 単項&, 単項*, ポインタ型, case, defaultで使う
  Node*  rhs;            // 2項演算子, 代入(値)で使う
  int    int_val;        // 数値リテラル, caseで使う
  char*  str_val;        // 文字列リテラルの場合に使う
  int    str_key;        // 文字列リテラルの場合に使う
  char*  name;           // 関数の定義, 関数呼び出し, 変数の参照, caseで使う
  Var*   var;            // ND_LVARの場合に使う
  Type*  type;           // ND_TYPE, ND_FUNCDEF, ND_FUNCPROT(戻り値), ND_DECLで使う
  Node** args_node;      // 関数の定義で使う(パース->意味解析)
  Var**  args_var;       // 関数の定義で使う(意味解析->コード生成)
  int    offset;         // 関数の定義で使う(意味解析->コード生成)
  char*  goto_label;     // break, continue, case, defaultで使う(意味解析->コード生成)
  int    local_label;    // if, while, for, &&, ||で使う(意味解析->コード生成)
  StructMember* struct_member; // ND_DOTの場合に使う
};

void error0(char* file, int line, char* fmt);
void error1(char* file, int line, char* fmt, char* arg1);
void error2(char* file, int line, char* fmt, char* arg1, char* arg2);
void error3(char* file, int line, char* fmt, char* arg1, char* arg2, char* arg3);
void error_at0(char* file, int line, char* loc, char* fmt);
void error_at1(char* file, int line, char* loc, char* fmt, char* arg1);
void error_at2(char* file, int line, char* loc, char* fmt, char* arg1, char* arg2);

extern char** node_kinds;

void gen(Node* node);

// ここまで#includeの代わりに展開した部分

// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include "10cc.h"

char* escape(char*);

void gen_ref(Node* node) {
  printf("# gen_ref %s\n", node_kinds[node->kind]);
  switch(node->kind) {
  case ND_DEREF:
    gen(node->lhs);
    break;
  case ND_VARREF:
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->var->offset);
    printf("  push rax\n");
    break;
  case ND_GVARREF:
    printf("  push offset %s\n", node->name);
    break;
  case ND_DOT: {
    Type* type = node->lhs->type;
    if(type->kind != TY_STRUCT) {
      error_at0(__FILE__, __LINE__, node->loc, "構造体の参照ではありません");
    }
    StructMember* member = node->struct_member;
    gen_ref(node->lhs);
    printf("  pop rax\n");
    printf("  add rax, %d\n", member->offset);
    printf("  push rax\n");
    break;
  }
  default:
    error_at1(__FILE__, __LINE__, node->loc, "%sは変数の参照ではありません", node_kinds[node->kind]);
    break;
  }
}

void gen_ref_push(Node* node) {
  printf("  pop rax\n");
  if(node->type->kind == TY_INT) {
    printf("  mov eax, [rax]\n");
  } else if(node->type->kind == TY_CHAR) {
    printf("  movzx eax, BYTE PTR [rax]\n");
  } else {
    printf("  mov rax, [rax]\n");
  }
  printf("  push rax\n");
}

void gen_assign(Node* lval, Node* rval) {
  gen_ref(lval);
  gen(rval);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  if(lval->type->kind == TY_INT) {
    printf("  mov [rax], edi\n");
  } else if(lval->type->kind == TY_CHAR) {
    printf("  mov [rax], dil\n");
  } else {
    printf("  mov [rax], rdi\n");
  }
  printf("  push rdi\n");
}

void gen(Node* node) {
  if(node == NULL) {
    error0(__FILE__, __LINE__, "ノードがありません");
  }
  printf("# gen %s\n", node_kinds[node->kind]);
  switch (node->kind) {
  case ND_NUM: {
    printf("  push %d\n", node->int_val);
    break;
  }
  case ND_STR: {
    printf("  push offset .LC%d # \"%s\"\n", node->str_key, escape(node->str_val));
    break;
  }
  case ND_ADDR: {
    gen_ref(node->lhs);
    break;
  }
  case ND_DEREF: {
    gen(node->lhs);
    gen_ref_push(node);
    break;
  }
  case ND_DOT: {
    gen_ref(node);
    gen_ref_push(node);
    break;
  }
  case ND_VARREF:
  case ND_GVARREF: {
    gen_ref(node);
    if(node->type->kind == TY_ARRAY) {
      return;
    }
    gen_ref_push(node);
    break;
  }
  case ND_LAND: {
    int land_label = node->local_label;
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lfalse%d\n", land_label);
    gen(node->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lfalse%d\n", land_label);
    printf("  push 1\n");
    printf("  jmp .Lend%d\n", land_label);
    printf(".Lfalse%d:\n", land_label);
    printf("  push 0\n");
    printf(".Lend%d:\n", land_label);
    break;
  }
  case ND_LOR: {
    int lor_label = node->local_label;
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  jne .Ltrue%d\n", lor_label);
    gen(node->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  jne .Ltrue%d\n", lor_label);
    printf("  push 0\n");
    printf("  jmp .Lend%d\n", lor_label);
    printf(".Ltrue%d:\n", lor_label);
    printf("  push 1\n");
    printf(".Lend%d:\n", lor_label);
    break;
  }
  case ND_ASSIGN: {
    printf("# ND_ASSIGN %s %s\n", node_kinds[node->lhs->kind], node_kinds[node->rhs->kind]);
    gen_assign(node->lhs, node->rhs);
    break;
  }
  case ND_ASSIGN_ADD: {
    gen_ref(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  add [rax], rdi\n");
    printf("  mov edi, [rax]\n");
    printf("  push rdi\n");
    break;
  }
  case ND_ASSIGN_SUB: {
    gen_ref(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  sub [rax], rdi\n");
    printf("  mov edi, [rax]\n");
    printf("  push rdi\n");
    break;
  }
  case ND_ASSIGN_MUL: {
    gen_ref(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  imul [rax], rdi\n");
    printf("  mov edi, [rax]\n");
    printf("  push rdi\n");
    break;
  }
  case ND_ASSIGN_DIV: {
    error0(__FILE__, __LINE__, "未実装");
  }
  case ND_ASSIGN_MOD: {
    error0(__FILE__, __LINE__, "未実装");
  }
  case ND_NOT: {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    printf("  push rax\n");
    break;
  }
  case ND_RETURN: {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    break;
  }
  case ND_IF: {
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    int if_label = node->local_label;
    if(node->els) {
      printf("  je  .Lelse%d\n", if_label);
      gen(node->then);
      printf("  jmp .Lend%d\n", if_label);
      printf(".Lelse%d:\n", if_label);
      gen(node->els);
    } else {
      printf("  je  .Lend%d\n", if_label);
      gen(node->then);
    }
    printf(".Lend%d:\n", if_label);
    break;
  }
  case ND_WHILE: {
    int while_label = node->local_label;
    printf(".Lbegin%d:\n", while_label);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", while_label);
    gen(node->body);
    printf("  jmp .Lbegin%d\n", while_label);
    printf(".Lend%d:\n", while_label);
    break;
  }
  case ND_FOR: {
    int for_label = node->local_label;
    if(node->init) gen(node->init);
    printf(".Lbegin%d:\n", for_label);
    if(node->cond) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", for_label);
    }
    gen(node->body);
    if(node->inc) gen(node->inc);
    printf("  jmp .Lbegin%d\n", for_label);
    printf(".Lend%d:\n", for_label);
    break;
  }
  case ND_SWITCH: {
    int switch_label = node->local_label;
    gen(node->cond);
    printf("  pop rax\n");
    char** keys = map_keys(node->case_map);
    for(int i = 0; keys[i]; i++) {
      int* val = map_get(node->case_map, keys[i]);
      printf("  cmp rax, %d\n", *val);
      printf("  je %s\n", keys[i]);
    }
    if(node->has_default) {
      printf("  jmp .Ldefault%d\n", switch_label);
    } else {
      printf("  jmp .Lend%d\n", switch_label);
    }
    gen(node->body);
    printf(".Lend%d:\n", switch_label);
    break;
  }
  case ND_BLOCK: {
    for(int i = 0; node->stmts[i]; i++) {
      gen(node->stmts[i]);
    }
    break;
  }
  case ND_CALL: {
    if(node->args_call[0]) {
      gen(node->args_call[0]);
    }
    if(node->args_call[1]) {
      gen(node->args_call[1]);
    }
    if(node->args_call[2]) {
      gen(node->args_call[2]);
    }
    if(node->args_call[3]) {
      gen(node->args_call[3]);
    }
    if(node->args_call[4]) {
      gen(node->args_call[4]);
    }
    if(node->args_call[5]) {
      gen(node->args_call[5]);
    }
    if(node->args_call[5]) {
      printf("  pop r9\n");
    }
    if(node->args_call[4]) {
      printf("  pop r8\n");
    }
    if(node->args_call[3]) {
      printf("  pop rcx\n");
    }
    if(node->args_call[2]) {
      printf("  pop rdx\n");
    }
    if(node->args_call[1]) {
      printf("  pop rsi\n");
    }
    if(node->args_call[0]) {
      printf("  pop rdi\n");
    }
    printf("  mov al, 0\n"); // 浮動小数点数の数をALに入れる必要があるが、今は扱わないので0を入れておく
    printf("  call %s@PLT\n", node->name);
    printf("  push rax\n");
    break;
  }
  case ND_FUNCDEF: {
    printf("  .text\n");
    printf("  .globl %s\n", node->name);
    printf("%s:\n", node->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", node->offset);
    if(node->args_var[0]) {
      if(node->args_var[0]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], edi\n", node->args_var[0]->offset);
      } else if(node->args_var[0]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], dil\n", node->args_var[0]->offset);
      } else {
        printf("  mov [rbp-%d], rdi\n", node->args_var[0]->offset);
      }
    }
    if(node->args_var[1]) {
      if(node->args_var[1]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], esi\n", node->args_var[1]->offset);
      } else if(node->args_var[1]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], sil\n", node->args_var[1]->offset);
      } else {
        printf("  mov [rbp-%d], rsi\n", node->args_var[1]->offset);
      }
    }
    if(node->args_var[2]) {
      if(node->args_var[2]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], edx\n", node->args_var[2]->offset);
      } else if(node->args_var[2]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], dl\n", node->args_var[2]->offset);
      } else {
        printf("  mov [rbp-%d], rdx\n", node->args_var[2]->offset);
      }
    }
    if(node->args_var[3]) {
      if(node->args_var[3]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], ecx\n", node->args_var[3]->offset);
      } else if(node->args_var[3]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], cl\n", node->args_var[3]->offset);
      } else {
        printf("  mov [rbp-%d], rcx\n", node->args_var[3]->offset);
      }
    }
    if(node->args_var[4]) {
      if(node->args_var[4]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], r8d\n", node->args_var[4]->offset);
      } else if(node->args_var[4]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], r8b\n", node->args_var[4]->offset);
      } else {
        printf("  mov [rbp-%d], r8\n", node->args_var[4]->offset);
      }
    }
    if(node->args_var[5]) {
      if(node->args_var[5]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], r9d\n", node->args_var[5]->offset);
      } else if(node->args_var[5]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], r9b\n", node->args_var[5]->offset);
      } else {
        printf("  mov [rbp-%d], r9\n", node->args_var[5]->offset);
      }
    }
    gen(node->body);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    break;
  }
  case ND_FUNCPROT: {
    break;
  }
  case ND_STRDEF: {
    printf("  .data\n");
    printf(".LC%d:\n", node->str_key);
    printf("  .string \"%s\"\n", escape(node->str_val));
    break;
  }
  case ND_STRUCT: {
    break;
  }
  case ND_ENUM: {
    break;
  }
  case ND_PROGRAM: {
    for(int i = 0; node->strings[i]; i++) {
      gen(node->strings[i]);
    }
    for(int i = 0; node->funcs[i]; i++) {
      gen(node->funcs[i]);
    }
    break;
  }
  case ND_DECL: {
    if(!node->rhs) break;
    gen_assign(node->lhs, node->rhs);
    break;
  }
  case ND_GDECL: {
    printf("  .data\n");
    printf("  .globl %s\n", node->name);
    printf("%s:\n", node->name);
    printf("  .zero %d\n", node->var->size);
    break;
  }
  case ND_GDECL_EXTERN: {
    break;
  }
  case ND_BREAK: {
    printf("  jmp %s\n", node->goto_label);
    break;
  }
  case ND_CONTINUE: {
    printf("  jmp %s\n", node->goto_label);
    break;
  }
  case ND_CASE: {
    printf("%s:\n", node->goto_label);
    gen(node->lhs);
    break;
  }
  case ND_DEFAULT: {
    printf("%s:\n", node->goto_label);
    gen(node->lhs);
    break;
  }
  default: {
    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->kind) {
      case ND_ADD:
        printf("  add rax, rdi\n");
        break;
      case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
      case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
      case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
      case ND_MOD:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        printf("  mov rax, rdx\n");
        break;
      case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
      case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
      case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
      case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
      default:
        error_at1(__FILE__, __LINE__, node->loc, "%sは対応していないノードです", node_kinds[node->kind]);
        break;
    }
    printf("  push rax\n");
    break;
  }
  }

  printf("# endgen %s\n", node_kinds[node->kind]);
}

char* escape(char* str) {
  int len = strlen(str);
  char* buf = calloc(len * 2 + 1, sizeof(char));
  int i = 0;
  int j = 0;
  for(; i < len;) {
    if(str[i] == '\n') {
      buf[j++] = '\\';
      buf[j] = 'n';
    } else if(str[i] == '\t') {
      buf[j++] = '\\';
      buf[j] = 't';
    } else if(str[i] == '"') {
      buf[j++] = '\\';
      buf[j] = '"';
    } else if(str[i] == '\\') {
      buf[j++] = '\\';
      buf[j] = '\\';
    } else {
      buf[j] = str[i];
    }
    i++;
    j++;
  }
  return buf;
}
