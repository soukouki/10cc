
int printf();

char* section_name;
int section_count;
int is_fall;

int section(char* name) {
  section_name = name;
  section_count = 0;
}

int assert(int expected, int actual, char* msg) {
  section_count = section_count + 1;
  if(expected != actual) {
    printf("*** Assertion failed: %s\n", msg);
    printf("    Expected: %d\n", expected);
    printf("    Actual: %d\n", actual);
    char* ordinal;
    if(section_count % 10 == 1)
      ordinal = "st";
    else if(section_count % 10 == 2)
      ordinal = "nd";
    else if(section_count % 10 == 3)
      ordinal = "rd";
    else
      ordinal = "th";
    if(section_count == 11) ordinal = "th";
    if(section_count == 12) ordinal = "th";
    if(section_count == 13) ordinal = "th";
    printf("    Test for the %d%s %s\n\n", section_count, ordinal, section_name);
    is_fall = 1;
    return 1;
  }
}

int return_value_1() {
  return 0;
}
int return_value_2() {
  return 42;
}
int return_value() {
  section("値を返す");
  assert(0, return_value_1(), "return_value_1");
  assert(42, return_value_2(), "return_value_2");
}

int four_arithmetic() {
  section("四則演算");
  assert(3,  1 + 2, "1 + 2");
  assert(0,  1 + 2 - 3, "1 + 2 - 3");
  assert(1,  1 - 2 + 3 + 4 - 5, "1 - 2 + 3 + 4 - 5");
  assert(41, 1 + 20 * 2, "1 + 20 * 2");
  assert(9,  (20 - 1) / 2, "(20 - 1) / 2");
}

int monominal() {
  section("単項+, 単項-");
  assert(10, -10 + 20, "-10 + 20");
  assert(10, +10, "+10");
  assert(10, - -10, "- -10");
}

int compare() {
  section("比較演算子");
  assert(0, 2 == 3, "2 == 3");
  assert(1, 2 != 3, "2 != 3");
  assert(1, 2 <= 2, "2 <= 2");
  assert(0, 2 < 2,  "2 < 2");
  assert(1, 2 >= 2, "2 >= 2");
  assert(0, 2 > 2,  "2 > 2");
}

int one_char_variable_1() {
  int a;
  a = 1;
  return 1;
}
int one_char_variable_2() {
  int a;
  a = 1;
  return a;
}
int one_char_variable_3() {
  int a;
  a = 1;
  int b;
  b = 2;
  return a + b;
}
int one_char_variable() {
  section("一文字変数");
  assert(1, one_char_variable_1(), "one_char_variable_1");
  assert(1, one_char_variable_2(), "one_char_variable_2");
  assert(3, one_char_variable_3(), "one_char_variable_3");
}

int multi_char_variable_1() {
  int abc;
  abc = 12;
  return abc;
}
int multi_char_variable_2() {
  int _;
  _ = 3;
  _ = _ * 10;
  return _ + 4;
}
int multi_char_variable() {
  section("複数文字, 特殊文字変数");
  assert(12, multi_char_variable_1(), "multi_char_variable_1");
  assert(34, multi_char_variable_2(), "multi_char_variable_2");
}

int return_statement_1() {
  return 10;
}
int return_statement_2() {
  return 10;
  20;
}
int return_statement_3() {
  return 10;
  return 20;
}
int return_statement() {
  section("return文");
  assert(10, return_statement_1(), "return_statement_1");
  assert(10, return_statement_2(), "return_statement_2");
  assert(10, return_statement_3(), "return_statement_3");
}

int if_statement_1() {
  if(1)
    return 10;
}
int if_statement_2() {
  if(0)
    return 10;
  return 20;
}
int if_statement_3() {
  if(0)
    return 10;
  return 20;
}
int if_statement_4() {
  if(0)
    return 10;
  else
    return 20;
}
int if_statement_5() {
  if(0)
    return 10;
  else if(1)
    return 20;
  else
    return 30;
  return 40;
}
int if_statement_6() {
  if(0)
    return 10;
  else if(0)
    return 20;
  else
    return 30;
  return 40;
}
int if_statement() {
  section("if文");
  assert(10, if_statement_1(), "if_statement_1");
  assert(20, if_statement_2(), "if_statement_2");
  assert(20, if_statement_3(), "if_statement_3");
  assert(20, if_statement_4(), "if_statement_4");
  assert(20, if_statement_5(), "if_statement_5");
  assert(30, if_statement_6(), "if_statement_6");
}

