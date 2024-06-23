
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "10cc.h"

#define MAX_STRING_LENGTH 1000

// 新しいトークンを作成してcurにつなげる·
static Token* new_token(TokenKind kind, Token *cur, char *str, int len, char* file, int line) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  tok->file = file;
  tok->line = line;
  cur->next = tok;
  return tok;
}

Token* tokenize(char *p, char* file) {
  Token head;
  head.next = NULL;
  Token *cur = &head;
  int line = 1;

  while(*p) {
    // 空白文字をスキップ
    if(*p == '\n') {
      p++;
      line++;
      continue;
    }
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
        if(*p == '\n') {
          line++;
        }
        if(*p == '\0') {
          error_at0(__FILE__, __LINE__, start, "コメントが閉じられていません");
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
      cur = new_token(TK_SYMBOL, cur, p, 2, file, line);
      p += 2;
      continue;
    }

    if(*p == '+' || *p == '-') {
      if((*(p + 1) == '+' || *(p + 1) == '-') && *(p + 1) == *p) {
        cur = new_token(TK_SYMBOL, cur, p, 2, file, line);
        p += 2;
      } else {
        cur = new_token(TK_SYMBOL, cur, p++, 1, file, line);
      }
      continue;
    }

    if(
      *p == '(' || *p == ')' || *p == '{' || *p == '}' || *p == '[' || *p == ']' ||
      *p == '*' || *p == '/' || *p == '%' ||
      *p == '>' || *p == '<' || *p == '=' ||
      *p == ';' || *p == ',' || *p == '.' || *p == '&' || *p == ':'
    ) {
      cur = new_token(TK_SYMBOL, cur, p++, 1, file, line);
      continue;
    }

    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0, file, line);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if(*p == '"') {
      p++;
      char* str = calloc(1, MAX_STRING_LENGTH);
      int len = 0;
      while(*p != '"') {
        if(*p == '\0') {
          error_at0(__FILE__, __LINE__, p, "文字列が閉じられていません");
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
            error_at0(__FILE__, __LINE__, p, "無効なエスケープシーケンスです");
          }
        } else {
          str[len++] = *p;
        }
        p++;
      }
      cur = new_token(TK_STR, cur, str, len, file, line);
      p++;
      continue;
    }

    if(*p == '\'') {
      p++;
      if(*p == '\\') {
        p++;
        if(*p == 'n') {
          cur = new_token(TK_NUM, cur, p, 1, file, line);
          cur->val = '\n';
        } else if(*p == 't') {
          cur = new_token(TK_NUM, cur, p, 1, file, line);
          cur->val = '\t';
        } else if(*p == '\\') {
          cur = new_token(TK_NUM, cur, p, 1, file, line);
          cur->val = '\\';
        } else if(*p == '\'') {
          cur = new_token(TK_NUM, cur, p, 1, file, line);
          cur->val = '\'';
        } else {
          error_at0(__FILE__, __LINE__, p, "無効なエスケープシーケンスです");
        }
      } else {
        cur = new_token(TK_NUM, cur, p, 1, file, line);
        cur->val = *p;
      }
      p++;
      if(*p != '\'') {
        error_at0(__FILE__, __LINE__, p, "文字リテラルは1文字でなければいけません");
      }
      p++;
      continue;
    }

    if(('a' <= *p && *p <= 'z') ||( 'A' <= *p && *p <= 'Z') || *p == '_') {
      char* start = p;
      while(('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z') || *p == '_' || ('0' <= *p && *p <= '9')) {
        p++;
      }
      if(p - start == 6 && !memcmp(start, "return", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 2 && !memcmp(start, "if", 2)) {
        cur = new_token(TK_SYMBOL, cur, start, 2, file, line);
        continue;
      }
      if(p - start == 4 && !memcmp(start, "else", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 5 && !memcmp(start, "while", 5)) {
        cur = new_token(TK_SYMBOL, cur, start, 5, file, line);
        continue;
      }
      if(p - start == 3 && !memcmp(start, "for", 3)) {
        cur = new_token(TK_SYMBOL, cur, start, 3, file, line);
        continue;
      }
      if(p - start == 6 && !memcmp(start, "sizeof", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 3 && !memcmp(start, "int", 3)) {
        cur = new_token(TK_SYMBOL, cur, start, 3, file, line);
        continue;
      }
      if(p - start == 4 && !memcmp(start, "char", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 4 && !memcmp(start, "void", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 6 && !memcmp(start, "struct", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 4 && !memcmp(start, "enum", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 7 && !memcmp(start, "typedef", 7)) {
        cur = new_token(TK_SYMBOL, cur, start, 7, file, line);
        continue;
      }
      if(p - start == 6 && !memcmp(start, "extern", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 5 && !memcmp(start, "break", 5)) {
        cur = new_token(TK_SYMBOL, cur, start, 5, file, line);
        continue;
      }
      if(p - start == 8 && !memcmp(start, "continue", 8)) {
        cur = new_token(TK_SYMBOL, cur, start, 8, file, line);
        continue;
      }
      if(p - start == 6 && !memcmp(start, "switch", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 4 && !memcmp(start, "case", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 7 && !memcmp(start, "default", 7)) {
        cur = new_token(TK_SYMBOL, cur, start, 7, file, line);
        continue;
      }
      if(p - start == 5 && !memcmp(start, "const", 5)) {
        // constはすべて無視！！！www
        continue;
      }
      cur = new_token(TK_IDENT, cur, start, p - start, file, line);
      continue;
    }

    error_at0(__FILE__, __LINE__, p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0, file, line);
  return head.next;
}

