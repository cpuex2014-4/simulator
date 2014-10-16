#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define REGSIZE 32
#define BUFF 65536
#define MLINE 8
#define MROW 65536
#define ADDIU 0x9
#define JR 0x9
#define LUI 0x9
#define ORI 0x9
#define NOP 0x0
#define LW 0x23
#define ADDU 0x21
#define SUBU 0x13
#define JUMP 0x2
#define SLT 0x2A
#define BEQ 0x4

unsigned int reg[REGSIZE];	// 32 register
unsigned int rZ, rV, rN, rCarry;	// condition register
unsigned int memoryA[MLINE][MROW];	// 2MB=512k Word SRAM Memory
										// MLINE=32, MROW=65536
unsigned int opBuff[BUFF];	// ファイルから読み込む命令列
unsigned int operation;	// その瞬間に実行する命令
unsigned int pc;		// program counter: jump -> memory[pc]
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

unsigned int subu (unsigned int subuA, unsigned int subuB) {
	// R-type
	// subu rs rt rd
	// rd <- rs - rt
	// subuA : rs, subuB : rt
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ;

	subuB = ~subuB;

	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		if(bit == 0) {
			c=1;
			bitA = ((subuA << (31-bit)) >> (31-bit)) >> bit;
			bitB = ((subuB << (31-bit)) >> (31-bit)) >> bit;
			bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
			c = (bitA&bitB) | (bitB&c) | (bitA&c);
			rd = rd | (bitQ << bit);
		} else {
			bitA = ((subuA << (31-bit)) >> (31-bit)) >> bit;
			bitB = ((subuB << (31-bit)) >> (31-bit)) >> bit;
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
	rCarry = c;

	return rd;
}

unsigned int addu(unsigned int adduA, unsigned int adduB){
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ;

	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		bitA = ((adduA << (31-bit)) >> (31-bit)) >> bit;
		bitB = ((adduB << (31-bit)) >> (31-bit)) >> bit;

		bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
		c = (bitA&bitB) | (bitB&c) | (bitA&c);

		rd = rd | (bitQ << bit);
		bit++;
	}
	rCarry = c;
	return rd;
}


/* レジスタ内容表示器 */
void printRegister(void) {
// unsigned int rZ, rV, rN, rCarry;	// condition register
	int i;

	printf("R[Condition] ZVNC = %x/%x/%x/%x \n", rZ, rV, rN, rCarry);
	for(i=0; i<REGSIZE; i++) {
		if(i%8 == 0) 
			printf("R[%2d->%2d] : ", i, i+8);
		else if( (i+1)%8 == 0 ) {
			printf("%4x\n",reg[i]);
		} else {
			printf("%4x ",reg[i]);
		}
	}

}

