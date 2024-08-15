
// ここから#includeの代わりに展開した部分

extern void* NULL;
typedef char bool;
extern char true;
extern char false;
int strcmp();
int printf();
int strncmp();
void* calloc();
int strncpy();
int strlen();

typedef struct Map Map;

Map* map_new();
void* map_get (Map* map, const char* key);
void map_put (Map* map, const char* key, void* value); // 同じキーがある場合はうまく動作しない
void map_delete(Map* map, const char* key);
void** map_values(Map* map);
char** map_keys(Map* map);

typedef enum TokenKind TokenKind;
enum TokenKind {
  TK_SYMBOL,
  TK_IDENT,
  TK_NUM,
  TK_STR,
  TK_CHAR,
  TK_EOF,
};

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token* next;
  int val;
  char* str;
  int len;
  char* file;
  int line;
};

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

  // 関数・コンパイラマジック
  ND_CALL,     // 関数呼び出し, name, args_callを持つ
  ND_SIZEOF,   // sizeof, lhsを持つ
  ND_OFFSETOF, // offsetof, lhs, nameを持つ

  // 構文
  ND_VARREF,   // 変数の参照, name, varを持つ
  ND_GVARREF,  // グローバル変数の参照, name, varを持つ
  ND_ARRAYREF, // 配列の参照, lhs, rhsを持つ
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
  ND_CONVERT,  // 符号拡張, lhs, old_type, new_typeを持つ

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
  // Map<char*, int>
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
  char*  name;           // 関数の定義, 関数呼び出し, 変数の参照, case, offsetofで使う
  Var*   var;            // ND_LVARの場合に使う
  Type*  type;           // ND_TYPE, ND_FUNCDEF, ND_FUNCPROT(戻り値), ND_DECLで使う
  Type*  old_type;       // ND_CONVERTで使う
  Type*  new_type;       // ND_CONVERTで使う
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

Type* char_type();
Type* int_type();
Type* long_type();
Type* void_type();
Type* ptr_type(Type* ptr_to);
Type* arr_type(Type* ptr_to, int array_size);
Type* struct_type(char* name);

extern Token *token;

// ここまで#includeの代わりに展開した部分

// #include <stdbool.h>
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include "10cc.h"

// Map<name, Node(Type)>
Map* typedef_map;

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて真を返す
// それ以外の場合には偽を返す
static bool consume(char* op) {
  if(
    token->kind != TK_SYMBOL ||
    strlen(op) != token->len ||
    strncmp(token->str, op, token->len) != 0
  ) return false;

  token = token->next;
  return true;
}

// 次のトークンが識別子のときには、識別子の名前を返し、トークンを1つ読み進める
// それ以外の場合にはNULLを返す
static char* consume_ident() {
  if(token->kind != TK_IDENT) return NULL;
  char* name = calloc(token->len + 1, sizeof(char));
  strncpy(name, token->str, token->len);
  token = token->next;
  return name;
}

// 次のトークンが期待している記号のときには。トークンを1つ読み進める
// それ以外の場合にはエラーを報告する
static void expect(char* op) {
  if(
    token->kind != TK_SYMBOL ||
    strlen(op) != token->len ||
    strncmp(token->str, op, token->len) != 0
  ) error_at1(__FILE__, __LINE__, token->str, "'%s'ではありません", op);

  token = token->next;
  return;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す
// それ以外の場合にはエラーを報告する
static int expect_number() {
  if(token->kind != TK_NUM) {
    error_at0(__FILE__, __LINE__, token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

// 次のトークンが期待している記号のときには真を返す
static bool is_next(char* op) {
  if(
    token->kind != TK_SYMBOL ||
    strlen(op) != token->len ||
    strncmp(token->str, op, token->len) != 0
  ) return false;
  return true;
}

static bool at_eof() {
  return token->kind == TK_EOF;
}

Node* new_node(NodeKind kind, char* loc) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->loc = loc;
  return node;
}

Node* new_node_1branch(NodeKind kind, char* loc, Node* lhs) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->loc = loc;
  node->lhs = lhs;
  return node;
}

Node* new_node_2branches(NodeKind kind, char* loc, Node* lhs, Node* rhs) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->loc = loc;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_node_num(char* loc, int val) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->loc = loc;
  node->int_val = val;
  return node;
}

Node* new_node_str(char* loc, char* str) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_STR;
  node->loc = loc;
  node->str_val = str;
  node->str_key = -1;
  return node;
}