int while_statement_1() {
  while(0) return 12;
  return 34;
}
int while_statement_2() {
  while(1) return 56;
  return 78;
}
int while_statement_3() {
  int a;
  a = 0;
  while(a < 5) a = a + 1;
  return a;
}
int while_statement_4() {
  int a;
  a = 0;
  while(1) if (a == 5) return a; else a = a + 1;
}
int while_statement() {
  section("while文");
  assert(34, while_statement_1(), "while_statement_1");
  assert(56, while_statement_2(), "while_statement_2");
  assert(5,  while_statement_3(), "while_statement_3");
  assert(5,  while_statement_4(), "while_statement_4");
}

int for_statement_1() {
  for(;;) return 3;
}
int for_statement_2() {
  int a;
  a = 0;
  for(;;) if(a == 5) return a; else a = a + 1;
}
int for_statement_3() {
  int a;
  for(a = 0; a < 9; a = a + 1) return 4;
}
int for_statement_4() {
  int a;
  for(a = 0; a < 9; a = a + 1) 1;
  return a;
}
int for_statement_5() {
  int a;
  for(a = 9;;) if(a == 9) return a;
}
int for_statement_6() {
  int a;
  for(a = 0; a < 9; a = a + 1) if(a == 5) return a;
}
int for_statement_7() {
  int sum;
  sum = 0;
  int i;
  for(i = 0; i <= 10; i = i + 1) sum = sum + i;
  return sum;
}
int for_statement() {
  section("for文");
  assert(3,  for_statement_1(), "for_statement_1");
  assert(5,  for_statement_2(), "for_statement_2");
  assert(4,  for_statement_3(), "for_statement_3");
  assert(9,  for_statement_4(), "for_statement_4");
  assert(9,  for_statement_5(), "for_statement_5");
  assert(5,  for_statement_6(), "for_statement_6");
  assert(55, for_statement_7(), "for_statement_7");
}

int block_statement_1() {
  {
    return 10;
  }
}
int block_statement_2() {
  {
    return 10;
    20;
  }
}
int block_statement_3() {
  {
    10;
    return 20;
  }
}
int block_statement_4() {
  {
    int a;
    a = 10;
    return a;
  }
}
int block_statement_5() {
  if(1) {
    return 20;
  }
}
int block_statement_6() {
  if(1) {
    int a;
    a = 10;
    return a;
  }
}
int block_statement_7() {
  if(0) {
    int a;
    a = 10;
    return a;
  } else {
    int b;
    b = 20;
    return b;
  }
}
int block_statement_8() {
  int a;
  a = 0;
  while(1) {
    a = a + 10;
    if(a == 30) {
      return a;
    }
  }
}
int block_statement_9() {
  int a;
  for(a = 0; a < 100; a = a + 10) {
    if(a == 40) {
      return a;
    }
  }
}
int block_statement() {
  section("ブロック文");
  assert(10, block_statement_1(), "block_statement_1");
  assert(10, block_statement_2(), "block_statement_2");
  assert(20, block_statement_3(), "block_statement_3");
  assert(10, block_statement_4(), "block_statement_4");
  assert(20, block_statement_5(), "block_statement_5");
  assert(10, block_statement_6(), "block_statement_6");
  assert(20, block_statement_7(), "block_statement_7");
  assert(30, block_statement_8(), "block_statement_8");
  assert(40, block_statement_9(), "block_statement_9");
}

int  function_prototype_func();
int  function_prototype_1arg1(int);
int  function_prototype_1arg2(int b);
int  function_prototype_2arg1(int a, int b);
int  function_prototype_2arg2(int x, int); // xは実際の定義とは異なる名前
int *function_prototype_pointer(int *a);

int function_prototype_1() {
  return function_prototype_func();
}
int function_prototype_2() {
  return function_prototype_1arg1(10);
}
int function_prototype_3() {
  return function_prototype_1arg2(10);
}
int function_prototype_4() {
  return function_prototype_2arg1(2, 8);
}
int function_prototype_5() {
  return function_prototype_2arg2(2, 8);
}
int function_prototype_6() {
  int a;
  a = 10;
  return *function_prototype_pointer(&a);
}

int function_prototype_func() {
  return 10;
}
int function_prototype_1arg1(int a) {
  return a;
}
int function_prototype_1arg2(int b) {
  return b;
}
int function_prototype_2arg1(int a, int b) {
  return a + b;
}
int function_prototype_2arg2(int a, int b) {
  return a + b;
}
int *function_prototype_pointer(int *a) {
  return a;
}

