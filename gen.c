#include <stdio.h>

#include "10cc.h"

int local_label = 0;

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
  case ND_GVARREF:
    printf("  push offset %s\n", node->name);
    break;
  default:
    error("%sは変数の参照ではありません", node_kinds[node->kind]);
    break;
  }
}

void gen_ref_push(Node* node) {
  printf("  pop rax\n");
  if(node->type->kind == TY_INT) {
    printf("  mov eax, [rax]\n");
  } else if(node->type->kind == TY_CHAR) {
    printf("  movzx eax, BYTE PTR [rax]\n");
  } else {
    printf("  mov rax, [rax]\n");
  }
  printf("  push rax\n");
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
    if(lval->type->kind == TY_INT) {
      printf("  mov [rax], edi\n");
    } else if(lval->type->kind == TY_CHAR) {
      printf("  mov [rax], dil\n");
    } else {
      printf("  mov [rax], rdi\n");
    }
    break;
  }
  case ND_ADDR: {
    gen_ref(node->lhs);
    break;
  }
  case ND_DEREF: {
    gen(node->lhs);
    gen_ref_push(node);
    break;
  }
  case ND_VARREF:
  case ND_GVARREF: {
    gen_ref(node);
    gen_ref_push(node);
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
    printf(".text\n");
    printf("%s:\n", node->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", node->offset);
    if(node->args_var[0]) {
      if(node->args_var[0]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], edi\n", node->args_var[0]->offset);
      } else if(node->args_var[0]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], dil\n", node->args_var[0]->offset);
      } else {
        printf("  mov [rbp-%d], rdi\n", node->args_var[0]->offset);
      }
    }
    if(node->args_var[1]) {
      if(node->args_var[1]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], esi\n", node->args_var[1]->offset);
      } else if(node->args_var[1]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], sil\n", node->args_var[1]->offset);
      } else {
        printf("  mov [rbp-%d], rsi\n", node->args_var[1]->offset);
      }
    }
    if(node->args_var[2]) {
      if(node->args_var[2]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], edx\n", node->args_var[2]->offset);
      } else if(node->args_var[2]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], dl\n", node->args_var[2]->offset);
      } else {
        printf("  mov [rbp-%d], rdx\n", node->args_var[2]->offset);
      }
    }
    if(node->args_var[3]) {
      if(node->args_var[3]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], ecx\n", node->args_var[3]->offset);
      } else if(node->args_var[3]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], cl\n", node->args_var[3]->offset);
      } else {
        printf("  mov [rbp-%d], rcx\n", node->args_var[3]->offset);
      }
    }
    if(node->args_var[4]) {
      if(node->args_var[4]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], r8d\n", node->args_var[4]->offset);
      } else if(node->args_var[4]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], r8b\n", node->args_var[4]->offset);
      } else {
        printf("  mov [rbp-%d], r8\n", node->args_var[4]->offset);
      }
    }
    if(node->args_var[5]) {
      if(node->args_var[5]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], r9d\n", node->args_var[5]->offset);
      } else if(node->args_var[5]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], r9b\n", node->args_var[5]->offset);
      } else {
        printf("  mov [rbp-%d], r9\n", node->args_var[5]->offset);
      }
    }
    gen(node->body);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    break;
  }
  case ND_FUNCPROT: {
    break;
  }
  case ND_PROGRAM: {
    for(int i = 0; node->funcs[i]; i++) {
      gen(node->funcs[i]);
    }
    break;
  }
  case ND_DECL: {
    break;
  }
  case ND_GDECL: {
    printf(".data\n");
    printf("%s:\n", node->name);
    printf("  .zero %d\n", node->var->size);
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