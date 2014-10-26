#include <stdio.h>
#include <stdlib.h>

unsigned int addiu(unsigned int adduA, unsigned int Imm) {
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ;

	/* 符号格調 */
	if( (Imm & 0x8000) == 0x8000 ) {
//		printf("符号拡張:%x\n", Imm);
		Imm = Imm | 0xFFFF0000;
//		printf("-> %x\n", Imm);
	}
	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		bitA = ((adduA << (31-bit)) >> (31-bit)) >> bit;
		bitB = ((Imm << (31-bit)) >> (31-bit)) >> bit;

		bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
		c = (bitA&bitB) | (bitB&c) | (bitA&c);
//		printf("\tbitA:%x, bitB:%x, bitQ:%x, c:%x\n",bitA,bitB,bitQ,c);

		rd = rd | (bitQ << bit);
		bit++;
	}
//	rCarry = c;
	return rd;
}

int main () {
	unsigned int x=1,y=1;
	unsigned int xtemp,ytemp;
	unsigned int ans;
//	ADDIU :	[$29: 0x804821] + [im: 0xfff4] => [$29: 0x4815]

	xtemp = 0x8048FF;
	ytemp= 0x8000;		//16bit負数

	printf("0x%x-0x%x = 0x%x\n", xtemp, ytemp, xtemp-ytemp);
	ans = addiu(xtemp, ytemp);
	printf("ans = 0x%x\n", ans);

	xtemp = 0x8048FF;
	ytemp= 0xFFF4;		//16bit負数

	printf("0x%x-0x%x = 0x%x\n", xtemp, ytemp, xtemp-ytemp);
	ans = addiu(xtemp, ytemp);
	printf("ans = 0x%x\n", ans);



/*
	for(x=0x10000;x<0x20000; x = x + 139) {
		for(y=0x7FF0;y<0x8100; y++) {
			xtemp = x;
			ytemp = y & 0xFFFF;
			ans = addiu(xtemp, ytemp);
			if(ans != xtemp-(ytemp&0xFFFFFFF)) 
				printf("xtemp:%x, ytemp:%x, ans=%x\n", xtemp, ytemp, ans);
		}
		if(x%1390 == 0) printf("count:%8x\n",x);
	}
*/
	return 0;
}