int function_prototype() {
  section("関数プロトタイプ");
  assert(10, function_prototype_1(), "function_prototype_1");
  assert(10, function_prototype_2(), "function_prototype_2");
  assert(10, function_prototype_3(), "function_prototype_3");
  assert(10, function_prototype_4(), "function_prototype_4");
  assert(10, function_prototype_5(), "function_prototype_5");
  assert(10, function_prototype_6(), "function_prototype_6");
}

int call0();
int call1(int);
int call2(int, int);
int call3(int, int, int);
int call4(int, int, int, int);
int call5(int, int, int, int, int);
int call6(int, int, int, int, int, int);

int function_call_with_0arg_1() {
  return call0();
}
int function_call_with_0arg_2() {
  return call0() + 10;
}
int function_call_with_0arg_3() {
  call0();
  return call0() + call0();
}
int function_call_with_0arg_4() {
  int a;
  a = call0();
  return a;
}
int function_call_with_0arg_5() {
  {
    call0();
    return call0();
  }
}
int function_call_with_0arg() {
  section("0引数関数呼び出し");
  assert(100, function_call_with_0arg_1(), "function_call_with_0arg_1");
  assert(110, function_call_with_0arg_2(), "function_call_with_0arg_2");
  assert(200, function_call_with_0arg_3(), "function_call_with_0arg_3");
  assert(100, function_call_with_0arg_4(), "function_call_with_0arg_4");
  assert(100, function_call_with_0arg_5(), "function_call_with_0arg_5");
}

int function_call_with_1arg_1() {
  return call1(10);
}
int function_call_with_1arg_2() {
  call1(10);
  return call1(10) * 2;
}
int function_call_with_1arg_3() {
  int a;
  a = call1(10);
  return a;
}
int function_call_with_1arg_4() {
  if(1)
    return call1(10);
  else
    return call1(20);
}
int function_call_with_1arg() {
  section("1引数関数呼び出し");
  assert(110, function_call_with_1arg_1(), "function_call_with_1arg_1");
  assert(220, function_call_with_1arg_2(), "function_call_with_1arg_2");
  assert(110, function_call_with_1arg_3(), "function_call_with_1arg_3");
  assert(110, function_call_with_1arg_4(), "function_call_with_1arg_4");
}

int function_call_with_multiarg_1() {
  return call2(10, 20);
}
int function_call_with_multiarg_2() {
  return call3(10, 20, 30);
}
int function_call_with_multiarg_3() {
  return call4(10, 20, 30, 39);
}
int function_call_with_multiarg_4() {
  return call5(1, 2, 3, 4, 5);
}
int function_call_with_multiarg_5() {
  return call6(1, 2, 3, 4, 5, 6);
}
int function_call_with_multiarg_6() {
  return call2(3, 5 + 5);
}
int function_call_with_multiarg_7() {
  return call2(5 * 2, 3);
}
int function_call_with_multiarg() {
  section("複数引数関数呼び出し");
  assert(30, function_call_with_multiarg_1(), "function_call_with_multiarg_1");
  assert(60, function_call_with_multiarg_2(), "function_call_with_multiarg_2");
  assert(99, function_call_with_multiarg_3(), "function_call_with_multiarg_3");
  assert(15, function_call_with_multiarg_4(), "function_call_with_multiarg_4");
  assert(21, function_call_with_multiarg_5(), "function_call_with_multiarg_5");
  assert(13, function_call_with_multiarg_6(), "function_call_with_multiarg_6");
  assert(13, function_call_with_multiarg_7(), "function_call_with_multiarg_7");
}

int function_definition_with_0arg_nobody() {
  return 0;
}
int function_definition_with_0arg_return() {
  return 42;
}
int function_definition_with_0arg_call_other_A() {
  return 42;
}
int function_definition_with_0arg_call_other_B() {
  return function_definition_with_0arg_call_other_A();
}
int function_definition_with_0arg_variable() {
  int a;
  a = 42;
  return a;
}
int function_definition_with_0arg() {
  section("0引数関数定義");
  assert(0,  function_definition_with_0arg_nobody(), "function_definition_with_0arg_nobody");
  assert(42, function_definition_with_0arg_return(), "function_definition_with_0arg_return");
  assert(42, function_definition_with_0arg_call_other_B(), "function_definition_with_0arg_call_other_B");
  assert(42, function_definition_with_0arg_variable(), "function_definition_with_0arg_variable");
}

