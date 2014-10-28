#include <stdio.h>
#include <string.h>
#include <math.h>
#include "fpu.h"

int main (int argc, char* argv[]) {

  if (argc != 2) {
    puts("16進数でテストケースを与えると、結果を標準出力に、間違えたケースを標準エラー出力に出力します");
    puts("使用例: ./fpuTest fadd");
    return 0;
  }

  if (strcmp(argv[1], "fadd") == 0) {

    uni a, b;
    while(scanf("%x %x", &a.u, &b.u) == 2) {

      printf("%08x\n", fadd(a.u, b.u));

      if (faddCheck(a, b) == 0) {
        uni ans;
        ans.f = a.f + b.f;
        fprintf(stderr, "WrongAnswer: %08x %08x Result=%08x Answer=%08x\n",
                a.u, b.u, fadd(a.u, b.u), ans.u);

      }

    }

  } else if (strcmp(argv[1], "fmul") == 0){

    uni a, b;
    while(scanf("%x %x", &a.u, &b.u) == 2) {

      printf("%08x\n", fmul(a.u, b.u));

      if (fmulCheck(a, b) == 0) {
        uni ans;
        ans.f = a.f * b.f;
        fprintf(stderr, "WrongAnswer: %08x %08x Result=%08x Answer=%08x\n",
                a.u, b.u, fmul(a.u, b.u), ans.u);
      }

    }


  } else if (strcmp(argv[1], "finv") == 0) {

    uni a;
    while(scanf("%x", &a.u) == 1) {
      printf("%08x\n", finv(a.u));
      finvCheck(a);
    }

  } else if (strcmp(argv[1], "itof") == 0) {
    uni a;
    int n;
    while(scanf("%x", &n) == 1) { // signedを受け取る
      a.u = n;
      printf("%08x\n", itof(a.u));
      itofCheck(a);
    }
  } else if (strcmp(argv[1], "ftoi") == 0) {
    uni a;
    while(scanf("%x", &a.u) == 1) {
      printf("%08x\n", ftoi(a.u));
      ftoiCheck(a);
    }

  } else if (strcmp(argv[1], "feq") == 0) {
    uni a, b;

    while(scanf("%x %x", &a.u, &b.u) == 2) {
      printf("%08x\n", feq(a.u, b.u));
      feqCheck(a, b);
    }

  } else if (strcmp(argv[1], "flt") == 0) {
    uni a, b;

    while(scanf("%x %x", &a.u, &b.u) == 2) {
      printf("%08x\n", flt(a.u, b.u));
      fltCheck(a, b);
    }

  } else if (strcmp(argv[1], "fle") == 0) {
    uni a, b;

    while(scanf("%x %x", &a.u, &b.u) == 2) {
      printf("%08x\n", fle(a.u, b.u));
      fleCheck(a, b);
    }

  } else {

    fprintf(stderr, "target error: %s", argv[1]);

  }
  return 0;
}