Node* new_node_char(char* loc, int val) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_CHAR;
  node->loc = loc;
  node->int_val = val;
  return node;
}

Node* new_node_ident(NodeKind kind, char* loc, char* name) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->loc = loc;
  node->name = name;
  return node;
}

/*
program    = (func | extern? decl ";" | struct ";" | enum ";" | typedef ";")*
func       = decl "(" (param ("," param)*)? ")" ("{" block | ";")
block      = stmt* "}"
stmt       = "return" expr? ";"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" (decl | expr? ";") expr? ";" expr? ")" stmt
           | "switch" "(" expr ")" stmt
           | "{" block
           | "break" ";"
           | "continue" ";"
           | "case" (num | ident) ":" stmt // identは列挙型のメンバ名のみ
           | "default" ":" stmt
           | decl ";"
           | expr ";"
struc      = "struct" ident "{" (decl ";")* "}"
enu        = "enum" ident "{" ident (, ident)* "}"
typede     = "typedef" type ident

decl       = specifier pointer ident ("[" num "]")* ("=" expr)?
type       = specifier pointer       ("[" num "]")*
pointer    = "*"*
param      = decl | type
specifier  = "int"
           | "char"
           | "long"
           | "void"
           | "struct" ident
           | "enum" ident
           | ident // ただしtypedefされたもののみ

expr        = assign
assign      = primary ("=" | "+=" | "-=" | "*=" | "/=" | "%=") assign
            | logical_or
logical_or  = logical_and ("||" logical_and)*
logical_and = equality ("&&" equality)*
equality    = relational ("==" relational | "!=" relational)*
relational  = add ("<" add | "<=" add | ">" add | ">=" add)*
add         = mul ("+" mul | "-" mul)*
mul         = unary ("*" unary | "/" unary | "%" unary)*
unary       = ("+" | "-" | "*" | "&" | "!")? primary
            | "sizeof" unary
            | "sizeof" "(" type ")"
            | "__builtin_offsetof" "(" type "," ident ")"
            | "++" unary
            | "--" unary
primary     = num
            | str
            | char
            | "(" expr ")"
            | ident
            | ident call
            | primary "."  ident
            | primary "->" ident
            | primary "++"
            | primary "--"
            | primary arrayref // 多段階はパースはできるものの、コード生成は未対応
arrayref    = "[" expr "]
call        = "(" (expr ("," expr)*)? ")"
*/

Node* parse();
static Node* program();
static Node* func();
static Node* block();
static Node* stmt();
static Node* struc();
static Node* enu();
static Node* typede();

// paramでバックトラックが必要なので、decl, type, specifierは失敗したらトークンを戻した上でNULLを返す
static Node* decl();           // ND_DECLを返す
static Node* type();           // ND_TYPEを返す
static Node* pointer(Node* t); // ND_TYPEを返す
static Node* specifier();      // ND_TYPEを返す
static Node* param();          // ND_DECLを返す

static Node* assign();
static Node* expr();
static Node* logical_or();
static Node* logical_and();
static Node* equality();
static Node* relational();
static Node* add();
static Node* mul();
static Node* unary();
static Node* primary();
static Node* arrayref(Node* node);
static Node* call(char* name);

Node* parse() {
  return program();
}

static Node* program() {
  typedef_map = map_new();
  int i = 0;
  Node* p[1000];
  while(!at_eof()) {
    if(is_next("struct")) {
      p[i++] = struc();
      expect(";");
      continue;
    }
    if(is_next("enum")) {
      p[i++] = enu();
      expect(";");
      continue;
    }
    if(is_next("typedef")) {
      typede();
      expect(";");
      continue;
    }
    Token* origin = token;
    if(consume("extern")) {
      Node* extern_var = decl();
      if(extern_var == NULL) {
        error_at0(__FILE__, __LINE__, token->str, "extern宣言が不正です");
      }
      expect(";");
      extern_var->kind = ND_GDECL_EXTERN;
      p[i++] = extern_var;
      continue;
    }
    Node* global_var = decl();
    if(global_var != NULL && consume(";")) {
      global_var->kind = ND_GDECL;
      p[i++] = global_var;
      continue;
    }
    token = origin;
    p[i++] = func();
  }
  p[i] = NULL;
  Node* program = new_node(ND_PROGRAM, token->str);
  program->funcs = calloc(i + 1, sizeof(Node*));
  for(int j = 0; j < i; j++) {
    program->funcs[j] = p[j];
  }
  return program;
}

