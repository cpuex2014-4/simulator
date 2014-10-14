#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define REGSIZE 32
#define BUFF 65536
#define MLINE 8
#define MROW 65536
#define ADDU 0x21
#define ADDIU 0x9
#define JR 0x9
#define LUI 0x9
#define ORI 0x9
#define NOP 0x0
#define LW 0x23
#define ADDU 0x11
#define SUBU 0x13

unsigned int reg[REGSIZE];	// 32 register
unsigned int memory[MLINE][MROW];	// 2MB=512k Word SRAM Memory
										// MLINE=32, MROW=65536
unsigned int opBuff[BUFF];	// ファイルから読み込む命令列
unsigned int operation;	// その瞬間に実行する命令
unsigned int pc;		// program counter: jump -> reg[pc]
unsigned int mipsStatus;	// status
int jumpFlg=0;	// pcを変更するプログラムが実行されたら1。ジャンプ後0に戻る






/* レジスタ内容表示器 */
void printRegister() {
	int i;

	printf("[PC] = 0x%x\n", pc);
	for(i=0; i<REGSIZE; i++) {
		if( (i+1)%8 == 0 ) {
			printf("reg[%2d]=%4x\n", i, reg[i]);
		} else {
			printf("reg[%2d]=%4x, ", i, reg[i]);
		}
	}
}

/* opcodeが0の時の操作を、末尾6ビットによって決める */
unsigned int funct (unsigned int pc, unsigned int instruction) {
	unsigned int function = 0;
	unsigned int rt=0;
	unsigned int rs=0;
	unsigned int rd=0;
	unsigned int im=0;
	
	function = instruction & 0x3F;
	switch (function) {
		case (JR) :
			printf("jr switch has selected.\n");
			rd = instruction & 0x7c0;
			pc = reg[rd];
			jumpFlg = 1;
			rd = 0;
			break;
		case (ADDU) :
			printf("add switch has selected.\n");
			rd = (instruction >> 16) & 0x1F;
			rs = (instruction >> 21) & 0x1F;
			rt = (instruction >> 11 ) & 0x78;
			printf("[rd] 0x%2x(%u), ", rd, rd);
			printf("[rs] 0x%2x(%u), ", rs, rs);
			printf("[rt] 0x%4x(%u)\n", rt, rt);
			break;
		case (SUBU) :
			printf("add switch has selected.\n");
			rd = (instruction >> 16) & 0x1F;	// 0x  1E: 0000011111 0000000000000000
			rs = (instruction >> 21) & 0x1F;		// 0x  F : 1111100000 0000000000000000
			rt = (instruction >> 11 ) & 0x78;	// 0x 78 : 0000000000 1111100000000000
			printf("[rd] 0x%2x(%u), ", rd, rd);
			printf("[rs] 0x%2x(%u), ", rs, rs);
			printf("[rt] 0x%4x(%u)\n", rt, rt);
			break;
		default :
			printf("Default switch has selected.\n");
	}
	printf("[function:%2x]\n", function);
	return pc;
}
/* デコーダ */
/*
仕様
	input	operation : unsigned int
	output	pc	: unsigned int
	(ex)
		pc = encode(program)
*/
unsigned int decoder (unsigned int instruction) {
	unsigned int opcode;
	unsigned int rt=0;
	unsigned int rs=0;
	unsigned int rd=0;
	unsigned int im=0;

	printf("[instruction: 0x%2x]\n", instruction);
	opcode = instruction >> 26;	// opcode: 6bitの整数
	printf("[opcode:%2x]\n", opcode);
	if(opcode == 0) funct(pc, instruction);
	else if (opcode == ADDIU) {
		printf("[opcode] addiu ");
		rd = (instruction >> 16) & 0x1F;	// 0x  1E: 00000111110000000000000000
		rs = (instruction >> 21) & 0x1F;		// 0x  F : 11111000000000000000000000
		im = instruction & 0x0000FFFF;		// 0xFFFF: 00000000001111111111111111
		printf("[rd] 0x%2x(%u), ", rd, rd);
		printf("[rs] 0x%2x(%u), ", rs, rs);
		printf("[im] 0x%4x(%u)\n", im, im);
	}
	else if (opcode == ADDU) {
		printf("[opcode] addu \n");
		rd = (instruction >> 16) & 0x1F;	// 0x  1E: 00000111110000000000000000
		rs = (instruction >> 21) & 0x1F;	// 0x  F : 11111000000000000000000000
		im = instruction & 0x0000FFFF;		// 0xFFFF: 00000000001111111111111111
		printf("[rd] 0x%2x(%u), ", rd, rd);
		printf("[rs] 0x%2x(%u), ", rs, rs);
		printf("[im] 0x%4x(%u)\n", im, im);
	}
	else if (opcode == ADDIU) {
		;
	}
	else if (opcode == ADDIU) {
		;
	}
	else if (opcode == ADDIU) {
		;
	}
	else if (opcode == LW) {	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード
		printf("[opcode] LW \n");
		;
	}
	pc = pc + 4;
	return 1;
}

/* コア命令セット */
/*
仕様



*/
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



/* ALU */
/*
仕様



*/
/*
構成
	sub(a,b)	-
	mul(a,b)	*
	div(a,b)	/
	cmp(a,b)	if (a == b) then 1 else 0
	jmp(reg)	レジストリ($reg)へ飛ぶ


*/






/* FPU */
/*
仕様



*/
/*
構成
	fadd(a,b)	a +. b
	fsub(a,b)	a -. b
	fmul(a,b)	a *. b
	fdiv(a,b)	a /. b
	fcmp(a,b)	if (a == b) then 1 else 0
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


int main (int argc, char* argv[]) {
	int fd = 0;

	int i;
	unsigned int pAddr = 0;

	/* Program Counter init */
	pc = 0x400000;
	pAddr = (pc - 0x400000) / 4;
	/* register init */
	for(i=0; i<REGSIZE; i++) {
		reg[i] = 0;
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
	}

	/* 読み込んだ内容をメモリへ書き込む */

	/* 1word(32bit)ごとに1命令実行。実行終了する度にPC = pc+4; */
	/* 当面はPCを進める度にレジスタの内容を全て書き出す */

	/* User Text Segment : [00400000] ... [00440000] */
	/* Kernel Text Segment : [80000000] ... [80010000] */
	/* User data segment : [10000000] ... [10040000] */
	/* User Stack : [7FFFF7C4] ... [80000000] */
	/* Kernel Data Segment : [90000000] ... [90010000] */


	while(pAddr < 8) {	// unsigned int
		printf("[pAddr: 0x%x], ", pAddr);
		printRegister();	// その時の命令とレジスタの値を表示する
		if(jumpFlg == 0) {
			operation = opBuff[pAddr];
			decoder(operation);
		} else {
			operation = opBuff[pAddr];
			jumpFlg = 0;
			decoder(operation);
		}
		printf("\n===================== next ========================\n");
//		pc = pc + 4;
		pAddr = (pc - 0x400000) / 4;
		if(pc > 0x440000) break;
	}
	
	close(fd);
	return 0;
}

