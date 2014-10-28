#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "fpu.h"
#define CONST_ZERO 0
#define CONST_MINUS_ZERO 0x80000000
#define CONST_NAN ~0
#define CONST_INF   0x7f800000
#define CONST_MINUS_INF 0xff800000


uint32_t ops (uni a, uni b) {

  if (isNaN (a)) {
    return CONST_NAN;
  } else if (isInf(a) && isInf(b)
             && (a.Float.sign ^ b.Float.sign)) {
    return CONST_NAN;
  } else if (isInf(a)) {

    return (a.Float.sign) ? CONST_MINUS_INF : CONST_INF;

  } else if (isInf(b)) {

    return (b.Float.sign) ? CONST_MINUS_INF : CONST_INF;

  } else if (isZero(a) && isZero(b)) {

    if (a.Float.sign & b.Float.sign) {
      return CONST_MINUS_ZERO;
    } else {
      return CONST_ZERO;
    }
  } else {
    if(isZero(a)) {
      return b.u;
    } else if (isZero(b)) {
      return a.u;
    }
  }
  fprintf(stderr, "error: %08x %08x\n", a.u , b.u);
  assert(0);
}

uint32_t fadd (uint32_t a1, uint32_t b1) {
  uni a, b;
  uni ans;
  a.u = a1; b.u = b1;

  //compare abs
  if (uni_abscomp (a, b) < 0) { // swap
    uni t;
    t = a; a = b; b = t;
  }

  if (isDenormal(a)) {
    assert(isDenormal(b)); //bの方が絶対値が小さいため

    //非正規化数同士の和は0 or -0を返す仕様
    return (a.Float.sign) ? CONST_MINUS_ZERO : CONST_ZERO;
  }

  if (isDenormal(b)) {
    b.u = (b.Float.sign) ? CONST_MINUS_ZERO : CONST_ZERO;
  }



  // ops
  if (a.Float.exp == 0 || a.Float.exp == 255 ||
      b.Float.exp == 0 || b.Float.exp == 255) {
    return ops(a, b);
  }


  // bit shift
  uint32_t d = a.Float.exp - b.Float.exp;
  uint32_t na = (1 << (23 + 3))|(a.Float.frac << 3);
  uint32_t nb = (1 << (23 + 3))|(b.Float.frac << 3);

  int flg = 0;
  uint32_t i;

  for (i = 0; i < d; i++) {
    flg |= nb&1;
    nb = (nb >> 1);
  }
  nb |= flg;
  nb *= (a.Float.sign == b.Float.sign)? 1:-1;

  // sum
  uint32_t frac = na + nb;

  int t = (int) log2(frac);

  // frac
  if (frac == 0) {
    return 0;
  } else if (t < 26) {
    frac = frac << (26 - t);
    int exp = a.Float.exp - (26 - t);
    if (exp <= 0) {
      if (a.Float.sign) {
        return CONST_MINUS_ZERO;
      } else {
        return CONST_ZERO;
      }
    } else {
      ans.Float.exp = exp;
    }
  } else if (t > 26) {
    assert(t==27);
    frac = (frac>>1)|(frac&1);
    ans.Float.exp = a.Float.exp + 1;
  } else {
    // t == 26 繰り上がりなし
    ans.Float.exp = a.Float.exp;
  }


  uint32_t ulp   = 1 & (frac>>3);
  uint32_t guard = 1 & (frac>>2);
  uint32_t round = 1 & (frac>>1);
  uint32_t stick = 1 & frac;


  if (guard & (ulp|round|stick)) {
    frac += (1<<3);
    if (1&(frac>>27)) {
      ans.Float.exp += 1;
    }
  }

  if (ans.Float.exp >= 255) {
    frac = 0;
  }

  ans.Float.frac = (frac >> 3);
  ans.Float.sign = a.Float.sign;

  return ans.u;
}

int faddCheck (uni a, uni b) {
  uni ans, result;
  ans.f = a.f + b.f;
  result.u = fadd(a.u, b.u);

  if (isNaN(ans) && isNaN(result)) {
    return 1;
  }

  if (isInf(ans) && isInf(result)) {
    if (ans.Float.sign != result.Float.sign) {
      return 0;
    }
    return 1;
  }

  if (isDenormal(ans) && isZero(result)) {
    if (ans.Float.sign != result.Float.sign) {
      return 0;
    }
    return 1;
  }

  if (isDenormal(a) || isDenormal(b)) {

    if (isDenormal(a) && isDenormal(b) && isZero(result)) {
      //非正規化数同士の和は0 or -0を返す仕様
      return (result.Float.sign == ans.Float.sign) ? 1 : 0;
    }

    if ((isDenormal(a) && result.u == b.u)||
        (isDenormal(b) && result.u == a.u)) {
          return 1;
    }
    return 0;
  }

  int flg = 1;

  if (ans.Float.sign != result.Float.sign) {
    flg = 0;
  }

  if (ans.Float.exp != result.Float.exp) {
    flg = 0;
  }

  if (abs(ans.Float.frac - result.Float.frac) > 1) {
    //仮数部の1bitの誤差のみ許容
    flg = 0;
  }

  return flg;

}
