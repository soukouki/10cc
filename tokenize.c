
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "10cc.h"

// 新しいトークンを作成してcurにつなげる·
static Token* new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
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

    // コメントをスキップ
    if(strncmp(p, "//", 2) == 0) {
      p += 2;
      while(*p != '\n') {
        p++;
      }
      continue;
    }
    if(strncmp(p, "/*", 2) == 0) {
      char* start = p;
      p += 2;
      while(strncmp(p, "*/", 2) != 0) {
        if(*p == '\0') {
          error_at(start, "コメントが閉じられていません");
        }
        p++;
      }
      p += 2;
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
      *p == ',' || *p == '&' || *p == '[' || *p == ']'
    ) {
      cur = new_token(TK_SYMBOL, cur, p++, 1);
      continue;
    }

    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if(*p == '"') {
      p++;
      char* start = p;
      while(*p != '"') {
        if(*p == '\0') {
          error_at(start, "文字列が閉じられていません");
        }
        p++;
      }
      cur = new_token(TK_STR, cur, start, p - start);
      cur->str[p - start] = '\0'; // 文字列の終端を設定
      p++;
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
      if(p - start == 6 && !memcmp(start, "sizeof", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6);
        continue;
      }
      if(p - start == 3 && !memcmp(start, "int", 3)) {
        cur = new_token(TK_SYMBOL, cur, start, 3);
        continue;
      }
      if(p - start == 4 && !memcmp(start, "char", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4);
        continue;
      }
      cur = new_token(TK_IDENT, cur, start, p - start);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

