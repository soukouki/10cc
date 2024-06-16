
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "10cc.h"

#define MAX_STRING_LENGTH 1000

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
          ERROR_AT(start, "コメントが閉じられていません");
        }
        p++;
      }
      p += 2;
      continue;
    }

    if(
      strncmp(p, ">=", 2) == 0 || strncmp(p, "<=", 2) == 0 ||
      strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
      strncmp(p, "->", 2) == 0
    ) {
      cur = new_token(TK_SYMBOL, cur, p, 2);
      p += 2;
      continue;
    }

    if(
      *p == '(' || *p == ')' || *p == '{' || *p == '}' || *p == '[' || *p == ']' ||
      *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '%' ||
      *p == '>' || *p == '<' || *p == '=' ||
      *p == ';' || *p == ',' || *p == '.' || *p == '&'
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
      char* str = calloc(1, MAX_STRING_LENGTH);
      int len = 0;
      while(*p != '"') {
        if(*p == '\0') {
          ERROR_AT(p, "文字列が閉じられていません");
        }
        if(*p == '\\') {
          p++;
          if(*p == 'n') {
            str[len++] = '\n';
          } else if(*p == 't') {
            str[len++] = '\t';
          } else if(*p == '\\') {
            str[len++] = '\\';
          } else if(*p == '"') {
            str[len++] = '"';
          } else {
            ERROR_AT(p, "無効なエスケープシーケンスです");
          }
        } else {
          str[len++] = *p;
        }
        p++;
      }
      cur = new_token(TK_STR, cur, str, len);
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
      if(p - start == 6 && !memcmp(start, "struct", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6);
        continue;
      }
      if(p - start == 4 && !memcmp(start, "enum", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4);
        continue;
      }
      if(p - start == 7 && !memcmp(start, "typedef", 7)) {
        cur = new_token(TK_SYMBOL, cur, start, 7);
        continue;
      }
      if(p - start == 6 && !memcmp(start, "extern", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6);
        continue;
      }
      cur = new_token(TK_IDENT, cur, start, p - start);
      continue;
    }

    ERROR_AT(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

