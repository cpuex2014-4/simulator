#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "fpu.h"

uint32_t ftoi (uint32_t a) {
  // 最近接丸め 中間の値なら、0から遠い方を選ぶ (C言語のround関数と同じ)
  uni t; t.u = a;
  int shift = 150 - t.Float.exp;
  uint32_t frac = (1<<23)|t.Float.frac;
  uint32_t ans;

  if (31 < abs(shift)) {
    return 0;
  } else if (shift == 0) {
    return (t.Float.sign)?-frac:frac;
  }else if (0<shift) {
    ans = frac >> (shift-1);
    ans += 1;
    ans >>= 1;
  } else {
    ans = frac << (-shift);
  }

  return (t.Float.sign) ? -ans : ans;
}

int ftoiCheck (uni a) {
  uni ans,result;
  ans.u = round(a.f);
  result.u = ftoi(a.u);

  if (ans.u != result.u ) {
    fprintf (stderr, "wrong: %08x Result: %08x Ans: %08x\n",
             a.u, result.u, ans.u);
  }

  return (ans.u == result.u) ? 1 : 0;
}
