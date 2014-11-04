#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include "fpu/C/fpu.h"

/* general */
#define PCINIT 0x4000
#define REGSIZE 32
#define FPREGSIZE 32
#define BUFF 65536
#define MEMORYSIZE (1024u*1024u*2u)

/* opcode */
#define JUMP  0x2
#define JAL   0x3
#define BEQ   0x4
#define BNE   0x5
#define ADDIU 0x9
#define FPU   0x11
#define LW    0x23
#define SW    0x2B
#define INOUT 0x3F
#define SRCV  0x1C
#define SSND  0x1D

/* function */
#define SLL   0x0
#define SRL   0x2
#define JR    0x8
#define ADDU  0x21
#define SUBU  0x22
#define AND   0x24
#define OR    0x25
#define SLT   0x2A
#define NOP   0x7F

/* fpfunction */
#define MFC1M 0x0	// fMt
#define MFC1F 0x0	// Function
#define MTC1M 0x4
#define MTC1F 0x0
#define MOVSM 0x16
#define MOVSF 0x6
#define ADDSM 0x16	// fMt
#define ADDSF 0x0	// Function

/* argument */
#define BREAKPOINT "--break"
#define PRINTREG "--reg"
#define SERIALIN "--serial"
#define HELP "--help"
#define HIDE "--hide"
#define ARGMAX 20

/* register */
#define REGAT 1
#define REGGP 28
#define REGSP 29
#define REGFP 30
#define REGRA 31



unsigned int reg[REGSIZE];	// 32 register
unsigned int regold[REGSIZE];	// 32 register
unsigned int fpreg[REGSIZE];	// 32 fpregister
//unsigned int rZ, rV, rN, rCarry;	// condition register
unsigned int operation;	// 実行中命令
//unsigned int pc;		// program counter: jump -> memory[pc]
unsigned int HI=0, LO=0;
unsigned int opNum[128];	//各命令の実行回数 opNum[OPCODE]++ の形で使用
unsigned int fopNum[128];	//各浮動小数点命令の実行回数 fopNum[OPCODE]++ の形で使用
unsigned int funcNum[128];	//各命令の実行回数 funcNum[OPCODE]++ の形で使用
int jumpFlg=0;	// pcを変更するプログラムが実行されたら1。ジャンプ後0に戻る
int printreg=-1;

/*
RRB (RS-232C Receive Byte)
31 - 26 = 011100
25 - 21 = 00000
20 - 16 : rt
15 -  0 : 0000000000000000RS-232Cを通じて1バイト受信し、0拡張してレジスタrt内に保存する。
FIFOが空の場合、ブロッキングする。
RSB (RS-232C Send Byte)
31 - 26 = 011101
25 - 21 = 00000
20 - 16 : rt
15 -  0 : 0000000000000000レジスタrt内の下位8ビットをRS-232Cを通じて送信する。
FIFOが一杯の場合、ブロッキングする。 
*/
/* Receive byte from serial port */

void printhelp(void) {
	printf("使用法: sim [アセンブリファイル] [オプション]...\n");
	printf("\t--break <num>\t\tブレイクポイントを指定\n");
	printf("\t--serialin <ファイル名>\t\tシリアルポートからの入力を指定\n");
	printf("\t--reg <num>\t\tレジスタ表示を無効化\n");
	printf("\t-- <num>\t\t逐次実行\n");
	printf("\t--hide <num>\t\t各命令実行時の詳細データを表示しない\n");

}

