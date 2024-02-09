#include <stdio.h>

#include "10cc.h"

int local_label = 0;

char* kinds [] = {
  "ND_ADD",
  "ND_SUB",
  "ND_MUL",
  "ND_DIV",
  "ND_EQ",
  "ND_NE",
  "ND_LT",
  "ND_LE",
  "ND_NUM",
  "ND_REF",
  "ND_ASSIGN",
  "ND_LVAR",
  "ND_RETURN",
  "ND_IF",
  "ND_WHILE",
  "ND_FOR",
};

void gen_ref(Node* node) {
  if(node->kind != ND_LVAR && node->kind != ND_REF) error("変数の参照ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->var->offset);
  printf("  push rax\n");
}

void gen(Node* node) {
  if(node == NULL) {
    error("ノードがありません");
  }
  printf("# gen %s\n", kinds[node->kind]);
  switch (node->kind)
  {
  case ND_NUM:
    printf("  push %d\n", node->val);
    break;
  case ND_ASSIGN:
    Node* lval = node->lhs;
    Node* rval = node->rhs;
    if(lval->kind != ND_LVAR) error("代入の左辺値が変数ではありません");
    gen_ref(lval);
    gen(rval);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    break;
  case ND_REF:
    gen_ref(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    break;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    break;
  case ND_IF:
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
  case ND_WHILE:
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
  case ND_FOR:
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
  default:
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
  }

  printf("# endgen %s\n", kinds[node->kind]);
}