
int printf();
int main() {
  // long long l1;
  // long long l2;
  // long long l0_;
  // long long l1_;
  long l1 = 2147483647 * 2 + 2;
  long l2 = 0;
  long l0_ = 2147483647;
  long l1_ = l0_ * 2 + 2;
  printf("%d\n", l1 != l2);
  printf("%d\n", l1_ != l2);
}
