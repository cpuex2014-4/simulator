#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/* subuA - subuB */
int main(void){
	unsigned int ret=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ;

	unsigned int subuA = 0x0;
	unsigned int subuB = 0x30;
	unsigned int nsubuB = 0;

	ret = subuA-subuB;
	printf("subuA-subuB = 0x%x(%u)\n", ret, ret);
	subuB = ~subuB;
	nsubuB = ~subuB;
	printf("nsubuB = 0x%x(%d)\n", nsubuB, (int)nsubuB);

	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		if(bit == 0) {
			c=1;
			bitA = ((subuA << (31-bit)) >> (31-bit)) >> bit;
			printf("bit:%u\tbitA = %x, ", bit, bitA);
			bitB = ((subuB << (31-bit)) >> (31-bit)) >> bit;
			printf("bitB = %x, ", bitB);

			bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
			c = (bitA&bitB) | (bitB&c) | (bitA&c);
	
			printf("bitQ = %x, c=%u\n", bitQ, c);
			ret = ret | (bitQ << bit);
		} else {
			bitA = ((subuA << (31-bit)) >> (31-bit)) >> bit;
			printf("bit:%u\tbitA = %x, ", bit, bitA);
			bitB = ((subuB << (31-bit)) >> (31-bit)) >> bit;
			printf("bitB = %x, ", bitB);
	
			bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
			c = (bitA&bitB) | (bitB&c) | (bitA&c);
	
			printf("bitQ = %x, c=%u\n", bitQ, c);
			ret = ret | (bitQ << bit);

		}
		bit++;
	}

	printf("subuA-subuB = 0x%x(%u), c=%u\n", ret, ret, c);

	return ret;
}