int function_definition_with_multiarg_id(int a) {
  return a;
}
int function_definition_with_multiarg_add(int a, int b) {
  return a + b;
}
int function_definition_with_multiarg_fib(int n) {
  if(n == 0) return 0;
  if(n == 1) return 1;
  return function_definition_with_multiarg_fib(n - 1) + function_definition_with_multiarg_fib(n - 2);
}
int function_definition_with_multiarg() {
  section("複数引数関数定義");
  assert(13, function_definition_with_multiarg_id(13), "function_definition_with_multiarg_id");
  assert(13, function_definition_with_multiarg_add(3, 10), "function_definition_with_multiarg_add");
  assert(13, function_definition_with_multiarg_fib(7), "function_definition_with_multiarg_fib");
}

int addr_and_deref_1() {
  int a;
  a = 42;
  int *p;
  p = &a;
  return *p;
}
int addr_and_deref() {
  section("単項&, 単項*");
  assert(42, addr_and_deref_1(), "addr_and_deref_1");
}

int one_level_pointer_1() {
  int a;
  a = 42;
  int *p;
  p = &a;
  return *p;
}
int one_level_pointer_2() {
  int a;
  int *b;
  b = &a;
  *b = 42;
  return a;
}
int one_level_pointer_deref(int *p) {
  return *p;
}
int one_level_pointer_3() {
  int a;
  a = 42;
  return one_level_pointer_deref(&a);
}
int one_level_pointer_setref(int *p) {
  *p = 42;
  return 0;
}
int one_level_pointer_4() {
  int a;
  one_level_pointer_setref(&a);
  return a;
}
int one_level_pointer_add(int *a, int *b) {
  return *a + *b;
}
int one_level_pointer_5() {
  int a;
  a = 20;
  int b;
  b = 22;
  return one_level_pointer_add(&a, &b); 
}
int one_level_pointer() {
  section("一重ポインタ");
  assert(42, one_level_pointer_1(), "one_level_pointer_1");
  assert(42, one_level_pointer_2(), "one_level_pointer_2");
  assert(42, one_level_pointer_3(), "one_level_pointer_3");
  assert(42, one_level_pointer_4(), "one_level_pointer_4");
  assert(42, one_level_pointer_5(), "one_level_pointer_5");
}

int multi_level_pointer_1() {
  int a;
  a = 42;
  int *p;
  p = &a;
  int **q;
  q = &p;
  return **q;
}
int multi_level_pointer_2() {
  int a;
  int *b;
  int **c;
  b = &a;
  c = &b;
  **c = 42;
  return a;
}
int multi_level_pointer() {
  section("多重ポインタ");
  assert(42, multi_level_pointer_1(), "multi_level_pointer_1");
  assert(42, multi_level_pointer_2(), "multi_level_pointer_2");
}

int *alloc_int(int, int, int, int);
int pointer_add() {
  section("ポインタ加算");
  int *a;
  a = alloc_int(1, 2, 3, 4);
  assert(2, *(a + 1), "*(a + 1)");
  assert(3, *(a + 2), "*(a + 2)");
  assert(2, *(a + 3 - 2), "*(a + 3 - 2)");
}

int sizeof_one_level_pointer() {
  section("一重ポインタのsizeof演算子");
  assert(4, sizeof(1), "sizeof(1)");
  assert(4, sizeof 1 , "sizeof 1");
  int a;
  assert(4, sizeof(a), "sizeof(a)");
  int *p;
  assert(8, sizeof(p), "sizeof(p)");
  assert(4, sizeof(sizeof(1)), "sizeof(sizeof(1))");
}

int sizeof_multi_level_pointer() {
  section("多重ポインタのsizeof演算子");
  int **a;
  assert(8, sizeof(a), "sizeof(a)");
  assert(8, sizeof(a + 4), "sizeof(a + 4)");
  assert(8, sizeof(*a), "sizeof(*a)");
}

int array_declaration() {
  section("配列宣言");
  int a[1];
  *a = 2;
  assert(2, *a, "*a");
  int b[2];
  *b = 3;
  assert(3, *b, "*b");
}

int sizeof_array() {
  section("配列のsizeof演算子");
  int a[1];
  assert(4, sizeof(a), "sizeof(a)");
  int b[2];
  assert(8, sizeof(b), "sizeof(b)");
  int *c[3];
  assert(24, sizeof(c), "sizeof(c)");
  int **d[4];
  assert(32, sizeof(d), "sizeof(d)");
}

int array_index_access() {
  section("配列の添字アクセス");
  int a[3];
  *a = 1;
  assert(1, a[0], "a[0]");
  a[0] = 2;
  assert(2, *a, "*a");
  a[1] = 3;
  assert(3, a[1], "a[1]");
  a[1] = 4;
  assert(4, *(a + 1), "*(a + 1)");
  a[1] = 5;
  int* p;
  p = a;
  assert(5, *(p + 1), "*(p + 1)");
  (a)[1] = 6;
  assert(6, (a)[1], "(a)[1]");
  1[a] = 7;
  assert(7, 1[a], "1[a]");
  int *b[3];
  int n;
  n = 8;
  b[1] = &n;
  assert(8, *b[1], "*b[1]");
}

