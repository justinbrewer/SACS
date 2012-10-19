## Daniel J. Ellard -- 02/21/94
## palindrome.asm -- read a line of text and test if it is a palindrome.
##
## A palindrome is a string that is exactly the same when read backwards
## as when it is read forward (e.g. anna).
##
## Whitespace is considered significant in this version, so "ahahaha"
## is a palindrome while "aha haha" is not.
##
## Register usage:
## $t1 - A - initially, the address of the first character in the string S.
## $t2 - B - initially, the address of the the last character in S.
## $t3 - the character at address A.
## $t4 - the character at address B.
## $v0 - syscall parameter / return values.
## $a0 - syscall parameters.
## $a1 - syscall parameters.

	.text
main:
	
## read the string S:
	la $a0, string_space
	nop
	nop
	nop
	nop
	
	li $a1, 1024
	nop
	nop
	nop
	nop
	
	li $v0, 8		# load "read_string" code into $v0.
	nop
	nop
	nop
	nop
	
	syscall
	nop
	nop
	nop
	nop

	la $t1, string_space	# A = S.
	nop
	nop
	nop
	nop
	
	la $t2, string_space	# we need to move B to the end
	nop
	nop
	nop
	nop
	
length_loop:			# length of the string
	lb $t3, ($t2)		# load the byte at addr B into $t3.
	nop
	nop
	nop
	nop
	
	beqz $t3, end_length_loop # if $t3 == 0, branch out of loop.
	nop
	nop
	nop
	nop

	addi $t2, $t2, 1	# otherwise, increment B,
	nop
	nop
	nop
	nop
	
	b length_loop		# and repeat the loop.
	nop
	nop
	nop
	nop

end_length_loop:
	subi $t2, $t2, 2	# subtract 2 to move B back past
	nop
	nop
	nop
	nop

# the '\0' and '\n'.

test_loop:
	bge $t1, $t2, is_palin	# if A >= B, it's a palindrome.
	nop
	nop
	nop
	nop

	lb $t3, ($t1)		# load the byte at addr A into $t3,
	nop
	nop
	nop
	nop
	
	lb $t4, ($t2)		# load the byte at addr B into $t4.
	nop
	nop
	nop
	nop
	
	bne $t3, $t4, not_palin # if $t3 != $t4, not a palindrome.
	nop
	nop
	nop
	nop

# Otherwise,

	addi $t1, $t1, 1	# increment A,
	nop
	nop
	nop
	nop
	
	subi $t2, $t2, 1	# decrement B,
	nop
	nop
	nop
	nop
	
	b test_loop		# and repeat the loop.
	nop
	nop
	nop
	nop

is_palin:			# print the is_palin_msg, and exit.

	la $a0, is_palin_msg
	nop
	nop
	nop
	nop
	
	li $v0, 4
	nop
	nop
	nop
	nop
	
	syscall
	nop
	nop
	nop
	nop
	
	b exit
	nop
	nop
	nop
	nop

not_palin:
	la $a0, not_palin_msg	## print the not_palin_msg, and exit.
	nop
	nop
	nop
	nop
	
	li $v0, 4
	nop
	nop
	nop
	nop
	
	syscall
	nop
	nop
	nop
	nop
	
	b exit
	nop
	nop
	nop
	nop

exit:				# exit the program
	li $v0, 10		# load "exit" code into $v0.
	nop
	nop
	nop
	nop
	
	syscall			# make the system call.
	nop
	nop
	nop
	nop

	.data
is_palin_msg: .asciiz "The string is a palindrome.\n"
not_palin_msg: .asciiz "The string is not a palindrome.\n"
string_space: .space 1024	# reserve 1024 bytes for the string.
