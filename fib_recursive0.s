fib(0):	addiu	$sp, $sp, -12		スタックポインタを12減らす
		sw	$ra, 8($sp)				メモリの(スタックポインタ+8)の位置に呼び出し元関数の戻りアドレスを保存する
		sw	$a0, 4($sp)				メモリの(スタックポインタ+4)の位置に$a0の内容を保存する
		addu	$t1, $zero, $zero	$t1に0を代入する
		slt	$t0, $zero, $a0			$a0と0を比較し、$a0の方が小さければ$t0に1を代入
		beq	$t0, $zero, exit		$t0が0ならEXITに飛ぶ
		addiu	$a0, $a0, -1		$a0=$a0-1
		jal	fib						fib(0)を呼ぶ
		sw	$v0, 0($sp)				メモリの(スタックポインタ)の位置に$v0の内容を保存する
		lw	$a0, 4($sp)				メモリの(スタックポインタ+4)の位置から読み出し、内容を$a0に格納する
		addiu	$a0, $a0, -2		$a0=$a0-2
		jal	fib						fib(0)を呼ぶ
		lw	$t1, 0($sp)				メモリの(スタックポインタ)の位置から読み出し、内容を$t1に格納する
		addu	$t1, $t1, $v0		$t1=$t1+0
		lw	$ra, 8($sp)				メモリの(スタックポインタ+8)の位置から読み出し、内容を$raに格納する
exit(15): addiu	$sp, $sp, 12		スタックポインタを12増やす
		addu	$v0, $t1, $zero		$v0=$t1+0
		jr	$ra						$raのアドレスに飛ぶ
<>
$ra	戻りアドレス
$sp	スタックポインタ


<>
addiu	001001 rs rt imm
sw		101011 base rt offset(16bit)
addu	000000 rs rt rd 00000 100001
slt		000000 rs rt rd 00000 101010
beq		000100 rs rt 0000000000000000
// beq I-Type: 000100 rs rt BranchAddr 	等しいなら分岐 
jal		000011 target
lw		100011 base rt offset(16bit)
jr		000000 rs 000000000000000 001000

12	0000000000001100
-12	1111111111110100
