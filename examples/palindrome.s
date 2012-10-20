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
	li $a1, 1024
	li $v0, 8		# load "read_string" code into $v0.
	syscall

	la $t1, string_space	# A = S.
	la $t2, string_space	# we need to move B to the end

length_loop:			# length of the string
	lb $t3, ($t2)		# load the byte at addr B into $t3.
	beqz $t3, end_length_loop # if $t3 == 0, branch out of loop.

	addi $t2, $t2, 1	# otherwise, increment B,
	b length_loop		# and repeat the loop.

end_length_loop:
	subi $t2, $t2, 2	# subtract 2 to move B back past

# the '\0' and '\n'.

test_loop:
	bge $t1, $t2, is_palin	# if A >= B, it's a palindrome.

	lb $t3, ($t1)		# load the byte at addr A into $t3,
	lb $t4, ($t2)		# load the byte at addr B into $t4.
	bne $t3, $t4, not_palin # if $t3 != $t4, not a palindrome.

# Otherwise,

	addi $t1, $t1, 1	# increment A,
	subi $t2, $t2, 1	# decrement B,
	b test_loop		# and repeat the loop.

is_palin:			# print the is_palin_msg, and exit.

	la $a0, is_palin_msg
	li $v0, 4
	syscall
	b exit

not_palin:
	la $a0, not_palin_msg	## print the not_palin_msg, and exit.
	li $v0, 4
	syscall
	b exit

exit:				# exit the program
	li $v0, 10		# load "exit" code into $v0.
	syscall			# make the system call.


	.data
is_palin_msg: .asciiz "The string is a palindrome.\n"
not_palin_msg: .asciiz "The string is not a palindrome.\n"
string_space: .space 1024	# reserve 1024 bytes for the string.
