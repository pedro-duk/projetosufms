# Aliases
CC = gcc
LD = ld

# Localização do Kernel
KERNEL_ADDR	= 0x1000

# Flags do compilador
CCOPTS = -Wall -g -m32 -c -fomit-frame-pointer -O2 -fno-builtin

# Flags do linker
LDOPTS = -nostartfiles -nostdlib -melf_i386

# Alvos do Makefile
all: bootblock buildimage kernel image

kernel: kernel.o
	$(LD) $(LDOPTS) -Ttext $(KERNEL_ADDR) -o kernel $<

bootblock: bootblock.o
	$(LD) $(LDOPTS) -Ttext 0x0 -o bootblock $<

buildimage: buildimage.o
	$(CC) -o buildimage $<

# Constrói imagem (para floppy)
image: bootblock buildimage kernel
	./buildimage --extended ./bootblock ./kernel

# Coloca imagem no stick USB
boot: image
	dd if=./image of=/dev/sdb bs=512

# Deleta arquivos gerados
clean:
	rm -f buildimage.o kernel.o
	rm -f buildimage image bootblock kernel

# Compilar buildimage
buildimage.o:
	$(CC) -c -o buildimage.o buildimage.c

# Compilar arquivos em C
%.o:%.c
	$(CC) $(CCOPTS) $<

# Assembling
%.o:%.s
	$(CC) $(CCOPTS) $<

# Produzindo input do assembler
%.s:%.c
	$(CC) $(CCOPTS) -S $<
