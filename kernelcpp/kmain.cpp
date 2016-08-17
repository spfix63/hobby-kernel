
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "code/colors.h"
#include "code/Console.h"
#include "code/IDT.h"
#include "code/IPC.h"
#include "code/Keyboard.h"
#include "code/MemoryManagement.h"
#include "code/PIC.h"
#include "code/PortIO.h"
#include "code/TaskManager.h"
#include "code/mem/pmm.h"
#include "code/mem/vmm.h"
#include "code/mem/heap.h"


int strcmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1==*s2))
        s1++,s2++;
    return *(const unsigned char*)s1-*(const unsigned char*)s2;
}


void *memcpy (void *dest, const void *src, size_t len)
{
    const uint8_t *sp = (const uint8_t *)src;
    uint8_t *dp = (uint8_t *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
	return dest;
}


static inline
int irqEnabled()
{
    int f;
    asm volatile ( "pushf\n\t"
                   "popl %0"
                   : "=g"(f) );
    return f & ( 1 << 9 );
}

void switch_to_user_mode()
{
	// Set up a stack structure for switching to user mode.
	/*cli; \ */
     
	asm volatile("  \
		mov $0x23, %ax; \
		mov %ax, %ds; \
		mov %ax, %es; \
		mov %ax, %fs; \
		mov %ax, %gs; \
				   \
		mov %esp, %eax; \
		pushl $0x23; \
		pushl %eax; \
		pushf; \
		pushl $0x1B; \
		push $1f; \
		iret; \
		1: \
	");
}



class Program1 : public Launchable
{
public:
	Program1(){ counter = 0; }

	virtual void launch()
	{
		IO::Console::kprintln("Program1: launched.");
		/*for (int i = 0; i < 5; i++) 
		{
		
			counter++;
			int *shsh = new int[counter%128 + 1];
			*shsh = counter;
			delete[] shsh;
		}*/
		
		while (true)
		{
			uint32_t size = 0;
			char *msg = (char *)IPC::getInstance().receiveMessage(size);
			if (msg)
			{
				int cursorX = *msg++;
				int cursorY = *msg++;
				
				unsigned char *videoram = (unsigned char *) COLOR_TEXT_MODE;
				int pos = 2*(cursorY*80 + cursorX);
				videoram[pos] = *(msg);
				videoram[pos + 1] = COLOR_WHITE;
			}
		}
		
		/*IPC::getInstance().sendMessage(1, "Jonas", 6);
		IPC::getInstance().sendMessage(1, "Jonas1", 7);
		IPC::getInstance().sendMessage(1, "Jonas2", 7);
		IPC::getInstance().sendMessage(1, "Jonas3", 7);
		IPC::getInstance().sendMessage(1, "Jonas4", 7);
			
		uint32_t size = 0;
		char *msg = (char *)IPC::getInstance().receiveMessage(size);
		IO::Console::kprint((int)size);
		IO::Console::kprint(" ");;
		IO::Console::kprintln(msg);
		msg = (char *)IPC::getInstance().receiveMessage(size);
		IO::Console::kprint((int)size);
		IO::Console::kprint(" ");;
		IO::Console::kprintln(msg);
		msg = (char *)IPC::getInstance().receiveMessage(size);
		IO::Console::kprint((int)size);
		IO::Console::kprint(" ");;
		IO::Console::kprintln(msg);
		msg = (char *)IPC::getInstance().receiveMessage(size);
		IO::Console::kprint((int)size);
		IO::Console::kprint(" ");;
		IO::Console::kprintln(msg);
		msg = (char *)IPC::getInstance().receiveMessage(size);
		IO::Console::kprint((int)size);
		IO::Console::kprint(" ");;
		IO::Console::kprintln(msg);
		*/
	}
	
	int getResult()
	{
		return counter;
	}
	
	virtual const char *getName()
	{
		return "Program1";
	}
	
private:
	unsigned long counter;
};

class Launcher : public KeyboardListener
{
public:
	Launcher() 
		: m_current(0)
	{
		IO::Console::kprintln("Launcher: enter program name.");
	}
	
	void add(Launchable *program)
	{
		m_programs[m_current++] = program;
		m_current %= 10;
	}
	
	void lineEntered(const char *line)
	{
		TaskManager &tm = TaskManager::getInstance();
		IO::Console::kprintln();
		Launchable *prog = NULL;
		for (int i = 0; i < m_current; i++)
		{
			if (!strcmp(m_programs[i]->getName(), line))
			{
				prog = m_programs[i];
				break;
			}
		}
		if (prog)
		{
			tm.createThread(prog);
		
		}
		else
			IO::Console::kprintln("Couldn't find it.");
	}
	
private:
	Launchable *m_programs[10];
	int m_current;
};



extern "C" void kmain(void)
{
	IO::Console::kclearScreen();
	
	extern unsigned int magic;
	//extern void *mbd; unused for now
	if (magic != 0x2BADB002)
	{
		IO::Console::kprintln("Error");
	  /* Something went not according to specs. Print an error */
	  /* message and halt, but do *not* rely on the multiboot */
	  /* data structure. */
		return;
	}
	
	
	PIC::remap(0x20, 0x28);
	IDT::setupHandlers();
	IDT::load();
	
	Keyboard::setup();
	
	int *a = (int *)0x00000000;
	int *b = (int *)0x00100000;
	//if true, A20 is disabled => real mode (or 16bit protected mode -_-).
	if (*a == *b)
	{
		IO::Console::kprintln("L20 off");
	}
	else
	{
		IO::Console::kprintln("L20 on");
	}
	
	init_pmm (0x00300000);
	init_vmm ();
	IO::Console::kprintln("heap");	

	// can't call new until TaskManager is init.
	// init_heap ();
	
		
	IO::Console::kprintln("main alloc end");	
	
	TaskManager &tm = TaskManager::getInstance();
	tm.init();
	
	// IO::Console::kprintln("Init--");
	asm("sti");	
	
	// Launcher lll;
	Program1 pg1;
	// lll.add(&pg1);
	// Keyboard::setListener(&lll);
	// tm.createThread(&pg1);
	tm.createProcess(&pg1);
	
	
	/*if (irqEnabled())
	{
		IO::Console::kprintln("IRQ Enabled");
	}
	else
	{
		IO::Console::kprintln("IRQ Disabled");
	}*/
	unsigned long seed = 123456;
	for (;;) 
	{
		for (int i = 0; i < 1000000; i++);
		
		seed = seed * 1103515245 + 12345;
		
		unsigned char x = seed % 80;
		unsigned char y = seed % 25;
		
		unsigned char ms[3] = {x, y, 'Z'};
		IPC::getInstance().sendMessage(1, ms, 3);
		// IO::Console::kprint(pg1.getResult());
		// IO::Console::kprintln();
	}
}


void *operator new(size_t size)
{
    return Heap::currentHeap->kmalloc(size);
}
 
void *operator new[](size_t size)
{
    return Heap::currentHeap->kmalloc(size);
}
 
void operator delete(void *p)
{
    Heap::currentHeap->kfree(p);
}
 
void operator delete[](void *p)
{
	Heap::currentHeap->kfree(p);
}

inline void *operator new(size_t, void *p)     throw() { return p; };
inline void *operator new[](size_t, void *p)   throw() { return p; };
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };

namespace __cxxabiv1 
{
	/* guard variables */
 
	/* The ABI requires a 64-bit type.  */
	__extension__ typedef int __guard __attribute__((mode(__DI__)));
 
	extern "C" int __cxa_guard_acquire (__guard *);
	extern "C" void __cxa_guard_release (__guard *);
	extern "C" void __cxa_guard_abort (__guard *);
 
	extern "C" int __cxa_guard_acquire (__guard *g) 
	{
		return !*(char *)(g);
	}
 
	extern "C" void __cxa_guard_release (__guard *g)
	{
		*(char *)g = 1;
	}
 
	extern "C" void __cxa_guard_abort (__guard *)
	{
 
	}
}









