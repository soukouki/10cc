
#include "10cc.h"

char* escape(char*);

void gen_ref(Node* node) {
  printf("# gen_ref %s\n", node_kinds[node->kind]);
  switch(node->kind) {
  case ND_DEREF:
    gen(node->lhs);
    break;
  case ND_VARREF:
    printf("# name: %s\n", node->name);
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->var->offset + 8);
    printf("  push rax\n");
    break;
  case ND_GVARREF:
    printf("# name: %s\n", node->name);
    printf("  push offset %s\n", node->name);
    break;
  case ND_DOT: {
    Type* type = node->lhs->type;
    if(type->kind != TY_STRUCT) {
      error_at0(__FILE__, __LINE__, node->loc, "構造体の参照ではありません");
    }
    StructMember* member = node->struct_member;
    gen_ref(node->lhs);
    printf("  pop rax\n");
    printf("  add rax, %d\n", member->offset);
    printf("  push rax\n");
    break;
  }
  default:
    error_at1(__FILE__, __LINE__, node->loc, "%sは変数の参照ではありません", node_kinds[node->kind]);
    break;
  }
}

void gen_ref_push(Node* node) {
  printf("  pop rax\n");
  if(node->type->kind == TY_INT) {
    printf("  mov eax, [rax]\n");
  } else if(node->type->kind == TY_CHAR) {
    printf("  movsx eax, BYTE PTR [rax]\n");
  } else if(node->type->kind == TY_LONG || node->type->kind == TY_PTR) {
    printf("  mov rax, [rax]\n");
  } else {
    error1(__FILE__, __LINE__, "%sの参照は未実装", type_kinds[node->type->kind]);
  }
  printf("  push rax\n");
}

void gen_assign(Node* lval, Node* rval) {
  gen_ref(lval);
  gen(rval);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  if(lval->type->kind == TY_INT) {
    printf("  mov [rax], edi\n");
  } else if(lval->type->kind == TY_CHAR) {
    printf("  mov [rax], dil\n");
  } else if(lval->type->kind == TY_LONG || lval->type->kind == TY_PTR) {
    printf("  mov [rax], rdi\n");
  } else {
    error1(__FILE__, __LINE__, "%sへの代入は未実装", type_kinds[lval->type->kind]);
  }
  printf("  push rdi\n");
}

