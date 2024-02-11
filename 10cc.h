
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

typedef struct Type Type;
typedef struct Var Var;
typedef struct FuncDefASTTypeAndName FuncDefASTTypeAndName;
typedef struct FuncDefAST FuncDefAST;
typedef struct FuncDefSemantics FuncDefSemantics;
typedef struct Node Node;

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
  // 2項演算子はlhs, rhsを持つ

  // リテラル
  ND_NUM, // valを持つ

  // ポインタ
  ND_ADDR,  // 単項&, lhsを持つ
  ND_DEREF, // 単項*, lhsを持つ

  // 構文
  ND_VARREF,  // 変数の評価, name, varを持つ
  ND_CALL,    // 関数呼び出し, name, args_callを持つ
  ND_ASSIGN,  // 代入, lhs, rhsを持つ
  ND_LVAR,    // 左辺値, varを持つ
  ND_RETURN,  // return, lhsを持つ
  ND_IF,      // if文, cond, then, elsを持つ
  ND_WHILE,   // while文, cond, bodyを持つ
  ND_FOR,     // for文, init, cond, inc, bodyを持つ
  ND_BLOCK,   // ブロック, stmtsを持つ
  ND_VARDEF,  // 変数の定義, var, nameを持つ
  ND_FUNCDEF, // 関数定義, name, args_name, bodyを持つ

  // 型
  ND_INT,     // int型(意味解析時に置き換える)

  // その他
  ND_PROGRAM, // プログラム全体, funcsを持つ
  ND_IDENT,   // 識別子(意味解析時に置き換える), nameを持つ
  ND_POINTER, // 型に現れるポインタ(意味解析時に置き換える), lhsを持つ
} NodeKind;

typedef enum {
  TY_INT,
  TY_PTR,
} TypeKind;

struct Type {
  TypeKind kind;
  Type* ptr_to;
  int size;
};

struct Var {
  Var* next;
  char* name;
  int name_len;
  Type* type; // 意味解析時に計算される
  int offset; // 意味解析時に計算される
};

struct FuncDefASTTypeAndName {
  Node* type;
  char* name;
};

// パース->意味解析で使う
struct FuncDefAST {
  Node* ret_type;
  FuncDefASTTypeAndName** args;
};

// 意味解析->コード生成で使う
struct FuncDefSemantics {
  Type* ret_type;
  Var** args;
  int   stack_offset;
};

struct Node {
  NodeKind kind;
  Node*    init;      // forで使う
  Node*    inc;       // forで使う
  Node*    cond;      // if, while, forで使う
  Node*    then;      // ifで使う
  Node*    els;       // ifで使う
  Node*    body;      // while, for, 関数の定義で使う
  Node**   stmts;     // ブロックで使う
  Node**   args_call; // 関数呼び出しで使う
  Node**   funcs;     // プログラムで使う
  Node*    lhs;       // 2項演算子, 代入, return, 単項&, 単項*, 変数定義で使う
  Node*    rhs;       // 2項演算子, 代入で使う
  int      val;       // 数値リテラルの場合に使う
  char*    name;      // 関数の定義, 関数呼び出し, 変数の参照で使う
  Var*     var;       // ND_LVAR, ND_VARREFで使う
  FuncDefAST*       func_def_ast;       // 関数の定義で使う(パース->意味解析)
  FuncDefSemantics* func_def_semantics; // 関数の定義で使う(意味解析->コード生成)
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

extern char** node_kinds;

Node* new_node_2branch(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
Node* new_node_ident(NodeKind kind, char* name);

Token* tokenize(char *p);
Node* parse();
Node* analyse_semantics(Node* node);
void gen(Node* node);

extern Token *token;
extern char* user_input;
