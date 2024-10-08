
#include "std.h"

#include "map.h"

typedef enum TokenKind TokenKind;
enum TokenKind {
  TK_SYMBOL,
  TK_IDENT,
  TK_NUM,
  TK_STR,
  TK_CHAR,
  TK_EOF,
};

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token* next;
  int val;
  char* str;
  int len;
  char* file;
  int line;
};

// ここを書き換えたときには、main.cのtype_kindsも書き換えること！
typedef enum TypeKind TypeKind;
enum TypeKind {
  TY_CHAR,
  TY_INT,
  TY_LONG,
  TY_PTR,
  TY_ARRAY,
  TY_STRUCT,
  TY_VOID,
};

typedef struct Type Type;
struct Type {
  TypeKind kind;
  Type* ptr_to;
  int array_size;
  char* struct_name;
};

typedef struct StructMember StructMember;
struct StructMember {
  Type* type;
  int offset;
};

typedef struct Struct Struct;
struct Struct {
  char* name;
  // Map<name, StructMember>
  Map* members;
  int size;
};

typedef struct Var Var;
struct Var {
  char* name;
  int offset; // 意味解析時に計算される, ローカル変数の場合のみ
  Type* type;
  int size;
  bool is_extern;
};

typedef enum NodeKind NodeKind;
// ここを書き換えたときには、main.cのnode_kindsも書き換えること！
enum NodeKind {
  // 2項演算子(lhs, rhsを持つ)
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_MOD,
  // 比較演算子
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,     // GT, GEはLT, LEを使って表現できる
  // 論理演算子
  ND_LAND,
  ND_LOR,
  // 代入演算子
  ND_ASSIGN,     // 代入, lhs, rhsを持つ
  ND_ASSIGN_ADD, // 加算代入, lhs, rhsを持つ
  ND_ASSIGN_SUB, // 減算代入, lhs, rhsを持つ
  ND_ASSIGN_MUL, // 乗算代入, lhs, rhsを持つ
  ND_ASSIGN_DIV, // 除算代入, lhs, rhsを持つ
  ND_ASSIGN_MOD, // 剰余代入, lhs, rhsを持つ
  // 単項演算子(lhsを持つ)
  ND_NOT,    // 単項not, lhsを持つ
  // ポインタ
  ND_ADDR,   // 単項&, lhsを持つ
  ND_DEREF,  // 単項*, lhsを持つ
  // 構造体
  ND_DOT,    // lhs, nameを持つ

  // リテラル
  ND_NUM, // valを持つ
  ND_STR, // valを持つ
  ND_CHAR,// valを持つ(意味解析時にND_NUMに置き換える)

  // 関数・コンパイラマジック
  ND_CALL,     // 関数呼び出し, name, args_callを持つ
  ND_SIZEOF,   // sizeof, lhsを持つ
  ND_OFFSETOF, // offsetof, lhs, nameを持つ

  // 構文
  ND_VARREF,   // 変数の参照, name, varを持つ
  ND_GVARREF,  // グローバル変数の参照, name, varを持つ
  ND_ARRAYREF, // 配列の参照, lhs, rhsを持つ
  ND_RETURN,   // return, lhsを持つ
  ND_IF,       // if文, cond, then, elsを持つ
  ND_WHILE,    // while文, cond, bodyを持つ
  ND_FOR,      // for文, init, cond, inc, bodyを持つ
  ND_SWITCH,   // switch文, cond, bodyを持つ
  ND_BLOCK,    // ブロック, stmtsを持つ
  ND_FUNCDEF,  // 関数定義, name, args_name, args_type, ret_type, bodyを持つ
  ND_FUNCPROT, // 関数プロトタイプ, name, args_name, args_type, ret_typeを持つ
  ND_STRDEF,   // 文字列定義, name, str_valを持つ
  ND_STRUCT,   // 構造体, name, struct_membersを持つ
  ND_ENUM,     // 列挙型, name, enum_membersを持つ
  ND_BREAK,    // break, goto_labelを持つ
  ND_CONTINUE, // continue, goto_labelを持つ
  ND_CASE,     // case, int_val, goto_labelを持つ
  ND_DEFAULT,  // default, goto_labelを持つ

