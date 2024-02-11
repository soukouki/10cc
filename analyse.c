
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "10cc.h"

Var* locals;

static Var* find_var(char* name, int len) {
  for(Var* var = locals; var; var = var->next) {
    if(var->name_len == len && !memcmp(var->name, name, len)) {
      return var;
    }
  }
  return NULL;
}

static Var* new_var(char* name, int len, Type* type) {
  Var* var = calloc(1, sizeof(Var));
  var->name = name;
  var->name_len = len;
  var->next = locals;
  int offset = locals ? locals->offset + type->size : type->size;
  var->offset = offset;
  locals = var;
  return var;
}

static Type* type_from_node(Node* node) {
  switch(node->kind) {
  case ND_INT: {
    Type* type = calloc(1, sizeof(Type));
    type->kind = TY_INT;
    type->size = 8;
    return type;
  }
  case ND_POINTER: {
    Type* type = calloc(1, sizeof(Type));
    type->kind = TY_PTR;
    type->ptr_to = type_from_node(node->lhs);
    type->size = 64;
    return type;
  }
  default: {
    error("その型には対応していません %s %s", node_kinds[node->kind], node->name);
  }
  }
}

static Node* type_into_pointer(Node* node) {
  switch(node->kind) {
  case ND_INT: {
    Node* new_node = new_node_ident(ND_POINTER, node->name);
    new_node->lhs = node;
    return new_node;
  }
  case ND_POINTER: {
    Node* new_node = new_node_ident(ND_POINTER, node->name);
    new_node->lhs = type_into_pointer(node->lhs);
    return new_node;
  }
  default: {
    error("その型には対応していません %s %s", node_kinds[node->kind], node->name);
  }
  }
}

Node* analyse_semantics(Node* node) {
  printf("# analyse_semantics %s\n", node_kinds[node->kind]);
  switch(node->kind) {
  case ND_PROGRAM: {
    for(int i = 0; node->funcs[i]; i++) {
      node->funcs[i] = analyse_semantics(node->funcs[i]);
    }
    return node;
  }
  case ND_FUNCDEF: {
    locals = NULL;
    printf("#   function %s\n", node->name);
    Node* func = new_node_ident(ND_FUNCDEF, node->name);

    FuncDefSemantics* semantics = calloc(1, sizeof(FuncDefSemantics));
    Type* ret_type = type_from_node(node->func_def_ast->ret_type);
    semantics->ret_type = type_from_node(node->func_def_ast->ret_type);
    
    semantics->args = calloc(6, sizeof(Var*));
    for(int i = 0; node->func_def_ast->args[i]; i++) {
      FuncDefASTTypeAndName* arg = node->func_def_ast->args[i];
      Var* var = new_var(arg->name, strlen(arg->name), type_from_node(arg->type));
      semantics->args[i] = var;
    }
    func->func_def_semantics = semantics;

    func->body = analyse_semantics(node->body);
    semantics->stack_offset = locals ? locals->offset : 0;
    return func;
  }
  case ND_BLOCK: {
    for(int i = 0; node->stmts[i]; i++) {
      node->stmts[i] = analyse_semantics(node->stmts[i]);
    }
    return node;
  }
  case ND_VARDEF: {
    Type* type = type_from_node(node->lhs);
    node->var = new_var(node->name, strlen(node->name), type);
    return node;
  }
  case ND_RETURN: {
    node->lhs = analyse_semantics(node->lhs);
    return node;
  }
  case ND_ASSIGN: {
    Node* lhs = new_node_ident(ND_LVAR, node->lhs->name);
    Var* var = find_var(lhs->name, strlen(lhs->name));
    if(!var) {
      error("変数%sは定義されていません", lhs->name);
    }
    lhs->var = var;
    node->lhs = lhs;
    node->rhs = analyse_semantics(node->rhs);
    return node;
  }
  case ND_CALL: {
    for(int i = 0; node->args_call[i]; i++) {
      node->args_call[i] = analyse_semantics(node->args_call[i]);
    }
    return node;
  }
  case ND_IDENT: {
    Var* var = find_var(node->name, strlen(node->name));
    if(!var) {
      error("変数%sは定義されていません", node->name);
    }
    Node* new_node = new_node_ident(ND_VARREF, node->name);
    new_node->var = var;
    return new_node;
  }
  default: {
    if(node->init) {
      node->init = analyse_semantics(node->init);
    }
    if(node->inc) {
      node->inc = analyse_semantics(node->inc);
    }
    if(node->cond) {
      node->cond = analyse_semantics(node->cond);
    }
    if(node->then) {
      node->then = analyse_semantics(node->then);
    }
    if(node->els) {
      node->els = analyse_semantics(node->els);
    }
    if(node->body) {
      node->body = analyse_semantics(node->body);
    }
    if(node->lhs) {
      node->lhs = analyse_semantics(node->lhs);
    }
    if(node->rhs) {
      node->rhs = analyse_semantics(node->rhs);
    }
    return node;
  }
  }
}
