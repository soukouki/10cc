
// ここから#includeの代わりに展開した部分

extern void* NULL;
typedef char bool;
extern char true;
extern char false;
int printf();
int sprintf();
void* calloc();

typedef struct Map Map;

Map* map_new();
void* map_get (Map* map, const char* key);
void map_put (Map* map, const char* key, void* value); // 同じキーがある場合はうまく動作しない
void map_delete(Map* map, const char* key);
void** map_values(Map* map);
char** map_keys(Map* map);

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
  Map* members;
  int size;
};

typedef struct Var Var;
struct Var {
  char* name;
  int offset; // 意味解析時に計算される, ローカル変数の場合のみ
  Type* type; // 意味解析時でのみ使う
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
  Type*  type;           // ND_TYPE, ND_FUNCDEF, ND_FUNCPROT(戻り値), ND_DECLで使う
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

Node* analyze_semantics(Node* node);

// ここまで#includeの代わりに展開した部分

// #include <stdlib.h>
// #include <string.h>
// #include <stdio.h>
// #include "10cc.h"

// Map<char*, Node(ND_STRDEF)>
Map* string_map;
int  string_count = 0;

// Map<char*, *int>
Map* constant_map; // enumの定数用

// Map<char*, Node(ND_FUNCDEF|ND_FUNCPROT)>
Map* func_map;

// Map<char*, Var>
Map* global_map;

// Map<char*, Var>
Map* local_map;
int local_offset;

int local_label = 0;

// Map<char*, *int>
Map* case_map;

// NULLチェックをしてから使うこと
char* break_label;
char* continue_label;
char* case_label; // 文字列中に%dを含み、その%dをcaseのint_valで置き換える
char* default_label;
bool  has_default;

// 関数の戻り値の型。return時に使う
Type* return_type;

static Var* find_var(char* name) {
  return map_get(local_map, name);
}

static int size_of(Type* type);

static Var* new_local_var(char* name, Type* type) {
  Var* var = calloc(1, sizeof(Var));
  var->name = name;
  local_offset += size_of(type);
  var->offset = local_offset;
  var->type = type;
  var->size = size_of(type);
  printf("#     new_var %s %s %d\n", type_kinds[var->type->kind], var->name, var->offset);
  map_put(local_map, name, var);
  return var;
}

static Var* new_global_var(char* name, Type* type) {
  Var* var = calloc(1, sizeof(Var));
  var->name = name;
  var->type = type;
  var->size = size_of(type);
  Var* exists_var = map_get(global_map, name);
  if(exists_var) {
    if(exists_var->is_extern) {
      map_delete(global_map, name);
    } else {
      error1(__FILE__, __LINE__, "変数%sはすでに定義されています", name);
    }
  }
  map_put(global_map, name, var);
  return var;
}

static Var* new_global_extern_var(char* name, Type* type) {
  Var* var = calloc(1, sizeof(Var));
  var->name = name;
  var->type = type;
  var->size = size_of(type);
  var->is_extern = true;
  Var* exists_var = map_get(global_map, name);
  if(exists_var) {
    if(exists_var->is_extern) {
      map_delete(global_map, name);
    } else {
      error1(__FILE__, __LINE__, "変数%sはすでに定義されています", name);
    }
  }
  map_put(global_map, name, var);
  return var;
}

Type* char_type() {
  Type* type = calloc(1, sizeof(Type));
  type->kind = TY_CHAR;
  return type;
}

Type* int_type() {
  Type* type = calloc(1, sizeof(Type));
  type->kind = TY_INT;
  return type;
}

Type* long_type() {
  Type* type = calloc(1, sizeof(Type));
  type->kind = TY_LONG;
  return type;
}

Type* void_type() {
  Type* type = calloc(1, sizeof(Type));
  type->kind = TY_VOID;
  return type;
}

Type* ptr_type(Type* type) {
  Type* p = calloc(1, sizeof(Type));
  p->kind = TY_PTR;
  p->ptr_to = type;
  return p;
}

Type* arr_type(Type* type, int size) {
  Type* a = calloc(1, sizeof(Type));
  a->kind = TY_ARRAY;
  a->ptr_to = type;
  a->array_size = size;
  return a;
}

Type* struct_type(char* name) {
  Type* type = calloc(1, sizeof(Type));
  type->kind = TY_STRUCT;
  type->struct_name = name;
  return type;
}

Node* int_convert(Node* node, Type* new_type) {
  Node* sign_extend = new_node(ND_CONVERT, node->loc);
  sign_extend->lhs = node;
  sign_extend->old_type = node->type;
  sign_extend->new_type = new_type;
  return sign_extend;
}

// もし数値以外の型が来た場合はNULLを返す
Type* max_type(Type* left, Type* right) {
  TypeKind lkind = left->kind;
  TypeKind rkind = right->kind;
  if(lkind != TY_CHAR && lkind != TY_INT && lkind != TY_LONG) {
    return NULL;
  }
  if(rkind != TY_CHAR && rkind != TY_INT && rkind != TY_LONG) {
    return NULL;
  }
  if(lkind == TY_LONG || rkind == TY_LONG) {
    return long_type();
  }
  if(lkind == TY_INT || rkind == TY_INT) {
    return int_type();
  }
  return char_type();
}

static int size_of(Type* type) {
  if(!type) {
    error0(__FILE__, __LINE__, "size_of: typeがNULL");
  }
  switch(type->kind) {
  case TY_CHAR:
    return 1;
  case TY_INT:
    return 4;
  case TY_LONG:
    return 8;
  case TY_PTR:
    return 8;
  case TY_ARRAY:
    return size_of(type->ptr_to) * type->array_size;
  case TY_STRUCT: {
    Struct* s = map_get(global_map, type->struct_name);
    return s->size;
  }
  case TY_VOID:
    return 0;
  }
  error0(__FILE__, __LINE__, "size_of: 未対応の型");
  return 0;
}

static int alignment(Type* type) {
  if(!type) {
    error0(__FILE__, __LINE__, "alignment: typeがNULL");
  }
  switch(type->kind) {
  case TY_CHAR:
    return 1;
  case TY_INT:
    return 4;
  case TY_LONG:
    return 8;
  case TY_PTR:
    return 8;
  case TY_ARRAY:
    return alignment(type->ptr_to);
  case TY_STRUCT: {
    Struct* s = map_get(global_map, type->struct_name);
    int max = 0;
    void* values_tmp = map_values(s->members);
    StructMember** values = values_tmp;
    for(int i = 0; values[i]; i++) {
      StructMember* sm = values[i];
      int a = alignment(sm->type);
      if(a > max) {
        max = a;
      }
    }
    return max;
  }
  case TY_VOID:
    return 0;
  }
  error0(__FILE__, __LINE__, "alignment: 未対応の型");
  return 0;
}

static int get_padding(int offset, Type* type) {
  int align = alignment(type);
  // 規格書では
  // > A structure type describes a sequentially allocated nonempty set of member objects
  // となっているが、ここでは空の構造体も考慮して進める
  if(align == 0) {
    return 0;
  }
  return (align - (offset % align)) % align;
}

typedef struct NodeAndType NodeAndType;

struct NodeAndType {
  Node* node;
  Type* type;
};

static NodeAndType* return_statement(Node* node) {
  NodeAndType* nat = calloc(1, sizeof(NodeAndType));
  nat->node = node;
  return nat;
}

static NodeAndType* return_expression(Node* node, Type* type) {
  NodeAndType* nat = calloc(1, sizeof(NodeAndType));
  nat->node = node;
  nat->type = type;
  node->type = type; // genで代入時に、正しいバイト長で変数を扱えるようにするために使う
  return nat;
}

static NodeAndType* analyze(Node* node) {
  printf("# analyze_semantics %s\n", node_kinds[node->kind]);
  switch(node->kind) {
  case ND_PROGRAM: {
    string_map = map_new();
    constant_map = map_new();
    func_map = map_new();
    global_map = map_new();
    for(int i = 0; node->funcs[i]; i++) {
      node->funcs[i] = analyze_semantics(node->funcs[i]);
    }
    void* values = map_values(string_map);
    Node** strings = values;
    node->strings = strings;
    return return_statement(node);
  }
  case ND_FUNCDEF: {
    local_map = map_new();
    local_offset = 0;
    Map* case_map_save = case_map;
    case_map = map_new();
    break_label = NULL;
    continue_label = NULL;
    case_label = NULL;
    has_default = false;
    return_type = node->type;
    
    printf("#   function %s\n", node->name);

    map_put(func_map, node->name, node);

    int i = 0;
    node->args_var = calloc(6, sizeof(Var*));
    while(node->args_node[i]) {
      Node* arg = node->args_node[i];
      Var* var = new_local_var(arg->name, arg->type);
      node->args_var[i] = var;
      i++;
    }
    node->args_var[i] = NULL;
    node->body = analyze_semantics(node->body);
    node->offset = local_offset;

    case_map = case_map_save;
    return return_statement(node);
  }
  case ND_FUNCPROT: {
    printf("#   function prototype %s\n", node->name);
    map_put(func_map, node->name, node);
    return return_statement(node);
  }
  case ND_STRUCT: {
    printf("#   struct %s\n", node->name);
    Struct* s = calloc(1, sizeof(Struct));
    s->name = node->name;
    s->members = map_new();
    int i = 0;
    int offset = 0;
    Type* alignment_max = void_type();

    while(node->struct_members[i]) {
      Node* member = node->struct_members[i];
      StructMember* sm = calloc(1, sizeof(StructMember));
      sm->type = member->type;

      offset += get_padding(offset, member->type);
      if(alignment(alignment_max) < alignment(member->type)) {
        alignment_max = member->type;
      }
      sm->offset = offset;

      offset += size_of(member->type);
      map_put(s->members, member->name, sm);
      i++;
    }

    offset += get_padding(offset, alignment_max);
    s->size = offset;
    map_put(global_map, node->name, s);
    return return_statement(node);
  }
  case ND_ENUM: {
    printf("#   enum %s\n", node->name);
    int i = 0;
    while(node->enum_members[i]) {
      Node* member = node->enum_members[i];
      int* p = calloc(1, sizeof(int));
      *p = i;
      map_put(constant_map, member->name, p);
      i++;
    }
    return return_statement(node);
  }
  case ND_GDECL: {
    printf("#   global var %s\n", node->name);
    Var* var = new_global_var(node->name, node->type);
    node->var = var;
    return return_statement(node);
  }
  case ND_GDECL_EXTERN: {
    printf("#   global var extern %s\n", node->name);
    Var* var = new_global_extern_var(node->name, node->type);
    node->var = var;
    return return_statement(node);
  }
  case ND_BLOCK: {
    for(int i = 0; node->stmts[i]; i++) {
      node->stmts[i] = analyze_semantics(node->stmts[i]);
    }
    return return_statement(node);
  }
  case ND_DECL: {
    Var* var = new_local_var(node->name, node->type);
    node->var = var;
    node->lhs = new_node(ND_VARREF, node->loc);
    node->lhs->var = var;
    node->lhs->name = node->name;
    node->lhs->type = var->type;
    if(node->rhs) {
      NodeAndType* nat = analyze(node->rhs);
      node->rhs = nat->node;
      TypeKind lkind = node->lhs->type->kind;
      TypeKind rkind = nat->type->kind;
      if(lkind != rkind) {
        node->rhs = int_convert(node->rhs, node->lhs->type);
      }
    }
    return return_statement(node);
  }
  case ND_RETURN: {
    node->lhs = analyze_semantics(node->lhs);
    TypeKind lkind = node->lhs->type->kind;
    // もし戻り値の型と違うならば、変換する
    if((lkind == TY_CHAR || lkind == TY_INT || lkind == TY_LONG) && return_type->kind != lkind) {
      node->lhs = int_convert(node->lhs, return_type);
    }
    return return_statement(node);
  }
  case ND_CALL: {
    for(int i = 0; node->args_call[i]; i++) {
      node->args_call[i] = analyze_semantics(node->args_call[i]);
    }
    Node* func = map_get(func_map, node->name);
    if(!func) {
      error_at1(__FILE__, __LINE__, node->loc, "関数%sは定義されていません", node->name);
    }
    return return_expression(node, func->type);
  }
  case ND_IDENT: {
    // ここに来るのは変数参照のみ
    Var* local_var = find_var(node->name);
    if(local_var) {
      Node* local_var_ref = new_node(ND_VARREF, node->loc);
      local_var_ref->var = local_var;
      local_var_ref->name = node->name;
      return return_expression(local_var_ref, local_var->type);
    }
    Var* global_var = map_get(global_map, node->name);
    if(global_var) {
      Node* global_var_ref = new_node(ND_GVARREF, node->loc);
      global_var_ref->var = global_var;
      global_var_ref->name = node->name;
      return return_expression(global_var_ref, global_var->type);
    }
    int* constant_num = map_get(constant_map, node->name);
    if(constant_num) {
      Node* num = new_node_num(node->loc, *constant_num);
      return return_expression(num, int_type());
    }
    error_at1(__FILE__, __LINE__, node->loc, "変数%sは定義されていません", node->name);
  }
  case ND_ADD: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    TypeKind rkind = rhs->type->kind;
    node->rhs = rhs->node;

    if(lkind == TY_ARRAY) {
      lkind = TY_PTR;
    }
    if(rkind == TY_ARRAY) {
      rkind = TY_PTR;
    }
    Type* max = max_type(lhs->type, rhs->type);
    if(max) max = max_type(max, int_type());
    if(max) {
      if(max->kind != lkind) {
        node->lhs = int_convert(lhs->node, max);
      }
      if(max->kind != rkind) {
        node->rhs = int_convert(rhs->node, max);
      }
      return return_expression(node, max);
    }
    if(lkind == TY_PTR && rkind == TY_PTR) {
      error_at0(__FILE__, __LINE__, node->loc, "ポインタ同士の加算はできません");
    }
    if(lkind == TY_PTR) {
      Node* size = new_node_num(node->loc, size_of(lhs->type->ptr_to));
      Node* mul = new_node_2branches(ND_MUL, node->loc, node->rhs, size);
      return return_expression(new_node_2branches(node->kind, node->loc, node->lhs, mul), lhs->type);
    }
    if(rkind == TY_PTR) {
      Node* size = new_node_num(node->loc, size_of(rhs->type->ptr_to));
      Node* mul = new_node_2branches(ND_MUL, node->loc, node->lhs, size);
      return return_expression(new_node_2branches(node->kind, node->loc, mul, node->rhs), rhs->type);
    }
    error_at2(__FILE__, __LINE__, node->loc, "%sと%sの加減算はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_SUB: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    TypeKind rkind = rhs->type->kind;
    node->rhs = rhs->node;

    if(lkind == TY_ARRAY) {
      lkind = TY_PTR;
    }
    if(rkind == TY_ARRAY) {
      rkind = TY_PTR;
    }
    Type* max = max_type(lhs->type, rhs->type);
    if(max) max = max_type(max, int_type());
    if(max) {
      if(max->kind != lkind) {
        node->lhs = int_convert(lhs->node, max);
      }
      if(max->kind != rkind) {
        node->rhs = int_convert(rhs->node, max);
      }
      return return_expression(node, max);
    }
    if((lkind == TY_PTR || lkind == TY_ARRAY) && (rkind == TY_PTR || rkind == TY_ARRAY)) {
      if(size_of(lhs->type->ptr_to) != size_of(rhs->type->ptr_to)) {
        error_at0(__FILE__, __LINE__, node->loc, "ポインタ同士の引き算は同じ型同士でないといけません(仕様ではさらにcompatibleであることが求められています)");
      }
      // ポインタの先の型によって割ってあげないといけない
      Node* size = new_node_num(node->loc, size_of(lhs->type->ptr_to));
      Node* sub = new_node_2branches(ND_SUB, node->loc, node->lhs, node->rhs);
      return return_expression(new_node_2branches(ND_DIV, node->loc, sub, size), int_type());
    }
    if(lkind == TY_PTR) {
      Node* size = new_node_num(node->loc, size_of(lhs->type->ptr_to));
      Node* mul = new_node_2branches(ND_MUL, node->loc, node->rhs, size);
      return return_expression(new_node_2branches(node->kind, node->loc, node->lhs, mul), lhs->type);
    }
    if(rkind == TY_PTR) {
      Node* size = new_node_num(node->loc, size_of(rhs->type->ptr_to));
      Node* mul = new_node_2branches(ND_MUL, node->loc, node->lhs, size);
      return return_expression(new_node_2branches(node->kind, node->loc, mul, node->rhs), rhs->type);
    }
    error_at2(__FILE__, __LINE__, node->loc, "%sと%sの加減算はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_MUL:
  case ND_DIV: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    TypeKind rkind = rhs->type->kind;
    node->rhs = rhs->node;

    Type* max = max_type(lhs->type, rhs->type);
    if(max) max = max_type(max, int_type());
    if(max) {
      if(max->kind != lkind) {
        node->lhs = int_convert(lhs->node, max);
      }
      if(max->kind != rkind) {
        node->rhs = int_convert(rhs->node, max);
      }
      return return_expression(node, max);
    }
    error_at2(__FILE__, __LINE__, node->loc, "%sと%sの乗除算はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_MOD: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    TypeKind rkind = rhs->type->kind;
    node->rhs = rhs->node;

    Type* max = max_type(lhs->type, rhs->type);
    if(max) max = max_type(max, int_type());
    if(max) {
      if(max->kind != lkind) {
        node->lhs = int_convert(lhs->node, max);
      }
      if(max->kind != rkind) {
        node->rhs = int_convert(rhs->node, max);
      }
      return return_expression(node, max);
    }
    error_at2(__FILE__, __LINE__, node->loc, "%sと%sの剰余算はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    TypeKind rkind = rhs->type->kind;
    node->rhs = rhs->node;

    Type* max = max_type(lhs->type, rhs->type);
    if(max) max = max_type(max, int_type());
    if(max) {
      if(max->kind != lkind) {
        node->lhs = int_convert(lhs->node, max);
      }
      if(max->kind != rkind) {
        node->rhs = int_convert(rhs->node, max);
      }
      return return_expression(node, max);
    }
    if(lkind == TY_PTR && rkind == TY_PTR) {
      return return_expression(node, int_type());
    }
    if(lkind == TY_PTR && rkind == TY_INT) {
      return return_expression(node, int_type());
    }
    if(lkind == TY_INT && rkind == TY_PTR) {
      return return_expression(node, int_type());
    }
    error_at2(__FILE__, __LINE__, node->loc, "%sと%sの比較はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_LAND: {
    node->local_label = local_label++;
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    TypeKind rkind = rhs->type->kind;
    node->rhs = rhs->node;

    Type* max = max_type(lhs->type, rhs->type);
    if(max) max = max_type(max, int_type());
    if(max) {
      if(max->kind != lkind) {
        node->lhs = int_convert(lhs->node, max);
      }
      if(max->kind != rkind) {
        node->rhs = int_convert(rhs->node, max);
      }
      return return_expression(node, max);
    }

    error_at2(__FILE__, __LINE__, node->loc, "%sと%sの論理積はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_LOR: {
    node->local_label = local_label++;
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    TypeKind rkind = rhs->type->kind;
    node->rhs = rhs->node;

    Type* max = max_type(lhs->type, rhs->type);
    if(max) max = max_type(max, int_type());
    if(max) {
      if(max->kind != lkind) {
        node->lhs = int_convert(lhs->node, max);
      }
      if(max->kind != rkind) {
        node->rhs = int_convert(rhs->node, max);
      }
      return return_expression(node, max);
    }
    
    error_at2(__FILE__, __LINE__, node->loc, "%sと%sの論理和はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_ASSIGN: {
    NodeAndType* lhs = analyze(node->lhs);
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    node->rhs = rhs->node;
    if(lhs->type->kind != rhs->type->kind && (lhs->type->kind == TY_CHAR || lhs->type->kind == TY_INT || lhs->type->kind == TY_LONG)) {
      node->rhs = int_convert(rhs->node, lhs->type);
    }
    return return_expression(node, lhs->type);
  }
  case ND_ASSIGN_ADD:
  case ND_ASSIGN_SUB:
  case ND_ASSIGN_MUL:
  case ND_ASSIGN_DIV:
  case ND_ASSIGN_MOD: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    TypeKind rkind = rhs->type->kind;
    node->rhs = rhs->node;

    Type* max = max_type(lhs->type, rhs->type);
    if(max) max = max_type(max, int_type());
    if(max) {
      if(max->kind != lkind) {
        node->lhs = int_convert(lhs->node, max);
      }
      if(max->kind != rkind) {
        node->rhs = int_convert(rhs->node, max);
      }
      return return_expression(node, max);
    }
    if(lkind == TY_PTR && rkind == TY_INT && (node->kind == ND_ASSIGN_ADD || node->kind == ND_ASSIGN_SUB)) {
      return return_expression(node, lhs->type);
    }
    if(lkind == TY_INT && rkind == TY_PTR && node->kind == ND_ASSIGN_ADD) {
      return return_expression(node, lhs->type);
    }
    error_at2(__FILE__, __LINE__, node->loc, "%sと%sの複合代入演算はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_DEREF: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    if(lkind != TY_PTR && lkind != TY_ARRAY) {
      error_at1(__FILE__, __LINE__, node->loc, "ポインタや配列でない%sを参照しようとしました", type_kinds[lkind]);
    }
    return return_expression(node, lhs->type->ptr_to);
  }
  case ND_ADDR: {
    NodeAndType* lhs = analyze(node->lhs);
    node->lhs = lhs->node;
    return return_expression(node, ptr_type(lhs->type));
  }
  case ND_SIZEOF: {
    if(node->lhs->kind == ND_TYPE) {
      // sizeof(型)の場合
      Node* type = node->lhs;
      Node* num = new_node_num(node->loc, size_of(type->type));
      return return_expression(num, int_type());
    }
    // sizeof(式)の場合
    NodeAndType *nat = analyze(node->lhs);
    Node* num = new_node_num(node->loc, size_of(nat->type));
    return return_expression(num, int_type());
  }
  case ND_OFFSETOF: {
    Type* type = node->lhs->type;
    if(type->kind != TY_STRUCT) {
      error_at0(__FILE__, __LINE__, node->loc, "offsetofの対象が構造体ではありません");
    }
    Struct* s = map_get(global_map, type->struct_name);
    StructMember* sm = map_get(s->members, node->name);
    Node* num = new_node_num(node->loc, sm->offset);
    return return_expression(num, int_type());
  }
  case ND_NOT: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    Type* max = max_type(lhs->type, int_type());
    if(max) max = max_type(max, int_type());
    if(max) {
      if(max->kind != lkind) {
        node->lhs = int_convert(lhs->node, max);
      }
      return return_expression(node, max);
    }
    return return_expression(node, int_type());
  }
  case ND_DOT: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    if(lkind != TY_STRUCT) {
      error_at1(__FILE__, __LINE__, node->loc, "構造体でない%sのメンバを参照しようとしました", type_kinds[lkind]);
    }
    Struct* s = map_get(global_map, lhs->type->struct_name);
    StructMember* sm = map_get(s->members, node->name);
    if(!sm) {
      error_at2(__FILE__, __LINE__, node->loc, "構造体%sにメンバ%sはありません", lhs->type->struct_name, node->name);
    }
    node->struct_member = sm;
    return return_expression(node, sm->type);
  }
  case ND_NUM: {
    return return_expression(node, int_type());
  }
  case ND_STR: {
    Node* exists_str = map_get(string_map, node->str_val);
    if(exists_str) {
      node->str_key = exists_str->str_key;
      return return_expression(node, ptr_type(char_type()));
    }
    node->str_key = string_count;
    Node* str_def = new_node(ND_STRDEF, node->loc);
    str_def->str_val = node->str_val;
    str_def->str_key = string_count;
    map_put(string_map, node->str_val, str_def);
    string_count++;
    return return_expression(node, ptr_type(char_type()));
  }
  case ND_CHAR: {
    // genはND_NUMで処理する
    node->kind = ND_NUM;
    return return_expression(node, char_type());
  }
  case ND_ARRAYREF: {
    Node* add = new_node_2branches(ND_ADD, node->loc, node->lhs, node->rhs);
    Node* deref = new_node_1branch(ND_DEREF, node->loc, add);
    NodeAndType* nat = analyze(deref);
    return return_expression(deref, nat->type);
  }
  case ND_IF: {
    node->local_label = local_label++;
    node->cond = analyze_semantics(node->cond);
    node->then = analyze_semantics(node->then);
    if(node->els) {
      node->els = analyze_semantics(node->els);
    }
    return return_statement(node);
  }
  case ND_WHILE: {
    node->local_label = local_label++;
    char* break_label_save = break_label;
    break_label = calloc(1, sizeof(char) * 12);
    sprintf(break_label, ".Lend%d", node->local_label);

    char* continue_label_save = continue_label;
    continue_label = calloc(1, sizeof(char) * 12);
    sprintf(continue_label, ".Lbegin%d", node->local_label);

    node->cond = analyze_semantics(node->cond);
    node->body = analyze_semantics(node->body);

    break_label = break_label_save;
    continue_label = continue_label_save;
    return return_statement(node);
  }
  case ND_FOR: {
    node->local_label = local_label++;
    
    char* break_label_save = break_label;
    break_label = calloc(1, sizeof(char) * 12);
    sprintf(break_label, ".Lend%d", node->local_label);

    char* continue_label_save = continue_label;
    continue_label = calloc(1, sizeof(char) * 12);
    sprintf(continue_label, ".Lbegin%d", node->local_label);

    if(node->init) {
      node->init = analyze_semantics(node->init);
    }
    if(node->inc) {
      node->inc = analyze_semantics(node->inc);
    }
    if(node->cond) {
      node->cond = analyze_semantics(node->cond);
    }
    node->body = analyze_semantics(node->body);

    break_label = break_label_save;
    continue_label = continue_label_save;
    return return_statement(node);
  }
  case ND_SWITCH: {
    node->local_label = local_label++;

    char* break_label_save = break_label;
    break_label = calloc(1, sizeof(char) * 12);
    sprintf(break_label, ".Lend%d", node->local_label);

    char* continue_label_save = continue_label;
    case_label = calloc(1, sizeof(char) * 12);
    sprintf(case_label, ".Lcase%d_%%d", node->local_label);

    char* default_label_save = default_label;
    default_label = calloc(1, sizeof(char) * 12);
    sprintf(default_label, ".Ldefault%d", node->local_label);

    bool has_default_save = has_default;

    Map* case_map_save = case_map;
    case_map = map_new();

    node->cond = analyze_semantics(node->cond);
    node->body = analyze_semantics(node->body);
    node->case_map = case_map;
    node->has_default = has_default;

    break_label = break_label_save;
    continue_label = continue_label_save;
    default_label = default_label_save;
    case_map = case_map_save;
    has_default = has_default_save;
    return return_statement(node);
  }
  case ND_BREAK: {
    if(!break_label) {
      error_at0(__FILE__, __LINE__, node->loc, "breakはループ内でのみ使えます");
    }
    node->goto_label = break_label;
    return return_statement(node);
  }
  case ND_CONTINUE: {
    if(!continue_label) {
      error_at0(__FILE__, __LINE__, node->loc, "continueはループ内でのみ使えます");
    }
    node->goto_label = continue_label;
    return return_statement(node);
  }
  case ND_CASE: {
    if(!case_label) {
      error_at0(__FILE__, __LINE__, node->loc, "caseはswitch内でのみ使えます");
    }
    node->goto_label = calloc(1, sizeof(char) * 12);
    int case_val;
    if(node->name) {
      int* case_val_pointer = map_get(constant_map, node->name);
      case_val = *case_val_pointer;
    } else {
      case_val = node->int_val;
    }
    sprintf(node->goto_label, case_label, case_val);
    int* case_val_pointer = calloc(1, sizeof(int));
    *case_val_pointer = case_val;
    map_put(case_map, node->goto_label, case_val_pointer);

    node->lhs = analyze_semantics(node->lhs);

    return return_statement(node);
  }
  case ND_DEFAULT: {
    if(!default_label) {
      error_at0(__FILE__, __LINE__, node->loc, "defaultはswitch内でのみ使えます");
    }
    node->goto_label = default_label;

    node->lhs = analyze_semantics(node->lhs);
    has_default = true;

    return return_statement(node);
  }
  default: {
    if(node->init) {
      node->init = analyze_semantics(node->init);
    }
    if(node->inc) {
      node->inc = analyze_semantics(node->inc);
    }
    if(node->cond) {
      node->cond = analyze_semantics(node->cond);
    }
    if(node->then) {
      node->then = analyze_semantics(node->then);
    }
    if(node->els) {
      node->els = analyze_semantics(node->els);
    }
    if(node->body) {
      node->body = analyze_semantics(node->body);
    }
    if(node->lhs) {
      node->lhs = analyze_semantics(node->lhs);
    }
    if(node->rhs) {
      node->rhs = analyze_semantics(node->rhs);
    }
    return return_statement(node);
  }
  }
}

Node* analyze_semantics(Node* node) {
  return analyze(node)->node;
}