static Node* func() {
  Node* typ = type();
  if(typ == NULL) {
    error_at0(__FILE__, __LINE__, token->str, "関数の戻り値の型がありません");
  }
  char* name = consume_ident();
  printf("#   function %s", name);
  if(name == NULL) {
    error_at0(__FILE__, __LINE__, token->str, "関数名がありません");
  }
  expect("(");
  Node* func = new_node_ident(ND_FUNCDEF, token->str, name);
  func->type = typ->type;
  func->args_node = calloc(6, sizeof(Node*));
  int i = 0;
  while(!consume(")")) {
    Node* para = param();
    func->args_node[i++] = para;
    if(!consume(",")) {
      expect(")");
      break;
    }
  }
  if(consume(";")) {
    printf(" prototype\n");
    func->kind = ND_FUNCPROT;
    return func;
  } else {
    printf("\n");
    expect("{");
    Node* bloc = block();
    func->body = bloc;
    return func;
  }
}

static Node* block() {
  int i = 0;
  Node* b[1000];
  while(!consume("}")) {
    b[i++] = stmt();
  }
  b[i] = NULL;
  Node* block = new_node(ND_BLOCK, token->str);
  block->stmts = calloc(i + 1, sizeof(Node*));
  for(int j = 0; j < i; j++) {
    block->stmts[j] = b[j];
  }
  return block;
}

static Node* stmt() {
  if(consume("return")) {
    if(consume(";")) {
      // 空のreturn文は42を返す
      return new_node_1branch(ND_RETURN, token->str, new_node_num(token->str, 42));
    }
    Node* e = expr();
    expect(";");
    return new_node_1branch(ND_RETURN, token->str, e);
  }
  if(consume("if")) {
    expect("(");
    Node* cond = expr();
    expect(")");
    Node* then = stmt();
    Node* els = NULL;
    if(consume("else")) {
      els = stmt();
    }
    Node* i = new_node(ND_IF, token->str);
    i->cond = cond;
    i->then = then;
    i->els = els;
    return i;
  }
  if(consume("while")) {
    expect("(");
    Node* cond = expr();
    expect(")");
    Node* body = stmt();
    Node* w = new_node(ND_WHILE, token->str);
    w->cond = cond;
    w->body = body;
    return w;
  }
  if(consume("for")) {
    expect("(");
    Node* init = NULL;
    Node* cond = NULL;
    Node* inc = NULL;
    if(!consume(";")) {
      // decl | expr ";" という構文になっていて、先にdeclを試して、失敗したらバックトラックしてexprを試す
      init = decl();
      if(init == NULL) {
        init = expr();
      }
      expect(";");
    }
    if(!consume(";")) {
      cond = expr();
      expect(";");
    }
    if(!consume(")")) {
      inc = expr();
      expect(")");
    }
    Node* body = stmt();
    Node* f = new_node(ND_FOR, token->str);
    f->init = init;
    f->cond = cond;
    f->inc = inc;
    f->body = body;
    return f;
  }
  if(consume("switch")) {
    expect("(");
    Node* cond = expr();
    expect(")");
    Node* body = stmt();
    Node* s = new_node(ND_SWITCH, token->str);
    s->cond = cond;
    s->body = body;
    return s;
  }
  if(consume("{")) {
    return block();
  }
  if(consume("break")) {
    expect(";");
    return new_node(ND_BREAK, token->str);
  }
  if(consume("continue")) {
    expect(";");
    return new_node(ND_CONTINUE, token->str);
  }
  if(consume("case")) {
    if(token->kind == TK_NUM) {
      int val = expect_number();
      expect(":");
      Node* c = new_node_1branch(ND_CASE, token->str, stmt());
      c->int_val = val;
      return c;
    } else if(token->kind == TK_IDENT) {
      char* ident = consume_ident();
      expect(":");
      Node* c = new_node_1branch(ND_CASE, token->str, stmt());
      c->name = ident;
      return c;
    }
  }
  if(consume("default")) {
    expect(":");
    return new_node_1branch(ND_DEFAULT, token->str, stmt());
  }
  Node* dec = decl();
  if(dec != NULL) {
    expect(";");
    return dec;
  }
  Node* e = expr();
  expect(";");
  return e;
}

