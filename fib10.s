1	j	main
000010 10101
fib.10:			(2:		10)
2	bge	bge_else.24

3	addiu	$v1, $v0, -1
001001 00011 00010 1111111111111111
4	sw	0($v0), $sp

5	addu	$v0, $v1

6	addiu	$sp, 8

7	j	fib.10

8	addiu	$sp, -8

9	lw	$v1, 0($sp)

10	addiu	$v1, $v1, -2

11	sw	4($v0), $sp

12	addu	$v0, $v1

13	addiu	$sp, 8

14	j	fib.10

15	addiu	$sp, -8

16	lw	$v1, 4($sp)

17	addu	$v0, $v1, $v0

18	jr	$ra


bge_else.24:	(19:	10011)
19	addiu	$v0, $zero, 1

20	jr	$ra

main:			(21:	10101)
21	addiu	$v0, $zero, 10
001001 00010 00000 000000000001010
22	j	fib.10
000010 000010



