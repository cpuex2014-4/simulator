#ifndef CONST_H
#define CONST_H

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
#define MFC1F 0x0	// Function
#define MTC1F 0x0	// Function
#define MOVSF 0x6	// Function
#define MFC1M 0x0	// fMt
#define MTC1M 0x4	// fMt
#define MOVSM 0x10	// fMt

#define ADDSM 0x10	// fMt
#define ADDSF 0x0	// Function
#define FSUB 0x1	// Function
#define FMUL 0x2	// Function
#define FDIV 0x3	// Function

#define FTOIF 0x24	// Function
#define FTOIM 0x10	// fMt
#define ITOFF 0x20	// Function
#define ITOFM 0x14	// fMt

#define CEQ		0x32
#define COLT	0x34
#define COLE	0x36


/* argument */
#define BREAKPOINT "--break"
#define PRINTREG "--reg"
#define SERIALIN "--serialin"
#define HELP "--help"
#define HIDE "--hide"
#define SEQUENTIAL "--sequential"
#define PRINTMEM "--memory"
#define FPUNATIVE "--native"
#define SERIALOUT "--serialout"
#define ARGMAX 20

/* register */
#define REGAT 1
#define REGGP 28
#define REGSP 29
#define REGFP 30
#define REGRA 31


#endif
