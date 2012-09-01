#Example Accumulator Program

	.text

main:	
	load x
	mul x
	mul a
	stor a
	load x
	mul b
	add a
	add c
	stor y
	end

	.data
x:	.word 3
a:	.word 7
b:	.word 5
c:	.word 4
y:	.word 0