  ND_DECL,         // 宣言, name, type, var, lhs, rhs(Nullable)を持つ 変数定義や関数の仮引数で使う
  ND_GDECL,        // グローバル変数宣言, name, type, var, lhs(Nullable)を持つ
  ND_GDECL_EXTERN, // グローバル変数宣言(外部参照), name, typeを持つ
  ND_TYPE,         // 型, typeを持つ
  ND_IDENT,        // 識別子(意味解析時に置き換える), nameを持つ
  ND_CONVERT,  // 符号拡張, lhs, old_type, new_typeを持つ

  // その他
  ND_PROGRAM, // プログラム全体, funcs, stringsを持つ
};

typedef struct Node Node;
struct Node {
  NodeKind kind;
  char*  loc;            // エラー表示用
  Node*  init;           // forで使う
  Node*  inc;            // forで使う
  Node*  cond;           // if, while, for, switchで使う
  Node*  then;           // ifで使う
  Node*  els;            // ifで使う
  Node*  body;           // while, for, switchで使う
  // Map<char*, int>
  Map*   case_map;       // switchで使う ラベルをキーにして、caseの値を値にもつ
  bool   has_default;    // switchで使う
  Node** stmts;          // ブロックで使う
  Node** args_call;      // 関数呼び出しで使う
  Node** funcs;          // プログラムで使う
  Node** strings;        // プログラムで使う
  Node** struct_members; // 構造体のメンバ, 中身はND_DECL
  Node** enum_members;   // 列挙型のメンバ, 中身はND_IDENT
  Node*  lhs;            // 2項演算子, 代入(型), return, 単項&, 単項*, ポインタ型, case, defaultで使う
  Node*  rhs;            // 2項演算子, 代入(値)で使う
  int    int_val;        // 数値リテラル, caseで使う
  char*  str_val;        // 文字列リテラルの場合に使う
  int    str_key;        // 文字列リテラルの場合に使う
  char*  name;           // 関数の定義, 関数呼び出し, 変数の参照, case, offsetofで使う
  Var*   var;            // ND_LVARの場合に使う
  Type*  type;           // ND_TYPE, ND_FUNCDEF, ND_FUNCPROT(戻り値), ND_DECL, 各種値で使う
  Type*  old_type;       // ND_CONVERTで使う
  Type*  new_type;       // ND_CONVERTで使う
  Node** args_node;      // 関数の定義で使う(パース->意味解析)
  Var**  args_var;       // 関数の定義で使う(意味解析->コード生成)
  int    offset;         // 関数の定義で使う(意味解析->コード生成)
  char*  goto_label;     // break, continue, case, defaultで使う(意味解析->コード生成)
  int    local_label;    // if, while, for, &&, ||で使う(意味解析->コード生成)
  StructMember* struct_member; // ND_DOTの場合に使う
};

void error0(char* file, int line, char* fmt);
void error1(char* file, int line, char* fmt, char* arg1);
void error2(char* file, int line, char* fmt, char* arg1, char* arg2);
void error3(char* file, int line, char* fmt, char* arg1, char* arg2, char* arg3);
void error_at0(char* file, int line, char* loc, char* fmt);
void error_at1(char* file, int line, char* loc, char* fmt, char* arg1);
void error_at2(char* file, int line, char* loc, char* fmt, char* arg1, char* arg2);

extern char** node_kinds;
extern char** type_kinds;

Node* new_node(NodeKind kind, char* loc);
Node* new_node_1branch(NodeKind kind, char* loc, Node* lhs);
Node* new_node_2branches(NodeKind kind, char* loc, Node* lhs, Node* rhs);
Node* new_node_num(char* loc, int val);
Node* new_node_ident(NodeKind kind, char* loc, char* name);

Type* char_type();
Type* int_type();
Type* long_type();
Type* void_type();
Type* ptr_type(Type* ptr_to);
Type* arr_type(Type* ptr_to, int array_size);
Type* struct_type(char* name);

char* read_file(char* path);
Token* tokenize(char *p);
Node* parse();
Node* analyze_semantics(Node* node);
void gen(Node* node);

extern Token *token;
extern char* user_input;
