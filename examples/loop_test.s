
	.text

main:	li $1, 0
	li $2, 32

loop:	subi $2, $2, 1
	bge  $2, $1, loop

	li $2, 10
	syscall
	