static Node* struc() {
  expect("struct");
  char* name = consume_ident();
  if(name == NULL) {
    error_at0(__FILE__, __LINE__, token->str, "構造体名がありません");
  }
  Node* str = new_node_ident(ND_STRUCT, token->str, name);
  expect("{");
  int i = 0;
  Node* s[1000];
  while(!consume("}")) {
    Node* mem = decl();
    if(mem == NULL) {
      error_at0(__FILE__, __LINE__, token->str, "構造体のメンバが不正です");
    }
    s[i++] = mem;
    expect(";");
  }
  s[i] = NULL;
  str->struct_members = calloc(i + 1, sizeof(Node*));
  for(int j = 0; j < i; j++) {
    str->struct_members[j] = s[j];
  }
  return str;
}

static Node* enu() {
  expect("enum");
  char* name = consume_ident();
  if(name == NULL) {
    error_at0(__FILE__, __LINE__, token->str, "列挙型名がありません");
  }
  Node* en = new_node_ident(ND_ENUM, token->str, name);
  expect("{");
  int i = 0;
  Node* e[1000];
  while(!consume("}")) {
    char* ident = consume_ident();
    if(ident == NULL) {
      error_at0(__FILE__, __LINE__, token->str, "列挙型のメンバ名がありません");
    }
    e[i++] = new_node_ident(ND_IDENT, token->str, ident);
    if(!consume(",")) {
      expect("}");
      break;
    }
  }
  e[i] = NULL;
  en->enum_members = calloc(i + 1, sizeof(Node*));
  for(int j = 0; j < i; j++) {
    en->enum_members[j] = e[j];
  }
  return en;
}

static Node* typede() {
  expect("typedef");
  Node* t = type();
  if(t == NULL) {
    error_at0(__FILE__, __LINE__, token->str, "型がありません");
  }
  char* name = consume_ident();
  if(name == NULL) {
    error_at0(__FILE__, __LINE__, token->str, "名前がありません");
  }
  map_put(typedef_map, name, t);
  return NULL;
}

static Node* decl() {
  Token* origin = token; // バックトラックがあるので、return時に元のトークンの位置に戻す
  Node* spec = specifier();
  if(spec == NULL) {
    token = origin;
    return NULL;
  }
  Node* point = pointer(spec);
  char* name = consume_ident();
  if(name == NULL) {
    token = origin;
    return NULL;
  }
  Node* arr = point;
  while(consume("[")) {
    Node* t = arr;
    arr = new_node(ND_TYPE, token->str);
    arr->type = arr_type(t->type, expect_number());
    expect("]");
  }
  Node* assign = NULL;
  if(consume("=")) {
    assign = expr();
  }
  
  Node* decl = new_node_ident(ND_DECL, token->str, name);
  decl->type = arr->type;
  decl->rhs = assign;
  return decl;
}

static Node* type() {
  Token* origin = token; // バックトラックがあるので、return時に元のトークンの位置に戻す
  Node* spec = specifier();
  if(spec == NULL) {
    token = origin;
    return NULL;
  }
  Node* arr = spec;
  while(consume("[")) {
    Node* t = arr;
    arr = new_node(ND_TYPE, token->str);
    arr->type = arr_type(t->type, expect_number());
    expect("]");
  }
  Node* t = pointer(arr);
  return t;
}

static Node* pointer(Node* t) {
  while(consume("*")) {
    Node* ptr = new_node(ND_TYPE, token->str);
    ptr->type = ptr_type(t->type);
    t = ptr;
  }
  return t;
}

static Node* specifier() {
  Token* origin = token; // バックトラックがあるので、return時に元のトークンの位置に戻す
  if(consume("char")) {
    Node* t = new_node(ND_TYPE, token->str);
    t->type = char_type();
    return t;
  }
  if(consume("int")) {
    Node* t = new_node(ND_TYPE, token->str);
    t->type = int_type();
    return t;
  }
  if(consume("long")) {
    Node* t = new_node(ND_TYPE, token->str);
    t->type = long_type();
    return t;
  }
  if(consume("void")) {
    Node* t = new_node(ND_TYPE, token->str);
    t->type = void_type();
    return t;
  }
  if(consume("struct")) {
    char* name = consume_ident();
    if(name == NULL) {
      error_at0(__FILE__, __LINE__, token->str, "構造体名がありません");
    }
    Node* t = new_node(ND_TYPE, token->str);
    t->type = struct_type(name);
    return t;
  }
  if(consume("enum")) {
    char* name = consume_ident();
    if(name == NULL) {
      error_at0(__FILE__, __LINE__, token->str, "列挙型名がありません");
    }
    Node* t = new_node(ND_TYPE, token->str);
    t->type = int_type(); // 簡単にするためにint型として扱う
    return t;
  }
  char* name = consume_ident();
  if(name == NULL) {
    token = origin;
    return NULL;
  }
  Node* t = map_get(typedef_map, name);
  if(t != NULL) {
    return t;
  }
  token = origin;
  return NULL;
}

static Node* param() {
  Node* dec = decl();
  if(dec != NULL) {
    return dec;
  }
  Node* typ = type();
  if(typ != NULL) {
    Node* decl = new_node(ND_DECL, token->str);
    decl->type = typ->type;
    decl->name = "";
    return decl;
  }
  error_at0(__FILE__, __LINE__, token->str, "引数が不正です");
  return NULL;
}

static Node* expr() {
  return assign();
}

static Node* assign() {
  Token* origin = token;
  Node* prime = unary();
  if(prime != NULL) {
    if(consume("=")) {
      return new_node_2branches(ND_ASSIGN, token->str, prime, expr());
    }
    if(consume("+=")) {
      return new_node_2branches(ND_ASSIGN_ADD, token->str, prime, expr());
    }
    if(consume("-=")) {
      return new_node_2branches(ND_ASSIGN_SUB, token->str, prime, expr());
    }
    if(consume("*=")) {
      return new_node_2branches(ND_ASSIGN_MUL, token->str, prime, expr());
    }
    if(consume("/=")) {
      return new_node_2branches(ND_ASSIGN_DIV, token->str, prime, expr());
    }
    if(consume("%=")) {
      return new_node_2branches(ND_ASSIGN_MOD, token->str, prime, expr());
    }
  }
  token = origin;
  return logical_or();
}

static Node* logical_or() {
  Node* node = logical_and();

  for(;;) {
    if(consume("||")) {
      node = new_node_2branches(ND_LOR, token->str, node, logical_and());
    } else {
      return node;
    }
  }
}

static Node* logical_and() {
  Node* node = equality();

  for(;;) {
    if(consume("&&")) {
      node = new_node_2branches(ND_LAND, token->str, node, equality());
    } else {
      return node;
    }
  }
}

static Node* equality() {
  Node* node = relational();

  for(;;) {
    if(consume("==")) {
      node = new_node_2branches(ND_EQ, token->str, node, relational());
    } else if(consume("!=")) {
      node = new_node_2branches(ND_NE, token->str, node, relational());
    } else {
      return node;
    }
  }
}

static Node* relational() {
  Node* node = add();

  for(;;) {
    if(consume("<")) {
      node = new_node_2branches(ND_LT, token->str, node, add());
    } else if(consume("<=")) {
      node = new_node_2branches(ND_LE, token->str, node, add());
    } else if(consume(">")) {
      node = new_node_2branches(ND_LT, token->str, add(), node);
    } else if(consume(">=")) {
      node = new_node_2branches(ND_LE, token->str, add(), node);
    } else {
      return node;
    }
  }
}

static Node* add() {
  Node* node = mul();

  for(;;) {
    if(consume("+")) {
      node = new_node_2branches(ND_ADD, token->str, node, mul());
    } else if(consume("-")) {
      node = new_node_2branches(ND_SUB, token->str, node, mul());
    } else {
      return node;
    }
  }
}

static Node* mul() {
  Node* node = unary();

  for(;;) {
    if(consume("*")) {
      node = new_node_2branches(ND_MUL, token->str, node, unary());
    } else if(consume("/")) {
      node = new_node_2branches(ND_DIV, token->str, node, unary());
    } else if(consume("%")) {
      node = new_node_2branches(ND_MOD, token->str, node, unary());
    } else {
      return node;
    }
  }
}

