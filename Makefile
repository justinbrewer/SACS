CC = gcc
CFLAGS = -c -I./src -O0 -g
LL = gcc
LFLAGS = 

OBJDIR = obj
BUILDDIR = build

SACSOBJ = $(OBJDIR)/main.o $(OBJDIR)/mem.o $(OBJDIR)/asm.o $(OBJDIR)/list.o $(OBJDIR)/exec.o

all: init sacs

init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BUILDDIR)

sacs: $(SACSOBJ)
	$(LL) $(LFLAGS) -o $(BUILDDIR)/sacs $(SACSOBJ)

clean:
	rm -rf $(OBJDIR) $(BUILDDIR)

$(OBJDIR)/main.o: src/asm.h src/mem.h src/exec.h src/main.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/main.o src/main.c

$(OBJDIR)/mem.o: src/mem.h src/mem.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/mem.o src/mem.c

$(OBJDIR)/asm.o: src/asm.h src/asm.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/asm.o src/asm.c

$(OBJDIR)/exec.o: src/mem.h src/exec.h src/instr.h src/exec.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/exec.o src/exec.c

$(OBJDIR)/list.o: src/list.h src/list.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/list.o src/list.c
