fib:
	addiu	$t0, $zero, 0	s=0
	addiu	$t1, $zero, 1	a=1
	addiu	$t2, $zero, 0	b=0
	addiu	$t3, $zero, 0	i=0
loop:	iがt4以下の間ループする
	slt	$t4, $t3, $a0	(if i<t4 then t=1)
	beq	$t4, $zero, exit	(if t=0 then goto line(15))
	addiu	$t2, $t1, 0		a = b+0
	addu	$t1, $t0, $t1	a = s+a
	addiu	$t0, $t2, 0		b = s+0
	addiu	$t3, $t3, 1		i++
	j	loop				goto "loop"
exit:
	addiu	$v0, $t2, 0		return b


















