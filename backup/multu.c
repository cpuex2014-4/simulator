#include <stdio.h>
#include <stdlib.h>


int multu(unsigned int rs, unsigned int rt) {
	// R-type
	// or $rs $rt $rd
	// rd <- rs or rt
//	unsigned int rs = 0x10F0;
//	unsigned int rt = 0x100F;
	unsigned int rd=0;
	int bit=31;
	int subbit=31;
//	int i=0;
	unsigned int bitA[32], bitB[32];
	unsigned int bitQ[64] = { 0 };
	unsigned int carry = 0;
	while (bit >= 0) {
		bitA[bit] = ((rs << (31-bit)) >> (31-bit)) >> bit;
		bitB[bit] = ((rt << (31-bit)) >> (31-bit)) >> bit;
		bit--;
	}

/* a[subbit] * B[bit] =   a1a2a3a4
						* b1b2b3b4
	= 										[a1*b4]	[a2*b4]	[a3*b4]	[a4*b4]
		+ 							[a1*b3]	[a2*b3]	[a3*b3]	[a4*b3]
		+ 			[a1*b2]	[a2*b2]	[a3*b2]	[a4*b2]
		+ [a1*b1]	[a2*b1]	[a3*b1]	[a4*b1]
	= Q[bit]
 */

//		bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
//		c = (bitA&bitB) | (bitB&c) | (bitA&c);
	for(bit=0; bit<32; bit++) {
		if(bitB[bit] == 0) continue;
		else {	// bitA[subbit] == 1
			for(subbit=0; subbit<32; subbit++) {
				if(bitA[subbit]==1 && bitQ[bit+subbit]==1 && carry == 0) {
					carry=1;
					bitQ[bit+subbit] = 0;
				} else if (bitA[subbit]==1 && bitQ[bit+subbit]==1 && carry == 1) {
					carry=1;
					bitQ[bit+subbit] = 1;
				} else if ( (bitA[subbit] | bitQ[bit+subbit]) == 1 && carry == 1) {
					carry=1;
					bitQ[bit+subbit] = 0;
				} else if (bitA[subbit]==0 && bitQ[bit+subbit]==0 && carry ==1) {
					carry=0;
					bitQ[bit+subbit] = 1;
				} else if (bitA[subbit]==0 && bitQ[bit+subbit]==0 && carry ==0) {
					carry=0;
					bitQ[bit+subbit] = 0;
				} else {
					carry=0;
					bitQ[bit+subbit] = 1;
				}
			}
		}
	}
	printf("\n");
	for(bit=0;bit<32;bit++) {
		rd = rd | bitQ[bit] << bit;
//		if(bitQ[bit] != 0) printf("0x%x, ",rd);
	}

	for(bit=32;bit<64;bit++) {
		if(bitQ[bit] != 0)
			carry = 1;
	}
//	printf("\n");
		for(bit=31;bit>=0;bit--) {
//			printf("%u", bitQ[bit]);
		}
		printf("\n");
	if(rd != rs*rt || carry == 1) {
/*		printf("                                ");
		for(bit=31;bit>=0;bit--) {
			printf("%u", bitA[bit]);
		}
		printf("\n                                ");
		for(bit=31;bit>=0;bit--) {
			printf("%u", bitB[bit]);
		}
		printf("\n");
		for(bit=63;bit>=0;bit--) {
			printf("%u", bitQ[bit]);
		}
		printf("\n");
*/
		if(carry != 0) printf("Overflow\n");
		printf("%u*%u = %u (ans = %u)\n", rs, rt, rd, rs*rt);
		printf("\n======================================================\n");
	}
	return 0;
}

int main(void) {
	int i, j, m, n;

	multu(0xFFFFFFF,0xFFFFFFF);

	for(i=1; i<10; i++) {
		for(j=1; j<10; j++) {
			m=rand()%2000000;
			n=rand()%300000;
			m = multu(m, n);
		}
	}
	if(m==0) ;
	return 0;
}
