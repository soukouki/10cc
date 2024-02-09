
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
  ND_RETURN,
  ND_IF,
  ND_WHILE,
  ND_FOR,
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
  Node* init; // forで使う
  Node* inc;  // forで使う
  Node* cond; // if, while, forで使う
  Node* then; // ifで使う
  Node* els;  // ifで使う
  Node* body; // while, forで使う
  Node* lhs;  // 2項演算子, 代入, 変数の参照, returnで使う
  Node* rhs;  // 2項演算子, 代入で使う
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