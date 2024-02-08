#include <stdio.h>

#include "10cc.h"

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
};

void gen_ref(Node* node) {
  if(node->kind != ND_LVAR && node->kind != ND_REF) error("変数の参照ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->var->offset);
  printf("  push rax\n");
}

void gen(Node* node) {
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

  printf("# end gen %s\n", kinds[node->kind]);
}