
#include "10cc.h"

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

static char* get_file_and_line(char* p, char** file, int* line) {
  char* start = p;
  if(strncmp(p, "//", 2) != 0) {
    return NULL;
  }
  p += 3;
  char* file_start = p;
  bool is_appearance_of_newline = false;
  while(('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z') || ('0' <= *p && *p <= '9') || *p == '_' || *p == '.' || *p == '-') {
    p++;
  }
  char* file_end = p;
  if(*p != ':') {
    return NULL;
  }
  p++;
  char* line_start = p;
  while(isdigit(*p)) {
    p++;
  }
  if(*p != '\n') {
    return NULL;
  }
  p++;
  *file = calloc(100, 1);
  memcpy(*file, file_start, file_end - file_start);
  char* line_str = calloc(10, 1);
  memcpy(line_str, line_start, p - line_start);
  *line = strtol(line_str, NULL, 10) + 1;
  free(line_str);
  printf("# file: %s, line: %d\n", *file, *line);
  return p;
}

Token* tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;
  int line = 1;
  char* file = "unknown";

  while(*p) {
    if(isspace(*p)) {
      p++;
      continue;
    }

    // `// <file>:<line><改行>`という形式のコメントの場合、fileとlineを更新する
    char* new_p = get_file_and_line(p, &file, &line);
    if(new_p != NULL) {
      p = new_p;
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
      strncmp(p, "->", 2) == 0 ||
      strncmp(p, "++", 2) == 0 || strncmp(p, "--", 2) == 0 ||
      strncmp(p, "&&", 2) == 0 || strncmp(p, "||", 2) == 0 ||
      strncmp(p, "+=", 2) == 0 || strncmp(p, "-=", 2) == 0 ||
      strncmp(p, "*=", 2) == 0 || strncmp(p, "/=", 2) == 0 ||
      strncmp(p, "%=", 2) == 0
    ) {
      cur = new_token(TK_SYMBOL, cur, p, 2, file, line);
      p += 2;
      continue;
    }

    if(
      *p == '(' || *p == ')' || *p == '{' || *p == '}' || *p == '[' || *p == ']' ||
      *p == '*' || *p == '/' || *p == '%' || *p == '+' || *p == '-' ||
      *p == '>' || *p == '<' || *p == '=' ||
      *p == ';' || *p == ',' || *p == '.' || *p == '&' || *p == ':' || *p == '!'
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
      char* str = calloc(1, 1000);
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
          } else if(*p == '0') {
            str[len++] = '\0';
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
          cur = new_token(TK_CHAR, cur, p, 1, file, line);
          cur->val = '\n';
        } else if(*p == 't') {
          cur = new_token(TK_CHAR, cur, p, 1, file, line);
          cur->val = '\t';
        } else if(*p == '\\') {
          cur = new_token(TK_CHAR, cur, p, 1, file, line);
          cur->val = '\\';
        } else if(*p == '\'') {
          cur = new_token(TK_CHAR, cur, p, 1, file, line);
          cur->val = '\'';
        } else if(*p == '0') {
          cur = new_token(TK_CHAR, cur, p, 1, file, line);
          cur->val = '\0';
        } else {
          error_at0(__FILE__, __LINE__, p, "無効なエスケープシーケンスです");
        }
      } else {
        cur = new_token(TK_CHAR, cur, p, 1, file, line);
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
      if(p - start == 6 && !strncmp(start, "return", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 2 && !strncmp(start, "if", 2)) {
        cur = new_token(TK_SYMBOL, cur, start, 2, file, line);
        continue;
      }
      if(p - start == 4 && !strncmp(start, "else", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 5 && !strncmp(start, "while", 5)) {
        cur = new_token(TK_SYMBOL, cur, start, 5, file, line);
        continue;
      }
      if(p - start == 3 && !strncmp(start, "for", 3)) {
        cur = new_token(TK_SYMBOL, cur, start, 3, file, line);
        continue;
      }
      if(p - start == 6 && !strncmp(start, "sizeof", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 3 && !strncmp(start, "int", 3)) {
        cur = new_token(TK_SYMBOL, cur, start, 3, file, line);
        continue;
      }
      if(p - start == 4 && !strncmp(start, "char", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 4 && !strncmp(start, "void", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 6 && !strncmp(start, "struct", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 4 && !strncmp(start, "enum", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 7 && !strncmp(start, "typedef", 7)) {
        cur = new_token(TK_SYMBOL, cur, start, 7, file, line);
        continue;
      }
      if(p - start == 6 && !strncmp(start, "extern", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 5 && !strncmp(start, "break", 5)) {
        cur = new_token(TK_SYMBOL, cur, start, 5, file, line);
        continue;
      }
      if(p - start == 8 && !strncmp(start, "continue", 8)) {
        cur = new_token(TK_SYMBOL, cur, start, 8, file, line);
        continue;
      }
      if(p - start == 6 && !strncmp(start, "switch", 6)) {
        cur = new_token(TK_SYMBOL, cur, start, 6, file, line);
        continue;
      }
      if(p - start == 4 && !strncmp(start, "case", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      if(p - start == 7 && !strncmp(start, "default", 7)) {
        cur = new_token(TK_SYMBOL, cur, start, 7, file, line);
        continue;
      }
      if(p - start == 4 && !strncmp(start, "long", 4)) {
        cur = new_token(TK_SYMBOL, cur, start, 4, file, line);
        continue;
      }
      // constは無視
      if(p - start == 5 && !strncmp(start, "const", 5)) {
        continue;
      }
      // staticは無視
      if(p - start == 6 && !strncmp(start, "static", 6)) {
        continue;
      }
      if(p - start == 18 && !strncmp(start, "__builtin_offsetof", 18)) {
        cur = new_token(TK_SYMBOL, cur, start, 18, file, line);
        continue;
      }
      if(p - start == 8 && !strncmp(start, "__FILE__", 8)) {
        cur = new_token(TK_STR, cur, file, strlen(file), file, line);
        continue;
      }
      if(p - start == 8 && !strncmp(start, "__LINE__", 8)) {
        cur = new_token(TK_NUM, cur, start, 0, file, line);
        cur->val = line;
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

