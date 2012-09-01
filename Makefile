CC = gcc
CFLAGS = -c -I./src -O0
LL = gcc
LFLAGS = 

OBJDIR = obj
TOBJDIR = tobj
BUILDDIR = build

all: init

init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BUILDDIR)

tinit: init
	mkdir -p $(TOBJDIR)

test: tinit memtest

memtest: tinit $(OBJDIR)/mem.o
	$(CC) $(CFLAGS) -o $(TOBJDIR)/memtest.o tests/memtest.c
	$(LL) $(LFLAGS) -o $(BUILDDIR)/memtest $(OBJDIR)/mem.o $(TOBJDIR)/memtest.o

clean:
	rm -rf $(OBJDIR) $(TOBJDIR) $(BUILDDIR)

$(OBJDIR)/mem.o: src/mem.h src/mem.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/mem.o src/mem.c
