#include <stdio.h>


int main(void) {
	// R-type
	// or $rs $rt $rd
	// rd <- rs or rt
	unsigned int rs = 5;
	unsigned int rt = 1024;
	unsigned int rd=0;
	int bit=0;
	int subbit=0;
	unsigned int bitA[32], bitB[32];
	unsigned int bitQ[64] = { 0 };
	unsigned int carry = 0;
	while (bit < 32) {
		bitA[bit] = ((rs << (31-bit)) >> (31-bit)) >> bit;
		bitB[bit] = ((rt << (31-bit)) >> (31-bit)) >> bit;
		bit++;
	}
/* a[subbit] * B[bit] =   a1a2a3a4
						* b1b2b3b4
	= 										[a1*b4]	[a2*b4]	[a3*b4]	[a4*b4]
		+ 							[a1*b3]	[a2*b3]	[a3*b3]	[a4*b3]
		+ 			[a1*b2]	[a2*b2]	[a3*b2]	[a4*b2]
		+ [a1*b1]	[a2*b1]	[a3*b1]	[a4*b1]
	= Q[bit]
 */


	for(bit=0; bit < 32; bit++) {
		for(subbit=0; subbit < 32; subbit++) {
			bitQ[bit+subbit] = bitQ[bit+subbit] + (bitA[bit] & bitB[subbit]);
		}
	}
	for(bit=0;bit<32;bit++) {
		rd = rd | bitQ[bit] << bit;
	}

	printf("%u*%u=%u\n", rs, rt, rd);
	return 0;
}
