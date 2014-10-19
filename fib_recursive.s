fib:
	addiu	$sp, $sp, -12
	sw	$ra, 8($sp)
	sw	$a0, 4($sp)
	addu	$t1, $zero, $zero
	slt	$t0, $zero, $a0
	beq	$t0, $zero, exit
	addiu	$a0, $a0, -1
	jal	fib
	sw	$v0, 0($sp)
	lw	$a0, 4($sp)
	addiu	$a0, $a0, -2
	jal	fib
	lw	$t1, 0($sp)
	addu	$t1, $t1, $v0
	lw	$ra, 8($sp)
exit:
	addiu	$sp, $sp, 12
	addu	$v0, $t1, $zero
	jr	$ra
