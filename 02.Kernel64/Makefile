NASM = nasm -f elf64
CC = clang -c -m64 -ffreestanding -msoft-float
LD = ld -melf_x86_64 -T ../elf_x86_64.x -nostdlib -e main -Ttext 0x200000
OBJCOPY = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJECTDIR = Temp
SOURCEDIR = Source

all: prepare Kernel64.bin

prepare:
	mkdir -p $(OBJECTDIR)

dep:
	@echo [*] Make Dependency file
	make -C $(OBJECTDIR) -f ../Makefile InternalDependency
	@echo [*] Make Dependency file complete

ExecuteInternalBuild: dep
	make -C $(OBJECTDIR) -f ../Makefile Kernel64.elf

Kernel64.bin: ExecuteInternalBuild
	$(OBJCOPY) $(OBJECTDIR)/Kernel64.elf $@

clean:
	rm -f *.bin
	rm -f $(OBJECTDIR)/*.*


ENTRYSRC = ../$(SOURCEDIR)/EntryPoint.s
ENTRYOBJ = EntryPoint.o

CFILES = $(wildcard ../$(SOURCEDIR)/*.c)
ASMFILES = $(wildcard ../$(SOURCEDIR)/*.asm)
COBJS = $(notdir $(patsubst %.c,%.o,$(CFILES)))
ASMOBJS = $(notdir $(patsubst %.asm,%.o,$(ASMFILES)))

$(ENTRYOBJ): $(ENTRYSRC)
	$(NASM) -o $@ $<

%.o: ../$(SOURCEDIR)/%.c
	$(CC) -c $<

%.o: ../$(SOURCEDIR)/%.asm
	$(NASM) -o $@ $<

InternalDependency:
	$(CC) -MM $(CFILES) > Dependency.dep

Kernel64.elf: $(ENTRYOBJ) $(COBJS) $(ASMOBJS)
	$(LD) -o $@ $^

ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif