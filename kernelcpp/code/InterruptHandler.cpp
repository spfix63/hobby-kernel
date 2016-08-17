#include "Console.h"
#include "InterruptHandler.h"
#include "PIC.h"

static void handle()
{
	PIC::sendEOI(0);
}

void IDT::ISR::isr_0(void)
{
	handle();
	IO::Console::kprintln("Divide By Zero Error 0");
}

void IDT::ISR::isr_1(void)
{
	handle();
	IO::Console::kprintln("Debug Error 1");
}

void IDT::ISR::isr_2(void)
{
	handle();
	IO::Console::kprintln("NMI Interrupt 2");
}

void IDT::ISR::isr_3(void)
{
	handle();
	IO::Console::kprintln("Breakpoint 3");
}

void IDT::ISR::isr_4(void)
{
	handle();
	IO::Console::kprintln("Overflow 4");
}

void IDT::ISR::isr_5(void)
{
	handle();
	IO::Console::kprintln("BOUND Range Exceeded 5");
}

void IDT::ISR::isr_6(void)
{
	handle();
	IO::Console::kprintln("Invalid Opcode 6");
}

void IDT::ISR::isr_7(void)
{
	handle();
	IO::Console::kprintln("Device Not Available 7");
}

void IDT::ISR::isr_8(void)
{
	handle();
	IO::Console::kprintln("Double Fault 8");
}

void IDT::ISR::isr_9(void)
{
	handle();
	IO::Console::kprintln("Coprocessor Segment Overrun 9");
}

void IDT::ISR::isr_10(void)
{
	handle();
	IO::Console::kprintln("Invalid TSS 10");
}

void IDT::ISR::isr_11(void)
{
	handle();
	IO::Console::kprintln("Segment Not Present 11");
}

void IDT::ISR::isr_12(void)
{
	handle();
	IO::Console::kprintln("Stack Segment Fault 12");
}

void IDT::ISR::isr_13(void)
{
	handle();
	IO::Console::kprintln("General Protection Fault 13");
}

void IDT::ISR::isr_14(void)
{
	handle();
	IO::Console::kprintln("Page Fault 14");
}

void IDT::ISR::isr_16(void)
{
	handle();
	IO::Console::kprintln("FPU Floating-Point Error 16");
}

void IDT::ISR::isr_17(void)
{
	handle();
	IO::Console::kprintln("Alignment Check 17");
}

void IDT::ISR::isr_18(void)
{
	handle();
	IO::Console::kprintln("Machine Check 18");
}

void IDT::ISR::isr_19(void)
{
	handle();
	IO::Console::kprintln("SIMD Floating-Point 19");
}

void IDT::IRQ::timer(void)
{
	IO::Console::kprintln("timer handler");
	PIC::sendEOI(0);
}

void IDT::IRQ::sink(void)
{
	IO::Console::kprintln("IRQ sink");
	PIC::sendEOI(0);
}
