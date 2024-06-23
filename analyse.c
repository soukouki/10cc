
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "10cc.h"

// Map<name, Node(ND_STRDEF)>
Map* string_map;
int  string_count = 0;

// Map<name, *int>
Map* constant_map; // enumの定数用

// Map<name, Node(ND_FUNCDEF|ND_FUNCPROT)>
Map* func_map;

// Map<name, Var>
Map* global_map;

// Map<name, Var>
Map* local_map;
int local_offset;

int local_label = 0;

// Map<name, *int>
Map* case_map;

// NULLチェックをしてから使うこと
char* break_label;
char* continue_label;
char* case_label; // 文字列中に%dを含み、その%dをcaseのint_valで置き換える
char* default_label;
bool  has_default;

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

static int size_of(Type* type) {
  if(!type) {
    error0(__FILE__, __LINE__, "size_of: typeがNULL");
  }
  switch(type->kind) {
  case TY_CHAR:
    return 1;
  case TY_INT:
    return 4;
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
    while(node->struct_members[i]) {
      Node* member = node->struct_members[i];
      StructMember* sm = calloc(1, sizeof(StructMember));
      sm->type = member->type;
      sm->offset = offset;
      offset += size_of(member->type);
      map_put(s->members, member->name, sm);
      i++;
    }
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
    }
    return return_statement(node);
  }
  case ND_RETURN: {
    node->lhs = analyze_semantics(node->lhs);
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
  case ND_ADD:
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
    if(lkind == TY_INT && rkind == TY_INT) {
      return return_expression(node, lhs->type);
    }
    if(lkind == TY_CHAR && rkind == TY_CHAR) {
      return return_expression(node, lhs->type);
    }
    if(lkind == TY_PTR && rkind == TY_PTR) {
      error_at0(__FILE__, __LINE__, node->loc, "ポインタ同士の加減算はできません");
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
    if(lkind == TY_INT && rkind == TY_INT) {
      return return_expression(node, lhs->type);
    }
    if(lkind == TY_CHAR && rkind == TY_CHAR) {
      return return_expression(node, lhs->type);
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
    if(lkind == TY_INT && rkind == TY_INT) {
      return return_expression(node, lhs->type);
    }
    if(lkind == TY_CHAR && rkind == TY_CHAR) {
      return return_expression(node, lhs->type);
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
    if(lkind == TY_INT && rkind == TY_INT) {
      return return_expression(node, int_type());
    }
    if(lkind == TY_CHAR && rkind == TY_CHAR) {
      return return_expression(node, int_type());
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
    if(lkind == TY_INT && rkind == TY_INT) {
      return return_expression(node, lhs->type);
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
    if(lkind == TY_INT && rkind == TY_INT) {
      return return_expression(node, lhs->type);
    }
    error_at2(__FILE__, __LINE__, node->loc, "%sと%sの論理和はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_ASSIGN: {
    NodeAndType* lhs = analyze(node->lhs);
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    node->rhs = rhs->node;
    // 型チェックはとりあえず置いとく
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
    if(lkind == TY_INT && rkind == TY_INT) {
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
      Node* type = node->lhs;
      Node* num = new_node_num(node->loc, size_of(type->type));
      return return_expression(num, int_type());
    }
    NodeAndType *nat = analyze(node->lhs);
    Node* num = new_node_num(node->loc, size_of(nat->type));
    return return_expression(num, int_type());
  }
  case ND_NOT: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    if(lkind != TY_INT) {
      error_at1(__FILE__, __LINE__, node->loc, "intでない%sを否定しようとしました", type_kinds[lkind]);
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
