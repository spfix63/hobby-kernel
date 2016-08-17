#ifndef INTERRUPTHANDLER_H
#define INTERRUPTHANDLER_H

namespace IDT
{

namespace ISR
{

void isr_0(void);
void isr_1(void);
void isr_2(void);
void isr_3(void);
void isr_4(void);
void isr_5(void);
void isr_6(void);
void isr_7(void);
void isr_8(void);
void isr_9(void);
void isr_10(void);
void isr_11(void);
void isr_12(void);
void isr_13(void);
void isr_14(void);
void isr_16(void);
void isr_17(void);
void isr_18(void);
void isr_19(void);

}

namespace IRQ
{

void timer(void);
void sink(void);

}

}
#endif

