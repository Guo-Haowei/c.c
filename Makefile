# $@ = target file
# $< = first dependency
# $^ = all dependencies

C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)
OBJ = ${C_SOURCES:.c=.o} 

CFLAGS = -g -m32 -Wall -Wextra -Werror

# First rule is the one executed when no parameters are fed to the Makefile
# all: run

# Notice how dependencies are built as needed
kernel.bin: kernel_entry.o ${OBJ}
	i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

kernel_entry.o: kernel/kernel_entry.asm
	nasm $< -f elf -o $@

%.o: %.c ${HEADERS}
	i686-elf-gcc -I. ${CFLAGS} -ffreestanding -c $< -o $@
	
bootsect.bin: boot/bootsect.asm
	nasm $< -f bin -o $@

os-image.bin: bootsect.bin kernel.bin
	cat $^ > $@

# run: os-image.bin
# 	qemu-system-i386 -fda $<

clean:
	rm -f *.bin *.o kernel/*.o drivers/*.o
