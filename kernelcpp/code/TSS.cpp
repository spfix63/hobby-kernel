
#include <cstdint>

struct gdt_entry_bits
{
	unsigned int limit_low:16;
	unsigned int base_low : 24;
     //attribute byte split into bitfields
	unsigned int accessed :1;
	unsigned int read_write :1; //readable for code, writable for data
	unsigned int conforming_expand_down :1; //conforming for code, expand down for data
	unsigned int code :1; //1 for code, 0 for data
	unsigned int always_1 :1; //should be 1 for everything but TSS and LDT
	unsigned int DPL :2; //priveledge level
	unsigned int present :1;
     //and now into granularity
	unsigned int limit_high :4;
	unsigned int available :1;
	unsigned int always_0 :1; //should always be 0
	unsigned int big :1; //32bit opcodes for code, dword stack for data
	unsigned int gran :1; //1 to use 4k page addressing, 0 for byte addressing
	unsigned int base_high :8;
} __attribute__((packed)); //or __attribute__((packed))

// A struct describing a Task State Segment.
struct tss_entry_struct
{
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // everything below here is unusued now.. 
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         
   uint32_t cs;        
   uint32_t ss;        
   uint32_t ds;        
   uint32_t fs;       
   uint32_t gs;         
   uint32_t ldt;      
   uint16_t trap;
   uint16_t iomap_base;
} __attribute__((packed));
 
typedef struct tss_entry_struct tss_entry_t;

/**Ok, this is going to be hackish, but we will salvage the gdt_entry_bits struct to form our TSS descriptor
So some of these names of the fields will actually be different.. maybe I'll fix this later..**/
tss_entry_t tss_entry;

extern "C" void writeTss(gdt_entry_bits *g)
{
	// Firstly, let's compute the base and limit of our entry into the GDT.
	uint32_t base = (uint32_t) &tss_entry;
	uint32_t limit = base + sizeof(tss_entry);

	// Now, add our TSS descriptor's address to the GDT.
	g->base_low=base&0xFFFFFF; //isolate bottom 24 bits
	g->accessed=1; //This indicates it's a TSS and not a LDT. This is a changed meaning
	g->read_write=0; //This indicates if the TSS is busy or not. 0 for not busy
	g->conforming_expand_down=0; //always 0 for TSS
	g->code=1; //For TSS this is 1 for 32bit usage, or 0 for 16bit.
	g->always_1=0; //indicate it is a TSS
	g->DPL=3; //same meaning
	g->present=1; //same meaning
	g->limit_high=(limit&0xF0000)>>16; //isolate top nibble
	g->available=0;
	g->always_0=0; //same thing
	g->big=0; //should leave zero according to manuals. No effect
	g->gran=0; //so that our computed GDT limit is in bytes, not pages
	g->base_high=(base&0xFF000000)>>24; //isolate top byte.

	// Ensure the TSS is initially zero'd.
	//memset(&tss_entry, 0, sizeof(tss_entry));
	for (unsigned int i = 0; i < sizeof(tss_entry); i++)
		((char *)&tss_entry)[i] = 0;

	tss_entry.ss0  = 0x10;//REPLACE_KERNEL_DATA_SEGMENT;  // Set the kernel stack segment.
	tss_entry.esp0 = 0x0;//REPLACE_KERNEL_STACK_ADDRESS; // Set the kernel stack pointer.
}