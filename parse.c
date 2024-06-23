
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "10cc.h"

// Map<name, Node(Type)>
Map* typedef_map;

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
    memcmp(token->str, op, token->len) != 0
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
program    = (func | extern? decl ";" | struct ";" | enum ";" | typedef ";")*
func       = decl "(" (param ("," param)*)? ")" ("{" block | ";")
block      = stmt* "}"
stmt       = "return" expr ";"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" expr? ";" expr? ";" expr? ")" stmt
           | "switch" "(" expr ")" stmt
           | "{" block
           | "break" ";"
           | "continue" ";"
           | "case" num ":" stmt
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
           | "struct" ident
           | "enum" ident
           | ident // ただしtypedefされたもののみ

expr       = assign
assign     = primary "=" assign
           | equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary | "%" unary)*
unary      = ("+" | "-" | "*" | "&")? primary
           | "sizeof" unary
           | "sizeof" "(" type ")"
           | "++" unary
           | "--" unary
primary    = num
           | str
           | "(" expr ")"
           | ident
           | ident call
           | primary "."  ident
           | primary "->" ident
           | primary "++"
           | primary "--"
           | primary arrayref // 多段階はパースはできるものの、コード生成は未対応
arrayref   = "[" expr "]
call       = "(" (expr ("," expr)*)? ")"
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
  Node* p[MAX_FUNCS];
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
      init = expr();
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
    int val = expect_number();
    expect(":");
    Node* c = new_node_1branch(ND_CASE, token->str, stmt());
    c->int_val = val;
    return c;
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
  Node* s[MAX_STRUCT_MEMBERS];
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
  Node* e[MAX_ENUM_MEMBERS];
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
  if(prime != NULL && consume("=")) {
    return new_node_2branches(ND_ASSIGN, token->str, prime, expr());
  }
  token = origin;
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
  if(consume("++")) {
    // ++a は (a = a + 1) として処理する
    Node* node = unary();
    return new_node_2branches(ND_ASSIGN, token->str, node, new_node_2branches(ND_ADD, token->str, node, new_node_num(token->str, 1)));
  }
  if(consume("--")) {
    Node* node = unary();
    return new_node_2branches(ND_ASSIGN, token->str, node, new_node_2branches(ND_SUB, token->str, node, new_node_num(token->str, 1)));
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
      // もし__LINE__と__FILE__なら、その値のリテラルにしておく
      if(strcmp(ident, "__LINE__") == 0) {
        prim = new_node_num(token->str, token->line);
      } else if(strcmp(ident, "__FILE__") == 0) {
        prim = new_node_str(token->str, token->file);
      } else {
        prim = new_node_ident(ND_IDENT, token->str, ident);
      }
    }
  } else if (token->kind == TK_NUM) {
    prim = new_node_num(token->str, expect_number());
  } else if (token->kind == TK_STR) {
    prim = new_node_str(token->str, token->str);
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
      // a++ は ( (a = a + 1) - 1) として処理する
      Node* node = prim;
      Node* one = new_node_num(token->str, 1);
      prim = new_node_2branches(ND_SUB, token->str, new_node_2branches(ND_ASSIGN, token->str, node, new_node_2branches(ND_ADD, token->str, node, one)), one);
    } else if(consume("--")) {
      Node* node = prim;
      Node* one = new_node_num(token->str, 1);
      prim = new_node_2branches(ND_ADD, token->str, new_node_2branches(ND_ASSIGN, token->str, node, new_node_2branches(ND_SUB, token->str, node, one)), one);
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
