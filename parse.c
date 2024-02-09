#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "10cc.h"

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて真を返す
// それ以外の場合には偽を返す
bool consume(char* op) {
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
char* consume_ident() {
  if(token->kind != TK_IDENT) return NULL;
  char* name = calloc(token->len + 1, sizeof(char));
  strncpy(name, token->str, token->len);
  token = token->next;
  return name;
}

// 次のトークンが期待している記号のときには。トークンを1つ読み進める
// それ以外の場合にはエラーを報告する
void expect(char* op) {
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
int expect_number() {
  if(token->kind != TK_NUM) {
    error_at(token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる·
Token* new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

Var* find_var(char* name, int len) {
  for(Var* var = locals; var; var = var->next) {
    if(var->len == len && !memcmp(name, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

Token* tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p) {
    // 空白文字をスキップ
    if(isspace(*p)) {
      p++;
      continue;
    }

    if(strncmp(p, ">=", 2) == 0 || strncmp(p, "<=", 2) == 0 || strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0) {
      cur = new_token(TK_SYMBOL, cur, p, 2);
      p += 2;
      continue;
    }

    if(
      *p == '+' || *p == '-' || *p == '*' || *p == '/' ||
      *p == '(' || *p == ')' || *p == '>' || *p == '<' ||
      *p == '=' || *p == ';' || *p == '{' || *p == '}' ||
      *p == ','
    ) {
      cur = new_token(TK_SYMBOL, cur, p++, 1);
      continue;
    }

    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z' || *p == '_') {
      char* start = p;
      while('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z' || *p == '_' || '0' <= *p && *p <= '9') {
        p++;
      }
      if(p - start == 6 && !memcmp(start, "return", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6);
        continue;
      }
      if(p - start == 2 && !memcmp(start, "if", 2)) {
        cur = new_token(TK_SYMBOL, cur, start, 2);
        continue;
      }
      if(p - start == 4 && !memcmp(start, "else", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4);
        continue;
      }
      if(p - start == 5 && !memcmp(start, "while", 5)) {
        cur = new_token(TK_SYMBOL, cur, start, 5);
        continue;
      }
      if(p - start == 3 && !memcmp(start, "for", 3)) {
        cur = new_token(TK_SYMBOL, cur, start, 3);
        continue;
      }
      Var* var = find_var(start, p - start);
      if(!var) {
        var = calloc(1, sizeof(Var));
        var->name = calloc(p - start + 1, sizeof(char));
        memcpy(var->name, start, p - start);
        var->len = p - start;
        var->offset = locals ? locals->offset + 8 : 8;
        var->next = locals;
        locals = var;
      }
      cur = new_token(TK_IDENT, cur, start, p - start);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
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
  node->var = find_var(name, strlen(name));
  return node;
}


/*
program    = stmt*
stmt       = assign ";"
           | "return" expr ";"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" expr? ";" expr? ";" expr? ")" stmt
           | "{" stmt* "}"
assign     = (ident "=")? expr
expr       = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num
           | ident ("(" (expr ("," expr)*)? ")")? // ひとまず0引数の関数のみ対応
           | "(" expr ")"
*/

Node* program();
Node* stmt();
Node* assign();
Node* expr();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();

Node* program() {
  int i = 0;
  while(!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node* stmt() {
  if(consume("return")) {
    Node* e = expr();
    expect(";");
    return new_node(ND_RETURN, e, NULL);
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
    Node* i = new_node(ND_IF, NULL, NULL);
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
    Node* w = new_node(ND_WHILE, NULL, NULL);
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
    Node* f = new_node(ND_FOR, NULL, NULL);
    f->init = init;
    f->cond = cond;
    f->inc = inc;
    f->body = body;
    return f;
  }
  if(consume("{")) {
    int i = 0;
    Node* b[100];
    while(!consume("}")) {
      b[i++] = stmt();
    }
    b[i] = NULL;
    Node* block = new_node(ND_BLOCK, NULL, NULL);
    block->stmts = calloc(i + 1, sizeof(Node*));
    for(int j = 0; j < i; j++) {
      block->stmts[j] = b[j];
    }
    return block;
  }
  Node* e = assign();
  expect(";");
  return e;
}

Node* assign() {
  if(token->next->kind == TK_SYMBOL && token->next->len == 1 && token->next->str[0] == '=') {
    char* lvals = consume_ident();
    expect("=");
    Node* lval = new_node_ident(ND_LVAR, lvals);
    Node* node = new_node(ND_ASSIGN, lval, expr());
    return node;
  }
  return expr();
}

Node* expr() {
  return equality();
}

Node* equality() {
  Node* node = relational();

  for(;;) {
    if(consume("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if(consume("!=")) {
      node = new_node(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

Node* relational() {
  Node* node = add();

  for(;;) {
    if(consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if(consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else if(consume(">")) {
      node = new_node(ND_LT, add(), node);
    } else if(consume(">=")) {
      node = new_node(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

Node* add() {
  Node* node = mul();

  for(;;) {
    if(consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if(consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

Node* mul() {
  Node* node = unary();

  for(;;) {
    if(consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if(consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node* unary() {
  if(consume("+")) {
    return unary();
  }
  if(consume("-")) {
    return new_node(ND_SUB, new_node_num(0), unary());
  }
  return primary();
}

Node* primary() {
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
    node->args = calloc(6, sizeof(Node*));
    for(int j = 0; j < i; j++) {
      node->args[j] = args[j];
    }
    return node;
  }
  if(ident != NULL) {
    return new_node_ident(ND_REF, ident);
  }
  return new_node_num(expect_number());
}
