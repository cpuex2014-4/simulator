001001 00000 01000 0000000000000000		24080000	ADDIU	r8 <- r0 + 0
001001 00000 01001 0000000000000001		24090001	ADDIU	r9 <- r0 + 1
001001 00000 01010 0000000000000000		240a0000	ADDIU	r10 <- r0 + 0	*
001001 00000 01011 0000000000000000		240b0000	ADDIU	r11 <- r0 + 0
000000 01011 00100 01100 00000 101010	0164602a	SLT		?(r11 < r4)
000100 00000 01100 0000000000000000		100c0000	BEQ		if r12 == r0 -> pc = 0
001001 01001 01010 0000000000000000		252a0000	ADDIU	r10 <- r9 + 0
000000 01000 01001 01001 00000 100001	01094821	ADDU	r9 <- r8 + r9	
001001 01010 01000 0000000000000000		25480000	ADDIU	r8 <- r9 + 0
001001 01011 01011 0000000000000001		256b0001	ADDIU	r9 <- r9 + 1
000010 00000 00000 0000000000000100		08000004	BEQ		if r0 == r0 -> pc = 0x4000008 (-> *)
001001 01010 00010 0000000000000000		25420000	ADDIU	r2 <- r10 + 0

