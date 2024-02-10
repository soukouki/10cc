
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

  // ポインタ
  ND_ADDR,
  ND_DEREF,

  // 構文
  ND_REF,     // 変数の評価
  ND_CALL,    // 関数呼び出し
  ND_ASSIGN,
  ND_LVAR,    // 左辺値(代入される側の値) (今は変数のみ)
  ND_RETURN,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_VARDEF,  // 変数の定義
  ND_FUNCDEF, // 関数定義

  // その他
  ND_PROGRAM, // プログラム全体
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
  Node*  init;      // forで使う
  Node*  inc;       // forで使う
  Node*  cond;      // if, while, forで使う
  Node*  then;      // ifで使う
  Node*  els;       // ifで使う
  Node*  body;      // while, forで使う
  Node** stmts;     // ブロックで使う
  Node** args_call; // 関数呼び出しで使う
  Node** funcs;     // プログラムで使う
  Node*  lhs;       // 2項演算子, 代入, return, 単項&, 単項*で使う
  Node*  rhs;       // 2項演算子, 代入で使う
  int    val;       // 数値リテラルの場合に使う
  char*  name;      // 関数の定義, 関数呼び出し, 変数の参照で使う
  Var*   var;       // ND_LVARの場合に使う
  Var**  args_def;  // 関数の定義で使う
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

Token* tokenize(char *p);
Node* program();
void gen(Node* node);
Var* find_var(char* name, int len);

extern Token *token;
extern char* user_input;
