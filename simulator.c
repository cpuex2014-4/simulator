#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>

#define PCINIT 0x400000
#define REGSIZE 32
#define BUFF 65536
#define MEMORYSIZE (512lu * 1024lu)
#define ADDIU 0x9
#define JR 0x9
#define LUI 0x9
#define ORI 0x9
#define NOP 0x0
#define LW 0x23
#define SW 0x2B
#define JAL 0x2B
#define INOUT 0x3F
#define ADDU 0x21
#define SUBU 0x13
#define JUMP 0x2
#define SLT 0x2A
#define BEQ 0x4
#define AND 0x24
#define OR 0x25

unsigned int reg[REGSIZE];	// 32 register
unsigned int rZ, rV, rN, rCarry;	// condition register
unsigned int operation;	// その瞬間に実行する命令
unsigned int pc;		// program counter: jump -> memory[pc]
unsigned int HI=0, LO=0;
int jumpFlg=0;	// pcを変更するプログラムが実行されたら1。ジャンプ後0に戻る

unsigned int slt(unsigned int rs, unsigned int rt) {
// slt rs, rt, rd
// R-type
// rs < rt ならばレジスタ rd に 1 を代入、そうでなければ 0 を代入。 
	unsigned int rd;

	if(rs < rt) {
		rd = 1;
	} else {
		rd = 0;
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
	unsigned int bitA, bitB;
	unsigned int bitQ=0;

	rt = ~rt;

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

unsigned int addui(unsigned int adduA, unsigned int Imm) {
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ=0;


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

void jr(unsigned int rs) {
	jumpFlg = 1;
	pc = reg[rs] + PCINIT;
}


/* レジスタ内容表示器 */
void printRegister(void) {
// unsigned int rZ, rV, rN, rCarry;	// condition register
	int i;

//	printf("R[Condition] ZVNC = %x/%x/%x/%x \n", rZ, rV, rN, rCarry);
/*	for(i=0; i<REGSIZE; i++) {
		if(i%16 == 0) 
			printf("R[%2d->%2d] : ", i, i+16);
		else if( (i+1)%16 == 0 ) {
			printf("%4x\n",reg[i]);
		} else {
			printf("%4x ",reg[i]);
		}
	}
*/
	for(i=0; i<REGSIZE; i++) {
		if(i == 0) {
			printf("$zr=%6x, ",reg[i]);
		} else if( i == 1 ) {
			printf("$at=%6x, ",reg[i]);
		} else if( i == 2 || i == 3 ) {
			printf("$v%d=%6x, ", i-2,reg[i]);
		} else if( i >= 4 && i < 8 ) {
			printf("$a%d=%6x, ", i-4,reg[i]);
		} else if( i >= 8 && i < 16 ) {
			printf("$t%d=%6x, ", i-8, reg[i]);
			if(i==10) printf("\n");
		} else if( i >= 16 && i < 24 ) {
			printf("$s%d=%6x, ", i-16, reg[i]);
			if(i==21) printf("\n");
		} else if( i == 24 || i == 25 ) {
			printf("$t%d=%6x, ", i-16, reg[i]);
		} else if( i == 26 || i == 27 ) {
			printf("$k%d=%6x, ",i-26,reg[i]);
		} else if( i == 28 ) {
			printf("$gp=%6x, ",reg[i]);
		} else if( i == 29 ) {
			printf("$sp=%6x, ",reg[i]);
		} else if( i == 30 ) {
			printf("$fp=%6x, ",reg[i]);
		} else if( i == 31 ) {
			printf("$ra=%6x",reg[i]);
		} else {
			printf("N/A");
		}
	}
	printf("\n");

}

/* opcodeが0の時の操作を、末尾6ビットによって決める */
unsigned int funct (unsigned int pc, unsigned int instruction) {
	unsigned int function = 0;
	unsigned int rs=0;
	unsigned int rt=0;
	unsigned int rd=0;
	unsigned int im=0;
	if (im == 0) ;

	/* [op] [rs] [rt] [rd] [shamt] [funct] */
	/* 先頭&末尾6bitは0であることが保証済み */
	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	rd = (instruction >> 11 ) & 0x1F;
	
	function = instruction & 0x3F;
	printf("\t[function:%2x]\n", function);
	switch (function) {
		case (NOP) :
			printf("\t<Function: NOP>\n");
			break;
		case (JR) :
			printf("\t<Function: JR>\n");
			rd = instruction & 0x7c0;
			pc = reg[rd];
			jumpFlg = 1;
			rd = 0;
			break;
		case (ADDU) :	// rd=rs+rt
			printf("\t<Function: ADDU> ");
			printf("$%2u(rs) 0x%2x, rt:$%2u 0x%2x ", rs, reg[rs], rt, reg[rt]);
			reg[rd] = addu(reg[rs], reg[rt]);
			printf("=> [rd:%u] 0x%2x\n", rd, reg[rd]);
			break;
		case (SUBU) :	// rd=rs-rt
			printf("\t<Function: SUBU>");
			printf("$%2u(rs) 0x%2x, rt:$%2u 0x%2x ", rs, reg[rs], rt, reg[rt]);
			reg[rd] = subu(reg[rs], reg[rt]);
			printf("=> [rd:%u] 0x%2x\n", rd, reg[rd]);
			break;
		case (SLT) :
			printf("\t<Function: SLT> < rs < rt? > :");
			printf("$%2u(rs) 0x%2x, rt:$%2u 0x%2x ", rs, reg[rs], rt, reg[rt]);
			reg[rd] = slt(reg[rs], reg[rt]);
			if(reg[rd]) {
				printf("\t=> rd:$%2u 0x%4x, <TRUE>\n", rd, reg[rd]);
			} else {
				printf("\t=> rd:$%2u 0x%4x, <FALSE>\n", rd, reg[rd]);
			}
			break;
		case (AND) :
			printf("\t<Function: AND>");
			printf("$%2u(rs) 0x%2x, rt:$%2u 0x%2x ", rs, reg[rs], rt, reg[rt]);
			reg[rd] = and(reg[rs], reg[rt]);
			printf("=> [rd:%u] 0x%2x\n", rd, reg[rd]);
			break;
		case (OR) :
			printf("\t<Function: OR>");
			printf("$%2u(rs) 0x%2x, rt:$%2u 0x%2x ", rs, reg[rs], rt, reg[rt]);
			reg[rd] = or(reg[rs], reg[rt]);
			printf("=> [rd:%u] 0x%2x\n", rd, reg[rd]);
			break;

		default :
			printf("Default switch has selected.\n");
	}
	return pc;
}
/* デコーダ */
unsigned int decoder (unsigned int instruction) {
	unsigned int opcode;
	unsigned int rt=0;
	unsigned int rs=0;
	unsigned int im=0;
	unsigned int jump=0;

	printf("\t[instruction: 0x%2x]\n", instruction);
	opcode = instruction >> 26;	// opcode: 6bitの整数
//	printf("\t[opcode:%2x]\n", opcode);

	/* 適当な時にswitch文に切り替え */
	if(opcode == 0) {
		funct(pc, instruction);
	} else if (opcode == ADDIU) {
		printf("\t[opcode] ADDIU(rt=rs+Imm) ");
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  1E: 00000111110000000000000000
		im = instruction & 0x0000FFFF;		// 0xFFFF: 00000000001111111111111111
		printf("\t$%2u(rs) 0x%2x, rt:$%2u 0x%2x, [im: 0x%4x] ", rs, reg[rs], rt, reg[rt], im);
		reg[rt] = addui(reg[rs], im);	// 処理部
		printf("=> rt:$%2u 0x%2x\n", rt, reg[rt]);
	} else if (opcode == JUMP) {
		printf("\t[opcode] J \n");
		jumpFlg = 1;
		jump = instruction & 0x3FFFFFF;
		printf("\t<jump_to> 0x%4x\n", jump);
		pc = jump*4 + PCINIT;	// jumpはpAddr形式
	} else if (opcode == BEQ) { // beq I-Type: 000100 rs rt BranchAddr 	等しいなら分岐 
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  F : 00000111110000000000000000
		printf("\t[opcode] BEQ < rs=rt? -> \n");
		jump = instruction & 0xFFFF;
		printf("\t$%2u(rs) 0x%2x, rt:$%2u 0x%2x\n", rs, reg[rs], rt, reg[rt]);
		if(reg[rs] == reg[rt]) {
			printf("\t<TRUE & JUMP> -> <jump_to> 0x%4x\n", jump);
			jumpFlg = 1;
			pc = pc + jump*4 + 4;	// jumpはpAddr形式
		} else {
			printf("\t<FALSE & NOP>\n");
		}
	} else if (opcode == LW) {	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード
		printf("\t[opcode] LW \n");
//		lw();
	} else if (opcode == SW) {
		printf("\t[opcode] SW \n");
//		sw();
	} else if (opcode == JAL) {
		printf("\t[opcode] JAL \n");
//		jal();
	} else if (opcode == INOUT) {		// シリアルポートから読み込み
		printf("\t[opcode] IN/OUT \n");
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
	return 1;
}


int pp(unsigned int opBuff[], int memory[]) {
	unsigned long count = 0;

	while(count < BUFF) {
		memory[count] = opBuff[count];
		count++;
	}
	return 0;
}

int main (int argc, char* argv[]) {
	unsigned int opBuff[BUFF];	// ファイルから読み込む命令列
	int fd = 0;
	unsigned int breakCount = 0;
	unsigned int *memory;
	int i;
	unsigned int pAddr = 0;
	unsigned long count=0;

	memory = (unsigned int *) calloc( MEMORYSIZE, sizeof(unsigned int) );
	if(memory == NULL) {
		perror("memory allocation error\n");
		return -1;
	}

	for(count=0; count<MEMORYSIZE; count++) {
		memory[count] = 0;
		count++;
	}
	/* 引数としてファイル名をとる。それ以外は終了 */
	if (argc != 2) {
		return -1;
		printf("please input file.\n");
	}
	/* ファイルから実行命令列を読み込む */
	fd = open(argv[1], O_RDONLY);
	i = read(fd, opBuff, BUFF);
	if(i<0) {
		perror("opBuff failed to read\n");
		return -1;
	} else {
		printf("\nopBuff succeeded to read\n");
	}
	count=0;
	while(count<20) {
		if(opBuff[count] != 0) printf("%2lu: %8x\n", count, opBuff[count]);
		count++;
	}
	count=0;
	while(count < BUFF) {
		memory[count] = opBuff[count];
		count++;
	}


	/* initialize */
	reg[0] = 0x0;	// Zero register
	/* Program Counter init */
	pc = PCINIT;
	pAddr = (pc - PCINIT) / 4;
	/* register init */
	for(i=0; i<REGSIZE; i++) {
		reg[i] = 0;
	}
	printf("\n=================== Initialize ====================\n");

	/* 読み込んだ内容をメモリへ書き込む */
	/* 現時点では入力文字列を問答無用でmemoryのPCINIT番地以降にコピーする */
//	pp(opBuff, memory, pc);	// preprocessor
	
	/* 1word(32bit)ごとに1命令実行。実行終了する度にPC = pc+4; */
	/* 当面はPCを進める度にレジスタの内容を全て書き出す */

	/* User Text Segment : [00400000] ... [00440000] */
	/* Kernel Text Segment : [80000000] ... [80010000] */
	/* User data segment : [10000000] ... [10040000] */
	/* User Stack : [7FFFF7C4] ... [80000000] */
	/* Kernel Data Segment : [90000000] ... [90010000] */


	while(pAddr < 0x10) {	// unsigned int
		printf("[pc: 0x%08x, line: %u]\n", pc, (pc-PCINIT)/4);
		reg[0] = 0;

//		fetch();	// memory[pc]の内容をロード?
		operation = memory[pAddr];
		if(operation != 0) {
			printRegister();	// その時の命令とレジスタの値を表示する
		}
		decoder(operation);
		printf("\n===================== next ========================\n");
//		pc = pc + 4; // 呼び出し先の関数で処理する
		pAddr = (pc - PCINIT) / 4;
		if(pc > 0x440000) pc = PCINIT;
		breakCount++;
		if (breakCount > 1500) break;
	}
	
	free(memory);
	memory = NULL;

	close(fd);
	return 0;
}

