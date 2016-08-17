

#include <cstring>
#include <cstdint>

#include "vmm.h"
#include "pmm.h"
#include "../Console.h"
// #include "idt.h"

uint32_t *page_directory = (uint32_t *)PAGE_DIR_VIRTUAL_ADDR;
uint32_t *page_tables = (uint32_t *)PAGE_TABLE_VIRTUAL_ADDR;

page_directory_t *current_page_directory = NULL;
page_directory_t *kernel_page_directory = NULL;

extern char pmm_paging_active;


void init_vmm ()
{
	uint32_t cr0;

	// Create a page directory.
	page_directory_t *pd = create_page_directory();
	kernel_page_directory = pd;
	
	// Set the current directory.
	switch_page_directory (pd);

	// Enable paging.
	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));

	// We need to map the page table where the physical memory manager keeps its page stack
	// else it will panic on the first "pmm_free_page".
	map_pmm_to_virtual_space();

	// Paging is now active. Tell the physical memory manager.
	pmm_paging_active = 1;

}

page_directory_t *create_page_directory()
{
	/*
		kuriant pirma puslapi isjungtas paginimas, mes galim ramiai prieit prie
		fiziniu adresu ir i juos rasyt. Tada ijungiamas paginimas, ir nebegalim, todel
		kitiem procesam reikia laikinai sumappint lenteliu vietas, kad i jas rasyt.
		
		negalima sumappint pmm_to_vmm kol nera paginimo, todel kernelio pagedir tai daroma atskirai. 
		Reiketu duplikuot koda, kad butu aiskiau...
	*/
	if (!kernel_page_directory)
	{
		page_directory_t *pd = (page_directory_t*)pmm_alloc_page ();

		IO::Console::kprint("PD: ");
		IO::Console::kprinthex((int)pd);
		IO::Console::kprintln();  
		// Initialise it.
		memset (pd, 0, 0x1000);

		// Identity map the first 4 MB.
		pd[0] = pmm_alloc_page () | PAGE_PRESENT | PAGE_WRITE;
		uint32_t *pt = (uint32_t*) (pd[0] & PAGE_MASK);
		for (int i = 0; i < 1024; i++)
			pt[i] = i*0x1000 | PAGE_PRESENT | PAGE_WRITE;

		// Assign the second-last table and zero it.
		pd[1022] = pmm_alloc_page () | PAGE_PRESENT | PAGE_WRITE;
		pt = (uint32_t*) (pd[1022] & PAGE_MASK);
		memset (pt, 0, 0x1000);

		// The last entry of the second-last table is the directory itself.
		pt[1023] = (uint32_t)pd | PAGE_PRESENT | PAGE_WRITE;

		// The last table loops back on the directory itself.
		pd[1023] = (uint32_t)pd | PAGE_PRESENT | PAGE_WRITE;
		
		return pd;
	}
	else
	{
		//sutvarkyt pd sunaikinima kai baigiasi procesas!
		page_directory_t *pd_phys = (page_directory_t*)pmm_alloc_page ();
		page_directory_t *pd = (page_directory_t *)0x7AB13000;
		uint32_t *pt_phys = 0;
		uint32_t *pt = pd + 1024;
				
		IO::Console::kprint("PD_PHYS: ");
		IO::Console::kprinthex((int)pd_phys);
		IO::Console::kprintln();  
		IO::Console::kprint("PD: ");
		IO::Console::kprinthex((int)pd);
		IO::Console::kprintln();
		
		map((uint32_t)pd, (uint32_t)pd_phys, PAGE_PRESENT | PAGE_WRITE);
		// Initialise it.
		memset (pd, 0, 0x1000);
		
		// Identity map the first 4 MB.
		pd[0] = pmm_alloc_page () | PAGE_PRESENT | PAGE_WRITE;
		pt_phys = (uint32_t*) (pd[0] & PAGE_MASK);
		
		map((uint32_t)pt, (uint32_t)pt_phys, PAGE_PRESENT | PAGE_WRITE);
		for (int i = 0; i < 1024; i++)
			pt[i] = i*0x1000 | PAGE_PRESENT | PAGE_WRITE;
		unmap((uint32_t)pt);
		
		// Assign the second-last table and zero it.
		pd[1022] = pmm_alloc_page () | PAGE_PRESENT | PAGE_WRITE;
		pt_phys = (uint32_t*) (pd[1022] & PAGE_MASK);
		map((uint32_t)pt, (uint32_t)pt_phys, PAGE_PRESENT | PAGE_WRITE);
		memset (pt, 0, 0x1000);

		// The last entry of the second-last table is the directory itself.
		pt[1023] = (uint32_t)pd_phys | PAGE_PRESENT | PAGE_WRITE;
		
		// The last table loops back on the directory itself.
		pd[1023] = (uint32_t)pd_phys | PAGE_PRESENT | PAGE_WRITE;
		
		unmap((uint32_t)pt);
		unmap((uint32_t)pd);
		
		//I pirmus 4 mb ieina PMM bitmapas, del to atskirai mappint nereik.
		return pd_phys;
	}
}

void destroy_page_directory(page_directory_t *)
{
}


void switch_page_directory (page_directory_t *pd)
{
	current_page_directory = pd;
	asm volatile ("mov %0, %%cr3" : : "r" (pd));
}

void map (uint32_t va, uint32_t pa, uint32_t flags)
{

	uint32_t virtual_page = va / 0x1000;
	uint32_t pt_idx = PAGE_DIR_IDX(virtual_page);

	// Find the appropriate page table for 'va'.
	
	if (page_directory[pt_idx] == 0)
	{
		// The page table holding this page has not been created yet.
		page_directory[pt_idx] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
		memset ((void *)page_tables[pt_idx*1024], 0, 0x1000);
	}

	// Now that the page table definately exists, we can update the PTE.
	page_tables[virtual_page] = (pa & PAGE_MASK) | flags;
}

void unmap (uint32_t va)
{
	uint32_t virtual_page = va / 0x1000;
	
	page_tables[virtual_page] = 0;
	// Inform the CPU that we have invalidated a page mapping.
	asm volatile ("invlpg (%0)" : : "a" (va));
}

char get_mapping (uint32_t va, uint32_t *pa)
{
	uint32_t virtual_page = va / 0x1000;
	uint32_t pt_idx = PAGE_DIR_IDX(virtual_page);

	// Find the appropriate page table for 'va'.
	if (page_directory[pt_idx] == 0)
		return 0;

	if (page_tables[virtual_page] != 0)
	{
		if (pa) *pa = page_tables[virtual_page] & PAGE_MASK;
		return 1;
	}
	return -1;
}

void page_fault ()
{
	uint32_t cr2;
	asm volatile ("mov %%cr2, %0" : "=r" (cr2));
	IO::Console::kprint("PAGE FAULT : cr2=");
	IO::Console::kprinthex((int)cr2);	
	IO::Console::kprintln();
	panic ("");
}
