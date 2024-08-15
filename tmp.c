
int printf();
void* memset();
void putchar();
void usleep();
void exit();
void fflush();

typedef int FloatTimes10000;

FloatTimes10000 mul(FloatTimes10000 a, FloatTimes10000 b) {
  return (a * b + 5000) / 10000;
}

int k;

void breakpoint() {}
int count = 0;

void update2(FloatTimes10000 *cos, FloatTimes10000 *sin, FloatTimes10000 cos_delta, FloatTimes10000 sin_delta) {
  count++;
  if(count == 77452) {
    printf("%d %p %p %d %d\n", count, cos, sin, cos_delta, sin_delta);
    breakpoint();
  }
  FloatTimes10000 new_cos = mul(*cos, cos_delta) - mul(*sin, sin_delta);
  FloatTimes10000 new_sin = mul(*sin, cos_delta) + mul(*cos, sin_delta);
  *cos = new_cos;
  *sin = new_sin;
}

// int main() {
//   FloatTimes10000 cos = 9662;
//   FloatTimes10000 sin = 2574;
//   update2(&cos, &sin, 9998, 200);
// }

int main() {
  FloatTimes10000 zTimes10000[1760];
  char b[1760];
  // printf("%c[2J", 32);
  FloatTimes10000 cosATimes10000 = 10000; FloatTimes10000 sinATimes10000 = 0;
  FloatTimes10000 cosBTimes10000 = 10000; FloatTimes10000 sinBTimes10000 = 0;
  for (;;) {
    memset(b, 32, 1760 * sizeof(char));
    memset(zTimes10000, 0, 1760 * sizeof(FloatTimes10000));
    FloatTimes10000 sinthetaTimes10000 = 0; FloatTimes10000 costhetaTimes10000 = 10000;
    for (int theta_times_14 = 0; theta_times_14 < 88;){
      FloatTimes10000 sinphiTimes10000 = 0; FloatTimes10000 cosphiTimes10000 = 10000;
      for (int phi_times_50 = 0; phi_times_50 < 314;) {
        FloatTimes10000 hTimes10000 = costhetaTimes10000 + 20000;
        FloatTimes10000 DTimes10000 = 100000000 / (
          mul(mul(sinphiTimes10000, hTimes10000), sinATimes10000)
          + mul(sinthetaTimes10000, cosATimes10000)
          + 50000);
        FloatTimes10000 tTimes10000 = mul(mul(sinphiTimes10000, cosATimes10000), hTimes10000) - mul(sinthetaTimes10000, sinATimes10000);
        int x = 40 + 30 * mul(DTimes10000, mul(mul(cosphiTimes10000, cosBTimes10000), hTimes10000) - mul(tTimes10000 , sinBTimes10000)) / 10000;
        int y = 12 + 15 * mul(DTimes10000, mul(mul(cosphiTimes10000, sinBTimes10000), hTimes10000) + mul(tTimes10000 , cosBTimes10000)) / 10000;
        int o = x + 80 * y;
        int _a = mul(mul(sinthetaTimes10000, sinATimes10000) - mul(mul(sinphiTimes10000, cosATimes10000), costhetaTimes10000), cosBTimes10000);
        int _b = mul(mul(sinphiTimes10000, sinATimes10000), costhetaTimes10000);
        int _c = mul(sinthetaTimes10000, cosATimes10000);
        int _d = mul(mul(cosphiTimes10000, sinBTimes10000), costhetaTimes10000);
        int N = 8 * (_a - _b - _c - _d) / 10000;
        if (22 > y && y > 0 && x > 0 && 80 > x && DTimes10000 > zTimes10000[o]) {
          zTimes10000[o] = DTimes10000;
          char bo;
          if (N >= 1) {
            bo = ".,-~:;=!*#$@"[N];
          } else {
            bo = '.';
          }
          b[o] = bo;
        } else {
        }
        phi_times_50++;
        update2(&cosphiTimes10000, &sinphiTimes10000, 9998, 200);
      }
      theta_times_14++;
      update2(&costhetaTimes10000, &sinthetaTimes10000, 9974 + theta_times_14 % 2, 714);
    }
    // printf("%c[H", 32);
    for (k = 0; 1761 > k; k++) {
      if (k % 80) {
        // putchar(b[k]);
      } else {
        // putchar(10);
      }
    }
    update2(&cosATimes10000, &sinATimes10000, 9992, 400);
    update2(&cosBTimes10000, &sinBTimes10000, 9998, 200);
    usleep(50000);
  }
}
