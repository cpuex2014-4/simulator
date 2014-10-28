#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "fpu.h"

// utils
int isNaN (uni a) {
  return (a.Float.exp == 255) && (a.Float.frac != 0);
}

int isDenormal(uni a) {
  return (a.Float.exp==0) ? 1 : 0;
}

int isZero(uni a) {
  return (a.Float.exp == 0) && (a.Float.frac == 0);
}

int isInf (uni a) {
  return (a.Float.exp == 255) && (a.Float.frac == 0);
}


int uni_abscomp (uni a, uni b) {
  uint32_t s = (a.u&(~(1<<31)));
  uint32_t t = (b.u&(~(1<<31)));
  return (s > t) ? 1 : ((s < t)? -1: 0);
}

uint32_t downTo(uint32_t a, uint32_t l, uint32_t r) {
  assert(l>=r);
  return ((~((~0)<<(1+l)))&a)>>r;
}

// debug print
void u_print_bits (uint32_t a) {
  int i;
  for(i=31;i>=0;i--) {
    printf("%d",((a>>i)&1?1:0));
  }
  putchar('\n');
}

void print_bits (uni a) {
  int i;
  printf("%d",(1&(a.Float.sign))?1:0);
  putchar('|');
  for(i=7;i>=0;i--) {
    printf("%d",(1&(a.Float.exp>>i))?1:0);
  }
  putchar('|');
  for(i=22;i>=0;i--) {
    printf("%d",(1&(a.Float.frac>>i))?1:0);
  }
  putchar('\n');
}

// print bits with debug message.
void u_print_bits_with (uint32_t a, char *c) {
  int i;
  for(i=31;i>=0;i--) {
    printf("%d",((a>>i)&1?1:0));
  }
  puts(c);
}
