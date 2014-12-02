#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "const.h"
#include "alu.h"

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
unsigned int slt(unsigned int rs, unsigned int rt) {
// slt rs, rt, rd
// R-type
// rs < rt ならばレジスタ rd に 1 を代入、そうでなければ 0 を代入。 
// $rsと$rtの値を符号付き整数として比較し、$rs が小さければ $rd に1を、そうでなければ $rd 0を格納
	unsigned int rd;

	unsigned int urs, urt;
	unsigned int trs, trt;

	if(rs||rt) { ; }

	urs = rs & 0x80000000;
	urt = rt & 0x80000000;
	trs = rs & 0x7FFFFFFF;
	trt = rt & 0x7FFFFFFF;

//	printf("(rs=%X,rt=%X,urs=%X,urt=%X,trs=%X,trt=%X)",rs,rt,urs,urt,trs,trt);

	if(urs == 0 && urt == 0) {
		if(trs < trt) {
			rd = 1;
		} else {
			rd = 0;
		}
	} else if (urs == 0 && urt != 0) {
		rd = 0;
	} else if (urs != 0 && urt == 0) {
		rd = 1;
	} else {
		if(trs < trt) {
			rd = 0;
		} else {
			rd = 1;
		}
	}

	return rd;
}

unsigned int mult(unsigned int rs, unsigned int rt) {
/*	int bit=0;
	int subbit=0;
	unsigned int bitA[32], bitB[32];
	unsigned int bitQ[64] = { 0 };
	unsigned int carry = 0;
*/
	unsigned int rd=0;
	if(rs||rt) { ; }



	return rd;
}


unsigned int or(unsigned int rs, unsigned int rt) {
	// R-type
	// or $rs $rt $rd
	// rd <- rs or rt
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int bitA, bitB;
	unsigned int bitQ = 0;

	/* 各ビットごとにOR演算 */
	while (bit < 32) {
		bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
		bitB = ((rt << (31-bit)) >> (31-bit)) >> bit;
		bitQ = bitA | bitB;
		rd = rd | (bitQ << bit);
		bit++;
	}
	return rd;
}

unsigned int and(unsigned int rs, unsigned int rt) {
	// R-type
	// and $rs $rt $rd
	// rd <- rs & rt
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int bitA, bitB;
	unsigned int bitQ = 0;

	/* 各ビットごとにAND演算 */
	while (bit < 32) {
		bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
		bitB = ((rt << (31-bit)) >> (31-bit)) >> bit;
		bitQ = bitA&bitB;
		rd = rd | (bitQ << bit);
		bit++;
	}
	return rd;
}

unsigned int subu (unsigned int rs, unsigned int rt) {
	// R-type
	// subu $rs $rt $rd
	// rd <- rs - rt
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA=0, bitB=0;
	unsigned int bitQ=0;

//	printf("(rt=%ld -> ", (long)rt);
	rt = ~rt;
//	printf("-rt=%ld)", (long)rt);
	
	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		if(bit == 0) {
			c=1;
			bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
			bitB = ((rt << (31-bit)) >> (31-bit)) >> bit;
			bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
			c = (bitA&bitB) | (bitB&c) | (bitA&c);
			rd = rd | (bitQ << bit);
		} else {
			bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
			bitB = ((rt << (31-bit)) >> (31-bit)) >> bit;
			bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
			c = (bitA&bitB) | (bitB&c) | (bitA&c);
			rd = rd | (bitQ << bit);
		}
		bit++;
	}
	return rd;
}

unsigned int addiu(unsigned int rs, unsigned int Imm, unsigned int stackPointer) {
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ;

	if (stackPointer == 29) {
		while (bit < 32) {
			bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
			bitB = ((Imm << (31-bit)) >> (31-bit)) >> bit;
	
			bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
			c = (bitA&bitB) | (bitB&c) | (bitA&c);
	
			rd = rd | (bitQ << bit);
			bit++;
		}
		rd = rd % MEMORYSIZE;
	} else {
		/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
		while (bit < 32) {
			bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
			bitB = ((Imm << (31-bit)) >> (31-bit)) >> bit;
	
			bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
			c = (bitA&bitB) | (bitB&c) | (bitA&c);
	
			rd = rd | (bitQ << bit);
			bit++;
		}
//		rCarry = c;
	}

	return rd;
}

unsigned int addu(unsigned int adduA, unsigned int adduB){
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ=0;

	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		bitA = ((adduA << (31-bit)) >> (31-bit)) >> bit;
		bitB = ((adduB << (31-bit)) >> (31-bit)) >> bit;

		bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
		c = (bitA&bitB) | (bitB&c) | (bitA&c);

		rd = rd | (bitQ << bit);
		bit++;
	}
//	rCarry = c;
	return rd;
}

unsigned int ori(unsigned int rs, unsigned int im) {
	unsigned int rt;

	rt = rs | im;


	return rt;
}