/* FPレジスタ内容表示器 */
void printFPRegister(void) {
// unsigned int rZ, rV, rN, rCarry;	// condition register
	int i;

//	printf("R[Condition] ZVNC = %X/%X/%X/%X \n", rZ, rV, rN, rCarry);
	for(i=0; i<FPREGSIZE; i++) {
		if(i%16 == 0) 
			printf("FP[%2d->%2d] : ", i, i+16);
		else if( (i+1)%16 == 0 ) {
			printf("%4X\n",fpreg[i]);
		} else {
			printf("%4X ",fpreg[i]);
		}
	}

/*
	for(i=0; i<FPREGSIZE; i++) {
		if(i == 0) {
			printf("$zr= 0x%-6X ",reg[i]);
		} else if( i == 1 ) {
			printf("$at= 0x%-6X ",reg[i]);
		} else if( i == 2 || i == 3 ) {
			printf("$v%d= 0x%-6X ", i-2,reg[i]);
		} else if( i >= 4 && i < 8 ) {
			printf("$a%d= 0x%-6X ", i-4,reg[i]);
		} else if( i >= 8 && i < 16 ) {
			printf("$t%d= 0x%-6X ", i-8, reg[i]);
			if(i==10) printf("\n");
		} else if( i >= 16 && i < 24 ) {
			printf("$s%d= 0x%-6X ", i-16, reg[i]);
			if(i==21) printf("\n");
		} else if( i == 24 || i == 25 ) {
			printf("$t%d= 0x%-6X ", i-16, reg[i]);
		} else if( i == 26 || i == 27 ) {
			printf("$k%d= 0x%-6X ",i-26,reg[i]);
		} else if( i == 28 ) {
			printf("$gp= 0x%-6X ",reg[i]);
		} else if( i == 29 ) {
			printf("$sp= 0x%-6X ",reg[i]);
		} else if( i == 30 ) {
			printf("$fp= 0x%-6X ",reg[i]);
		} else if( i == 31 ) {
			printf("$ra= 0x%-6X",reg[i]);
		} else {
			printf("N/A");
		}
	}
*/
	printf("\n");
}

unsigned int fpu(unsigned int pc, unsigned int instruction) {
	unsigned int fpfunction = 0;
	unsigned int fmt=0;
	unsigned int ft=0;
	unsigned int fs=0;
	unsigned int fd=0;
	unsigned int im=0;
	if ((im == fd) || (fs == ft) || (fmt == 0) || pc == 0) {
		;
	}
	/* [op] [rs] [rt] [rd] [shamt] [funct] */
	/* 先頭&末尾6bitは0であることが保証済み */
	fmt = (instruction >> 21) & 0x1F;
	ft  = (instruction >> 16) & 0x1F;
	fs  = (instruction >> 11 ) & 0x1F;
	fd  = (instruction >> 6 ) & 0x1F;
	printFPRegister();
/*
○ 			mfc1 				010001 00000 rt 	fs 00000 000000 	rt <- fs 	FPUレジスタ → 汎用レジスタ
○ 			mtc1 				010001 00100 rt 	fs 00000 000000 	fs <- rt 	汎用レジスタ → FPUレジスタ
○ 			mov.s 				010001 10000 00000 	fs fd 000110 	fd <- fs 	FPUレジスタ移動
○ 			add.s fd, fs, ft 	010001 10000 ft 	fs fd 000000 
*/
	fpfunction = instruction & 0x3F;
	printf("\t[fpfunction:%2X]\n", fpfunction);

	switch (fpfunction) {
		case (0) :
			if(fmt == MFC1M) {
				printf("\tMFC1 :");
				;
			} else if (MTC1M) {
				printf("\tMTC1 :");
				;
			} else if (ADDSM) {
				printf("\tADDS :");
				

				fpreg[fd]=fadd(fpreg[fs], fpreg[ft]);
			} else {
				printf("Unknown fmt.\n");
			}
			break;
		case (MOVSF) :	//
			if(fmt == MOVSM) { 
				printf("\tMOVS :");
			}
			break;
		case (OR) :
			printf("\tOR ");
			break;

		default :
			printf("Default FPswitch has selected.\n");
	}
	return 0;
}

unsigned int rrb(unsigned char* srBuff) {
	static unsigned int numin = 0;
	/* [op] [0-0] [rt] [^0] */
	/* 先頭&末尾6bitは0であることが保証済み */
	unsigned int rt;
	
	rt = (unsigned int) srBuff[numin];

	return rt;
}

/* Send byte to serial port */
unsigned int rsb(unsigned int rt, unsigned char* serial) {
	static unsigned int numout = 0;

	serial[numout] = (unsigned char) (rt & 0xFF);

	return 0;
}

