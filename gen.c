#include <stdio.h>

#include "10cc.h"

int local_label = 0;

char** node_kinds;

void gen_ref(Node* node) {
  switch(node->kind) {
  case ND_DEREF:
    gen(node->lhs);
    break;
  case ND_VARREF:
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->var->offset);
    printf("  push rax\n");
    break;
  default:
    error("変数の参照ではありません");
    break;
  }
}

void gen(Node* node) {
  if(node == NULL) {
    error("ノードがありません");
  }
  printf("# gen %s\n", node_kinds[node->kind]);
  switch (node->kind) {
  case ND_NUM: {
    printf("  push %d\n", node->val);
    break;
  }
  case ND_ASSIGN: {
    printf("# ND_ASSIGN %s %s\n", node_kinds[node->lhs->kind], node_kinds[node->rhs->kind]);
    Node* lval = node->lhs;
    Node* rval = node->rhs;
    gen_ref(lval);
    gen(rval);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    break;
  }
  case ND_ADDR: {
    gen_ref(node->lhs);
    break;
  }
  case ND_DEREF: {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    break;
  }
  case ND_VARREF: {
    gen_ref(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    break;
  }
  case ND_RETURN: {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    break;
  }
  case ND_IF: {
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    int if_label = local_label;
    local_label++;
    if(node->els) {
      printf("  je  .Lelse%d\n", if_label);
      gen(node->then);
      printf("  jmp .Lend%d\n", if_label);
      printf(".Lelse%d:\n", if_label);
      gen(node->els);
    } else {
      printf("  je  .Lend%d\n", if_label);
      gen(node->then);
    }
    printf(".Lend%d:\n", if_label);
    break;
  }
  case ND_WHILE: {
    int while_label = local_label;
    local_label++;
    printf(".Lbegin%d:\n", while_label);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", while_label);
    gen(node->body);
    printf("  jmp .Lbegin%d\n", while_label);
    printf(".Lend%d:\n", while_label);
    printf("  push 0\n"); // なんか必要
    break;
  }
  case ND_FOR: {
    int for_label = local_label;
    local_label++;
    if(node->init) gen(node->init);
    printf(".Lbegin%d:\n", for_label);
    if(node->cond) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", for_label);
    }
    gen(node->body);
    if(node->inc) gen(node->inc);
    printf("  jmp .Lbegin%d\n", for_label);
    printf(".Lend%d:\n", for_label);
    printf("  push 0\n"); // なんか必要
    local_label++;
    break;
  }
  case ND_BLOCK: {
    for(int i = 0; node->stmts[i]; i++) {
      gen(node->stmts[i]);
    }
    break;
  }
  case ND_CALL: {
    if(node->args_call[0]) {
      gen(node->args_call[0]);
      printf("  pop rdi\n");
    }
    if(node->args_call[1]) {
      gen(node->args_call[1]);
      printf("  pop rsi\n");
    }
    if(node->args_call[2]) {
      gen(node->args_call[2]);
      printf("  pop rdx\n");
    }
    if(node->args_call[3]) {
      gen(node->args_call[3]);
      printf("  pop rcx\n");
    }
    if(node->args_call[4]) {
      gen(node->args_call[4]);
      printf("  pop r8\n");
    }
    if(node->args_call[5]) {
      gen(node->args_call[5]);
      printf("  pop r9\n");
    }
    printf("  call %s\n", node->name);
    printf("  push rax\n");
    break;
  }
  case ND_FUNCDEF: {
    printf("%s:\n", node->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    Var* var = node->var;
    printf("  sub rsp, %d\n", var == NULL ? 0 : var->offset);
    if(node->args_def[0]) {
      printf("  mov [rbp-%d], rdi\n", node->args_def[0]->offset);
    }
    if(node->args_def[1]) {
      printf("  mov [rbp-%d], rsi\n", node->args_def[1]->offset);
    }
    if(node->args_def[2]) {
      printf("  mov [rbp-%d], rdx\n", node->args_def[2]->offset);
    }
    if(node->args_def[3]) {
      printf("  mov [rbp-%d], rcx\n", node->args_def[3]->offset);
    }
    if(node->args_def[4]) {
      printf("  mov [rbp-%d], r8\n", node->args_def[4]->offset);
    }
    if(node->args_def[5]) {
      printf("  mov [rbp-%d], r9\n", node->args_def[5]->offset);
    }
    gen(node->body);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    break;
  }
  case ND_PROGRAM: {
    for(int i = 0; node->funcs[i]; i++) {
      gen(node->funcs[i]);
    }
    break;
  }
  case ND_VARDEF: {
    break;
  }
  default: {
    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->kind) {
      case ND_ADD:
        printf("  add rax, rdi\n");
        break;
      case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
      case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
      case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
      case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
      case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
      case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
      case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
      default:
        error("不正なノードです");
        break;
    }
    printf("  push rax\n");
    break;
  }
  }

  printf("# endgen %s\n", node_kinds[node->kind]);
}