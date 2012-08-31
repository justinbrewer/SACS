CC = gcc
CFLAGS = -c -I./src -O0
LL = gcc
LFLAGS = 

OBJDIR = build
RESULT = sacs

clean:
	rm -rf $(OBJDIR) $(RESULT)
