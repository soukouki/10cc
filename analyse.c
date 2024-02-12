
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "10cc.h"

Var* locals;

static Var* find_var(char* name, int len) {
  for(Var* var = locals; var; var = var->next) {
    if(var->len == len && !memcmp(name, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

static Var* new_var(char* name, int len) {
  Var* var = calloc(1, sizeof(Var));
  var->name = name;
  var->len = len;
  var->next = locals;
  if(locals) {
    var->offset = locals->offset + 8;
  } else {
    var->offset = 8;
  }
  printf("#     new_var %s %d\n", var->name, var->offset);
  locals = var;
  return var;
}

Node* analyse_semantics(Node* node) {
  // printf("# analyse_semantics %s\n", node_kinds[node->kind]);
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
    int i = 0;
    node->args_def = calloc(6, sizeof(Var*));
    while(node->args_name[i]) {
      Var* var = new_var(node->args_name[i]->name, strlen(node->args_name[i]->name));
      node->args_def[i] = var;
      i++;
    }
    node->args_def[i] = NULL;
    node->body = analyse_semantics(node->body);
    node->var = locals;
    return node;
  }
  case ND_BLOCK: {
    for(int i = 0; node->stmts[i]; i++) {
      node->stmts[i] = analyse_semantics(node->stmts[i]);
    }
    return node;
  }
  case ND_VARDEF: {
    node->var = new_var(node->name, strlen(node->name));
    return node;
  }
  case ND_RETURN: {
    node->lhs = analyse_semantics(node->lhs);
    return node;
  }
  case ND_CALL: {
    for(int i = 0; node->args_call[i]; i++) {
      node->args_call[i] = analyse_semantics(node->args_call[i]);
    }
    return node;
  }
  case ND_IDENT: {
    // ここに来るのは変数参照のみ
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