/* opcodeが0の時の操作を、末尾6ビットによって決める */
unsigned int funct (unsigned int pc, unsigned int instruction) {
	unsigned int function = 0;
	unsigned int rs=0;
	unsigned int rt=0;
	unsigned int rd=0;
	unsigned int im=0;

	/* [op] [rs] [rt] [rd] [shamt] [funct] */
	/* 先頭&末尾6bitは0であることが保証済み */
	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	rd = (instruction >> 11 ) & 0x1F;
	
	function = instruction & 0x3F;
	printf("\t[function:%2x]\n", function);
	switch (function) {
		case (JR) :
			printf("\t<Function: JR>\n");
			rd = instruction & 0x7c0;
			pc = reg[rd];
			jumpFlg = 1;
			rd = 0;
			break;
		case (ADDU) :
			printf("\t<Function: ADDU> ");
			printf("rs:R[%u] 0x%2x(%u), rt:R[%u] 0x%2x(%u)\n", rs, reg[rs], reg[rs], rt, reg[rt], reg[rt]);
			reg[rd] = addu(reg[rs], reg[rt]);
			printf("=> [rd:%u] 0x%2x(%u)\n", rd, reg[rd], reg[rd]);
			break;
		case (SUBU) :
			printf("\t<Function: SUBU>");
			printf("rs:R[%u] 0x%2x(%u), rt:R[%u] 0x%2x(%u)\n", rs, reg[rs], reg[rs], rt, reg[rt], reg[rt]);
			break;
		case (SLT) :
			printf("\t<Function: SLT> < rs < rt? > :");
			printf("rs:R[%u] 0x%2x(%u), rt:R[%u] 0x%2x(%u)\n", rs, reg[rs], reg[rs], rt, reg[rt], reg[rt]);
			reg[rd] = slt(reg[rs], reg[rt]);
			if(reg[rd]) {
				printf("\t=> rd:R[%2u] 0x%4x(%u), <TRUE>\n", rd, reg[rd], reg[rd]);
			} else {
				printf("\t=> rd:R[%2u] 0x%4x(%u), <FALSE>\n", rd, reg[rd], reg[rd]);
			}
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
	unsigned int rd=0;
	unsigned int im=0;
	unsigned int jump=0;

	printf("\t[instruction: 0x%2x]\n", instruction);
	opcode = instruction >> 26;	// opcode: 6bitの整数
	printf("\t[opcode:%2x]\n", opcode);

	/* 適当な時にswitch文に切り替え */
	if(opcode == 0) funct(pc, instruction);
	else if (opcode == ADDIU) {
		printf("\t[opcode] ADDIU ");
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  1E: 00000111110000000000000000
		im = instruction & 0x0000FFFF;		// 0xFFFF: 00000000001111111111111111
		printf("\trs:R[%2u] 0x%2x(%u), rt:R[%2u] 0x%2x(%u), [im: 0x%4x] \n", rs, reg[rs], reg[rs], rt, reg[rt], reg[rt], im);
		reg[rt] = addui(reg[rs], im);
		printf("=> \trt:R[%2u] 0x%2x(%u)\n", rt, reg[rt], reg[rt]);
	}
	else if (opcode == JUMP) {
		printf("\t[opcode] J \n");
		jumpFlg = 1;
		jump = instruction & 0x3FFFFFF;
		printf("\t[instruction] 0x%08x(%u)\n", instruction, instruction);
		printf("\t<jump_to> 0x%4x(%u)\n", jump, jump);
		pc = jump*4 + 0x400000;	// jumpはpAddr形式
	}
	else if (opcode == BEQ) { // beq I-Type: 000100 rs rt BranchAddr 	等しいなら分岐 
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  F : 00000111110000000000000000
		printf("\t[opcode] BEQ < rs=rt? > \n");
		jump = instruction & 0xFFFF;
		printf("\t[instruction] 0x%08x(%u)\n", instruction, instruction);
		printf("\trs:R[%2u] 0x%2x(%u), rt:R[%2u] 0x%2x(%u)\n", rs, reg[rs], reg[rs], rt, reg[rt], reg[rt]);
		if(reg[rs] == reg[rt]) {
			printf("\t<TRUE & JUMP> -> <jump_to> 0x%4x(%u)\n", jump, jump);
			jumpFlg = 1;
			pc = jump*4 + 0x400000;	// jumpはpAddr形式
		} else {
			printf("\t<FALSE & NOP>\n");
		}

	}
	else if (opcode == LW) {	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード
		printf("\t[opcode] LW \n");
		;
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

/*
構成
		OPcode	funct	funcname			func
	0	000000	010000	add(a,b,rd)			a+b -> rd
	8	000100			addi(a,IMM,rt)		a+IMM -> rt	//IMM = immediate
*	9	000101			addui(a,uIMM,rt)	a+uIMM -> rt
*	0	000000	010001	addu(a,b,rd)		a+b (unsigned) -> rd
*	0	000000	010010	and(a,b,rd)			a&b -> rd
	C	001100			andi(a,IMM,rt)			a&IMM -> rt
*	4	000100			beq(a,b,)			if(a == b) then jump
*	5	000101			bne(a,b)			if(a != b) then jump
*	2	000001			jmp					jump
*	3	000011			jandl				jump and link	[?]
*	0	000000	001000	jr(rt)				jump to rt
*	0	000000	010100	lbu(rt)				load byte unsigned -> rt
	0	000000	010101	lhwu				load halfword unsigned -> rt
*	F	001111			lui					load upper immediate
*	23	010011			lw(rs,rt)			load word from M[rs] to rs
*	0	000000	010111	nor(a,b)			
*	0	000000	010101	or(a,b,rd)				a || b -> rd
*	D	001101			ori(a,IMM)
	0	000000	011010	slt					set less than
	A	001010			slti				set less than
	B	001011			sltiu				set less than
	0	000000	011011	sltu				set less than
*	0	000000	000000	sll					shift left logial
*	0	000000	000010	srl					shift right logical
*	28	011000			sb					store byte
	29	011001			sh					store halfword
	2B	011011			sw(rt,rs)			store from rt to M[rs]
	0	000000	010010	sub(a,b,rd)			a-b
*	0	000000	010011	subu(a,b,rd)		a-b (unsigned)

*をつけた命令を優先
*/


/* MEMORY */
/*
仕様
	Cap.	2MB = 2 * 1024 * 1024 B
	1 unit = sizeof(unsigned int)
	line	524288
	store(arg, address)	memory[address]にargの内容を保存
	load(arg, address)	memory[address]から1単位読み込み、argに代入
*/
int pp(unsigned int opBuff[], unsigned int* memory, unsigned int pc) {
	unsigned int memAddr = pc;
	unsigned int bufAddr = 0;
	unsigned int count = 0;

	memory = &pc;

	while(count < 0x40000) {
		*memory = opBuff[bufAddr];
		printf("memory[%x] = %x, ", memAddr, memory[memAddr]);
		memAddr++;
		memory++;
		bufAddr++;
		count++;
	}


	return 0;
}

int main (int argc, char* argv[]) {
	int fd = 0;
	unsigned int breakCount = 0;
	unsigned int *memory = malloc( sizeof(unsigned int) * 2 * 1024* 1024 );
	if(memory == NULL) {
		perror("memory allocation error\n");
		return -1;
	}

	int i;
	unsigned int pAddr = 0;

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
	}

	unsigned int count=0;
	while(count<100) {
		printf("%x, ", opBuff[count]);
		count++;
	}

	printf("\n");
	/* initialize */
	reg[0] = 0x0;	// Zero register
	/* Program Counter init */
	pc = 0x400000;
	pAddr = (pc - 0x400000) / 4;
	/* register init */
	for(i=0; i<REGSIZE; i++) {
		reg[i] = 0;
	}
	printf("\n=================== Initialize ====================\n");

	/* 読み込んだ内容をメモリへ書き込む */
	/* 現時点では入力文字列を問答無用でmemoryの0x400000番地以降にコピーする */
//	pp(opBuff, memory, pc);	// preprocessor
	
	/* 1word(32bit)ごとに1命令実行。実行終了する度にPC = pc+4; */
	/* 当面はPCを進める度にレジスタの内容を全て書き出す */

	/* User Text Segment : [00400000] ... [00440000] */
	/* Kernel Text Segment : [80000000] ... [80010000] */
	/* User data segment : [10000000] ... [10040000] */
	/* User Stack : [7FFFF7C4] ... [80000000] */
	/* Kernel Data Segment : [90000000] ... [90010000] */


	while(pAddr < 0x10) {	// unsigned int
		printf("[pAddr: 0x%03x], [pc: 0x%08x]\n", pAddr, pc);

//		fetch();	// memory[pc]の内容をロード?

		printRegister();	// その時の命令とレジスタの値を表示する
		operation = opBuff[pAddr];
		decoder(operation);
		printf("\n===================== next ========================\n");
//		pc = pc + 4; // 呼び出し先の関数で処理する
		pAddr = (pc - 0x400000) / 4;
		if(pc > 0x440000) pc = 0x400000;
		breakCount++;
		if (breakCount > 100) break;
	}
	
	free(memory);
	memory = NULL;

	close(fd);
	return 0;
}