/* store word */
// sw rs,rt,Imm => M[ R(rs)+Imm ] <- R(rt)	rtの内容をメモリのR(rs)+Imm(符号拡張)番地に書き込む
/* リトルエンディアン */
unsigned int sw(unsigned int rt, unsigned int address, unsigned int* memory, unsigned int stackPointer) {
	unsigned int addr0;
	unsigned int addr1;
	unsigned int addr2;
	unsigned int addr3;

	addr3 = (rt & 0xFF000000) >> 24;
	addr2 = (rt & 0x00FF0000) >> 16;
	addr1 = (rt & 0x0000FF00) >>  8;
	addr0 = rt & 0x000000FF;

	if (stackPointer == 31) {
		memory[address+3] = addr3;
		memory[address+2] = addr2;
		memory[address+1] = addr1;
		memory[address] = addr0;

	} else {
		memory[address+3] = addr3;
		memory[address+2] = addr2;
		memory[address+1] = addr1;
		memory[address] = addr0;
	}

	if(rt != ( memory[address] | memory[address+1] << 8 | memory[address+2] << 16 | memory[address+3] << 24 )) {
		printf("\nFailed to SW\n");
	}

	return 0;
}
/* load word */
// lw rs,rt,Imm => R(rt) <- M[ R(rs)+Imm ]	メモリのR(rs)+Imm番地の内容をrtに書き込む
unsigned int lw(unsigned int address, unsigned int* memory) {
	unsigned rt = 0;

	rt = memory[address] | memory[address+1] << 8 | memory[address+2] << 16 | memory[address+3] << 24;

	return rt;
}

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
// $rsと$rtの値を符号付き整数として比較し、$s が小さければ $d に1を、そうでなければ0を格納
	unsigned int rd;

	unsigned int urs, urt;
	unsigned int trs, trt;

	if(rs||rt) { ; }

	urs = rs & 0x80000000;
	urt = rt & 0x80000000;
	trs = rs & 0x7FFFFFFF;
	trt = rt & 0x7FFFFFFF;

	if(urs == 0 && urt == 0) {
		if(trs < trt) {
			rd = 1;
		} else {
			rd = 0;
		}
	} else if (urs == 0 && urt != 0) {
		return 0;
	} else if (urs != 0 && urt == 0) {
		return 1;
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



/* レジスタ内容表示器 */
void printRegister(void) {
// unsigned int rZ, rV, rN, rCarry;	// condition register
	int i;

//	printf("R[Condition] ZVNC = %X/%X/%X/%X \n", rZ, rV, rN, rCarry);
/*	for(i=0; i<REGSIZE; i++) {
		if(i%16 == 0) 
			printf("R[%2d->%2d] : ", i, i+16);
		else if( (i+1)%16 == 0 ) {
			printf("%4X\n",reg[i]);
		} else {
			printf("%4X ",reg[i]);
		}
	}
*/

	for(i=0; i<REGSIZE; i++) {
		if(reg[i] == 0) continue;
		if(i == 0) {
			printf("$zr=0x%-6X ",reg[i]);
		} else if( i == 1 ) {
			printf("$at=0x%-6X ",reg[i]);
		} else if( i == 2 || i == 3 ) {
			printf("$v%d=0x%-6X ", i-2,reg[i]);
		} else if( i >= 4 && i < 8 ) {
			printf("$a%d=0x%-6X ", i-4,reg[i]);
		} else if( i >= 8 && i < 16 ) {
			printf("$t%d=0x%-6X ", i-8, reg[i]);
			if(i==10) printf("\n");
		} else if( i >= 16 && i < 24 ) {
			printf("$s%d=0x%-6X ", i-16, reg[i]);
			if(i==21) printf("\n");
		} else if( i == 24 || i == 25 ) {
			printf("$t%d=0x%-6X ", i-16, reg[i]);
		} else if( i == 26 || i == 27 ) {
			printf("$k%d=0x%-6X ",i-26,reg[i]);
		} else if( i == 28 ) {
			printf("$gp=0x%-6X ",reg[i]);
		} else if( i == 29 ) {
			printf("$sp=0x%-6X ",reg[i]);
		} else if( i == 30 ) {
			printf("$fp=0x%-6X ",reg[i]);
		} else if( i == 31 ) {
			printf("$ra=0x%-6X",reg[i]);
		} else {
			printf("N/A");
		}
	}
	printf("\n");
}

/* opcodeが0の時の操作を、末尾6ビットによって決める */
unsigned int funct (unsigned int pc, unsigned int instruction, int* flag) {
	unsigned int function = 0;
	unsigned int rs=0;
	unsigned int rt=0;
	unsigned int rd=0;
	unsigned int shamt=0;
	unsigned int im=0;
	if (im == 0) { ; }

	/* [op] [rs] [rt] [rd] [shamt] [funct] */
	/* 先頭&末尾6bitは0であることが保証済み */
	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	rd = (instruction >> 11 ) & 0x1F;
	shamt = (instruction >> 6 ) & 0x1F;
	
	function = instruction & 0x3F;
//	if(instruction != 0) printf("\t[function:%2X]\n", function);
	switch (function) {
		case (JR) :
			pc = reg[rs];
			if(flag[1] != 1) printf("\tJR : $ra(%u) = 0x%X / pc -> 0x%X\n", rs, reg[rs], pc);
			jumpFlg = 1;
			funcNum[JR]++;
			break;
		case (ADDU) :	// rd=rs+rt
			reg[rd] = addu(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\tADDU :\t[$%2u: 0x%2X] + [$%2u: 0x%2X] => [$%2u: 0x%2X]\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			funcNum[ADDU]++;
			break;
		case (SUBU) :	// rd=rs-rt
			reg[rd] = subu(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\tSUBU :\t[$%2u 0x%2X], [$%2u 0x%2X] => [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			funcNum[SUBU]++;
			break;
		case (SLT) :
			if(flag[1] != 1) printf("\tSLT :\t?([$%2u: 0x%04x] < [$%2u: 0x%04x]) ", rs, reg[rs], rt, reg[rt]);
			reg[rd] = slt(reg[rs], reg[rt]);
			if(reg[rd]) {
				if(flag[1] != 1) printf("\t=> [$%2u: 0x%4X] <TRUE>\n", rd, reg[rd]);
			} else {
				if(flag[1] != 1) printf("\t=> [$%2u: 0x%4X] <FALSE>\n", rd, reg[rd]);
			}
			funcNum[SLT]++;
			break;
		case (SLL) :
			if(rs == 0 && shamt == 0) {
				if(flag[1] != 1) printf("\tNOP\n");
				funcNum[NOP]++;
				break;
			}
			reg[rd] = sll(reg[rs], shamt);
			if(flag[1] != 1) printf("\tSLL :\t%u << %u -> %u\n", reg[rs], shamt, reg[rd]);
			funcNum[SLL]++;
			break;
		case (SRL) :
			if(rs == 0 && shamt == 0)
				printf("\tNOP\n");
				funcNum[NOP]++;
				break;
			reg[rd] = srl(reg[rs], shamt);
			if(flag[1] != 1) printf("\tSRL :\t%u << %u -> %u\n", reg[rs], shamt, reg[rd]);
			funcNum[SRL]++;
			break;
		case (AND) :
			reg[rd] = and(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\t[$%2u:0x%4X] AND? [$%2u:0x%4X] \t=> [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			funcNum[AND]++;
			break;
		case (OR) :
			reg[rd] = or(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\t[$%2u:0x%4X] OR? [$%2u:0x%4X] \t=> [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			funcNum[OR]++;
			break;
		default :
			printf("Default switch has selected.\n");
	}
	return pc;
}
/* デコーダ */
unsigned int decoder (unsigned int pc, unsigned int instruction, unsigned int* memory,unsigned char* srBuff, unsigned char* serial, unsigned int breakCount, int* flag) {
	unsigned int opcode=0;
	unsigned int rt=0;
	unsigned int rs=0;
	unsigned int im=0;
	unsigned int jump=0;
	unsigned int line=0;
	unsigned int address=0;
	if(instruction != 0 && flag[1] != 1) {
		printf("[ops: %04u,pc: 0x%x]\n", breakCount, pc);
	}
//	printf("\t[instruction: 0x%2X]\n", instruction);
	opcode = instruction >> 26;	// opcode: 6bitの整数
//	printf("\t[opcode:%2X]\n", opcode);
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  1E: 00000111110000000000000000
		im = instruction & 0x0000FFFF;		// 0xFFFF: 00000000001111111111111111

	/* 適当な時にswitch文に切り替え */
	if(opcode == 0) {
		pc = funct(pc, instruction, flag);
	} else if (opcode == FPU) {
		if(flag[1] != 1) printf("\tFPU \n");
		fpu(pc, instruction);
	} else if (opcode == SRCV) {
		if(flag[1] != 1) printf("\tSRCV \n");
		rt = rrb(srBuff);
	} else if (opcode == SSND) {
		if(flag[1] != 1) printf("\tSSND \n");
		rt = (instruction >> 16) & 0x1F;		// 0x  1E: 00000111110000000000000000
		rsb(reg[rt], serial);
	} else if (opcode == ADDIU) {
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  1E: 00000111110000000000000000
		im = instruction & 0x0000FFFF;		// 0xFFFF: 00000000001111111111111111
		if( im >= 0x8000 ) {
			im = im | 0xFFFF0000;
//			printf("(%X)",Imm);
		}
		reg[rt] = (addiu(reg[rs], im, rt) & 0xFFFFFFFF);	// 処理部
		if(flag[1] != 1) printf("\tADDIU :\t[$%2u: 0x%4X] + [im: 0x%8X] => [$%2u: 0x%4X]\n", rs, reg[rs], im, rt, reg[rt]);
		opNum[ADDIU]++;
	} else if (opcode == JUMP) {
		jumpFlg = 1;
		jump = instruction & 0x3FFFFFF;
		if(flag[1] != 1) printf("\tJ :\t(jump_to) 0x%04x\n", jump*4);
		pc = jump*4 + PCINIT;	// jumpはpAddr形式
		opNum[JUMP]++;
	} else if (opcode == BEQ) { // beq I-Type: 000100 rs rt BranchAddr 	等しいなら分岐 
		rs = (instruction >> 21) & 0x1F;	// 0x   F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x   F : 00000111110000000000000000
		line = instruction & 0xFFFF;		// 0xFFFF : 00000000001111111111111111
		if(flag[1] != 1) printf("\tBEQ :\t?([$%2u 0x%2X]=[$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + line*4);
		if(reg[rs] == reg[rt]) {
			jumpFlg = 1;
			// 0x48 = 0x20 + 4 + line*4 +0x0	<-> line*4 = 0x48-0x24 <-> line = 9
			pc = pc + 4 + line*4;		// pAddr形式
			if(flag[1] != 1) printf("\t\t<TRUE & JUMP> -> (branch_to) 0x%04x\n", pc);
		} else {
			if(flag[1] != 1) printf("\t<FALSE & NOP>\n");
		}
		opNum[BEQ]++;
	} else if (opcode == BNE) { // beq I-Type: 000100 rs rt BranchAddr 	等しいなら分岐 
		rs = (instruction >> 21) & 0x1F;	// 0x   F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x   F : 00000111110000000000000000
		line = instruction & 0xFFFF;		// 0xFFFF : 00000000001111111111111111
		if(flag[1] != 1) printf("\tBNE :\t?([$%2u 0x%2X]!=[$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + line*4);
		if(reg[rs] != reg[rt]) {
			jumpFlg = 1;
			// 0x48 = 0x20 + 4 + line*4 +0x0	<-> line*4 = 0x48-0x24 <-> line = 9
			pc = pc + 4 + line*4;		// pAddr形式
			if(flag[1] != 1) printf("\t\t<TRUE & JUMP> -> (branch_to) 0x%04x\n", pc);
		} else {
			if(flag[1] != 1) printf("\t<FALSE & NOP>\n");
		}
		opNum[BNE]++;
	} else if (opcode == LW) {	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  1E: 00000111110000000000000000
		im = instruction & 0x0000FFFF;		// 0xFFFF: 00000000001111111111111111

		/* じかんがあるときにlw()内に移動する */
		if( im >= 0x8000 ) {	//imは16bit
			im = im & 0x00007FFF;
			if(flag[1] != 1) printf("(im:0x%X)", im);
			address = (reg[rs] + MEMORYSIZE - im);
		} else {
			address = (reg[rs]+im);
		}
		if(address > MEMORYSIZE) {
			printf("[] Memory overflow\n");
			exit(1);
		}
		address = address % MEMORYSIZE;
		if(flag[1] != 1) printf("\tLW :\tmemory[address:0x%04X]=0x%4X \n", address, memory[address]);
		reg[rt] = lw(address, memory);
		opNum[LW]++;
		if(flag[1] != 1) printf("\t\treg[rt(%u)] = 0x%X\n", rt, reg[rt]);
	} else if (opcode == SW) {
		// sw rs,rt,Imm => M[ R(rs)+Imm ] <- R(rt)	rtの内容をメモリのR(rs)+Imm番地に書き込む
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  1E: 00000111110000000000000000
		im = instruction & 0x0000FFFF;		// 0xFFFF: 00000000001111111111111111

		if(reg[rs] > MEMORYSIZE) {	/* rsがメモリサイズを越えているかどうかで分岐:この実装大丈夫か？ */
			if(flag[1] != 1) printf("(im:0x%X)/(reg[rs]:0x%X)\n", im, reg[rs]);
			if( im >= 0x8000 ) {	//符号拡張
				im = im & 0x00007FFF;
				address = (reg[rs]%MEMORYSIZE - im);
				
			} else {
				address = (reg[rs]%MEMORYSIZE + im);
			}
		} else {
			if( im >= 0x8000 ) {	//符号拡張
				im = im & 0x00007FFF;
				address = (reg[rs] - im);
			} else {
				address = (reg[rs] + im);
			}
		}
		address = address % MEMORYSIZE;
		if(flag[1] != 1) printf("\tSW :\t[address: 0x%04X-0x%04X] <- [$%2u(rt): 0x%2X]\n", address, address+3, rt, reg[rt]);
		if(address > MEMORYSIZE) {
			printf("[] Memory overflow\n");
			exit(1);
		}
		sw(reg[rt], address, memory, rs);
		if(flag[1] != 1) printf("\tmemory[address: 0x%04X-0x%04X] : %02X %02X %02X %02X\n", address, address+3, memory[address+3], memory[address+2], memory[address+1], memory[address]);
		opNum[SW]++;
	} else if (opcode == JAL) {
		jump = instruction & 0x3FFFFFF;
		jumpFlg = 1;
		if(flag[1] != 1) printf("\tJAL: 0x%04x ($ra <- pc[%X])\n", jump*4, pc+4);
		reg[31] = pc+4;
		pc = jump*4 + PCINIT;
		opNum[JAL]++;
	} else if (opcode == INOUT) {		// シリアルポートから読み込み
		if(flag[1] != 1) printf("\tIN/OUT \n");
//		inout();
	} else {
		printf("[Unknown OPCODE]\n");
	}

	if(jumpFlg == 0) {
		pc = pc + 4;
		jumpFlg = 0;
	} else {
		jumpFlg = 0;
	}
	return pc;
}

int main (int argc, char* argv[]) {
	unsigned int opBuff[BUFF];	// ファイルから読み込む命令列
	unsigned char srBuff[BUFF];	// ファイルから読み込むデータ列(シリアル通信用)
	int srRead=0;
	int fd1 = 0, fd2 = 0;
	unsigned int maxpc;
	unsigned int breakCount = 0;
	unsigned int breakpoint = 0x80;
	unsigned int pc = 0;
	unsigned int *memory;
	unsigned char *serial;
	int i=0;
	unsigned int pAddr = 0;
	unsigned int count = 0;
	unsigned int snum = 0;	// serial portに書き出したbyte数
	int sInFlag=0;
	int arg1,arg2,arg0;
	int flag[32];
	/* flag[0]:help flag[1]:hide  */

//	printf("\n\t====== Initialize ======\n");
	memory = (unsigned int *) calloc( MEMORYSIZE, sizeof(unsigned int) );
	if(memory == NULL) {
		perror("memory allocation error\n");
		return -1;
	}
	serial = (unsigned char *) calloc( MEMORYSIZE, sizeof(unsigned char) );
	if(serial == NULL) {
		perror("memory allocation error\n");
		return -1;
	}

	for(count=0; count<MEMORYSIZE; count++) {
		memory[count] = 0;
		if(memory[count] != 0) printf("memory\n");
		count++;
	}

	/* 引数として<ファイル:メモリ><ファイル:シリアルポート入力>をとる。それ以外は終了 */
	if (argc < 2) {
		return -1;
		printf("please input file.\n");
	}
	/* ファイルから実行命令列を読み込む */
	printf("%s\n", argv[1]);
	fd1 = open(argv[1], O_RDONLY);
	maxpc = read(fd1, opBuff, BUFF);
	if(maxpc==0) {
		perror("opBuff failed to read\n");
		return -1;
	} else {
//		printf("opBuff succeeded to read\n");
	}


	/* 引数の処理 */
	while(i < argc) {
		flag[0] = strcmp(argv[i], HELP);
		if(flag[0] == 0) { printhelp(); exit(1); }
		flag[1] = strcmp(argv[i], HIDE);
		if(flag[1] == 0) { flag[1] = 1; } else { flag[1] = 0; }
		arg0 = strcmp(argv[i], SERIALIN);
		if(arg0 == 0) { sInFlag = i; printf("%s\n", argv[sInFlag+1]); }
		arg1 = strcmp(argv[i], PRINTREG);
		if(arg1 == 0) { printreg = 1; }
		arg2 = strcmp(argv[i], BREAKPOINT);
		if(arg2 == 0 && argc >= i) { breakpoint = (unsigned int) atoi(argv[i+1]); printf("breakpoint = %u\n", breakpoint); }
		i++;
	}
	i=0;
	/* ファイルからシリアルポート経由で入力するデータ列を読み込む */
	if(sInFlag != 0) {	// 比較条件は後で変える
		fd2 = open(argv[sInFlag+1], O_RDONLY);
		if(fd2 < 0) { perror("Unknown error\n"); return 0; }
		srRead = read(fd2, srBuff, BUFF);
		
		if(srRead<0) {
			perror("srBuff failed to read\n");
			return -1;
		} else {
//			printf("opBuff succeeded to read\n");
		}
	}
	count=0;
	if(flag[1] != 1) {
		while(count<40) {
			if(opBuff[count] != 0) printf("%02u(pc:%2X):\t%8X\n", count, (count)*4, opBuff[count]);
			count++;
		}
		while(count<40) {
			if(srBuff[count] != 0) printf("SR %02u:\t%8X\n", count, srBuff[count]);
			count++;
		}
	}

	/* 入力文字列を問答無用でmemoryのPCINIT番地以降にコピーする */
	count = 0;
	printf("maxpc:0x%X(%uline)\n", maxpc+PCINIT, (maxpc)/4);
	while(count < BUFF) {
//		memory[count*4+PCINIT] = opBuff[count];
		if ( opBuff[count] != 0) {
			memory[4*count+3+PCINIT] = (opBuff[count] & 0xFF000000) >> 24;
			memory[4*count+2+PCINIT] = (opBuff[count] & 0x00FF0000) >> 16;
			memory[4*count+1+PCINIT] = (opBuff[count] & 0x0000FF00) >>  8;
			memory[4*count+PCINIT] = opBuff[count] & 0x000000FF;
//			printf("[ DEBUG ]\t%08X, %u\n", opBuff[count], count);
//			printf("[ DEBUG ]\t%02X%02X%02X%02X\n", memory[4*count+PCINIT+3], memory[4*count+PCINIT+2], memory[4*count+PCINIT+1], memory[4*count+PCINIT]);
		}
		count++;
	}

	count=0;

	/* initialize */
	/* register init */
	for(i=0; i<REGSIZE; i++) {
		reg[i] = 0;
	}
	for(i=0;i<128;i++) {
		opNum[i]=0;
	}
	for(i=0;i<128;i++) {
		funcNum[i]=0;
	}
	reg[REGSP] = 0x1FFF4;

	/* Program Counter init */
	pc = PCINIT;
	pAddr = pc;

	
	/* シミュレータ本体 */
	while(pc > PCINIT-1) {	// unsigned int
		if(flag[1] != 1) printf("\n\t====== next: %u ======\n", ((pc-PCINIT)/4));
		reg[0] = 0;

//		fetch();	// memory[pc]の内容をロード?
		operation = memory[pAddr] | memory[pAddr+1] << 8 | memory[pAddr+2] << 16 | memory[pAddr+3] << 24;
		pc = decoder(pc, operation, memory, srBuff, serial, breakCount, flag);
		if(operation != 0 && printreg != 1) {
			if(flag[1] != 1)  printRegister();	// 命令実行後のレジスタを表示する
		}
		pAddr = pc;
		if(pc > PCINIT*4) pc = PCINIT;
		breakCount++;
		if (breakCount > breakpoint) break;
		if(pc > (maxpc+PCINIT)) {
			printf("pc = 0x%08X, maxpc = 0x%08X\n", pc, maxpc+PCINIT);
			break;
		}
	}
	if(flag[1] != 0) printf("\n[ FINISHED ]\n");
	snum = 0;
	printf("\nメモリダンプ\n");
	while(snum < MEMORYSIZE-3) {
		if(snum >= PCINIT && snum < PCINIT*0x2) {
			snum++;
			continue;
		}
		if ( (memory[snum] != 0 || memory[snum+1] != 0 || memory[snum+2] != 0 || memory[snum+3] != 0) ) {
			printf("memory[0x%06X] = %02X %02X %02X %02X\n", snum, memory[snum+3], memory[snum+2], memory[snum+1], memory[snum]);
		}
		snum = snum + 4;
	}
	snum = 0;
	printf("\nレジスタ吐き出し\n");
	printRegister();


	printf("\n\n");
	printf("Total instructions: %u\n", breakCount);
	breakCount = breakCount - funcNum[NOP];
	printf("Total instructions (except NOP): %u\n", breakCount);
	printf("\n(OP)  : \t(Num), \t(Ratio)\n");
	printf("ADDIU : %6u, %03.2f (%%)\n", opNum[ADDIU], (double) 100*opNum[ADDIU]/breakCount);
	printf("LW    : %6u, %03.2f (%%)\n", opNum[LW], (double) 100*opNum[LW]/breakCount);
	printf("SW    : %6u, %03.2f (%%)\n", opNum[SW], (double) 100*opNum[SW]/breakCount);
	printf("JUMP  : %6u, %03.2f (%%)\n", opNum[JUMP], (double) 100*opNum[JUMP]/breakCount);
	printf("JAL   : %6u, %03.2f (%%)\n", opNum[JAL], (double) 100*opNum[JAL]/breakCount);
	printf("BEQ   : %6u, %03.2f (%%)\n", opNum[BEQ], (double) 100*opNum[BEQ]/breakCount);
	printf("BNE   : %6u, %03.2f (%%)\n", opNum[BNE], (double) 100*opNum[BNE]/breakCount);

	printf("JR    : %6u, %03.2f (%%)\n", funcNum[JR], (double) 100*funcNum[JR]/breakCount);
	printf("SUBU  : %6u, %03.2f (%%)\n", funcNum[SUBU], (double) 100*funcNum[SUBU]/breakCount);
	printf("AND   : %6u, %03.2f (%%)\n", funcNum[AND], (double) 100*funcNum[AND]/breakCount);
	printf("OR    : %6u, %03.2f (%%)\n", funcNum[OR], (double) 100*funcNum[OR]/breakCount);
	printf("ADDU  : %6u, %03.2f (%%)\n", funcNum[ADDU], (double) 100*funcNum[ADDU]/breakCount);
	printf("SLT   : %6u, %03.2f (%%)\n", funcNum[SLT], (double) 100*funcNum[SLT]/breakCount);
	printf("SLL   : %6u, %03.2f (%%)\n", funcNum[SLL], (double) 100*funcNum[SLL]/breakCount);
	printf("SRL   : %6u, %03.2f (%%)\n", funcNum[SRL], (double) 100*funcNum[SRL]/breakCount);
	printf("NOP   : %6u\n", funcNum[NOP]);




	snum = 0;
	sInFlag = 0;
	printf("\nシリアルポート出力 ");
	while(snum<BUFF) {
		if (serial[snum] != 0) {
			if(sInFlag == 0) { printf("\n"); }
			sInFlag = 1;
			printf("%c\n", serial[snum]);
		}
		snum++;
	}
	if(sInFlag == 0) { printf(":無し\n"); }

	free(memory);
	memory = NULL;
	free(serial);
	serial = NULL;

	close(fd1);
	close(fd2);
	return 0;
}

