#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


int main(void){
	unsigned int ret=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ;

	unsigned int adduA = 0x9000000F;
	unsigned int adduB = 0xF000000F;

	ret = adduA+adduB;
	printf("adduA+adduB = %x\n", ret);

	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		bitA = ((adduA << (31-bit)) >> (31-bit)) >> bit;
		printf("bit:%u\tbitA = %x, ", bit, bitA);
		bitB = ((adduB << (31-bit)) >> (31-bit)) >> bit;
		printf("bitB = %x, ", bitB);

		bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
		c = (bitA&bitB) | (bitB&c) | (bitA&c);

		printf("bitQ = %x, c=%u\n", bitQ, c);
		ret = ret | (bitQ << bit);
		bit++;
	}

	printf("adduA | adduB = %x, c=%u\n", ret, c);

	return ret;
}
