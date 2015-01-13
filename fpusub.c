#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "const.h"
#include "alu.h"
#include "print.h"
#include "fpu/C/fpu.h"
#include "fpusub.h"
#define SHOWFLGCHK	(flag[HIDEIND] == 1)

uint32_t fneg (uint32_t a) {
  uint32_t nega = (1<<31)^a;
  return nega;
}

uint32_t fabs (uint32_t a) {
	if(a>>31) {
		a = (1<<31)^a;
	}
	return a;
}

unsigned int fmadds(unsigned int fr, unsigned int fs, unsigned int ft) {
	unsigned int fd;
	/* 積の丸め？ */
	fd = fadd(fmul(fr, fs), ft);

	return fd;
}
unsigned int fmsubs(unsigned int fr, unsigned int fs, unsigned int ft) {
	unsigned int fd;
	/* 積の丸め？ */
	fd = fsub(fmul(fr, fs), ft);

	return fd;
}

unsigned int nfmadds(unsigned int fr, unsigned int fs, unsigned int ft) {
	unsigned int fd;
	/* 積の丸め？ */
	fd = fneg(fadd(fmul(fr, fs), ft));

	return fd;
}
unsigned int nfmsubs(unsigned int fr, unsigned int fs, unsigned int ft) {
	unsigned int fd;
	/* 積の丸め？ */
	fd = fneg(fsub(fmul(fr, fs), ft));

	return fd;
}

