#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "fpu.h"
#define CONST_ZERO 0
#define CONST_MINUS_ZERO 0x80000000
#define CONST_NAN ~0
#define CONST_INF   0x7f800000
#define CONST_MINUS_INF 0xff800000
double delta = 1/2048.0;

uint32_t getA (uint32_t k) {
  uni a;
  assert(k<2048);
  a.Float.sign = 0;
  a.Float.exp  = 127;
  a.Float.frac = k<<12;
  double t = a.f;

  a.f = (-1.0/t) * (1.0/(t+delta));

  return a.u;
}

uint32_t getB (uint32_t k) {
  uni a;
  assert(k<2048);
  a.Float.sign = 0;
  a.Float.exp  = 127;
  a.Float.frac = k<<12;
  double t = a.f;

  a.f = 0.5 * pow(sqrt(1.0/t) + sqrt(1.0/(t+delta)), 2);

  return a.u;
}

uint32_t finv (uint32_t x1) {
  uni x;
  x.u = x1;

  //optional
  if (isNaN(x)) {
    return CONST_NAN;
  }
  if (isInf(x)) {
    if (x.Float.sign) {
      return CONST_MINUS_ZERO;
    } else {
      return CONST_ZERO;
    }
  }
  if (isDenormal(x)|isZero(x)) {
    if (x.Float.sign) {
      return CONST_MINUS_INF;
    } else {
      return CONST_INF;
    }
  }

  //値を1.0以上2未満に正規化
  uni reg = x;
  int d = x.Float.exp - 127;
  reg.Float.exp = 127;
  reg.Float.sign = 0;

  //仮数部先頭11ビットをキーにする
  uint32_t key = downTo(x.u, 22, 12); //table size = 2048

  // aとbの値を取得
  uint32_t a = getA(key);
  uint32_t b = getB(key);

  uni ret;
  // inv(reg) == a*reg + b
  uint32_t ax = fmul(a, reg.u);
  ret.u = fadd(ax, b);

  //指数部、符号部の調整
  if (ret.Float.exp < d) {
    ret.Float.exp = 0;
  } else {
    ret.Float.exp -= d;
  }

  ret.Float.sign = x.Float.sign;

  return ret.u;
}


int finvCheck (uni a) {
  uni result, ans;
  int flg = 0;

  if (isDenormal(a)) {
    a.Float.frac = 0;
  }

  result.u = finv(a.u);
  ans.f = 1.0 / a.f;

  if (isNaN(result) && isNaN(ans)) {
    flg = 1;
  }
  if (isDenormal(result) && isDenormal(ans)) {
    flg = 1;
  }

  if (isInf(result) && isInf(ans)) {
    if (result.Float.sign == ans.Float.sign) {
      flg = 1;
    }
  }

  if (abs(result.u - ans.u) <= 3) { //仮数部の誤差の許容
    flg = 1;
  }

  if (flg == 0) {
    fprintf(stderr, "Wrong: inv(%08x|%f) = result:%08x(%.10f) / answer:%08x(%.10f)\n",
            a.u, a.f, result.u, result.f, ans.u, ans.f);
  }

  return flg;
}
