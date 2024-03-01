
#include "map.h"

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
  TY_INT,
  TY_PTR,
  TY_ARRAY,
} TypeKind;

typedef struct Type Type;

struct Type {
  TypeKind kind;
  Type* ptr_to;
  int array_size;
};

typedef struct Var Var;

struct Var {
  char* name;
  int offset; // 意味解析時に計算される
  Type* type; // 意味解析時でのみ使う
};

typedef enum {
  // 2項演算子(lhs, rhsを持つ)
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  // 比較演算子
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE, // GT, GEはLT, LEを使って表現できる
  // 単項演算子(lhsを持つ)
  ND_SIZEOF,   // sizeof演算子, lhsを持つ
  // ポインタ
  ND_ADDR,  // 単項&, lhsを持つ
  ND_DEREF, // 単項*, lhsを持つ

  // リテラル
  ND_NUM, // valを持つ

  // 構文
  ND_VARREF,   // 変数の評価, name, varを持つ
  ND_ARRAYREF, // 配列の参照, lhs, rhsを持つ
  ND_CALL,     // 関数呼び出し, name, args_callを持つ
  ND_ASSIGN,   // 代入, lhs, rhsを持つ
  ND_RETURN,   // return, lhsを持つ
  ND_IF,       // if文, cond, then, elsを持つ
  ND_WHILE,    // while文, cond, bodyを持つ
  ND_FOR,      // for文, init, cond, inc, bodyを持つ
  ND_BLOCK,    // ブロック, stmtsを持つ
  ND_FUNCDEF,  // 関数定義, name, args_name, args_type, ret_type, bodyを持つ
  ND_FUNCPROT, // 関数プロトタイプ, name, args_name, args_type, ret_typeを持つ

  ND_DECL,  // 宣言, name, typeを持つ 変数定義や関数の仮引数で使う
  ND_TYPE,  // 型, typeを持つ
  ND_IDENT, // 識別子(意味解析時に置き換える), nameを持つ

  // その他
  ND_PROGRAM, // プログラム全体, funcsを持つ
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node*  init;        // forで使う
  Node*  inc;         // forで使う
  Node*  cond;        // if, while, forで使う
  Node*  then;        // ifで使う
  Node*  els;         // ifで使う
  Node*  body;        // while, forで使う
  Node** stmts;       // ブロックで使う
  Node** args_call;   // 関数呼び出しで使う
  Node** funcs;       // プログラムで使う
  Node*  lhs;         // 2項演算子, 代入(型), return, 単項&, 単項*, ポインタ型で使う
  Node*  rhs;         // 2項演算子, 代入(値)で使う
  int    val;         // 数値リテラルの場合に使う
  char*  name;        // 関数の定義, 関数呼び出し, 変数の参照で使う
  Var*   var;         // ND_LVARの場合に使う
  Type*  type;        // ND_TYPE, ND_FUNCDEF, ND_FUNCPROT(戻り値), ND_DECLで使う
  Node** args_node;   // 関数の定義で使う(パース->意味解析)
  Var**  args_var;    // 関数の定義で使う(意味解析->コード生成)
  int    offset;      // 関数の定義で使う(意味解析->コード生成)
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

extern char** node_kinds;
extern char** type_kinds;

Node* new_node(NodeKind kind);
Node* new_node_1branch(NodeKind kind, Node* lhs);
Node* new_node_2branches(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
Node* new_node_ident(NodeKind kind, char* name);

Type* int_type();
Type* ptr_type(Type* ptr_to);
Type* arr_type(Type* ptr_to, int array_size);

Token* tokenize(char *p);
Node* parse();
Node* analyze_semantics(Node* node);
void gen(Node* node);

extern Token *token;
extern char* user_input;
