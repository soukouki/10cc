
typedef enum {
  TK_SYMBOL,
  TK_IDENT,
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

  // 構文
  ND_REF,  // 変数の評価
  ND_ASSIGN,
  ND_LVAR, // 左辺値(代入される側の値) (今は変数のみ)
} NodeKind;

typedef struct Var Var;

struct Var {
  Var* next;
  char* name;
  int len;
  int offset;
};

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node* lhs;  // ND_ADD, ND_SUB, ND_MUL, ND_DIV, ND_EQ, ND_NE, ND_LT, ND_LEの場合に使う
  Node* rhs;  // 同上
  int val;    // ND_NUMの場合に使う
  Var* var;   // ND_LVARの場合に使う
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

Token* tokenize(char *p);
Node* expr();
void gen(Node* node);
Var* find_var(char* name, int len);

extern Token *token;
extern char* user_input;
extern Node* code[100];
extern Var* locals;