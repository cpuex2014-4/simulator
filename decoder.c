#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "const.h"
#include "alu.h"
#include "print.h"
#include "fpu/C/fpu.h"
#include "decoder.h"


unsigned int decoder (
	unsigned int pc, 
	unsigned int instruction, 
	unsigned int* memory, 
	unsigned int* memInit,
	unsigned char* input, 
	unsigned char* srOut, 
	unsigned long long breakCount, 
	int* flag, 
	unsigned int* reg, 
	unsigned int* fpreg,
	unsigned int* opNum, 
	unsigned int* labelRec, 
	FILE* soFile, 
	unsigned int opcode
) {
	unsigned int rt, rs, address, im, rs_original;
	unsigned int jump;
	static long srInCount = 0;
	static long srOutCount = 0;
	int diff;

	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	im = instruction & 0xFFFF;

	diff = signExt(im);
	pc = pc + 4;

	if(instruction != 0 && flag[HIDEIND] == 0) {
		printf("[ops: %06llu, pc: 0x%x, instruction: 0x%2X]\n\t[opcode:%2X]\n", breakCount, pc, instruction, opcode);
	}
	switch (opcode) {
		case(ADDIU) :
			im = signExt(im);
			rs_original = reg[rs];
			reg[rt] = (addiu(reg[rs], im) & 0xFFFFFFFF);	// 処理部
			if(flag[HIDEIND] == 0) { printf("\tADDIU :\t[$%2u: 0x%4X] + [im: 0x%8X] => [$%2u: 0x%4X]\n", rs, rs_original, im, rt, reg[rt]); }
			opNum[ADDIU]++;
			break;
		case(JUMP) :
//			flag[JUMPFLG] = 1;
			jump = instruction & 0x3FFFFFF;
			pc = jump*4 + PCINIT;	// jumpはpc形式
			labelRec[pc]++;		
			opNum[JUMP]++;
			if(flag[HIDEIND] == 0) { printf("\tJump :\t(jump_to) 0x%04x\n", pc); }
			break;
		case(BEQ) : 
			if(flag[HIDEIND] != 1) { printf("\tBEQ :\t?([$%2u 0x%2X] = [$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + diff*4); }
			if(reg[rs] == reg[rt]) {
				pc = pc + diff*4;		// pc形式
				labelRec[pc]++;
				if(flag[HIDEIND] != 1) { printf("\t\t<TRUE & JUMP> -> (jump_to) 0x%04x\n", pc); }
			} else if(flag[HIDEIND] != 1) {
				printf("\t<FALSE & NOP>\n");
			}
			opNum[BEQ]++;
			break;
		case(BNE) :		// bne I-Type: 000101 rs rt BranchAddr 	等しくないなら分岐 
			if(flag[HIDEIND] != 1) { printf("\tBNE :\t?([$%2u 0x%2X]!=[$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + diff*4); }
			if(reg[rs] != reg[rt]) {
				pc = pc + diff*4;		// pc形式
				labelRec[pc]++;
				if(flag[HIDEIND] != 1) { printf("\t\t<TRUE & JUMP> -> (jump_to) 0x%04x\n", pc); }
			} else if(flag[HIDEIND] != 1) {
				printf("\t<FALSE & NOP>\n");
			}
			opNum[BNE]++;
			break;
		case(LW) :	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード
			opNum[LW]++;

			im = signExt(im);
			address = (reg[rs] + im);
	
			// Memory mapped I/O対応
			if( (address & MMIO) == MMIO) {
				if(address == MMIOREADRDY) {
					if (flag[INPUTSIZE] > srInCount) {
						reg[rt] = 1;
					} else {
						pc = flag[MAXPC];
					}
					if(flag[HIDEIND] != 1) { printf("\tMMIOREAD_Ready :\t [$%2u] <- %u\n", rt, reg[rt]); }
				} else if (address == MMIOREAD) {
					reg[rt] = input[srInCount];
					srInCount++;
					if(flag[HIDEIND] != 1) { printf("\tMMIOREAD(%2ld) :\t [$%2u] <- 0x%X\n", srInCount, rt, reg[rt]); }
				} else if (address == 0xFFFF0008) {
					reg[rt] = 1;
					if(flag[HIDEIND] != 1) { printf("\tMMIOWRITE_Ready :\t [$%2u] <- %u\n", rt, reg[rt]); }
				} else {
					fprintf(stderr, "[ ERROR ]\tirregular code(0x%X).\n", address);
					exit(1);
				}
			} else {
				if(address > MEMORYSIZE && (address & MMIO) != MMIO) {
					fprintf(stderr, "[ ERROR ] Memory overflow\n");
					exit(1);
				}
				if (memInit[address] == 0) {
					fprintf(stderr, "[ ERROR ]\tInstruction(0x%X) accessed to uninitialized address: %08X@%llu\n", instruction, address, breakCount);
//					exit(1);
				}
				reg[rt] = lw(address, memory);
				if(flag[HIDEIND] != 1) { printf("\tLW :\tmemory[address:0x%04X]=0x%4X \n\t\treg[rt(%u)] = 0x%X\n", address, memory[address], rt, reg[rt]); }
			}
			break;
		case(LWC1) :
			// LWC1 ft, offset(base) ; ft->rt , base -> rs, offset -> im
			//	-> fpreg[rt] = memory[reg[base]+offset]
			/* 	vAddr ¬ sign_extend(offset) + GPR[base]
				if vAddr[1..0] != 0^2 then
					SignalException(AddressError)
				endif 
				(pAddr, CCA) <- AddressTranslation (vAddr, DATA, LOAD)
				memword <- LoadMemory(CCA, WORD, pAddr, vAddr, DATA)
				StoreFPR(ft, UNINTERPRETED_WORD, memword)
			*/
			address = signExt(im) + reg[rs];
			if (address > MEMORYSIZE) {
				fprintf(stderr, "[ ERROR ]\tMEMORY OVERFLOW (pc:0x%X, instruction:0x%X, address:0x%Xcount:%llu)", pc, instruction, address, breakCount);
				exit(1);
			}
			if (memInit[address] == 0) {
				fprintf(stderr, "[ ERROR ]\tInstruction(0x%X) accessed to uninitialized address: %08X@%llu\n", instruction, address, breakCount);
//				exit(1);
			}
			fpreg[rt] = lw(address, memory);
			break;
		case(SW) :
			opNum[SW]++;
			address = reg[rs] + signExt(im);

			if( (address & MMIO) == MMIO) {
				if((fwrite(&reg[rt], sizeof(unsigned char), 1, soFile)) == 0) { fprintf(stderr, "failed to fwrite at \n"); }
				if(srOutCount < FILESIZE) {
					srOut[srOutCount] = reg[rt] & 0xFF;
					flag[OUTPUTSIZE]++;
					srOutCount++;
				} else {
					fprintf(stderr, "[ ERROR ]\tserial port size is over %d", FILESIZE);
				}
				if(flag[HIDEIND] != 1) {
					printf("\t(im:0x%X)/(reg[%2u]:0x%X)/(reg[%2u]:0x%X)\n", im, rs, reg[rs], rt, reg[rt]);
					printf("\tMMIOWRITE:\twrite to serialport (%X).\n", reg[rt]);
				}
			} else {
				if(address > MEMORYSIZE && (address & MMIO) != MMIO) {
					fprintf(stderr, "[ ERROR ]\tMemory overflow\n");
					exit(1);
				}
				if (address > MEMORYSIZE) {
					fprintf(stderr, "[ ERROR ]\tMEMORY OVERFLOW (pc:0x%X, instruction:0x%X, address:0x%Xcount:%llu)", pc, instruction, address, breakCount);
					exit(1);
				}
				sw(reg[rt], address, memory);
				if(flag[HIDEIND] != 1) { printf("\tmemory[address: 0x%04X-0x%04X] : %02X %02X %02X %02X\n", address, address+3, memory[address+3], memory[address+2], memory[address+1], memory[address]); }
				memInit[address] = 1;
			}
			break;
		case(SWC1) :
			// SWC1 ft, offset(base)
			address = reg[rs] + signExt(im);
			if (address > MEMORYSIZE) {
				fprintf(stderr, "[ ERROR ]\tMEMORY OVERFLOW (pc:0x%X, instruction:0x%X, address:0x%Xcount:%llu)", pc, instruction, address, breakCount);
				exit(1);
			}
			sw(fpreg[rt], address, memory);
			if(flag[HIDEIND] != 1) { printf("\tmemory[address: 0x%04X-0x%04X] : %02X %02X %02X %02X\n", address, address+3, memory[address+3], memory[address+2], memory[address+1], memory[address]); }
			memInit[address] = 1;
			break;
		case(LUI) :
			reg[rt] = im << 16;
			if(flag[HIDEIND] != 1) { printf("\tLUI :\tReg[$%2u] <- [0x%4X (imm)]\n", rt, im); }
			opNum[LUI]++;
			break;
		case(JAL) :
			jump = instruction & 0x3FFFFFF;
			reg[31] = pc;
			pc = jump*4 + PCINIT;
			opNum[JAL]++;
			labelRec[pc]++;
//			printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
			break;
		case(ORI) :
			opNum[ORI]++;
			rs_original = reg[rs];
			reg[rt] = ori(reg[rs], im);
			if(flag[HIDEIND] != 1) { printf("\tORI: [reg($%02u):0x%4X] OR? [imm:0x%4X] => [reg($%02u)] 0x%2X\n", rs, rs_original, im, rt, reg[rt]); }
			break;
		case(ANDI) :
			opNum[ANDI]++;
			rs_original = reg[rs];
			reg[rt] = reg[rs] & im;
			if(flag[HIDEIND] != 1) { printf("\tANDI: [$%2u:0x%4X] ANDi? [im:0x%4X] => [$%2u] 0x%4X\n", rs, rs_original, im, rt, reg[rt]); }
			break;
		default :
			fprintf(stderr, "[ ERROR ]\tUnknown switch has selected.(opcode: %X / pc: %X)\n", opcode, pc);
			flag[UNKNOWNOP]++;
	}
	return pc;
}

