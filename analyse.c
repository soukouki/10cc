
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "10cc.h"
#include "map.h"

Map* string_map;
int  string_count = 0;

Map* func_map;

Map* global_map;

Map* local_map;
int local_offset;

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
  map_put(global_map, name, var);
  return var;
}

Type* int_type() {
  Type* type = calloc(1, sizeof(Type));
  type->kind = TY_INT;
  return type;
}

Type* char_type() {
  Type* type = calloc(1, sizeof(Type));
  type->kind = TY_CHAR;
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

static int size_of(Type* type) {
  switch(type->kind) {
  case TY_INT:
    return 4;
  case TY_CHAR:
    return 1;
  case TY_PTR:
    return 8;
  case TY_ARRAY:
    return size_of(type->ptr_to) * type->array_size;
  }
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
    func_map = map_new();
    global_map = map_new();
    for(int i = 0; node->funcs[i]; i++) {
      node->funcs[i] = analyze_semantics(node->funcs[i]);
    }
    node->strings = (Node**)map_values(string_map);
    return return_statement(node);
  }
  case ND_FUNCDEF: {
    local_map = map_new();
    local_offset = 0;
    
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

    return return_statement(node);
  }
  case ND_FUNCPROT: {
    printf("#   function prototype %s\n", node->name);
    map_put(func_map, node->name, node);
    return return_statement(node);
  }
  case ND_GDECL: {
    printf("#   global var %s\n", node->name);
    Var* var = new_global_var(node->name, node->type);
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
      error_at(node->loc, "関数%sは定義されていません", node->name);
    }
    return return_expression(node, func->type);
  }
  case ND_IDENT: {
    // ここに来るのは変数参照のみ
    Var* var = find_var(node->name);
    _Bool is_global = var == NULL;
    if(!var) {
      var = map_get(global_map, node->name);
      if(!var) {
        error_at(node->loc, "変数%sは定義されていません", node->name);
      }
    }

    Node* new_node;
    if(is_global) {
      new_node = new_node_ident(ND_GVARREF, node->loc, node->name);
    } else {
      new_node = new_node_ident(ND_VARREF, node->loc, node->name);
    }
    new_node->var = var;
    if(var->type->kind == TY_ARRAY) {
      Node* addr = new_node_1branch(ND_ADDR, node->loc, new_node);
      return return_expression(addr, ptr_type(var->type->ptr_to));
    }
    return return_expression(new_node, new_node->var->type);
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
      error_at(node->loc, "ポインタ同士の加減算はできません");
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
    error_at(node->loc, "%sと%sの加減算はできません", type_kinds[lkind], type_kinds[rkind]);
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
    error_at(node->loc, "%sと%sの乗除算はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_DEREF: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    if(lkind == TY_ARRAY) {
      // int a[3]; *a = 1; が来たら、 *&aに変換する
      Node* addr = new_node_1branch(ND_ADDR, node->loc, node->lhs);
      return return_expression(new_node_1branch(ND_DEREF, node->loc, addr), lhs->type->ptr_to);
    }
    if(lkind != TY_PTR) {
      error_at(node->loc, "ポインタでない%sを参照しようとしました", type_kinds[lkind]);
    }
    return return_expression(node, lhs->type->ptr_to);
  }
  case ND_ADDR: {
    NodeAndType* lhs = analyze(node->lhs);
    node->lhs = lhs->node;
    return return_expression(node, ptr_type(lhs->type));
  }
  case ND_SIZEOF: {
    NodeAndType *nat = analyze(node->lhs);
    Node* num = new_node_num(node->loc, size_of(nat->type));
    return return_expression(num, int_type());
  }
  case ND_NUM: {
    return return_expression(node, int_type());
  }
  case ND_STR: {
    Node* str_def = new_node(ND_STRDEF, node->loc);
    str_def->str_val = node->str_val;
    str_def->str_key = string_count;
    node->str_key = string_count;
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
