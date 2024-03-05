#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "10cc.h"

int indent = 0;
_Bool is_head_of_line = true;

// インデント付きprintf
// fmtに改行文字が含まれている場合は、その後にインデントを付ける
// ただし、fmtの最後に改行文字が来た場合、次に呼ばれたときにインデントを付ける
static void printf_i(char* fmt, ...) {
  char* new_fmt = malloc(strlen(fmt) + indent + 100); // インデントの追加分として、適当に100バイト追加で確保する
  int fmt_len = strlen(fmt);
  int j = 0;
  for(int i = 0; i < fmt_len; i++) {
    if(is_head_of_line) {
      for(int k = 0; k < indent; k++) {
        new_fmt[j++] = ' ';
      }
      is_head_of_line = false;
    }
    new_fmt[j++] = fmt[i];
    if(fmt[i] == '\n') {
      is_head_of_line = true;
    }
  }
  new_fmt[j] = '\0';
  va_list ap;
  va_start(ap, fmt);
  vprintf(new_fmt, ap);
  va_end(ap);
  free(new_fmt);
}

static void print_binary_op(Node* node, char* op) {
  printf_i("(");
  print_node(node->lhs);
  printf_i(" %s ", op);
  print_node(node->rhs);
  printf_i(")");
}

static void print_type(Type* type) {
  printf_i("%s", type_kinds[type->kind] + 3);
  if(type->kind == TY_PTR) {
    printf_i("<");
    print_type(type->ptr_to);
    printf_i(">");
  }
  if(type->kind == TY_ARRAY) {
    printf_i("<");
    print_type(type->ptr_to);
    printf_i(", %d>", type->array_size);
  }
}

void print_node(Node* node) {
  if(node == NULL) {
    printf_i("NULL\n");
    return;
  }
  NodeKind kind = node->kind;
  switch(kind) {
  case ND_ADD:
    print_binary_op(node, "+");
    break;
  case ND_SUB:
    print_binary_op(node, "-");
    break;
  case ND_MUL:
    print_binary_op(node, "*");
    break;
  case ND_DIV:
    print_binary_op(node, "/");
    break;
  case ND_MOD:
    print_binary_op(node, "%");
    break;
  case ND_EQ:
    print_binary_op(node, "==");
    break;
  case ND_NE:
    print_binary_op(node, "!=");
    break;
  case ND_LT:
    print_binary_op(node, "<");
    break;
  case ND_LE:
    print_binary_op(node, "<=");
    break;
  case ND_SIZEOF:
    printf_i("sizeof(");
    print_node(node->lhs);
    printf_i(")");
    break;
  case ND_ADDR:
    printf_i("&");
    print_node(node->lhs);
    break;
  case ND_DEREF:
    printf_i("*");
    print_node(node->lhs);
    break;
  case ND_NUM:
    printf_i("%d", node->int_val);
    break;
  case ND_STR:
    if(node->str_key == -1) {
      printf_i("\"%s\"", node->str_val);
    } else {
      printf_i("(LC%d:\"%s\")", node->str_key, node->str_val);
    }
    break;
  case ND_VARREF:
    printf_i("L_%s", node->name);
    break;
  case ND_GVARREF:
    printf_i("G_%s", node->name);
    break;
  case ND_ARRAYREF:
    printf_i("%s[", node->lhs->name);
    print_node(node->rhs);
    printf_i("]");
    break;
  case ND_CALL:
    printf_i("%s(", node->name);
    for(int i = 0; node->args_call[i]; i++) {
      print_node(node->args_call[i]);
      if(node->args_call[i + 1]) {
        printf_i(", ");
      }
    }
    printf_i(")");
    break;
  case ND_ASSIGN:
    print_binary_op(node, ":=");
    break;
  case ND_RETURN:
    printf_i("return ");
    print_node(node->lhs);
    break;
  case ND_IF:
    printf_i("if(");
    print_node(node->cond);
    printf_i(") ");
    print_node(node->then);
    if(node->els) {
      printf_i(" else ");
      print_node(node->els);
    }
    break;
  case ND_WHILE:
    printf_i("while(");
    print_node(node->cond);
    printf_i(") ");
    print_node(node->body);
    break;
  case ND_FOR:
    printf_i("for(");
    print_node(node->init);
    printf_i("; ");
    print_node(node->cond);
    printf_i("; ");
    print_node(node->inc);
    printf_i(") ");
    print_node(node->body);
    break;
  case ND_BLOCK:
    printf_i("{\n");
    indent += 2;
    for(int i = 0; node->stmts[i]; i++) {
      print_node(node->stmts[i]);
      printf_i(";\n");
    }
    indent -= 2;
    printf_i("}\n");
    break;
  case ND_FUNCDEF:
    print_type(node->type);
    printf_i(" %s(", node->name);
    for(int i = 0; node->args_node[i]; i++) {
      print_type(node->args_node[i]->type);
      printf_i(" L_%s", node->args_node[i]->name);
      if(node->args_node[i + 1]) {
        printf_i(", ");
      }
    }
    printf_i(") ");
    print_node(node->body);
    break;
  case ND_FUNCPROT:
    print_type(node->type);
    printf_i(" %s(", node->name);
    for(int i = 0; node->args_node[i]; i++) {
      print_type(node->args_node[i]->type);
      printf_i(" %s", node->args_node[i]->name);
      if(node->args_node[i + 1]) {
        printf_i(", ");
      }
    }
    printf_i(");\n");
    break;
  case ND_STRDEF:
    printf_i("LC%d = \"%s\"", node->str_key, node->str_val);
    break;
  case ND_GDECL:
    print_type(node->type);
    printf_i(" G_%s", node->name);
    break;
  case ND_DECL:
    print_type(node->type);
    printf_i(" L_%s", node->name);
    break;
  case ND_TYPE:
    print_type(node->type);
    break;
  case ND_IDENT:
    printf_i("%s", node->name);
    break;
  case ND_PROGRAM:
    if(node->strings) {
      for(int i = 0; node->strings[i]; i++) {
        print_node(node->strings[i]);
        printf_i("\n");
      }
    }
    for(int i = 0; node->funcs[i]; i++) {
      print_node(node->funcs[i]);
      printf_i("\n");
    }
    break;
  default:
    printf_i("** unknown node kind %s **", node_kinds[kind]);
  }
  return;
}