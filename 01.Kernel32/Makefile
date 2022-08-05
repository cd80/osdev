NASM = nasm
CC = clang -c -m32 -ffreestanding -nostdlib
LD = ld -melf_i386 -T ../elf_i386.x -nostdlib -e main -Ttext 0x10200
OBJCOPY = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJECTDIR = Temp
SOURCEDIR = Source

all: prepare Kernel32.bin

prepare:
	mkdir -p $(OBJECTDIR)

$(OBJECTDIR)/EntryPoint.bin: $(SOURCEDIR)/EntryPoint.s
	$(NASM) -o $@ $<

dep:
	@echo [*] Make .dep
	make -C $(OBJECTDIR) -f ../Makefile InternalDependency
	@echo [*] Make .dep Complete

ExecuteInternalBuild: dep
	make -C $(OBJECTDIR) -f ../Makefile Kernel32.elf

$(OBJECTDIR)/Kernel32.elf.bin: ExecuteInternalBuild
	$(OBJCOPY) $(OBJECTDIR)/Kernel32.elf $@

Kernel32.bin: $(OBJECTDIR)/EntryPoint.bin $(OBJECTDIR)/Kernel32.elf.bin
	cat $^ > $@

clean:
	rm -f *.bin
	rm -f $(OBJECTDIR)/*


## for InternalDependency
ENTRYOBJ = main.o
CFILES = $(wildcard ../$(SOURCEDIR)/*.c)
ASMFILES = $(wildcard ../$(SOURCEDIR)/*.asm)
COBJS = $(subst main.o, , $(notdir $(patsubst %.c,%.o,$(CFILES))))
ASMOBJS = $(notdir $(patsubst %.asm,%.o,$(ASMFILES)))

%.o: ../$(SOURCEDIR)/%.c
	$(CC) -c $<

%.o: ../$(SOURCEDIR)/%.asm
	$(NASM) -f elf32 -o $@ $<

InternalDependency:
	$(CC) -MM $(CFILES) > Dependency.dep

Kernel32.elf: $(ENTRYOBJ) $(COBJS) $(ASMOBJS)
	$(LD) -o $@ $^

ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif
