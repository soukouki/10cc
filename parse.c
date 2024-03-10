
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "10cc.h"

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて真を返す
// それ以外の場合には偽を返す
static bool consume(char* op) {
  if(
    token->kind != TK_SYMBOL ||
    strlen(op) != token->len ||
    memcmp(token->str, op, token->len) != 0
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
    memcmp(token->str, op, token->len) != 0
  ) ERROR_AT(token->str, "'%s'ではありません", op);

  token = token->next;
  return;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す
// それ以外の場合にはエラーを報告する
static int expect_number() {
  if(token->kind != TK_NUM) {
    ERROR_AT(token->str, "数ではありません");
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
    memcmp(token->str, op, token->len) != 0
  ) return false;
  return true;
}

// 次の次のトークンが期待している記号のときには真を返す
static bool is_after_next(char* op) {
  if(token->next == NULL) return false;
  if(token->next->kind != TK_SYMBOL) return false;
  if(
    strlen(op) != token->next->len ||
    memcmp(token->next->str, op, token->next->len) != 0
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

Node* new_node_ident(NodeKind kind, char* loc, char* name) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->loc = loc;
  node->name = name;
  return node;
}

/*
program    = (func | decl ";" | struct ";")*
func       = decl "(" (param ("," param)*)? ")" (block | ";")
block      = "{" stmt* "}"
stmt       = assign ";"
           | decl ";"
           | "return" expr ";"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" assign? ";" expr? ";" assign? ")" stmt
           | block
struct     = "struct" ident "{" (decl ";")* "}"

decl       = specifier pointer ident ("[" num "]")*
type       = specifier pointer       ("[" num "]")*
pointer    = "*"*
param      = decl | type
specifier  = "int" | "char" | "struct" ident

assign     = expr ("=" expr)?
expr       = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary | "%" unary)*
unary      = ("+" | "-" | "*" | "&")? primary
           | "sizeof" unary
primary    = num
           | str
           | "(" expr ")"
           | ident
           | ident call
           | primary dot
           | primary arrayref // 多段階はパースはできるものの、コード生成は未対応
dot        = "." ident
arrayref   = "[" expr "]
call       = "(" (expr ("," expr)*)? ")"
*/

Node* parse();
static Node* program();
static Node* func();
static Node* block();
static Node* stmt();
static Node* struc();

// paramでバックトラックが必要なので、decl, type, specifierは失敗したらトークンを戻した上でNULLを返す
static Node* decl();           // ND_DECLを返す
static Node* type();           // ND_TYPEを返す
static Node* pointer(Node* t); // ND_TYPEを返す
static Node* specifier();      // ND_TYPEを返す
static Node* param();          // ND_DECLを返す

static Node* assign();
static Node* expr();
static Node* equality();
static Node* relational();
static Node* add();
static Node* mul();
static Node* unary();
static Node* primary();
static Node* dot(Node* node);
static Node* arrayref(Node* node);
static Node* call(char* name);

Node* parse() {
  return program();
}

static Node* program() {
  int i = 0;
  Node* p[MAX_FUNCS];
  while(!at_eof()) {
    if(is_next("struct")) {
      p[i++] = struc();
      expect(";");
      continue;
    }
    Token* origin = token;
    Node* globalvar = decl();
    if(globalvar != NULL && consume(";")) {
      globalvar->kind = ND_GDECL;
      p[i++] = globalvar;
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
    ERROR_AT(token->str, "関数の戻り値の型がありません");
  }
  char* name = consume_ident();
  printf("#   function %s", name);
  if(name == NULL) {
    ERROR_AT(token->str, "関数名がありません");
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
  Node* b[MAX_BLOCK_STMTS];
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
      init = assign();
      expect(";");
    }
    if(!consume(";")) {
      cond = expr();
      expect(";");
    }
    if(!consume(")")) {
      inc = assign();
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
  if(consume("{")) {
    return block();
  }
  Node* dec = decl();
  if(dec != NULL) {
    expect(";");
    return dec;
  }
  Node* e = assign();
  expect(";");
  return e;
}

static Node* struc() {
  expect("struct");
  char* name = consume_ident();
  if(name == NULL) {
    ERROR_AT(token->str, "構造体名がありません");
  }
  expect("{");
  int i = 0;
  Node* s[MAX_STRUCT_MEMBERS];
  while(!consume("}")) {
    Node* mem = decl();
    if(mem == NULL) {
      ERROR_AT(token->str, "構造体のメンバが不正です");
    }
    s[i++] = mem;
    expect(";");
  }
  s[i] = NULL;
  Node* str = new_node_ident(ND_STRUCT, token->str, name);
  str->struct_members = calloc(i + 1, sizeof(Node*));
  for(int j = 0; j < i; j++) {
    str->struct_members[j] = s[j];
  }
  return str;
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
  
  Node* decl = new_node_ident(ND_DECL, token->str, name);
  decl->type = arr->type;
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
  if(consume("struct")) {
    char* name = consume_ident();
    if(name == NULL) {
      ERROR_AT(token->str, "構造体名がありません");
    }
    Node* t = new_node(ND_TYPE, token->str);
    t->type = struct_type(name);
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
  ERROR_AT(token->str, "引数が不正です");
}

static Node* assign() {
  Node* node = expr();
  if(consume("=")) {
    return new_node_2branches(ND_ASSIGN, token->str, node, expr());
  }
  return node;
}

static Node* expr() {
  return equality();
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
    return unary();
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
  if(consume("sizeof")) {
    return new_node_1branch(ND_SIZEOF, token->str, unary());
  }
  return primary();
}

static Node* primary() {
  Node* prim = NULL;
  char* ident = NULL;
  if(consume("(")) {
    prim = expr();
    expect(")");
  } else if (ident = consume_ident()) {
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
  } else {
    ERROR_AT(token->str, "不正な式です");
  }
  while(is_next(".") || is_next("[")) {
    if(is_next(".")) {
      prim = dot(prim);
    } else if(is_next("[")) {
      prim = arrayref(prim);
    }
  }
  return prim;
}

static Node* dot(Node* node) {
  expect(".");
  Node* dot = new_node_1branch(ND_DOT, token->str, node);
  dot->name = consume_ident();
  return dot;
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
