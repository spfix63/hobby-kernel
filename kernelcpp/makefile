CC	=i586-pc-paulyteos-g++
CFLAGS	= -Wall -Wextra -Werror \
    -nostdlib -fno-builtin -nostartfiles -nodefaultlibs \
    -fno-exceptions -fno-rtti -fno-stack-protector -std=c++11
LD	=i586-pc-paulyteos-ld
CODEDIR=./code 
all:kernel.img
 
loader.o:loader.S GDT.inc
	nasm -f elf -o loader.o loader.s
	
./code/Interrupts.o:./code/Interrupts.s
	nasm -f elf -o ./code/Interrupts.o ./code/Interrupts.s
	
./code/thread_asm.o:./code/thread_asm.s
	nasm -f elf -o ./code/thread_asm.o ./code/thread_asm.s
	
./code/IDT.o:./code/IDT.cpp ./code/IDT.h ./code/Console.h ./code/PortIO.h ./code/ISR.h ./code/TaskManager.h
	$(CC) $(CFLAGS) -o ./code/IDT.o -c ./code/IDT.cpp	
			
./code/mem/vmm.o:./code/mem/vmm.c ./code/mem/vmm.h ./code/mem/common.h 
	$(CC) $(CFLAGS) -o ./code/mem/vmm.o -c ./code/mem/vmm.c	
	
./code/mem/pmm.o:./code/mem/pmm.h ./code/mem/pmm.c
	$(CC) $(CFLAGS) -o ./code/mem/pmm.o -c ./code/mem/pmm.c	
	
./code/mem/heap.o:./code/mem/heap.c ./code/mem/heap.h ./code/mem/common.h ./code/mem/vmm.h ./code/mem/pmm.h 
	$(CC) $(CFLAGS) -o ./code/mem/heap.o -c ./code/mem/heap.c
	
./code/mem/common.o:./code/mem/common.c ./code/mem/common.h
	$(CC) $(CFLAGS) -o ./code/mem/common.o -c ./code/mem/common.c
	
./code/IPC.o:./code/IPC.cpp ./code/IPC.h ./code/TaskManager.h ./code/MemoryManagement.h ./code/Console.h
	$(CC) $(CFLAGS) -o ./code/IPC.o -c ./code/IPC.cpp	
	
./code/MemoryManagement.o:./code/MemoryManagement.cpp ./code/MemoryManagement.h ./code/Console.h
	$(CC) $(CFLAGS) -o ./code/MemoryManagement.o -c ./code/MemoryManagement.cpp	
	
./code/KeyEvent.o:./code/KeyEvent.cpp ./code/KeyEvent.h
	$(CC) $(CFLAGS) -o ./code/KeyEvent.o -c ./code/KeyEvent.cpp
	
./code/Keyboard.o:./code/Keyboard.cpp ./code/Keyboard.h
	$(CC) $(CFLAGS) -o ./code/Keyboard.o -c ./code/Keyboard.cpp
	
./code/PIC.o:./code/PIC.cpp ./code/PIC.h ./code/PortIO.h ./code/Keyboard.h ./code/KeyEvent.h
	$(CC) $(CFLAGS) -o ./code/PIC.o -c ./code/PIC.cpp
	
./code/PortIO.o:./code/PortIO.cpp ./code/PortIO.h
	$(CC) $(CFLAGS) -o ./code/PortIO.o -c ./code/PortIO.cpp	
	
./code/Console.o:./code/Console.cpp ./code/Console.h ./code/colors.h
	$(CC) $(CFLAGS) -o ./code/Console.o -c ./code/Console.cpp	
	
./code/TSS.o:./code/TSS.cpp
	$(CC) $(CFLAGS) -o ./code/TSS.o -c ./code/TSS.cpp	
	
./code/TaskManager.o:./code/TaskManager.cpp ./code/TaskManager.h ./code/IPC.h ./code/thread_asm.o loader.o ./code/mem/heap.h
	$(CC) $(CFLAGS) -o ./code/TaskManager.o -c ./code/TaskManager.cpp	
	
kernel.o:loader.o kmain.cpp ./code/Interrupts.o ./code/TSS.cpp ./code/Console.h ./code/PIC.h ./code/Keyboard.h ./code/IDT.h ./code/MemoryManagement.h ./code/IPC.h
	$(CC) $(CFLAGS) -o kmain.o -c kmain.cpp
 
kernel.bin:kernel.o ./code/IPC.o ./code/TSS.o ./code/TaskManager.o  ./code/Console.o ./code/PortIO.o ./code/PIC.o ./code/Keyboard.o ./code/KeyEvent.o ./code/MemoryManagement.o ./code/IDT.o ./code/interrupts.o ./code/mem/common.o ./code/mem/heap.o ./code/mem/pmm.o ./code/mem/vmm.o
	$(LD) -T linker.ld -o kernel.bin loader.o kmain.o ./code/IPC.o ./code/TSS.o ./code/TaskManager.o  ./code/IDT.o ./code/Interrupts.o ./code/thread_asm.o ./code/Console.o ./code/PortIO.o ./code/PIC.o ./code/Keyboard.o ./code/KeyEvent.o ./code/MemoryManagement.o ./code/mem/common.o ./code/mem/heap.o ./code/mem/pmm.o ./code/mem/vmm.o

kernel.img:kernel.bin
	dd if=/dev/zero of=pad bs=1 count=750
	cat stage1 stage2 pad kernel.bin > kernel.img
 
clean:
	rm -f kmain.o loader.o ./code/Interrupts.o ./code/thread_asm.o ./code/IPC.o ./code/TSS.o ./code/TaskManager.o ./code/IDT.o ./code/MemoryManagement.o ./code/KeyEvent.o ./code/Keyboard.o ./code/PIC.o ./code/PortIO.o ./code/Console.o ./code/mem/common.o ./code/mem/heap.o ./code/mem/pmm.o ./code/mem/vmm.o kernel.o kernel.bin kernel.img
 
install:
	rm -f loader.o ./code/Interrupts.o ./code/thread_asm.o ./code/IPC.o ./code/TSS.o ./code/TaskManager.o ./code/mem/process.o ./code/IDT.o ./code/MemoryManagement.o  ./code/KeyEvent.o ./code/Keyboard.o ./code/PIC.o ./code/PortIO.o ./code/Console.o ./code/mem/common.o ./code/mem/heap.o ./code/mem/pmm.o ./code/mem/vmm.o kernel.o kernel.bin 
