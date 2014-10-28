#include <stdio.h>
#include <stdlib.h>
#include "fpu.h"

uni randUni(){

  uni ret;
  int i;
  ret.f=0.0;
  for(i=0;i<32;i++){
    if(rand()%2) {
      ret.u|=(1<<i);
    }
  }
  return ret;

}

char s[100];

int main () {

  long long cnt=0;
  unsigned i;
  srand(0);
  for(i=0;i<200000;i++) {
    uni a,b;
    a = randUni();
    b = randUni();
    /*
      if(isNaN(a)||isNaN(b)) {
      cnt++;continue;
    }
    */
    sprintf(s,"%08x %08x",a.u,b.u);
    printf("%s\n", s);
  }

  return 0;

}
