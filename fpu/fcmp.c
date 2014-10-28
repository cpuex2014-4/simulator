#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "fpu.h"

int feq (uint32_t a1, uint32_t b1) {
  uni a, b;
  a.u = a1;
  b.u = b1;

  if (isDenormal(a) || isZero(a)) {
    a.u = 0;
  }
  if (isDenormal(b) || isZero(b)) {
    b.u = 0;
  }

  if (isNaN(a) || isNaN(b)) {
    return 0;
  }


  return (a.u == b.u) ? 1 : 0;

}

int flt (uint32_t a1, uint32_t b1) {
  uni a, b;
  a.u = a1;
  b.u = b1;

  if (isDenormal(a) || isZero(a)) {
    a.u = 0;
  }
  if (isDenormal(b) || isZero(b)) {
    b.u = 0;
  }

  if (isNaN(a) || isNaN(b)) {
    return 0;
  }
  if (a.u>>31&b.u>>31) {//両方とも負
    return ((int)a.u > (int)b.u) ? 1 : 0;
  } else {
    return ((int)a.u < (int)b.u) ? 1 : 0;
  }
}

int fle (uint32_t a, uint32_t b) {
  return feq(a, b) | flt(a, b);
}

int fltCheck (uni a, uni b) {
  if (isDenormal(a) || isZero(a)) {
    a.u = 0;
  }
  if (isDenormal(b) || isZero(b)) {
    b.u = 0;
  }
  int result = flt(a.u, b.u);
  int answer = (a.f < b.f) ? 1 : 0;

  if (result != answer) {
    fprintf (stderr, "wrong: %08x (%f) < %08x (%f) result:%d\n",
             a.u, a.f, b.u, b.f, result);
  }

  return result == answer;
}

int feqCheck (uni a, uni b) {
  if (isDenormal(a) || isZero(a)) {
    a.u = 0;
  }
  if (isDenormal(b) || isZero(b)) {
    b.u = 0;
  }
  int result = feq(a.u, b.u);
  int answer = (a.f == b.f) ? 1 : 0;

  if (result != answer) {
    fprintf (stderr, "wrong: %08x (%f) == %08x (%f) result:%d\n",
             a.u, a.f, b.u, b.f, result);
  }

  return result == answer;
}

int fleCheck (uni a, uni b) {
  if (isDenormal(a) || isZero(a)) {
    a.u = 0;
  }
  if (isDenormal(b) || isZero(b)) {
    b.u = 0;
  }
  int result = fle(a.u, b.u);
  int answer = (a.f <= b.f) ? 1 : 0;

  if (result != answer) {
    fprintf (stderr, "wrong: %08x (%f) <= %08x (%f) result:%d\n",
             a.u, a.f, b.u, b.f, result);
  }

  return result <= answer;
}
