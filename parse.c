
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
  ) error_at(token->str, "'%s'ではありません", op);

  token = token->next;
  return;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す
// それ以外の場合にはエラーを報告する
static int expect_number() {
  if(token->kind != TK_NUM) {
    error_at(token->str, "数ではありません");
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

Node* new_node(NodeKind kind) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node* new_node_1branch(NodeKind kind, Node* lhs) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  return node;
}

Node* new_node_2branches(NodeKind kind, Node* lhs, Node* rhs) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_node_num(int val) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node* new_node_ident(NodeKind kind, char* name) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->name = name;
  return node;
}

/*
program    = func*
func       = type ident "(" (type ident ("," type ident)*)? ")" block
block      = "{" stmt* "}"
stmt       = assign ";"
           | type ident ";"
           | "return" expr ";"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" assign? ";" expr? ";" assign? ")" stmt
           | block
type       = "int" "*"*
assign     = expr ("=" expr)?
expr       = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-" | "*" | "&")? primary
primary    = num
           | ident
           | ident "(" (expr ("," expr)*)? ")"
           | "(" expr ")"
*/

Node* parse();
static Node* program();
static Node* func();
static Node* block();
static Node* stmt();
static Node* type();
static Node* assign();
static Node* expr();
static Node* equality();
static Node* relational();
static Node* add();
static Node* mul();
static Node* unary();
static Node* primary();

Node* parse() {
  return program();
}

static Node* program() {
  int i = 0;
  Node* p[100];
  while(!at_eof()) {
    p[i++] = func();
  }
  p[i] = NULL;
  Node* program = new_node(ND_PROGRAM);
  program->funcs = calloc(i + 1, sizeof(Node*));
  for(int j = 0; j < i; j++) {
    program->funcs[j] = p[j];
  }
  return program;
}

static Node* func() {
  type(); // 戻り地の型は今は無視する
  char* name = consume_ident();
  printf("#   function %s\n", name);
  if(name == NULL) {
    error_at(token->str, "関数名がありません");
  }
  expect("(");
  Node* func = new_node_ident(ND_FUNCDEF, name);
  func->args_name = calloc(6, sizeof(Node*));
  func->args_type = calloc(6, sizeof(Node*));
  if(is_next("int")) {
    Node* arg_type = type();
    char* arg_name = consume_ident();
    if(arg_name == NULL) {
      error_at(token->str, "引数名がありません");
    }
    func->args_name[0] = new_node_ident(ND_IDENT, arg_name);
    func->args_type[0] = arg_type;
    int i = 1;
    while(consume(",")) {
      arg_type = type();
      arg_name = consume_ident();
      if(arg_name == NULL) {
        error_at(token->str, "引数名がありません");
      }
      func->args_name[i] = new_node_ident(ND_IDENT, arg_name);
      func->args_type[i] = arg_type;
      i++;
    }
    func->args_name[i] = NULL;
    func->args_type[i] = NULL;
  }
  expect(")");
  expect("{");
  Node* bloc = block();
  func->body = bloc;
  return func;
}

static Node* block() {
  int i = 0;
  Node* b[100];
  while(!consume("}")) {
    b[i++] = stmt();
  }
  b[i] = NULL;
  Node* block = new_node(ND_BLOCK);
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
    return new_node_1branch(ND_RETURN, e);
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
    Node* i = new_node(ND_IF);
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
    Node* w = new_node(ND_WHILE);
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
    Node* f = new_node(ND_FOR);
    f->init = init;
    f->cond = cond;
    f->inc = inc;
    f->body = body;
    return f;
  }
  if(consume("{")) {
    return block();
  }
  if(is_next("int")) {
    Node* t = type();
    char* name = consume_ident();
    expect(";");
    Node* v = new_node_ident(ND_VARDEF, name);
    v->lhs = t;
    return v;
  }
  Node* e = assign();
  expect(";");
  return e;
}

static Node* type() {
  expect("int");
  Node* node = new_node(ND_INT);
  while(consume("*")) {
    Node* ptr = new_node(ND_PTR);
    ptr->lhs = node;
    node = ptr;
  }
  return node;
}

static Node* assign() {
  Node* node = expr();
  if(consume("=")) {
    return new_node_2branches(ND_ASSIGN, node, expr());
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
      node = new_node_2branches(ND_EQ, node, relational());
    } else if(consume("!=")) {
      node = new_node_2branches(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

static Node* relational() {
  Node* node = add();

  for(;;) {
    if(consume("<")) {
      node = new_node_2branches(ND_LT, node, add());
    } else if(consume("<=")) {
      node = new_node_2branches(ND_LE, node, add());
    } else if(consume(">")) {
      node = new_node_2branches(ND_LT, add(), node);
    } else if(consume(">=")) {
      node = new_node_2branches(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

static Node* add() {
  Node* node = mul();

  for(;;) {
    if(consume("+")) {
      node = new_node_2branches(ND_ADD, node, mul());
    } else if(consume("-")) {
      node = new_node_2branches(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

static Node* mul() {
  Node* node = unary();

  for(;;) {
    if(consume("*")) {
      node = new_node_2branches(ND_MUL, node, unary());
    } else if(consume("/")) {
      node = new_node_2branches(ND_DIV, node, unary());
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
    return new_node_2branches(ND_SUB, new_node_num(0), unary());
  }
  if(consume("*")) {
    return new_node_1branch(ND_DEREF, unary());
  }
  if(consume("&")) {
    return new_node_1branch(ND_ADDR, unary());
  }
  return primary();
}

static Node* primary() {
  if(consume("(")) {
    Node* node = expr();
    expect(")");
    return node;
  }
  char* ident = consume_ident();
  if(consume("(")) {
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
    Node* node = new_node_ident(ND_CALL, ident);
    node->name = ident;
    node->args_call = calloc(6, sizeof(Node*));
    for(int j = 0; j < i; j++) {
      node->args_call[j] = args[j];
    }
    return node;
  }
  if(ident != NULL) {
    Node* ref = new_node_ident(ND_IDENT, ident);
    return ref;
  }
  return new_node_num(expect_number());
}
