#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFF 65536




/* デコーダ */
/*
仕様
	input	program (1line) : unsigned int
	output	opcode	: unsigned int
	(ex)
		opcode = encode(program)
*/
unsigned int decoder (unsigned int program) {
	unsigned int opcode;

	opcode = program >> 26;
	return opcode;
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
*	0	000000	001000	jreg(rt)			jump to rs
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
	unsigned int opBuff[BUFF];


	/* 引数としてファイル名をとる。それ以外は終了 */
	if (argc != 2) {
		return -1;
		printf("please input file.\n");
	}	

	/* ファイルから実行命令列を読み込む */
	fd = open(argv[1], O_RDONLY);
	read(fd, opBuff, BUFF);
	
	/* 32文字(128byte)ごとに1命令実行。実行終了する度にPC++ */
	/* 当面はPCを進める度にレジスタの内容を全て書き出す */	


	close(fd);
	return 0;
}

