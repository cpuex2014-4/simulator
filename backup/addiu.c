#include <stdio.h>

unsigned int addiu(unsigned int adduA, unsigned int Imm) {
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ;


	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		bitA = ((adduA << (31-bit)) >> (31-bit)) >> bit;
		bitB = ((Imm << (31-bit)) >> (31-bit)) >> bit;

		bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
		c = (bitA&bitB) | (bitB&c) | (bitA&c);

		rd = rd | (bitQ << bit);
		bit++;
	}
//	rCarry = c;

	return rd;
}

int main () {
	unsigned int x=1,y=1;
	unsigned int ans;

	for(x=0;x<0x100000;x++) {
		for(y=0;y<0x100000;y++) {
		ans = addiu(x,y);
		if(ans != x+y) printf("%d\n", ans);
		}
	}
	return 0;
}
