
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "10cc.h"
#include "map.h"

Map* func_map;

Map* local_map;
int local_offset;

static Var* find_var(char* name) {
  return map_get(local_map, name);
}

static int size_of(Type* type);

static Var* new_var(char* name, Type* type) {
  Var* var = calloc(1, sizeof(Var));
  var->name = name;
  local_offset += size_of(type);
  var->offset = local_offset;
  var->type = type;
  printf("#     new_var %s %s %d\n", type_kinds[var->type->kind], var->name, var->offset);
  map_put(local_map, name, var);
  return var;
}

Type* int_type() {
  Type* type = calloc(1, sizeof(Type));
  type->kind = TY_INT;
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
  return nat;
}

static NodeAndType* analyze(Node* node) {
  // printf("# analyze_semantics %s\n", node_kinds[node->kind]);
  switch(node->kind) {
  case ND_PROGRAM: {
    func_map = map_new();
    for(int i = 0; node->funcs[i]; i++) {
      node->funcs[i] = analyze_semantics(node->funcs[i]);
    }
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
      Var* var = new_var(arg->name, arg->type);
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
  case ND_BLOCK: {
    for(int i = 0; node->stmts[i]; i++) {
      node->stmts[i] = analyze_semantics(node->stmts[i]);
    }
    return return_statement(node);
  }
  case ND_DECL: {
    Var* var = new_var(node->name, node->type);
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
      error("関数%sは定義されていません", node->name);
    }
    return return_expression(node, func->type);
  }
  case ND_IDENT: {
    // ここに来るのは変数参照のみ
    Var* var = find_var(node->name);
    if(!var) {
      error("変数%sは定義されていません", node->name);
    }
    Node* new_node = new_node_ident(ND_VARREF, node->name);
    new_node->var = var;
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
    if(lkind == TY_PTR && rkind == TY_PTR) {
      error("ポインタ同士の加減算はできません");
    }
    if(lkind == TY_PTR) {
      Node* mul = new_node_2branches(ND_MUL, node->rhs, new_node_num(size_of(lhs->type->ptr_to)));
      return return_expression(new_node_2branches(node->kind, node->lhs, mul), lhs->type);
    }
    if(rkind == TY_PTR) {
      Node* mul = new_node_2branches(ND_MUL, node->lhs, new_node_num(size_of(rhs->type->ptr_to)));
      return return_expression(new_node_2branches(node->kind, mul, node->rhs), rhs->type);
    }
    error("%sと%sの加減算はできません", type_kinds[lkind], type_kinds[rkind]);
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
    error("%sと%sの掛け算はできません", type_kinds[lkind], type_kinds[rkind]);
  }
  case ND_DEREF: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    if(lkind != TY_PTR && lkind != TY_ARRAY) {
      error("ポインタ型でない%sを参照しようとしました", type_kinds[lkind]);
    }
    return return_expression(node, lhs->type->ptr_to);
  }
  case ND_ADDR: {
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    return return_expression(node, ptr_type(lhs->type));
  }
  case ND_SIZEOF: {
    NodeAndType *nat = analyze(node->lhs);
    Node* num = new_node_num(size_of(nat->type));
    return return_expression(num, int_type());
  }
  case ND_NUM: {
    return return_expression(node, int_type());
  }
  case ND_ARRAYREF: {
    // a[i] を *(a+i) に変換する
    NodeAndType* lhs = analyze(node->lhs);
    TypeKind lkind = lhs->type->kind;
    node->lhs = lhs->node;
    NodeAndType* rhs = analyze(node->rhs);
    TypeKind rkind = rhs->type->kind;
    node->rhs = rhs->node;
    if(lkind != TY_PTR && lkind != TY_ARRAY) {
      error("ポインタ型でない%sを参照しようとしました", type_kinds[lkind]);
    }
    if(rkind != TY_INT) {
      error("整数型でない%sを参照しようとしました", type_kinds[rkind]);
    }
    Node* add = new_node_2branches(ND_ADD, node->lhs, node->rhs);
    Node* deref = new_node_1branch(ND_DEREF, add);
    return return_expression(deref, lhs->type->ptr_to);
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
