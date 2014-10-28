#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "fpu.h"

uint32_t itof (uint32_t a) { //signedの値をuint32_tとして受け取っている
  uni t, low, high;

  //unsignedからsignedに変換
  uint32_t sign = a>>31;
  t.u = (sign)?-a:a;

  low.u  = t.Float.frac; //下位23bit
  high.u = t.Float.exp;  //上位8bit

  //0x4b000000をfloatとしてみると2^23
  low.u += 0x4b000000;
  low.u = fadd(low.u, 0xcb000000);

  //0x56800000をfloatとしてみると2^46
  high.u += 0x56800000;
  high.u = fadd(high.u, 0xd6800000);

  return (sign<<31)|fadd(low.u, high.u);
}

int itofCheck (uni a) {
  uni ans,result;
  int n = a.u; //一度signedに直す必要がある
  ans.f = (float) n;
  result.u = itof(a.u);
  if (ans.u != result.u) {
    fprintf(stderr, "WrongAnswer:%d(%08x)->%d %08x/%d %08x\n",
                a.u, a.u, result.u, result.u, ans.u, ans.u);
  }

  return (ans.u == result.u) ? 1 : 0;
}
