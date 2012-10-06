CC = gcc
CFLAGS = -c -I./src -O0 -g
LL = gcc
LFLAGS = 

OBJDIR = obj
BUILDDIR = build

SACSOBJ = $(OBJDIR)/main.o $(OBJDIR)/mem.o $(OBJDIR)/asm.o $(OBJDIR)/asm_impl.o $(OBJDIR)/list.o $(OBJDIR)/exec_impl.o

MACHINE ?= gpr

all: init sacs
	mv $(BUILDDIR)/sacs $(MACHINE)Sim
init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BUILDDIR)

sacs: $(SACSOBJ)
	$(LL) $(LFLAGS) -o $(BUILDDIR)/sacs $(SACSOBJ)

clean:
	rm -rf $(OBJDIR) $(BUILDDIR) gprSim

$(OBJDIR)/main.o: src/asm.h src/mem.h src/exec.h src/main.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/main.o src/main.c

$(OBJDIR)/mem.o: src/mem.h src/mem.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/mem.o src/mem.c

$(OBJDIR)/asm.o: src/asm.h src/asm_impl.h src/asm.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/asm.o src/asm.c

$(OBJDIR)/asm_impl.o: src/asm.h src/asm_impl.h src/$(MACHINE)/asm_impl.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/asm_impl.o src/$(MACHINE)/asm_impl.c

$(OBJDIR)/exec_impl.o: src/mem.h src/exec.h src/$(MACHINE)/instr.h src/$(MACHINE)/exec_impl.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/exec_impl.o src/$(MACHINE)/exec_impl.c

$(OBJDIR)/list.o: src/list.h src/list.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/list.o src/list.c
