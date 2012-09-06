CC = gcc
CFLAGS = -c -I./src -O0 -g
LL = gcc
LFLAGS = 

OBJDIR = obj
TOBJDIR = tobj
BUILDDIR = build

#MACHINE ?= acc

all: init

init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BUILDDIR)

tinit: init
	mkdir -p $(TOBJDIR)

test: tinit memtest asmtest

memtest: tinit $(OBJDIR)/mem.o
	$(CC) $(CFLAGS) -o $(TOBJDIR)/memtest.o tests/memtest.c
	$(LL) $(LFLAGS) -o $(BUILDDIR)/memtest $(OBJDIR)/mem.o $(TOBJDIR)/memtest.o

asmtest: tinit $(OBJDIR)/asm.o $(OBJDIR)/asm_impl.o $(OBJDIR)/list.o
	$(CC) $(CFLAGS) -o $(TOBJDIR)/asmtest.o tests/asmtest.c
	$(LL) $(LFLAGS) -o $(BUILDDIR)/asmtest $(OBJDIR)/asm.o $(OBJDIR)/asm_impl.o $(OBJDIR)/list.o $(TOBJDIR)/asmtest.o

clean:
	rm -rf $(OBJDIR) $(TOBJDIR) $(BUILDDIR)

$(OBJDIR)/mem.o: src/mem.h src/mem.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/mem.o src/mem.c

$(OBJDIR)/asm.o: src/asm.h src/asm_impl.h src/asm.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/asm.o src/asm.c

$(OBJDIR)/asm_impl.o: src/asm.h src/asm_impl.h src/$(MACHINE)/asm_impl.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/asm_impl.o src/$(MACHINE)/asm_impl.c

$(OBJDIR)/list.o: src/list.h src/list.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/list.o src/list.c