int  global_variable_a;
int* global_variable_pointer;
int  global_variable_array[3];

int global_variable_set() {
  global_variable_a = 1;
}
int global_variable_get() {
  return global_variable_a;
}
int global_variable_conflict() {
  int global_variable_a;
  global_variable_a = 2;
}
int global_variable() {
  section("グローバル変数");
  assert(0, global_variable_a, "global_variable_a");
  assert(0, global_variable_pointer, "global_variable_pointer");
  assert(0, global_variable_array[0], "global_variable_array[0]");
  assert(0, global_variable_array[1], "global_variable_array[1]");
  assert(0, global_variable_array[2], "global_variable_array[2]");
  global_variable_set();
  assert(1, global_variable_a, "global_variable_a");
  assert(1, global_variable_get(), "global_variable_get");
  int b;
  global_variable_pointer = &b;
  b = 2;
  assert(2, *global_variable_pointer, "*global_variable_pointer");
  global_variable_array[1] = 3;
  assert(3, global_variable_array[1], "global_variable_array[1]");
  global_variable_conflict();
  assert(1, global_variable_a, "global_variable_a");
  int* p;
  p = &global_variable_a;
  *p = 2;
  assert(2, global_variable_a, "global_variable_a");
}

int string_type() {
  section("文字列型");
  char a;
  a = 1;
  assert(1, a, "a");
  char* b;
  b = &a;
  *b = 2;
  assert(2, a, "a");
  char c[3];
  c[0] = 3;
  c[1] = 4;
  c[2] = 5;
  assert(4, c[1], "c[1]");
  char d;
  d = 6;
  assert(8,  a + d, "a + d");
  assert(4,  d - a, "d - a");
  assert(12, a * d, "a * d");
  assert(3,  d / a, "d / a");
  assert(0,  a == d, "a == d");
  assert(1,  a != d, "a != d");
  assert(1,  a < d, "a < d");
  assert(0,  a > d, "a > d");
  assert(1,  a <= d, "a <= d");
  assert(0,  a >= d, "a >= d");
}

int string_literal() {
  section("文字列リテラル");
  char* a;
  a = "abc";
  assert(97, a[0], "a[0]");
  assert(98, a[1], "a[1]");
  assert(99, a[2], "a[2]");
  assert(0,  a[3], "a[3]");
  a = "";
  assert(0,  a[0], "a[0]");
  a = "abc";
  assert(97, a[0], "a[0]");
  assert(98, a[1], "a[1]");
  assert(99, a[2], "a[2]");
  assert(0,  a[3], "a[3]");
}

int comment() {
  section("コメント");
  // コメント
  /*
  assert(1, 0, "コメント");
  */
}

int sizeof_char() {
  section("文字型のsizeof演算子");
  char a;
  assert(1, sizeof(a), "sizeof(a)");
  char* b;
  assert(8, sizeof(b), "sizeof(b)");
  char c[3];
  assert(3, sizeof(c), "sizeof(c)");
}

int modulo_operator() {
  section("剰余演算子");
  assert(1, 10 % 3, "10 % 3");
  assert(2, 10 % 4, "10 % 4");
  assert(0, 10 % 5, "10 % 5");
}

int main() {
  return_value();
  four_arithmetic();
  monominal();
  compare();
  one_char_variable();
  multi_char_variable();
  return_statement();
  if_statement();
  while_statement();
  for_statement();
  block_statement();
  function_prototype();
  function_call_with_0arg();
  function_call_with_1arg();
  function_call_with_multiarg();
  function_definition_with_0arg();
  function_definition_with_multiarg();
  addr_and_deref();
  one_level_pointer();
  multi_level_pointer();
  pointer_add();
  sizeof_one_level_pointer();
  sizeof_multi_level_pointer();
  array_declaration();
  sizeof_array();
  array_index_access();
  global_variable();
  string_type();
  string_literal();
  comment();
  sizeof_char();

  if(is_fall) {
    printf("FAILED\n");
  } else {
    printf("OK\n");
  }
  return 0;
}

/*
TODO
- forのinitに宣言を入れられるようにする
- 多次元配列
- 型名のsizeof
- 配列のポインタ
- 暗黙の型変換(char -> int)
*/
