#include "PortIO.h"

using namespace IO;

void PortIO::outb(unsigned short port, unsigned char b)
{
	/*
	asm ( assembler template 
			: output operands                  // optional 
			: input operands                   // optional 
			: list of clobbered registers      // optional 
			);
	In the constraint, 'a' refers to EAX, 'b' to EBX, 'c' to ECX, 'd' to EDX, 'S' to ESI, and 'D' to EDI 
	(read the GCC manual for a full list), 
	assuming that you are coding for the IA32 architecture. 
	An equation sign indicates that your assembly code does not care 
	about the initial value of the mapped variable (which allows some optimization).
	*/
	asm ("outb %0, %1"
				: 
				: "a"(b), "Nd"(port)
				:
				);
}

void PortIO::outw(unsigned short port, unsigned short w)
{
	asm ("outw %0, %1;"
			: 
			: "a"(w), "Nd"(port)
			: 
			);
}

unsigned char PortIO::inb(unsigned short port)
{
	unsigned char ret;
    asm volatile( "inb %1, %0"
                  : "=a"(ret) 
				  : "Nd"(port) 
				  :);
    return ret;
}

unsigned short PortIO::inw(unsigned short port)
{
	unsigned short ret;
	asm ("inw %1, %0;"
			: "=a"(ret)
			: "Nd"(port)
			: 
			);
	return ret;
}

void PortIO::io_wait(void)
{
	asm volatile("jmp 1f\n\t"
                  "1:jmp 2f\n\t"
                  "2:");
}
