	.text

main:
	li $t0, 100
	la $t1, zero
	la $t2, xout
	la $t3, yout
	
	l.d $f0, 0($t1)		#zero constant
	
	l.d $f1, 4($t1)		#kx
	l.d $f2, 8($t1)		#ky
	
	l.d $f3, 0($t1)		#X0
	l.d $f4, 0($t1)		#X1
	l.d $f5, 12($t1)	#X2
	
	l.d $f6, 0($t1)		#Y0
	l.d $f7, 0($t1)		#Y1
	l.d $f8, 16($t1)	#Y2

loop:	subi $t0, $t0, 1
	beqz $t0, exit
	nop
	nop

	fmul $f3, $f1, $f4
	fsub $f3, $f3, $f5

	fmul $f6, $f2, $f7
	fsub $f6, $f6, $f8

	fadd $f5, $f4, $f0
	fadd $f4, $f3, $f0

	fadd $f8, $f7, $f0
	fadd $f7, $f6, $f0

	s.d $f3, 0($t2)
	s.d $f6, 0($t3)

	b loop
	nop
	nop
	
exit:	li $v0, 10
	syscall
	
	.data

zero:	.float	0
kx:	.float	1.65
ky:	.float	0.8
x2:	.float	-0.57
y2:	.float	-0.92

xout:	.float	0
yout:	.float	0
