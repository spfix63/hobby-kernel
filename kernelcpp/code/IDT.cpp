
#include "Console.h"
#include "IDT.h"
#include "mem/vmm.h"
extern "C" 
{
#include "ISR.h"
}
#include "Keyboard.h"
#include "PortIO.h"
#include "TaskManager.h"


/*
 * Interrupt types
 */
#define INT_0 0x8E00     // 1000111000000000 = present,ring0,int_gate
#define INT_3 0xEE00     // 1110111000000000 = present,ring3,int_gate

/* structure for an interrupt */
typedef struct
{
	uint16_t low_offset;                         // low nibble of offset to handler of interrupt 
	uint16_t selector;                           // GDT selector used 
	uint16_t settings;                           // settings for int 
	uint16_t high_offset;                        // high nibble to handler code 
}  __attribute__ ((packed)) x86_interrupt;

/* structure for the IDTR */
typedef struct
{
	uint16_t limit;				// size 
	x86_interrupt *base;     	// a pointer to the base of the IDT 
} __attribute__ ((packed)) idtr;

static void setHandler(int intID, void (*handler)(), int ring);

static x86_interrupt a_IDT[256];
static void (*a_Handlers[256])(void);
static idtr IDTR;
registers_t IDT::s_currentRegisters;

void IDT::load()
{
	IDTR.limit = 256*(sizeof(x86_interrupt)-1);
    IDTR.base = a_IDT;
	idtr *IDTRptr = &IDTR;
	asm volatile("LIDT (%0) ": :"p" (IDTRptr));
}
void init_timer(uint32_t frequency);
void IDT::setupHandlers()
{
	for (int i = 0; i < 256; i++)
		a_Handlers[i] = 0;
	setHandler(0, isr0, 0);
    setHandler(1, isr1, 0);
    setHandler(2, isr2, 0);
    setHandler(3, isr3, 0);
    setHandler(4, isr4, 0);
    setHandler(5, isr5, 0);
    setHandler(6, isr6, 0);
    setHandler(7, isr7, 0);
    setHandler(8, isr8, 0);
    setHandler(9, isr9, 0);
    setHandler(10, isr10, 0);
    setHandler(11, isr11, 0);
    setHandler(12, isr12, 0);
    setHandler(13, isr13, 0);
    setHandler(14, isr14, 0);
	a_Handlers[14] = page_fault;
    setHandler(15, isr15, 0);
    setHandler(16, isr16, 0);
    setHandler(17, isr17, 0);
    setHandler(18, isr18, 0);
    setHandler(19, isr19, 0);
	//reserved for intel
	setHandler(20, 0, 0);
	setHandler(21, 0, 0);
	setHandler(22, 0, 0);
	setHandler(23, 0, 0);
	setHandler(24, 0, 0);
	setHandler(25, 0, 0);
	setHandler(26, 0, 0);
	setHandler(27, 0, 0);
	setHandler(28, 0, 0);
	setHandler(29, 0, 0);
	setHandler(30, 0, 0);
	setHandler(31, 0, 0);
	
	setHandler(32, irq0, 0);
	a_Handlers[32] = TaskManager::handleTimerEvent;
	setHandler(33, irq1, 0);
	a_Handlers[33] = Keyboard::handleKeyboardEvent;
	setHandler(34, irq2, 0);
	setHandler(35, irq3, 0);
	setHandler(36, irq4, 0);
	setHandler(37, irq5, 0);
	setHandler(38, irq6, 0);
	setHandler(39, irq7, 0);
	setHandler(40, irq8, 0);
	setHandler(41, irq9, 0);
	setHandler(42, irq10, 0);
	setHandler(43, irq11, 0);
	setHandler(44, irq12, 0);
	setHandler(45, irq13, 0);
	setHandler(46, irq14, 0);
	setHandler(47, irq15, 0);
	init_timer(500);
}

//static int tick = 0;

extern "C" void irq_handler(registers_t regs)
{
	// Send an EOI (end of interrupt) signal to the PICs.
	// If this interrupt involved the slave.
	if (regs.int_no >= 40)
	{
		// Send reset signal to slave.
		IO::PortIO::outb(0xA0, 0x20);
	}
	// Send reset signal to master. (As well as slave, if necessary).
	IO::PortIO::outb(0x20, 0x20);

	
	/*if (tick++ % 20000 != 0)
		return;
	
	IO::Console::kprint("IRQ Interrupt: ");
	IO::Console::kprint((int)regs.int_no);
	IO::Console::kprintln();
	*/
	if (a_Handlers[regs.int_no] != 0)
		a_Handlers[regs.int_no]();
	/*
	IO::Console::kprint("IRQ Interrupt finished");
	IO::Console::kprintln();*/
}

void init_timer(uint32_t frequency)
{
	using namespace IO::PortIO;
   // Firstly, register our timer callback.
   //register_interrupt_handler(IRQ0, &timer_callback);

   // The value we send to the PIT is the value to divide it's input clock
   // (1193180 Hz) by, to get our required frequency. Important to note is
   // that the divisor must be small enough to fit into 16-bits.
   uint32_t divisor = 1193180 / frequency;

   // Send the command byte.
   outb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   uint8_t l = (uint8_t)(divisor & 0xFF);
   uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   // Send the frequency divisor.
   outb(0x40, l);
   outb(0x40, h);
}

extern "C" void isr_handler(registers_t regs)
{
	if (a_Handlers[regs.int_no] != 0)
		a_Handlers[regs.int_no]();
	else
	{
		//IO::Console::kprint("Interrupt: ");
		//IO::Console::kprint((int)regs.int_no);
		//IO::Console::kprintln();
	}
}

static void setHandler(int intID, void (*handler)(), int ring)
{
	uint16_t selector = 0x08;
	uint16_t settings;
	uint32_t offset = (uint32_t)handler;

	/* get CS selector */
	asm volatile("movw %%cs,%0" :"=g"(selector));

	/* set settings options depending on dpl */
	switch(ring)
	{
	case 0: 
		settings = INT_0; 
		break;
	case 1:
	case 2:
	case 3: 
		settings = INT_3; 
		break;
	}

	/* set actual values of int */
	a_IDT[intID].low_offset   = (offset & 0xFFFF);
	a_IDT[intID].selector     = selector;
	a_IDT[intID].settings     = settings;
	a_IDT[intID].high_offset  = (offset >> 16);
}