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

