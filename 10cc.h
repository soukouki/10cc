
typedef enum {
  TK_SYMBOL,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token* next;
  int val;
  char* str;
  int len;
};

typedef enum {
  // 演算子
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,

  // 比較演算子
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE, // GT, GEはLT, LEを使って表現できる

  // リテラル
  ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node* lhs;
  Node* rhs;
  int val;
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

Token* tokenize(char *p);
Node* expr();
extern Token *token;
extern char* user_input;
void gen(Node* node);
