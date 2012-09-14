#Example Stack Program

	.text

main:
	push c
	push b
	push x
	mul
	push a
	push x
	push x
	mul
	mul
	add
	add
	pop y
	prnt y
	end

	.data
x:	.word 3
a:	.word 7
b:	.word 5
c:	.word 4
y:	.word 0
