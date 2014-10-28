#include <stdio.h>

unsigned int sll(unsigned int rs, unsigned int shamt) {
	unsigned int rd;
	unsigned int i;

	for(i=0;i<shamt;i++) {
		rs = rs << 1;
	}
	rd = rs;
	return rd;
}
unsigned int srl(unsigned int rs, unsigned int shamt) {
	unsigned int rd;
	unsigned int i;

	for(i=0;i<shamt;i++) {
		rs = rs >> 1;
	}
	rd = rs;
	return rd;
}

int main() {
	unsigned int rs=0x1000;
	unsigned int shamt=13;
	unsigned int rd=0;

	rd = sll(rs, shamt);
	printf("rd = 0x%x\n", rd);	
	
	rd = srl(rs, shamt);
	printf("rd = 0x%x\n", rd);	

	return 0;
}
