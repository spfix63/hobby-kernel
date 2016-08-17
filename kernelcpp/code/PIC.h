#ifndef PIC_H
#define PIC_H

/*
 * IRQ 
 */
#define ALL       0xFF
#define TIMER     0
#define KEYBOARD  1
#define CASCADE   2
#define COM2_4    3
#define COM1_3    4
#define LPT       5
#define FLOPPY    6
#define FREE7     7
#define CLOCK     8
#define FREE9     9
#define FREE10    10
#define FREE11    11
#define PS2MOUSE  12
#define COPROC    13
#define IDE_1     14
#define IDE_2     15

namespace PIC
{
	void sendEOI(unsigned char irq);
	void remap(int offset1, int offset2);
}

#endif
