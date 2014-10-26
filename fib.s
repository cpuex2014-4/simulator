	j		main					
fib.10:
	bge		bge_else.24			
	addiu	$v1, $v0, -1					
	sw		0($v0), $sp					
	addu	$v0, $v1					
	addiu	$sp, 8					
	j		fib.10					
	addiu	$sp, -8					
	lw		$v1, 0($sp)					
	addiu	$v1, $v1, -2					
	sw		4($v0), $sp
	addu	$v0, $v1
	addiu	$sp, 8
	j		fib.10
	addiu	$sp, -8
	lw		$v1, 4($sp)
	addu	$v0, $v1, $v0
	jr		$ra
bge_else.24:
	addiu	$v0, $zero, 1
	jr		$ra
main:
	addiu	$v0, $zero, 10
	j		fib.10