static Node* unary() {
  if(consume("+")) {
    return new_node_2branches(ND_ADD, token->str, new_node_num(token->str, 0), unary());
  }
  if(consume("-")) {
    return new_node_2branches(ND_SUB, token->str, new_node_num(token->str, 0), unary());
  }
  if(consume("*")) {
    return new_node_1branch(ND_DEREF, token->str, unary());
  }
  if(consume("&")) {
    return new_node_1branch(ND_ADDR, token->str, unary());
  }
  if(consume("!")) {
    return new_node_1branch(ND_NOT, token->str, unary());
  }
  if(consume("sizeof")) {
    Token* origin = token;
    if(consume("(")) {
      Node* t = type();
      if(t != NULL) {
        expect(")");
        return new_node_1branch(ND_SIZEOF, token->str, t);
      }
    }
    token = origin;
    return new_node_1branch(ND_SIZEOF, token->str, unary());
  }
  if(consume("__builtin_offsetof")) {
    expect("(");
    Node* t = type();
    if(t == NULL) {
      error_at0(__FILE__, __LINE__, token->str, "型がありません");
    }
    expect(",");
    char* ident = consume_ident();
    if(ident == NULL) {
      error_at0(__FILE__, __LINE__, token->str, "メンバ名がありません");
    }
    expect(")");
    Node* offsetof = new_node_1branch(ND_OFFSETOF, token->str, t);
    offsetof->name = ident;
    return offsetof;
  }
  if(consume("++")) {
    // ++a は (a += 1) として処理する
    Node* node = unary();
    return new_node_2branches(ND_ASSIGN_ADD, token->str, node, new_node_num(token->str, 1));
  }
  if(consume("--")) {
    Node* node = unary();
    return new_node_2branches(ND_ASSIGN_SUB, token->str, node, new_node_num(token->str, 1));
  }
  return primary();
}

static Node* primary() {
  Node* prim = NULL;
  char* ident = NULL;
  if(consume("(")) {
    prim = expr();
    expect(")");
  } else if ((ident = consume_ident())) {
    if(is_next("(")) {
      prim = call(ident);
    } else {
      prim = new_node_ident(ND_IDENT, token->str, ident);
    }
  } else if (token->kind == TK_NUM) {
    prim = new_node_num(token->str, expect_number());
  } else if (token->kind == TK_STR) {
    prim = new_node_str(token->str, token->str);
    token = token->next;
  } else if (token->kind == TK_CHAR) {
    prim = new_node_char(token->str, token->val);
    token = token->next;
  } else {
    error_at0(__FILE__, __LINE__, token->str, "不正な式です");
  }
  while(is_next(".") || is_next("->") || is_next("[") || is_next("++") || is_next("--")) {
    if(consume(".")) {
      prim = new_node_1branch(ND_DOT, token->str, prim);
      prim->name = consume_ident();
      if(prim->name == NULL) {
        error_at0(__FILE__, __LINE__, token->str, "構造体のメンバ名がありません");
      }
    } else if(consume("->")) {
      Node* deref = new_node_1branch(ND_DEREF, token->str, prim);
      prim = new_node_1branch(ND_DOT, token->str, deref);
      prim->name = consume_ident();
      if(prim->name == NULL) {
        error_at0(__FILE__, __LINE__, token->str, "構造体のメンバ名がありません");
      }
    } else if(is_next("[")) {
      prim = arrayref(prim);
    } else if(consume("++")) {
      // a++ は ( (a += 1) - 1) として処理する
      Node* node = prim;
      Node* one = new_node_num(token->str, 1);
      prim = new_node_2branches(ND_SUB, token->str, new_node_2branches(ND_ASSIGN_ADD, token->str, node, one), one);
    } else if(consume("--")) {
      Node* node = prim;
      Node* one = new_node_num(token->str, 1);
      prim = new_node_2branches(ND_ADD, token->str, new_node_2branches(ND_ASSIGN_SUB, token->str, node, one), one);
    }
  }
  return prim;
}

static Node* arrayref(Node* node) {
  expect("[");
  Node* array = new_node_2branches(ND_ARRAYREF, token->str, node, expr());
  expect("]");
  return array;
}

static Node* call(char* name) {
  expect("(");
  // 6変数以上はスタック経由で渡したりして大変なので、サポートしない
  Node* args[6];
  int i = 0;
  if(!consume(")")) {
    args[i++] = expr();
    while(consume(",")) {
      args[i++] = expr();
    }
    expect(")");
  }
  Node* node = new_node_ident(ND_CALL, token->str, name);
  node->name = name;
  node->args_call = calloc(6, sizeof(Node*));
  for(int j = 0; j < i; j++) {
    node->args_call[j] = args[j];
  }
  return node;
}