void gen(Node* node) {
  if(node == NULL) {
    error0(__FILE__, __LINE__, "ノードがありません");
  }
  printf("# gen %s\n", node_kinds[node->kind]);
  switch (node->kind) {
  case ND_NUM: {
    printf("  push %d\n", node->int_val);
    break;
  }
  case ND_STR: {
    printf("  push offset .LC%d # \"%s\"\n", node->str_key, escape(node->str_val));
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
  case ND_DOT: {
    gen_ref(node);
    gen_ref_push(node);
    break;
  }
  case ND_VARREF:
  case ND_GVARREF: {
    gen_ref(node);
    if(node->type->kind == TY_ARRAY) {
      return;
    }
    gen_ref_push(node);
    break;
  }
  case ND_LAND: {
    int land_label = node->local_label;
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lfalse%d\n", land_label);
    gen(node->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lfalse%d\n", land_label);
    printf("  push 1\n");
    printf("  jmp .Lend%d\n", land_label);
    printf(".Lfalse%d:\n", land_label);
    printf("  push 0\n");
    printf(".Lend%d:\n", land_label);
    break;
  }
  case ND_LOR: {
    int lor_label = node->local_label;
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  jne .Ltrue%d\n", lor_label);
    gen(node->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  jne .Ltrue%d\n", lor_label);
    printf("  push 0\n");
    printf("  jmp .Lend%d\n", lor_label);
    printf(".Ltrue%d:\n", lor_label);
    printf("  push 1\n");
    printf(".Lend%d:\n", lor_label);
    break;
  }
  case ND_ASSIGN: {
    printf("# ND_ASSIGN %s %s\n", node_kinds[node->lhs->kind], node_kinds[node->rhs->kind]);
    gen_assign(node->lhs, node->rhs);
    break;
  }
  case ND_ASSIGN_ADD: {
    gen_ref(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  add [rax], rdi\n");
    switch(node->lhs->type->kind) {
    case TY_INT:
      printf("  mov edi, [rax]\n");
      break;
    case TY_CHAR:
      printf("  movzx edi, BYTE PTR [rax]\n");
      break;
    case TY_LONG:
    case TY_PTR:
      printf("  mov rdi, [rax]\n");
      break;
    default:
      error1(__FILE__, __LINE__, "%sの加算代入は未実装", type_kinds[node->lhs->type->kind]);
    }
    printf("  push rdi\n");
    break;
  }
  case ND_ASSIGN_SUB: {
    gen_ref(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  sub [rax], rdi\n");
    switch(node->lhs->type->kind) {
    case TY_INT:
      printf("  mov edi, [rax]\n");
      break;
    case TY_CHAR:
      printf("  movzx edi, BYTE PTR [rax]\n");
      break;
    case TY_LONG:
    case TY_PTR:
      printf("  mov rdi, [rax]\n");
      break;
    default:
      error1(__FILE__, __LINE__, "%sの減算代入は未実装", type_kinds[node->lhs->type->kind]);
    }
    printf("  push rdi\n");
    break;
  }
  case ND_ASSIGN_MUL: {
    gen_ref(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  imul [rax], rdi\n");
    switch(node->lhs->type->kind) {
    case TY_INT:
      printf("  mov edi, [rax]\n");
      break;
    case TY_CHAR:
      printf("  movzx edi, BYTE PTR [rax]\n");
      break;
    case TY_LONG:
      printf("  mov rdi, [rax]\n");
      break;
    default:
      error1(__FILE__, __LINE__, "%sの乗算代入は未実装", type_kinds[node->lhs->type->kind]);
    }
    printf("  push rdi\n");
    break;
  }
  case ND_ASSIGN_DIV: {
    error0(__FILE__, __LINE__, "除算代入は未実装");
  }
  case ND_ASSIGN_MOD: {
    error0(__FILE__, __LINE__, "剰余算代入は未実装");
  }
  case ND_NOT: {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    printf("  push rax\n");
    break;
  }
  case ND_RETURN: {
    gen(node->lhs);
    // r15を復元する
    printf("  mov r15, [rbp-8]\n");
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
    int if_label = node->local_label;
    if(node->els) {
      printf("  je  .Lelse%d\n", if_label);
      gen(node->then);
      printf("  pop rax\n");
      printf("  jmp .Lend%d\n", if_label);
      printf(".Lelse%d:\n", if_label);
      gen(node->els);
      printf("  pop rax\n");
    } else {
      printf("  je  .Lend%d\n", if_label);
      gen(node->then);
      printf("  pop rax\n");
    }
    printf(".Lend%d:\n", if_label);
    printf("  push 0\n"); // ifも文なので、必ずpushしておく
    break;
  }
  case ND_WHILE: {
    int while_label = node->local_label;
    printf(".Lbegin%d:\n", while_label);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", while_label);
    gen(node->body);
    printf("  pop rax\n");
    printf("  jmp .Lbegin%d\n", while_label);
    printf(".Lend%d:\n", while_label);
    printf("  push 0\n"); // whileも文なので、必ずpushしておく
    break;
  }
  case ND_FOR: {
    int for_label = node->local_label;
    if(node->init) gen(node->init);
    printf(".Lbegin%d:\n", for_label);
    if(node->cond) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", for_label);
    }
    gen(node->body);
    printf("  pop rax\n");
    if(node->inc) {
      gen(node->inc);
      printf("  pop rax\n");
    }
    printf("  jmp .Lbegin%d\n", for_label);
    printf(".Lend%d:\n", for_label);
    printf("  push 0\n"); // forも文なので、必ずpushしておく
    break;
  }
  case ND_SWITCH: {
    int switch_label = node->local_label;
    gen(node->cond);
    printf("  pop rax\n");
    char** keys = map_keys(node->case_map);
    for(int i = 0; keys[i]; i++) {
      int* val = map_get(node->case_map, keys[i]);
      printf("  cmp rax, %d\n", *val);
      printf("  je %s\n", keys[i]);
    }
    if(node->has_default) {
      printf("  jmp .Ldefault%d\n", switch_label);
    } else {
      printf("  jmp .Lend%d\n", switch_label);
    }
    gen(node->body);
    printf("  pop rax\n");
    printf(".Lend%d:\n", switch_label);
    printf("  push 0\n"); // switchも文なので、必ずpushしておく
    break;
  }
  case ND_BLOCK: {
    for(int i = 0; node->stmts[i]; i++) {
      gen(node->stmts[i]);
      printf("  pop rax\n");
    }
    printf("  push 0\n"); // BLOCKも文なので、必ずpushしておく
    break;
  }
  case ND_CALL: {
    if(node->args_call[0]) {
      gen(node->args_call[0]);
    }
    if(node->args_call[1]) {
      gen(node->args_call[1]);
    }
    if(node->args_call[2]) {
      gen(node->args_call[2]);
    }
    if(node->args_call[3]) {
      gen(node->args_call[3]);
    }
    if(node->args_call[4]) {
      gen(node->args_call[4]);
    }
    if(node->args_call[5]) {
      gen(node->args_call[5]);
    }
    if(node->args_call[5]) {
      printf("  pop r9\n");
    }
    if(node->args_call[4]) {
      printf("  pop r8\n");
    }
    if(node->args_call[3]) {
      printf("  pop rcx\n");
    }
    if(node->args_call[2]) {
      printf("  pop rdx\n");
    }
    if(node->args_call[1]) {
      printf("  pop rsi\n");
    }
    if(node->args_call[0]) {
      printf("  pop rdi\n");
    }
    printf("  mov al, 0\n"); // 浮動小数点数の数をALに入れる必要があるが、今は扱わないので0を入れておく
    // rspをr15に退避し、rspを16バイトアライメントにする
    printf("  mov [rbp-8], rsp\n");
    printf("  and rsp, -16\n");
    printf("  call %s\n", node->name);
    printf("  mov rsp, [rbp-8]\n");
    printf("  push rax\n");
    break;
  }
  case ND_FUNCDEF: {
    printf("  .text\n");
    printf("  .globl %s\n", node->name);
    printf("%s:\n", node->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", node->offset + 8);
    if(node->args_var[0]) {
      if(node->args_var[0]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], edi\n", node->args_var[0]->offset + 8);
      } else if(node->args_var[0]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], dil\n", node->args_var[0]->offset + 8);
      } else {
        printf("  mov [rbp-%d], rdi\n", node->args_var[0]->offset + 8);
      }
    }
    if(node->args_var[1]) {
      if(node->args_var[1]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], esi\n", node->args_var[1]->offset + 8);
      } else if(node->args_var[1]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], sil\n", node->args_var[1]->offset + 8);
      } else {
        printf("  mov [rbp-%d], rsi\n", node->args_var[1]->offset + 8);
      }
    }
    if(node->args_var[2]) {
      if(node->args_var[2]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], edx\n", node->args_var[2]->offset + 8);
      } else if(node->args_var[2]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], dl\n", node->args_var[2]->offset + 8);
      } else {
        printf("  mov [rbp-%d], rdx\n", node->args_var[2]->offset + 8);
      }
    }
    if(node->args_var[3]) {
      if(node->args_var[3]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], ecx\n", node->args_var[3]->offset + 8);
      } else if(node->args_var[3]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], cl\n", node->args_var[3]->offset + 8);
      } else {
        printf("  mov [rbp-%d], rcx\n", node->args_var[3]->offset + 8);
      }
    }
    if(node->args_var[4]) {
      if(node->args_var[4]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], r8d\n", node->args_var[4]->offset + 8);
      } else if(node->args_var[4]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], r8b\n", node->args_var[4]->offset + 8);
      } else {
        printf("  mov [rbp-%d], r8\n", node->args_var[4]->offset + 8);
      }
    }
    if(node->args_var[5]) {
      if(node->args_var[5]->type->kind == TY_INT) {
        printf("  mov [rbp-%d], r9d\n", node->args_var[5]->offset + 8);
      } else if(node->args_var[5]->type->kind == TY_CHAR) {
        printf("  mov [rbp-%d], r9b\n", node->args_var[5]->offset + 8);
      } else {
        printf("  mov [rbp-%d], r9\n", node->args_var[5]->offset + 8);
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
  case ND_STRDEF: {
    printf("  .data\n");
    printf(".LC%d:\n", node->str_key);
    printf("  .string \"%s\"\n", escape(node->str_val));
    break;
  }
  case ND_STRUCT: {
    break;
  }
  case ND_ENUM: {
    break;
  }
  case ND_PROGRAM: {
    for(int i = 0; node->strings[i]; i++) {
      gen(node->strings[i]);
    }
    for(int i = 0; node->funcs[i]; i++) {
      gen(node->funcs[i]);
    }
    break;
  }
  case ND_DECL: {
    if(!node->rhs) {
      printf("  push 0\n"); // 文は必ず1つpushする
      break;
    }
    gen_assign(node->lhs, node->rhs);
    break;
  }
  case ND_GDECL: {
    printf("  .data\n");
    printf("  .globl %s\n", node->name);
    printf("%s:\n", node->name);
    if(!node->rhs) {
      printf("  .zero %d\n", node->var->size);
      break;
    }
    switch(node->rhs->kind) {
    case ND_NUM:
      switch(node->var->type->kind) {
      case TY_INT:
        printf("  .long %d\n", node->rhs->int_val);
        break;
      case TY_CHAR:
        printf("  .byte %d\n", node->rhs->int_val);
        break;
      case TY_LONG:
      case TY_PTR:
        printf("  .quad %d\n", node->rhs->int_val);
        break;
      default:
        error1(__FILE__, __LINE__, "%sのグローバル変数の初期化式は未実装", type_kinds[node->rhs->type->kind]);
        break;
      }
      break;
    case ND_STR:
      printf("  .quad .LC%d\n", node->rhs->str_key);
      break;
    default:
      error1(__FILE__, __LINE__, "%sのグローバル変数の初期化式は未実装", node_kinds[node->rhs->kind]);
      break;
    }
    break;
  }
  case ND_GDECL_EXTERN: {
    break;
  }
  case ND_BREAK: {
    printf("  jmp %s\n", node->goto_label);
    break;
  }
  case ND_CONTINUE: {
    printf("  jmp %s\n", node->goto_label);
    break;
  }
  case ND_CASE: {
    printf("%s:\n", node->goto_label);
    gen(node->lhs);
    break;
  }
  case ND_DEFAULT: {
    printf("%s:\n", node->goto_label);
    gen(node->lhs);
    break;
  }
  case ND_CONVERT: {
    gen(node->lhs);
    printf("  pop rax\n");
    if(node->old_type->kind == TY_CHAR && node->new_type->kind == TY_INT) {
      printf("  movsx rax, al\n");
    } else if (node->old_type->kind == TY_INT && node->new_type->kind == TY_CHAR) {
      printf("  mov dil, al\n");
      printf("  movsx rax, dil\n");
    } else if (node->old_type->kind == TY_INT && node->new_type->kind == TY_LONG) {
      printf("# 問題の箇所\n");
      printf("  movsxd rax, eax\n");
    } else if (node->old_type->kind == TY_LONG && node->new_type->kind == TY_INT) {
      printf("  mov eax, eax\n");
    } else if(node->old_type->kind == TY_INT && node->new_type->kind == TY_VOID) {
      // return文では仮に42を入れるようにしているので、とりあえず無視する
    } else {
      error2(__FILE__, __LINE__, "%sから%sへの変換は未実装", type_kinds[node->old_type->kind], type_kinds[node->new_type->kind]);
    }
    printf("  push rax\n");
    break;
  }
  default: {
    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    char* reg1;
    char* reg2;
    switch(node->type->kind) {
    case TY_INT:
      reg1 = "eax";
      reg2 = "edi";
      break;
    case TY_CHAR:
      reg1 = "al";
      reg2 = "dil";
      break;
    case TY_LONG:
    case TY_PTR:
    case TY_ARRAY:
      reg1 = "rax";
      reg2 = "rdi";
      break;
    default:
      error1(__FILE__, __LINE__, "%sの演算は未実装", type_kinds[node->type->kind]);
      break;
    }

    switch(node->kind) {
      case ND_ADD:
        printf("  add %s, %s\n", reg1, reg2);
        break;
      case ND_SUB:
        printf("  sub %s, %s\n", reg1, reg2);
        break;
      case ND_MUL:
        printf("  imul %s, %s\n", reg1, reg2);
        break;
      case ND_DIV:
        if(node->type->kind != TY_INT) {
          error1(__FILE__, __LINE__, "%sの除算は未実装", type_kinds[node->type->kind]);
        }
        printf("  cdq\n");
        printf("  idiv edi\n");
        break;
      case ND_MOD:
        if(node->type->kind != TY_INT) {
          error1(__FILE__, __LINE__, "%sの剰余算は未実装", type_kinds[node->type->kind]);
        }
        printf("  cdq\n");
        printf("  idiv rdi\n");
        printf("  mov rax, rdx\n");
        break;
      case ND_EQ:
        printf("  cmp %s, %s\n", reg1, reg2);
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
      case ND_NE:
        printf("  cmp %s, %s\n", reg1, reg2);
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
      case ND_LT:
        printf("  cmp %s, %s\n", reg1, reg2);
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
      case ND_LE:
        printf("  cmp %s, %s\n", reg1, reg2);
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
      default:
        error_at1(__FILE__, __LINE__, node->loc, "%sは対応していないノードです", node_kinds[node->kind]);
        break;
    }
    printf("  push rax\n");
    break;
  }
  }

  printf("# endgen %s\n", node_kinds[node->kind]);
}

char* escape(char* str) {
  int len = strlen(str);
  char* buf = calloc(len * 2 + 1, sizeof(char));
  int i = 0;
  int j = 0;
  for(; i < len;) {
    if(str[i] == '\n') {
      buf[j++] = '\\';
      buf[j] = 'n';
    } else if(str[i] == '\t') {
      buf[j++] = '\\';
      buf[j] = 't';
    } else if(str[i] == '"') {
      buf[j++] = '\\';
      buf[j] = '"';
    } else if(str[i] == '\\') {
      buf[j++] = '\\';
      buf[j] = '\\';
    } else {
      buf[j] = str[i];
    }
    i++;
    j++;
  }
  return buf;
}
