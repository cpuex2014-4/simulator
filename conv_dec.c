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
unsigned int operation;	// その瞬間に実行する命令
unsigned int pc;		// program counter: jump -> memory[pc]
unsigned int HI=0, LO=0;
int jumpFlg=0;	// pcを変更するプログラムが実行されたら1。ジャンプ後0に戻る


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
			printf("rs:R[%u] 0x%2x, rt:R[%u] 0x%2x\n", rs, reg[rs], rt, reg[rt]);
//			reg[rd] = addu(reg[rs], reg[rt]);
			printf("=> [rd:%u] 0x%2x\n", rd, reg[rd]);
			break;
		case (SUBU) :	// rd=rs-rt
			printf("\t<Function: SUBU>");
			printf("rs:R[%u] 0x%2x, rt:R[%u] 0x%2x\n", rs, reg[rs], rt, reg[rt]);
//			reg[rd] = subu(reg[rs], reg[rt]);
			printf("=> [rd:%u] 0x%2x\n", rd, reg[rd]);
			break;
		case (SLT) :
			printf("\t<Function: SLT> < rs < rt? > :");
			printf("rs:R[%u] 0x%2x, rt:R[%u] 0x%2x\n", rs, reg[rs], rt, reg[rt]);
//			reg[rd] = slt(reg[rs], reg[rt]);
			if(reg[rd]) {
				printf("\t=> rd:R[%2u] 0x%4x, <TRUE>\n", rd, reg[rd]);
			} else {
				printf("\t=> rd:R[%2u] 0x%4x, <FALSE>\n", rd, reg[rd]);
			}
			break;
		case (AND) :
			printf("\t<Function: AND>");
			printf("rs:R[%u] 0x%2x, rt:R[%u] 0x%2x\n", rs, reg[rs], rt, reg[rt]);
//			reg[rd] = and(reg[rs], reg[rt]);
			printf("=> [rd:%u] 0x%2x\n", rd, reg[rd]);
			break;
		case (OR) :
			printf("\t<Function: OR>");
			printf("rs:R[%u] 0x%2x, rt:R[%u] 0x%2x\n", rs, reg[rs], rt, reg[rt]);
//			reg[rd] = or(reg[rs], reg[rt]);
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

	printf("\t[instruction: 0x%2x / ", instruction);
	opcode = instruction >> 26;	// opcode: 6bitの整数
//	printf("\t[opcode:%2x]\n", opcode);

	/* 適当な時にswitch文に切り替え */
	if(opcode == 0) funct(pc, instruction);
	else if (opcode == ADDIU) {
		printf("opcode : ADDIU(rt=rs+Imm)]");
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  1E: 00000111110000000000000000
		im = instruction & 0x0000FFFF;		// 0xFFFF: 00000000001111111111111111
		printf("\trs:$[%2u], rt:$[%2u], [im: 0x%4x] \n", rs, rt, im);
//		reg[rt] = addui(reg[rs], im);	// 処理部
		printf("=> \trt:R[%2u] 0x%2x\n", rt, reg[rt]);
	} else if (opcode == JUMP) {
		printf("opcode : J \n");
		jumpFlg = 1;
		jump = instruction & 0x3FFFFFF;
//		printf("\t[instruction] 0x%08x\n", instruction);
//		printf("\t<jump_to> 0x%4x\n", jump);
		pc = jump*4 + PCINIT;	// jumpはpAddr形式
	} else if (opcode == BEQ) { // beq I-Type: 000100 rs rt BranchAddr 	等しいなら分岐 
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		rt = (instruction >> 16) & 0x1F;	// 0x  F : 00000111110000000000000000
		printf("opcode : BEQ < rs=rt? -> \n");
		jump = instruction & 0xFFFF;
		printf("\t[instruction] 0x%08x\n", instruction);
		printf("\trs:R[%2u] 0x%2x, rt:R[%2u] 0x%2x\n", rs, reg[rs], rt, reg[rt]);
		if(reg[rs] == reg[rt]) {
			printf("\t<TRUE & JUMP> -> <jump_to> 0x%4x\n", jump);
			jumpFlg = 1;
			pc = jump*4 + PCINIT;	// jumpはpAddr形式
		} else {
			printf("\t<FALSE & NOP>\n");
		}
	} else if (opcode == LW) {	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード
		printf("opcode : LW \n");
//		lw();
	} else if (opcode == SW) {
		printf("opcode : SW \n");
//		sw();
	} else if (opcode == JAL) {
		printf("opcode : JAL \n");
//		jal();
	} else if (opcode == INOUT) {		// シリアルポートから読み込み
		printf("opcode : IN/OUT \n");
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

	while(pAddr < 0x10) {	// unsigned int
		reg[0] = 0;

		operation = memory[pAddr];
		decoder(operation);
		pAddr = (pc - PCINIT) / 4;
		breakCount++;
		if (breakCount > 100) break;
	}
	
	free(memory);
	memory = NULL;

	close(fd);
	return 0;
}



